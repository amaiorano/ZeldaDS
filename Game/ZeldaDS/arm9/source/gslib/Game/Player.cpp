#include "Player.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/AudioEngine.h"
#include "data/soundbank.h"
#include "GameItems.h"
#include "CharacterState.h"
#include "Sword.h"
#include "Boomerang.h"
#include "SceneGraph.h"
#include "ScrollingMgr.h"
#include "Camera.h"
#include "Enemy.h"
#include "MovementModel.h"
#include "WorldMapTile.h"
#include "GameFlowMgr.h"
#include "GameEvent.h"

// Player HSM

struct PlayerSharedStateData : CharacterSharedStateData
{
	typedef CharacterSharedStateData Base;

	PlayerSharedStateData()
		: mpBoomerang(NULL)
		, mScrollDir(ScrollDir::None)
	{
	}

	Boomerang* mpBoomerang;
	ScrollDir::Type mScrollDir;
};

typedef StateT<PlayerSharedStateData, Player, CharacterState> PlayerState;

struct PlayerStates
{
	//@TODO:
	// - Move Dead state under Alive
	// - Remove the Alive_ prefix from most states - it's just noise
	// - Add simple comment headers to separate major state groups

	struct Root : PlayerState
	{
		virtual Transition EvaluateTransitions()
		{
			if (Owner().mHealth.IsDead())
			{
				return InnerTransition<Dead>();
			}

			return InnerTransition<Alive>();
		}
	};

	struct Alive : PlayerState
	{
		virtual Transition EvaluateTransitions()
		{
			return InnerEntryTransition<Alive_Spawn>();
		}
	};

	struct Alive_Spawn : PlayerState
	{
		virtual void OnEnter()
		{
			PlayGlobalAnim(BaseAnim::Spawn);
		}

		virtual Transition EvaluateTransitions()
		{
			if (IsAnimFinished())
			{
				return SiblingTransition<Alive_Normal>();
			}

			return NoTransition();
		}
	};

	struct Alive_Normal : PlayerState
	{
		HsmTimeType mElapsedDamageTime;

		virtual void OnEnter()
		{
			SetAttribute(Data().mAttribCanTakeDamage, true);
		}

		virtual void OnExit()
		{
			// In case we get kicked out of this state early, make sure we don't remain invincible
			Owner().mHealth.SetInvincible(false);
		}

		virtual Transition EvaluateTransitions()
		{
			ScrollDir::Type scrollDir = ScrollDir::None;
			if ( !GameHelpers::IsPhysicalInScreenBounds(Owner(), &scrollDir) )
			{
				Alive_Scrolling::Args args;
				args.mScrollDir = scrollDir;
				return SiblingTransition<Alive_Scrolling>(args);
			}

			if (GameEvent* pGameEvent = WorldMap::Instance().GetGameEventIfExists(Owner().GetPosition()))
			{
				//@TODO: This sucks... use some kind of RTTI to improve this
				if (pGameEvent->mType == GameEventType::Warp)
				{
					WarpGameEvent* pWarpGameEvent = static_cast<WarpGameEvent*>(pGameEvent);
					if (Owner().GetPosition() == WorldMap::TileToWorldPos(pWarpGameEvent->mTilePos))
					{
						//printf("Warp to %s at (%d, %d)\n", pWarpGameEvent->mTargetWorldMap.c_str(), pWarpGameEvent->mTargetTilePos.x, pWarpGameEvent->mTargetTilePos.y);
						Alive_Warping::Args args;
						args.mpWarpGameEvent = pWarpGameEvent;
						return SiblingTransition<Alive_Warping>(args);
					}
				}
			}

			return InnerEntryTransition<Alive_Locomotion>();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			if (Owner().mDamageInfo.IsSet())
			{
				Owner().mHealth.OffsetValue(-Owner().mDamageInfo.mAmount);
				Owner().mHealth.SetInvincible(true, SEC_TO_FRAMES(0.2f));
				Owner().mLastDamagePushVector = Owner().mDamageInfo.mPushVector;
				Owner().mDamageInfo.Reset();
			}
		}
	};

