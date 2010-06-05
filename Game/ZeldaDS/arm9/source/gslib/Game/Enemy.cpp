#include "Enemy.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/MathEx.h"
#include "ActorSharedStateData.h"
#include "GameHelpers.h"
#include "Player.h"
#include "SceneGraph.h"
#include <cstdlib>

// Interface for core enemy AI
struct IEnemyAI
{
	virtual void Init(Enemy& enemy, CharacterSharedStateData& sharedStateData) {}
	virtual Transition& GetRootTransition() = 0;
};

struct EnemySharedStateData : CharacterSharedStateData
{
	EnemySharedStateData()
		: mpEnemyAI(0)
		, mAttribCanDamage(false)
	{
	}

	virtual ~EnemySharedStateData()
	{
		delete mpEnemyAI;
	}

	IEnemyAI* mpEnemyAI;
	Attribute<bool> mAttribCanDamage; // Can we damage player?
};

struct EnemyStates
{
	struct EnemyStateBase : CharacterStateBase<EnemySharedStateData, Enemy>
	{
	};

	struct Root : EnemyStateBase
	{
		virtual void OnEnter()
		{
			Data().mpEnemyAI->Init(Owner(), Data());
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
			return Data().mpEnemyAI->GetRootTransition();
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
};




//@TODO: Move to "gslib/Game/Enemies/Goriyas.cpp"
struct GoriyasStates : EnemyStates
{
	struct Main : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Moving>();
		}
	};

	struct Moving : EnemyStateBase
	{
		uint16 mUnitsMoved;
		uint16 mUnitsToMoveInOneDir;

		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Move);

			mUnitsMoved = 0;
			mUnitsToMoveInOneDir = MathEx::Rand(3, 5) * 16; // Move in random increments of 3 to 5 tiles
		}

		virtual void OnExit()
		{
			Owner().SetVelocity(InitZero);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (Owner().mDamageInfo.IsSet())
			{
				if (Owner().mDamageInfo.mEffect == DamageEffect::Stun)
				{
					return SiblingTransition<Stunned>();
				}

				return SiblingTransition<Hurt>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			const int16 moveSpeed = 1; // pixels/frame
			const int16 unitsToMove = moveSpeed * deltaTime;

			const Vector2I& dirVec = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());
			Owner().SetVelocity(dirVec * moveSpeed);

			mUnitsMoved += unitsToMove;

			if (mUnitsMoved >mUnitsToMoveInOneDir)
			{
				Owner().SetSpriteDir( (SpriteDir::Type)(((int)Owner().GetSpriteDir()+1) % SpriteDir::NumTypes) );
				PlayAnim(BaseAnim::Move); //@TODO: Annoying that we have to manually replay the anim on direction change...

				mUnitsMoved = 0;
				mUnitsToMoveInOneDir = MathEx::Rand(3, 5) * 16; // Move in random increments of 3 to 5 tiles
			}
		}
	};

	//@TODO: Move Stunned and Hurt to generic EnemyStates
	struct Stunned : EnemyStateBase
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
				return SiblingTransition<Hurt>();
			}

			mElapsedTime += deltaTime;
			if (mElapsedTime > SEC_TO_FRAMES(2.0))
			{
				return SiblingTransition<Moving>();
			}

			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			// Keep resetting stunned timer if restunned. Note that re-stunning happens
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

	struct Hurt : EnemyStateBase
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
			return SiblingTransition<Moving>();
		}
	};
};

//@TODO: Move to "gslib/Game/Enemies/Goriyas.h"
struct GoriyasAI : IEnemyAI
{
	virtual void Init(Enemy& enemy, CharacterSharedStateData& sharedStateData)
	{
		enemy.SetSpriteDir(SpriteDir::Down);
	}

	virtual Transition& GetRootTransition()
	{
		return InnerEntryTransition<GoriyasStates::Main>();
	}	
};



// Enemy class implementation

void Enemy::InitStateMachine()
{
	//@TODO: Push down to child class... (Goriyas.cpp)
	mpSharedStateData = new EnemySharedStateData();
	mpSharedStateData->mpEnemyAI = new GoriyasAI();

	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(mpSharedStateData);
	mStateMachine.SetInitialState<EnemyStates::Root>();
}

void Enemy::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	gameObjectInfo.mGameActor = GameActor::Goriyas;
}

void Enemy::OnCollision(const CollisionInfo& collisionInfo)
{
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
