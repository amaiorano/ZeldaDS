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
		: mScrollDir(ScrollDir::None)
	{
	}

	// Data...
	Sword mSword;
	Boomerang mBoomerang;
	ScrollDir::Type mScrollDir;
};

struct PlayerStates
{
	struct PlayerStateBase : CharacterStateBase<PlayerSharedStateData, Player>
	{
	};

	struct Root : PlayerStateBase
	{
		virtual void OnEnter()
		{
			Owner().SetGameActor(GameActor::Hero);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (Owner().mHealth.IsDead())
			{
				return InnerTransition<Dead>();
			}

			return InnerTransition<Alive>();
		}
	};

	struct Alive : PlayerStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Alive_Spawn>();
		}
	};

	struct Alive_Spawn : PlayerStateBase
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

	struct Alive_Normal : PlayerStateBase
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

	struct Alive_Scrolling : PlayerStateBase
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

			// Remove all enemies, weapons, etc.
			//@TODO: Should handle this somewhere else, perhaps via a SpawnManager
			WeaponList& playerWeapons = SceneGraph::Instance().GetPlayerWeaponList();
			for (WeaponList::iterator iter = playerWeapons.begin(); iter != playerWeapons.end(); ++iter)
			{
				SceneGraph::Instance().RemoveNodePostUpdate(**iter);
			}
			WeaponList& enemyWeapons = SceneGraph::Instance().GetEnemyWeaponList();
			for (WeaponList::iterator iter = enemyWeapons.begin(); iter != enemyWeapons.end(); ++iter)
			{
				SceneGraph::Instance().RemoveNodePostUpdate(**iter);
			}
			EnemyList& enemies = SceneGraph::Instance().GetEnemyList();
			for (EnemyList::iterator iter = enemies.begin(); iter != enemies.end(); ++iter)
			{
				SceneGraph::Instance().RemoveNodePostUpdate(**iter);
			}

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

	struct Alive_Locomotion : PlayerStateBase
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

	struct Alive_Locomotion_Idle : PlayerStateBase
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

	struct Alive_Locomotion_Move : PlayerStateBase
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

	struct Alive_Attack : PlayerStateBase
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Attack);
			AudioEngine::PlaySound(SFX_SWORD);

			Data().mSword.Init(Owner().GetSpriteDir());
			SceneGraph::Instance().AddNode(Data().mSword);
			Owner().AttachChild(&Data().mSword); // Make sword our child

			// Set sword position at halfway mark
			UpdateSwordPos(8);
		}

		virtual void OnExit()
		{
			Owner().DetachChild(&Data().mSword);
			//SceneGraph::Instance().RemoveNodePostUpdate(Data().mSword);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			//@HACK: This check makes sure we stay in attack for one more
			// state when the anim is finished to make sure the sword
			// gets removed from the scene. This is to avoid scheduling
			// removal + adding the sword in the same frame.
			if (IsAnimFinished() && Data().mSword.IsNodeInScene())
			{
				SceneGraph::Instance().RemoveNodePostUpdate(Data().mSword);
				return NoTransition();
			}
			else if (IsAnimFinished())
			{
				return SiblingTransition<Alive_Locomotion>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			if (Owner().GetAnimControl().GetFrameIndex() == 1)
			{
				UpdateSwordPos(16);
			}
		}

	private:
		void UpdateSwordPos(int16 offset)
		{
			const Vector2I& swordLocalPos = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir()) * (offset - 2);
			Data().mSword.SetLocalPosition(swordLocalPos);

		}
	};

	// For now, just the boomerang
	//@TODO: Don't enter this state if player can't use current item
	struct Alive_UseItem : PlayerStateBase
	{
		bool mItemUsed;

		virtual void OnEnter()
		{
			mItemUsed = false;

			if ( !Data().mBoomerang.IsNodeInScene() )
			{
				Vector2I launchDir = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());

				const uint32& keysHeld = InputManager::GetKeysHeld();
				launchDir.x = keysHeld & KEY_LEFT? -1.0f : keysHeld & KEY_RIGHT? 1.0f : launchDir.x;
				launchDir.y = keysHeld & KEY_UP? -1.0f : keysHeld & KEY_DOWN? 1.0f : launchDir.y;

				Data().mBoomerang.Init(&Owner(), launchDir);
				SceneGraph::Instance().AddNode(Data().mBoomerang);
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

	struct Dead : PlayerStateBase
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
	Base::Update(deltaTime); // Let base update state machine

	if (deltaTime > 0)
	{
		// If boomerang has returned, remove it from the scene
		if ( mpSharedStateData->mBoomerang.IsNodeInScene() && mpSharedStateData->mBoomerang.HasReturned() )
		{
			SceneGraph::Instance().RemoveNodePostUpdate(mpSharedStateData->mBoomerang);
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