	struct Alive_Scrolling : PlayerState
	{
		ScrollingMgr& mScrollingMgr;

		Alive_Scrolling()
			: mScrollingMgr(ScrollingMgr::Instance())
		{
		}

		struct Args : StateArgs
		{
			ScrollDir::Type mScrollDir;
		};

		virtual void OnEnter(const Args& args)
		{
			ASSERT(args.mScrollDir != ScrollDir::None);
			ASSERT(!mScrollingMgr.IsScrolling());

			mScrollingMgr.StartScrolling(args.mScrollDir);

			// Just play movement anim in current dir, but don't actually allow player movement
			PlayAnim(BaseAnim::Move);
		}

		virtual Transition EvaluateTransitions()
		{
			if ( !mScrollingMgr.IsScrolling() )
			{
				return SiblingTransition<Alive_Normal>();
			}

			return NoTransition();
		}
	};

	struct Alive_Warping : PlayerState
	{
		struct Args : StateArgs
		{
			WarpGameEvent* mpWarpGameEvent;
		};

		struct Data : Args
		{
		};

		Data mData;

		virtual void OnEnter(const Args& args)
		{
			ASSERT(args.mpWarpGameEvent);
			static_cast<Args&>(mData) = args;
		}

		virtual Transition EvaluateTransitions()
		{
			return InnerEntryTransition<Alive_Warping_Stairs>();
		}
	};

	//@TODO: Generalize
	struct WarpingStateBase : PlayerState
	{
		typedef Alive_Warping OuterType;
		
		typename OuterType::Data& GetOuterData()
		{
			return static_cast<OuterType*>(GetState<Alive_Warping>())->mData;
		}
	};

	struct Alive_Warping_Stairs : WarpingStateBase
	{
		float mSteps;
		WorldMapTile* mpWorldMapTile;
		Vector2I mPlayerStartPos;

		Alive_Warping_Stairs()
			: mSteps(0.0f)
			, mpWorldMapTile(0)
		{
		}

		virtual void OnEnter()
		{
			mPlayerStartPos = Owner().GetPosition();

			// Create a WorldMapTile that matches exactly the tile below us so that
			// when we move "down the stairs", it looks like we're going underneath
			// the tile
			const Vector2I& playerTilePos = WorldMap::WorldPosToTile(mPlayerStartPos);
			const Vector2I& worldMapTilePos = Vector2I(playerTilePos.x, playerTilePos.y + 1);
			
			mpWorldMapTile = new WorldMapTile(GameTileLayer::Background, worldMapTilePos);
			mpWorldMapTile->SetPosition( WorldMap::TileToWorldPos(worldMapTilePos) );
			SceneGraph::Instance().AddNode(mpWorldMapTile);

			Owner().SetSpriteDir(SpriteDir::Up);
			PlayAnim(BaseAnim::Move);
		}

		virtual void OnExit()
		{
			// No need to remove this, leaving the map removes all nodes (including player)
			//SceneGraph::Instance().RemoveNodePostUpdate(mpWorldMapTile);
		}

		virtual Transition EvaluateTransitions()
		{
			if (mSteps >= 16.0f)
			{
				return SiblingTransition<Alive_Warping_Done>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			mSteps += 0.25f;
			Owner().SetPosition(mPlayerStartPos + Vector2I(0.0f, mSteps)); 
		}
	};

	struct Alive_Warping_Done : WarpingStateBase
	{
		virtual void OnEnter()
		{
			WarpGameEvent* pWarpGameEvent = GetOuterData().mpWarpGameEvent;

			// This will cause the GameFlowMgr to load the new map, destroying the Player in the process
			GameFlowMgr::Instance().SetTargetWorldMap(pWarpGameEvent->mTargetWorldMap.c_str(), WorldMap::TileToWorldPos(pWarpGameEvent->mTargetTilePos));
		}
	};

