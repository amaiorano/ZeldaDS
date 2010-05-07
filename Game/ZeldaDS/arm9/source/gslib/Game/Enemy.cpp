#include "Enemy.h"
#include "ActorSharedStateData.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Game/GameHelpers.h"
#include "gslib/Math/MathEx.h"
#include "Player.h"
#include "SceneGraph.h"
#include <cstdlib>

// Interface for core enemy AI
struct IEnemyAI
{
	virtual void Init(Enemy& enemy, ActorSharedStateData& sharedStateData) {}
	virtual Transition& GetRootTransition() { return NoTransition(); }	
};

struct EnemyStates
{
	struct EnemySharedStateData : ActorSharedStateData
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
		Attribute<bool> mAttribCanDamage;
	};

	struct EnemyStateBase : ActorStateBase<EnemySharedStateData, Enemy>
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
			if (Data().mHealth <= 0)
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
			if (Owner().mDamageInfo.mAmount > 0 && Owner().mDamageInfo.mEffect != DamageEffect::Stun)
			{
				//@TODO: For now, just absorb the damage
				Data().mHealth -= Owner().mDamageInfo.mAmount;
				Owner().mDamageInfo.mAmount = 0;
			}

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
			if (Owner().mDamageInfo.mAmount > 0)
			{
				if (Owner().mDamageInfo.mEffect == DamageEffect::Stun)
				{
					return SiblingTransition<Stunned>();
				}
				else
				{
					FAIL(); // Non-stun handled in outer state
				}
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

	struct Stunned : EnemyStateBase
	{
		HsmTimeType mElapsedTime;

		virtual void OnEnter()
		{
			mElapsedTime = 0;
			PlayAnim(BaseAnim::Idle);
		}

		virtual void OnExit()
		{
			Owner().mDamageInfo.Reset();
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			mElapsedTime += deltaTime;

			if (mElapsedTime > SEC_TO_FRAMES(2.0))
			{
				return SiblingTransition<Moving>();
			}

			return NoTransition();
		}
	};
};

//@TODO: Move to "gslib/Game/Enemies/Goriyas.h"
struct GoriyasAI : IEnemyAI
{
	virtual void Init(Enemy& enemy, ActorSharedStateData& sharedStateData)
	{
		enemy.SetSpriteDir(SpriteDir::Down);
	}

	virtual Transition& GetRootTransition()
	{
		return InnerEntryTransition<GoriyasStates::Main>();
	}	
};



// Enemy class implementation

void Enemy::Init(const Vector2I& initPos)
{
	//@TODO: Push down to child class... (Goriyas.cpp)
	EnemyStates::EnemySharedStateData* pSharedStateData = new EnemyStates::EnemySharedStateData();
	pSharedStateData->mpEnemyAI = new GoriyasAI();

	//mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(pSharedStateData);
	mStateMachine.SetInitialState<EnemyStates::Root>();

	mInitPos = initPos;
}

void Enemy::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	gameObjectInfo.mGameActor = GameActor::Goriyas;
}

void Enemy::OnAddToScene()
{
	Base::OnAddToScene();

	SetPosition(mInitPos);
	mDamageInfo.Reset();
	PlayAnim(BaseAnim::Idle); // Set initial pose
}

void Enemy::Update(GameTimeType deltaTime)
{
	if (deltaTime > 0) // Total cop out, we don't handle deltaTime == 0 very well
	{
		mStateMachine.Update(deltaTime);
		
		//@TODO: Maybe state machine should provide some kind of hook for pre/post update?
		ActorSharedStateData& data = static_cast<ActorSharedStateData&>(mStateMachine.GetSharedStateData());
		data.PostHsmUpdate(deltaTime);
	}
	
	Base::Update(deltaTime);
}

void Enemy::OnCollision(const CollisionInfo& collisionInfo)
{
	if ( Player* pPlayer = DynamicCast<Player*>(collisionInfo.mpCollidingWith) )
	{
		if (static_cast<EnemyStates::EnemySharedStateData&>(mStateMachine.GetSharedStateData()).mAttribCanDamage)
		{
			static DamageInfo dmgInfo; //@MT_UNSAFE
			dmgInfo.mAmount = 1;
			dmgInfo.mPushVector = -collisionInfo.mPushVector;
			pPlayer->OnDamage(dmgInfo);
		}
	}
}

void Enemy::OnDamage(const DamageInfo& damageInfo)
{
	// Phew, so many hacks already!
	//@TODO: Remove this! We need to handle multiple OnDamage calls properly!
	// State machine should not rely on mDamageInfo.mAmount to stay in damaged state.
	// For instance, while stunned, if another stun ondamage happens, the enemy
	// should prolong (reset/reenter) it's stunned state. Or while stunned, if a
	// hurt ondamage happens, he should get hurt even while stunned.
	if (mDamageInfo.mAmount > 0 && mDamageInfo.mEffect == damageInfo.mEffect)
		return;

	mDamageInfo = damageInfo;
}
