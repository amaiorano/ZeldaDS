#include "Enemy.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/MathEx.h"
#include "CharacterState.h"
#include "GameHelpers.h"
#include "Player.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "MovementModel.h"
#include <cstdlib>

// Interface for core enemy AI
struct IEnemyAI
{
	virtual void Init(Enemy& enemy, CharacterSharedStateData& sharedStateData)
	{
		enemy.SetSpriteDir(GameHelpers::GetRandomSpriteDir());
	}

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
	typedef CharacterStateBase<EnemySharedStateData, Enemy> EnemyStateBase;

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

			return Data().mpEnemyAI->GetRootTransition();
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
};

///////////////////////////////////////////////////////////////////////////////
// Shared states for all enemies (could be moved to EnemyStates)
///////////////////////////////////////////////////////////////////////////////
struct EnemySharedStates : EnemyStates
{
	struct RandomMovement : EnemyStateBase
	{
		uint16 mUnitsMoved;
		uint16 mUnitsToMoveInOneDir;

		// Desired direction will not necessarily match current direction due to 8x8 grid movement
		SpriteDir::Type mDesiredDir;

		//@TODO: Pass this in somehow
		static const uint16 mMinTilesToMove = 3;
		static const uint16 mMaxTilesToMove = 8;

		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Move);

			mUnitsMoved = 0;
			mUnitsToMoveInOneDir = MathEx::Rand(mMinTilesToMove, mMaxTilesToMove) * 16;

			mDesiredDir = Owner().GetSpriteDir();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			const int16 moveSpeed = 1; // pixels/frame
			const int16 unitsToMove = moveSpeed * deltaTime;

			// Move on 8x8 grid
			Vector2I newVelocity(InitZero);
			SpriteDir::Type newDir = SpriteDir::None;
			MovementModel::MoveGrid8x8(Owner().GetPosition(), mDesiredDir, moveSpeed, newVelocity, newDir);

			if (Owner().GetSpriteDir() != newDir)
			{
				Owner().SetSpriteDir(newDir);
				PlayAnim(BaseAnim::Move);  // Update anim on direction change
			}
			Owner().SetVelocity(newVelocity);

			//@TODO: This check is one frame late, meaning we're already off screen. As a result,
			// we're off the 8x8 grid, and it looks crappy when we change to a perpendicular direction.
			// We could check if future bounds are outside the screen bounds (frame-based dependent,
			// however, just like the rest of this function)
			ScrollDir::Type scrollDir = ScrollDir::None;
			const bool leavingScreen = !GameHelpers::IsPhysicalInScreenBounds(Owner(), &scrollDir) 
				&& GameHelpers::ScrollToSpriteDir(scrollDir) == Owner().GetSpriteDir();

			if (mDesiredDir == Owner().GetSpriteDir()) // Count units moved in desired direction
			{
				mUnitsMoved += unitsToMove;
			}

			const bool changeDirection = mUnitsMoved > mUnitsToMoveInOneDir || leavingScreen;
			if (changeDirection)
			{
				mDesiredDir = GameHelpers::GetRandomSpriteDir(Owner().GetSpriteDir());
				mUnitsMoved = 0;
				mUnitsToMoveInOneDir = MathEx::Rand(mMinTilesToMove, mMaxTilesToMove) * 16;
			}
		}
	};
};

///////////////////////////////////////////////////////////////////////////////
// Goryias
///////////////////////////////////////////////////////////////////////////////
struct GoriyasStates : EnemyStates
{
	struct Main : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement
	{
		HsmTimeType mElapsedTime;
		HsmTimeType mTimeToAttack;

		virtual void OnEnter()
		{
			mElapsedTime = 0;
			mTimeToAttack = MathEx::Rand(SEC_TO_FRAMES(2), SEC_TO_FRAMES(4));
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			mElapsedTime += deltaTime;
			if (mElapsedTime > mTimeToAttack)
			{
				return SiblingTransition<Attack>();
			}
			return NoTransition();
		}
	};

	struct Attack : EnemyStateBase
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Attack);
			//@TODO: Throw boomerang
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			//@TODO: When boomerang returns, go back to Move
			if (IsAnimFinished())
			{
				return SiblingTransition<Move>();
			}
			return NoTransition();
		}
	};
};

struct GoriyasAI : IEnemyAI
{
	virtual Transition& GetRootTransition()
	{
		return InnerEntryTransition<GoriyasStates::Main>();
	}	
};