	struct Alive_Locomotion : PlayerState
	{
		virtual Transition EvaluateTransitions()
		{
			const uint32& currKeysPressed = InputManager::GetKeysPressed();

			const bool shouldAttack = (currKeysPressed & KEY_A/*KEY_Y*/) != 0;
			if (shouldAttack)
			{
				return SiblingTransition<Alive_Attack>();
			}

			const bool shouldUseItem = (currKeysPressed & KEY_B) != 0;
			if (shouldUseItem)
			{
				InputManager::ClearKeysPressed(KEY_B);
				return SiblingTransition<Alive_UseItem>();
			}

			return InnerEntryTransition<Alive_Locomotion_Idle>();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			// Whether idling or moving, always update our direction
			const uint32 currKeysHeld = InputManager::GetKeysHeld();
			if ( currKeysHeld & KEY_LEFT )	{ Owner().SetSpriteDir(SpriteDir::Left); }
			if ( currKeysHeld & KEY_RIGHT )	{ Owner().SetSpriteDir(SpriteDir::Right); }
			if ( currKeysHeld & KEY_UP )	{ Owner().SetSpriteDir(SpriteDir::Up); }
			if ( currKeysHeld & KEY_DOWN )	{ Owner().SetSpriteDir(SpriteDir::Down); }
		}

	};

	struct Alive_Locomotion_Idle : PlayerState
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Idle);
		}

		virtual Transition EvaluateTransitions()
		{
			const uint32 currKeysHeld = InputManager::GetKeysHeld();
			const bool shouldMove = currKeysHeld & KEYS_DIRECTION;
			if (shouldMove)
			{
				return SiblingTransition<Alive_Locomotion_Move>();
			}

			return NoTransition();
		}

	};

	struct Alive_Locomotion_Move : PlayerState
	{
		SpriteDir::Type mLastDir;

		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Move);
			mLastDir = Owner().GetSpriteDir();
		}

		virtual Transition EvaluateTransitions()
		{
			const uint32 currKeysHeld = InputManager::GetKeysHeld();
			const bool shouldMove = currKeysHeld & KEYS_DIRECTION;
			if (!shouldMove)
			{
				return SiblingTransition<Alive_Locomotion_Idle>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			const uint16 moveSpeed = 1; // pixels/frame

			Vector2I newVelocity(InitZero);
			SpriteDir::Type newDir = SpriteDir::None;

			//MovementModel::MoveDefault(Owner().GetPosition(), Owner().GetSpriteDir(), moveSpeed, newVelocity, newDir);
			MovementModel::MoveGrid8x8(Owner().GetPosition(), Owner().GetSpriteDir(), moveSpeed, newVelocity, newDir);

			Owner().SetVelocity(newVelocity);
			Owner().SetSpriteDir(newDir);

			// Update anim on direction change
			if (mLastDir != Owner().GetSpriteDir())
			{
				mLastDir = Owner().GetSpriteDir();
				PlayAnim(BaseAnim::Move);
			}
		}
	};

	struct Alive_Attack : PlayerState
	{
		Alive_Attack()
			: mpSword(0)
		{
		}

		virtual void OnEnter()
		{
			CreateSword();
			PlayAnim(BaseAnim::Attack);
			AudioEngine::PlaySound(SFX_SWORD);

			UpdateSwordPos(16);
		}

		virtual void OnExit()
		{
			DestroySword();
		}

		void CreateSword()
		{
			mpSword = new Sword();
			mpSword->Init(Owner().GetSpriteDir());
			SceneGraph::Instance().AddNode(mpSword);
			Owner().AttachChild(mpSword); // Make sword our child
		}

		void DestroySword()
		{
			ASSERT(mpSword);
			Owner().DetachChild(mpSword); // Technically detached for one frame :(
			SceneGraph::Instance().RemoveNodePostUpdate(mpSword);
		}

		virtual Transition EvaluateTransitions()
		{
			if (IsAnimFinished())
			{
				return SiblingTransition<Alive_Locomotion>();
			}

			return NoTransition();
		}

		virtual void PostAnimUpdate(HsmTimeType deltaTime)
		{
			if (Owner().GetAnimControl().GetFrameIndex() == 1)
			{
				UpdateSwordPos(8);
			}
		}

	private:
		void UpdateSwordPos(int16 offset)
		{
			const Vector2I& swordLocalPos = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir()) * (offset - 2);
			mpSword->SetLocalPosition(swordLocalPos);
		}

		Sword* mpSword;
	};

	// For now, just the boomerang
	//@TODO: Don't enter this state if player can't use current item
	struct Alive_UseItem : PlayerState
	{
		bool mItemUsed;

		virtual void OnEnter()
		{
			mItemUsed = false;

			if ( !Data().mpBoomerang )
			{
				Vector2I launchDir = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());

				const uint32& keysHeld = InputManager::GetKeysHeld();
				launchDir.x = keysHeld & KEY_LEFT? -1.0f : keysHeld & KEY_RIGHT? 1.0f : launchDir.x;
				launchDir.y = keysHeld & KEY_UP? -1.0f : keysHeld & KEY_DOWN? 1.0f : launchDir.y;

				Data().mpBoomerang = new Boomerang(true);
				Data().mpBoomerang->Init(&Owner(), launchDir);
				SceneGraph::Instance().AddNode(Data().mpBoomerang);

				mItemUsed = true;
			}

			if (mItemUsed)
			{
				PlayAnim(BaseAnim::UseItem);
			}
		}

		virtual Transition EvaluateTransitions()
		{
			if (!mItemUsed || IsAnimFinished())
			{
				return SiblingTransition<Alive_Locomotion>();
			}
			return NoTransition();
		}
	};

	struct Dead : PlayerState
	{
		virtual void OnEnter()
		{
			Owner().SetSpriteDir(SpriteDir::None);
			PlayAnim(BaseAnim::Die);
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			if (IsAnimFinished())
			{
				Owner().mHealth.Resurrect(); // Resurrect me (for now)
				//@TODO: Should call Owner().OnDead() here, and let game state machine do its thing
			}
		}
	};
};


