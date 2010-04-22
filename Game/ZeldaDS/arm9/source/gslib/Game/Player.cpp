#include "Player.h"
#include "gslib/Core/Core.h"
#include "gslib/Hsm/HsmStateMachine.h"
#include "gslib/Math/Vector2.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Hw/InputManager.h"
#include "gslib/Hw/AudioEngine.h"
#include "data/soundbank.h"
#include "GameItems.h"
#include "ActorSharedStateData.h"
#include "Sword.h"
#include "Boomerang.h"
#include "SceneGraph.h"
#include "ScrollingMgr.h"
#include "Camera.h"
#include "Enemy.h"
#include "MovementModel.h"

// Player HSM

struct PlayerStates
{
	struct PlayerSharedStateData : ActorSharedStateData
	{
		typedef ActorSharedStateData Base;

		PlayerSharedStateData()
			: mScrollDir(ScrollDir::None)
			, mAttribDamageable(false)
		{
		}

		virtual ~PlayerSharedStateData()
		{
		}

		virtual void PostHsmUpdate(HsmTimeType deltaTime)
		{
			if ( mBoomerang.IsNodeInScene() && mBoomerang.HasReturned() )
			{
				SceneGraph::Instance().RemoveNodePostUpdate(mBoomerang);
			}

			Base::PostHsmUpdate(deltaTime);
		}

		// Data...
		Sword mSword;
		Boomerang mBoomerang;
		ScrollDir::Type mScrollDir;

		Attribute<bool> mAttribDamageable;
	};

	struct PlayerStateBase : ActorStateBase<PlayerSharedStateData, Player>
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
			if (Data().mHealth <= 0)
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
			SetAttribute(Data().mAttribDamageable, true);
		}

		virtual void OnExit()
		{
			SetTakingDamage(false);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			ASSERT(Data().mScrollDir == ScrollDir::None);

			const BoundingBox& worldBBox = Owner().GetBoundingBox();
			BoundingBox screenBBox(Camera::Instance().WorldToScreen(worldBBox.pos), worldBBox.w, worldBBox.h);

			if (screenBBox.Left() < 0)						Data().mScrollDir = ScrollDir::Left;
			else if (screenBBox.Top() < 0)					Data().mScrollDir = ScrollDir::Up;
			else if (screenBBox.Right() > HwScreenSizeX)	Data().mScrollDir = ScrollDir::Right;
			else if (screenBBox.Bottom() > HwScreenSizeY)	Data().mScrollDir = ScrollDir::Down;

			if (Data().mScrollDir != ScrollDir::None)
			{
				return SiblingTransition<Alive_Scrolling>();
			}

			return InnerEntryTransition<Alive_Locomotion>();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			if (!Owner().mInvincible)
			{
				if (Owner().mDamageInfo.mAmount > 0) //@TODO: This is a shitty way to know if mDamageInfo has been set
				{
					SetTakingDamage(true);
					mElapsedDamageTime = 0;
				}
			}
			else
			{
				mElapsedDamageTime += deltaTime;

				if (mElapsedDamageTime >= SEC_TO_FRAMES(0.2f))
				{
					SetTakingDamage(false);
				}
			}
		}

	private:
		void SetTakingDamage(bool takingDamage)
		{
			if (takingDamage)
			{
				DamageInfo& dmgInfo = Owner().mDamageInfo;
				Data().mHealth -= dmgInfo.mAmount;
				Owner().mInvincible = true;
				//@HACK: Velocity set in Update
				//Owner().SetVelocity(Normalized(dmgInfo.mPushVector) * 3);
			}
			else
			{
				Owner().mInvincible = false;
				Owner().mDamageInfo.Reset();
				Owner().SetVelocity(InitZero);
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

			// Remove all enemies
			//@TODO: Should defer to an EnemyManager
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
				Data().mHealth = 0;
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

		virtual void OnExit()
		{
			Owner().SetVelocity(InitZero);
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

			//@HACK: If our direction changed while moving, replay the move anim to see the new directional one
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
				Data().mHealth = 6; // Resurrect me (for now)
			}
		}
	};
};


// Player class implementation

void Player::Init(const Vector2I& initPos)
{
	PlayerStates::PlayerSharedStateData* pSharedStateData = new PlayerStates::PlayerSharedStateData();
	mpSharedStateData = pSharedStateData;

	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(mpSharedStateData);
	mStateMachine.SetInitialState<PlayerStates::Root>();

	mInitPos = initPos;
	mInvincible = false;
	mInvincibleElapsedTime = 0;
}

void Player::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	gameObjectInfo.mGameActor = GameActor::Hero;
}

void Player::OnAddToScene()
{
	Base::OnAddToScene();

	SetPosition(mInitPos);
	mDamageInfo.Reset();
}

void Player::Update(GameTimeType deltaTime)
{
	mStateMachine.Update(deltaTime);

	//@TODO: Maybe state machine should provide some kind of hook for pre/post update?
	ActorSharedStateData& sharedData = static_cast<ActorSharedStateData&>(*mpSharedStateData);
	sharedData.PostHsmUpdate(deltaTime);

	//@HACK: I do this because the velocity is potentially crushed by the Moving state
	// while being hurt. Need some way for an outer state to set the velocity, and for
	// inner state velocity changes to be ignored.
	if (mInvincible)
	{
		SetVelocity(Normalized(mDamageInfo.mPushVector) * 3);		
	}

	Base::Update(deltaTime);
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

void Player::OnDamage(const DamageInfo& damageInfo)
{
	if (mInvincible)
		return;

	if (!static_cast<PlayerStates::PlayerSharedStateData&>(*mpSharedStateData).mAttribDamageable)
		return;

	mDamageInfo = damageInfo;
}