///////////////////////////////////////////////////////////////////////////////
// Snake
///////////////////////////////////////////////////////////////////////////////
struct SnakeStates : EnemyStates
{
	// Snake moves around randomly, but as soon as it sees the player, it strikes in the players
	// direction until it collides, at which point it resumes random movement

	struct Main : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement
	{
		typedef EnemySharedStates::RandomMovement Base;
		bool mShouldStrike;

		Move() : mShouldStrike(false)
		{
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			if (mShouldStrike)
			{
				return SiblingTransition<Attack>();
			}
			return NoTransition();
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			Base::PerformStateActions(deltaTime); // Move

			// Due to 8x8 grid movement, we may not be facing our desired direction, so we only
			// strike when we're on the grid line
			const bool facingDesiredDirection = mDesiredDir == Owner().GetSpriteDir();
			if (facingDesiredDirection)
			{
				// Project a bounding box in front of myself and see if it collides with the player
				BoundingBox fwdBBox = Owner().GetBoundingBox();
				
				switch (mDesiredDir)
				{
				case SpriteDir::Left:
					fwdBBox.pos.x -= HwScreenSizeX;
					fwdBBox.w = HwScreenSizeX;
					break;

				case SpriteDir::Right:
					fwdBBox.pos.x += Owner().GetWidth();
					fwdBBox.w = HwScreenSizeX;
					break;

				case SpriteDir::Up:
					fwdBBox.pos.y -= HwScreenSizeY;
					fwdBBox.h = HwScreenSizeY;
					break;

				case SpriteDir::Down:
					fwdBBox.pos.y += Owner().GetHeight();
					fwdBBox.h = HwScreenSizeY;
					break;

				default: FAIL(); break;
				}

				const Player* pPlayer = SceneGraph::Instance().GetPlayerList().front(); //@TODO: iterate for all players
				mShouldStrike = fwdBBox.CollidesWith(pPlayer->GetBoundingBox());

				//extern void DrawQuad(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color, uint16 alpha);
				//fwdBBox.pos = Camera::Instance().WorldToScreen(fwdBBox.pos);
				//DrawQuad(fwdBBox.pos.x, fwdBBox.pos.y, fwdBBox.w, fwdBBox.h, RGB15(255,0,0), 15);
			}
		}
	};

	struct Attack : EnemyStateBase
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			// If we hit anything (player or world), stop attacking
			if (Owner().mLastFrameCollision.mIsSet 
				|| !GameHelpers::IsPhysicalInScreenBounds(Owner())
				)
			{
				// Change direction to make sure we don't keep striking the player
				Owner().SetSpriteDir( GameHelpers::GetRandomSpriteDir(Owner().GetSpriteDir()) );

				return SiblingTransition<Move>();
			}

			return InnerEntryTransition<Alert>();
		}
	};

	struct Alert : EnemyStateBase
	{
		GameTimeType mElapsedTime;

		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Idle);
		}

		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			mElapsedTime += deltaTime;
			if (mElapsedTime > SEC_TO_FRAMES(0.3f))
			{
				return SiblingTransition<Strike>();
			}

			return NoTransition();
		}
	};

	struct Strike : EnemyStateBase
	{
		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Attack);
		}

		virtual void PerformStateActions(HsmTimeType deltaTime)
		{
			const int16 moveSpeed = 2; // pixels/frame
			const Vector2I& dirVec = GameHelpers::SpriteDirToUnitVector(Owner().GetSpriteDir());
			Owner().SetVelocity(dirVec * moveSpeed);
		}
	};
};

struct SnakeAI : IEnemyAI
{
	virtual Transition& GetRootTransition()
	{
		return InnerEntryTransition<SnakeStates::Main>();
	}	
};


// Enemy class implementation

void Enemy::Init(const Vector2I& initPos, GameActor::Type enemyType)
{
	mGameActor = enemyType;
	Base::Init(initPos);
}

void Enemy::InitStateMachine()
{
	IEnemyAI* pEnemyAI = NULL;
	switch (mGameActor)
	{
	case GameActor::Goriyas: pEnemyAI = new GoriyasAI(); break;
	case GameActor::Snake: pEnemyAI = new SnakeAI(); break;
	default: FAIL();
	}

	mpSharedStateData = new EnemySharedStateData();
	mpSharedStateData->mpEnemyAI = pEnemyAI;

	mStateMachine.SetDebugLevel(1);
	mStateMachine.SetOwner(this);
	mStateMachine.SetSharedStateData(mpSharedStateData);
	mStateMachine.SetInitialState<EnemyStates::Root>();
}

void Enemy::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	gameObjectInfo.mGameActor = mGameActor;
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
