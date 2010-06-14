#include "Enemy.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/MathEx.h"
#include "CharacterState.h"
#include "GameHelpers.h"
#include "Player.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "MovementModel.h"
#include "EnemyState.h"
#include <cstdlib>

struct EnemyStates
{
	struct Root : EnemyStateBase
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

	struct Alive : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Alive_Spawn>();
		}
	};

	struct Alive_Spawn : EnemyStateBase
	{
		virtual void OnEnter()
		{
			PlayGlobalAnim(BaseAnim::Spawn);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (IsAnimFinished())
			{
				return SiblingTransition<Alive_Main>();
			}

			return NoTransition();
		}
	};

	// AI's main state
	struct Alive_Main : EnemyStateBase
	{
		virtual void OnEnter()
		{
			SetAttribute(Data().mAttribCanDamage, true);
			SetAttribute(Data().mAttribCanTakeDamage, true);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Alive_Main_Locomotion>();
		}
	};

	struct Alive_Main_Locomotion : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (Owner().mDamageInfo.IsSet())
			{
				if (Owner().mDamageInfo.mEffect == DamageEffect::Stun)
				{
					return SiblingTransition<Alive_Main_Stunned>();
				}

				return SiblingTransition<Alive_Main_Hurt>();
			}

			return Owner().GetRootTransition();
		}
	};

	struct Alive_Main_Stunned : EnemyStateBase
	{
		HsmTimeType mElapsedTime;

		virtual void OnEnter()
		{
			mElapsedTime = 0;
			ASSERT(Owner().mDamageInfo.mEffect == DamageEffect::Stun);
			Owner().mDamageInfo.Reset();

			PlayAnim(BaseAnim::Idle);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			// Not crazy about this, probably should have single outer state
			// that transitions us to Hurt
			DamageInfo& dmgInfo = Owner().mDamageInfo;
			if (dmgInfo.IsSet() && dmgInfo.mEffect != DamageEffect::Stun)
			{
				return SiblingTransition<Alive_Main_Hurt>();
			}

			mElapsedTime += deltaTime;
			if (mElapsedTime > SEC_TO_FRAMES(2.0))
			{
				return SiblingTransition<Alive_Main_Locomotion>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			// Keep resetting Alive_Main_Stunned timer if reAlive_Main_Stunned. Note that re-stunning happens
			// a few times even on the initial stun since the boomerang continues to collide
			// with the enemy for a few frames. This is ok, not much we can do about it.
			DamageInfo& dmgInfo = Owner().mDamageInfo;

			if (dmgInfo.IsSet())
			{
				// Other types of damage should bump us out of this state
				ASSERT(dmgInfo.mEffect == DamageEffect::Stun);

				mElapsedTime = 0;
				dmgInfo.Reset();
			}
		}
	};

	struct Alive_Main_Hurt : EnemyStateBase
	{
		virtual void OnEnter()
		{
			//@TODO: For now, just absorb the damage. Eventually, knockback and
			// remain invincible for some time.
			DamageInfo& dmgInfo = Owner().mDamageInfo;
			ASSERT(dmgInfo.mEffect == DamageEffect::Hurt);
			
			Owner().mHealth.OffsetValue( -dmgInfo.mAmount );
			dmgInfo.Reset();
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return SiblingTransition<Alive_Main_Locomotion>();
		}
	};	

	struct Dead : EnemyStateBase
	{
		virtual void OnEnter()
		{
			PlayGlobalAnim(BaseAnim::Spawn); // Play same anim as spawn
			//@TODO: Tell someone we're dead
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			if (IsAnimFinished())
			{
				SceneGraph::Instance().RemoveNodePostUpdate(Owner());
			}
		}
	};

}; // struct EnemyStates

Enemy::Enemy()
	: mpSharedStateData(0)
{
}

void Enemy::InitStateMachine()
{
	mpSharedStateData = new EnemySharedStateData();

	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(mpSharedStateData);
	mStateMachine.SetInitialState<EnemyStates::Root>();
}

void Enemy::OnAddToScene()
{
	Base::OnAddToScene();

	//@TODO: This type of info will belong to the spawner eventually (perhaps passed into Init?)
	SetSpriteDir(GameHelpers::GetRandomSpriteDir());
}

void Enemy::Update(GameTimeType deltaTime)
{
	Base::Update(deltaTime);
	mLastFrameCollision.mIsSet = false;
}

void Enemy::OnCollision(const CollisionInfo& collisionInfo)
{
	mLastFrameCollision.mInfo = collisionInfo;
	mLastFrameCollision.mIsSet = true;

	if ( Player* pPlayer = DynamicCast<Player*>(collisionInfo.mpCollidingWith) )
	{
		if (mpSharedStateData->mAttribCanDamage)
		{
			static DamageInfo dmgInfo; //@MT_UNSAFE
			dmgInfo.mAmount = 1;
			dmgInfo.mPushVector = -collisionInfo.mPushVector;
			pPlayer->OnDamage(dmgInfo);
		}
	}
}