// Player class implementation

Player::Player()
	: mpSharedStateData(0)
	, mLastDamagePushVector(InitZero)
{
}

void Player::InitStateMachine()
{
	Base::InitStateMachine();
	mStateMachine.SetInitialState<PlayerStates::Root>();
	mpSharedStateData = static_cast<PlayerSharedStateData*>(&mStateMachine.GetSharedStateData());
}

SharedStateData* Player::CreateSharedStateData()
{
	return new PlayerSharedStateData();
}

void Player::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	Base::GetGameObjectInfo(gameObjectInfo);
	gameObjectInfo.mSpriteRenderGroupId = GameSpriteRenderGroup::Heroes;
	gameObjectInfo.mGameActor = GameActor::Hero;
}

void Player::Update(GameTimeType deltaTime)
{
	Base::Update(deltaTime);

	if (deltaTime > 0)
	{
		// If boomerang has returned, remove it from the scene
		if ( mpSharedStateData->mpBoomerang && mpSharedStateData->mpBoomerang->HasReturned() )
		{
			SceneGraph::Instance().RemoveNodePostUpdate(mpSharedStateData->mpBoomerang);
			mpSharedStateData->mpBoomerang = NULL;
		}

		// Knockback (override any player input velocity)
		if (mHealth.IsInvincible())
		{
			SetVelocity(Normalized(mLastDamagePushVector) * 3);
		}
	}
}

void Player::Render(GameTimeType deltaTime)
{
	// ScrollingMgr is updated just before Render(), potentially moving the camera, so
	// we make sure to keep the player on screen if we're scrolling right here
	if (ScrollingMgr::Instance().IsScrolling())
	{
		// This assert is valid except when we use debug quick scroll
		//ASSERT( mStateMachine.IsInState<PlayerStates::Alive_Scrolling>() );

		const Vector2I& screenPos = GetScreenPosition();
		Vector2I offset;
		offset.x = screenPos.x < 0? -screenPos.x : (screenPos.x + GetWidth()) > HwScreenSizeX? HwScreenSizeX-(screenPos.x + GetWidth()) : 0;
		offset.y = screenPos.y < 0? -screenPos.y : (screenPos.y + GetHeight()) > HwScreenSizeY? HwScreenSizeY-(screenPos.y + GetHeight()) : 0;
		SetPosition( GetPosition() + offset );
	}

	Base::Render(deltaTime);
}
