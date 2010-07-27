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

// Player HSM

struct PlayerSharedStateData : CharacterSharedStateData
{
	typedef CharacterSharedStateData Base;

	PlayerSharedStateData()
		: mpSword(NULL)
		, mpBoomerang(NULL)
		, mScrollDir(ScrollDir::None)
	{
	}

	// Data...
	Sword* mpSword;
	Boomerang* mpBoomerang;
	ScrollDir::Type mScrollDir;
};

struct PlayerStates
{
	typedef CharacterState<PlayerSharedStateData, Player> PlayerState;

	struct Root : PlayerState
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			ASSERT(Data().mScrollDir == ScrollDir::None);

			if ( !GameHelpers::IsPhysicalInScreenBounds(Owner(), &Data().mScrollDir) )
			{
				ASSERT(Data().mScrollDir != ScrollDir::None);
				return SiblingTransition<Alive_Scrolling>();
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

		virtual void OnEnter()
		{
			ASSERT(Data().mScrollDir != ScrollDir::None);
			ASSERT(!mScrollingMgr.IsScrolling());

			mScrollingMgr.StartScrolling(Data().mScrollDir);

			// Just play movement anim in current dir, but don't actually allow player movement
			PlayAnim(BaseAnim::Move);
		}

		virtual void OnExit()
		{
			Data().mScrollDir = ScrollDir::None;
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if ( !mScrollingMgr.IsScrolling() )
			{
				return SiblingTransition<Alive_Normal>();
			}

			return NoTransition();
		}
	};

	struct Alive_Locomotion : PlayerState
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			const uint32& currKeysPressed = InputManager::GetKeysPressed();

			const bool shouldAttack = (currKeysPressed & KEY_A) != 0;
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

			const bool shouldDie = ( InputManager::GetKeysPressed() & KEY_Y ) != 0;
			if (shouldDie)
			{
				Owner().mHealth.SetValue(0);
			}
		}

	};

	struct Alive_Locomotion_Idle : PlayerState
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Idle);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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
			ASSERT(!Data().mpSword);
			Data().mpSword = new Sword();
			Data().mpSword->Init(Owner().GetSpriteDir());
			SceneGraph::Instance().AddNode(Data().mpSword);
			Owner().AttachChild(Data().mpSword); // Make sword our child
		}

		void DestroySword()
		{
			if (Data().mpSword)
			{
				Owner().DetachChild(Data().mpSword); // Technically detached for one frame :(
				SceneGraph::Instance().RemoveNodePostUpdate(Data().mpSword);
				Data().mpSword = NULL;
			}
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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
			Data().mpSword->SetLocalPosition(swordLocalPos);
		}
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

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
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
	: mLastDamagePushVector(InitZero)
{
}

void Player::InitStateMachine()
{
	mpSharedStateData = new PlayerSharedStateData();

	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(mpSharedStateData);
	mStateMachine.SetInitialState<PlayerStates::Root>();
}

void Player::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
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
