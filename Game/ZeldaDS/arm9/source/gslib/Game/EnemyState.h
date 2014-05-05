#ifndef ENEMY_STATE_H
#define ENEMY_STATE_H

#include "CharacterState.h"
#include "Enemy.h"
#include "MovementModel.h"

// Contains base and shared enemy state machine stuff, included by state machine
// implementations of enemies

// Base class for enemy states
typedef StateWithOwner<Enemy, CharacterState> EnemyState;

// Shared states for all enemies
struct EnemySharedStates
{
	template <typename Base = EnemyState>
	struct RandomMovement : Base
	{
		// Bring in names that compiler cannot deduce
		using Base::PlayAnim;
		using Base::Owner;

		uint16 mUnitsMoved;
		uint16 mUnitsToMoveInOneDir;

		// Desired direction will not necessarily match current direction due to 8x8 grid movement
		SpriteDir::Type mDesiredDir;

		uint16 mMinTilesToMove;
		uint16 mMaxTilesToMove;
		uint16 mMoveSpeed;
		
		uint16 mMoveUpdateDelay;
		uint16 mMoveUpdatesElapsed;

		RandomMovement()
		{
			SetMinMaxTilesToMove();
			SetMoveSpeed();
		}

		void SetMinMaxTilesToMove(uint16 minTilesToMove = 3, uint16 maxTilesToMove = 8)
		{
			mMinTilesToMove = minTilesToMove;
			mMaxTilesToMove = maxTilesToMove;
		}

		void SetMoveSpeed(uint16 moveSpeed = 1, uint16 moveUpdateDelay = 0)
		{
			mMoveSpeed = moveSpeed;
			mMoveUpdateDelay = moveUpdateDelay;
		}

		virtual void OnEnter()
		{
			PlayAnim(BaseAnim::Move);

			mUnitsMoved = 0;
			mUnitsToMoveInOneDir = MathEx::Rand(mMinTilesToMove, mMaxTilesToMove) * 16;

			mDesiredDir = Owner().GetSpriteDir();
		}

		virtual void Update(GameTimeType deltaTime)
		{
			// Shitty way to slow down motion while using int vectors
			if (mMoveUpdateDelay > 0)
			{
				mMoveUpdatesElapsed += deltaTime;
				if (mMoveUpdatesElapsed < mMoveUpdateDelay)
					return;
				mMoveUpdatesElapsed = 0;
			}

			const int16 unitsToMove = mMoveSpeed * deltaTime;

			// Move on 8x8 grid
			Vector2I newVelocity(InitZero);
			SpriteDir::Type newDir = SpriteDir::None;
			MovementModel::MoveGrid8x8(Owner().GetPosition(), mDesiredDir, mMoveSpeed, newVelocity, newDir);

			if (Owner().GetSpriteDir() != newDir)
			{
				Owner().SetSpriteDir(newDir);
				PlayAnim(BaseAnim::Move); // Update anim on direction change
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

struct EnemyStates
{
	struct Root : EnemyState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mHealth.IsDead())
			{
				return InnerTransition<Dead>();
			}

			return InnerTransition<Alive>();
		}
	};

	struct Alive : EnemyState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Alive_Spawn>();
		}
	};

	struct Alive_Spawn : EnemyState
	{
		virtual void OnEnter()
		{
			PlayGlobalAnim(BaseAnim::Spawn);
		}

		virtual Transition GetTransition()
		{
			if (IsAnimFinished())
			{
				return SiblingTransition<Alive_Main>();
			}

			return NoTransition();
		}
	};

	// AI's main state
	struct Alive_Main : EnemyState
	{
		virtual void OnEnter()
		{
			SetStateValue(Owner().mSvCanDealDamage) = true;
			SetStateValue(Owner().mSvCanTakeDamage) = true;
		}

		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Alive_Main_Locomotion>();
		}
	};

	struct Alive_Main_Locomotion : EnemyState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mDamageInfo.IsSet())
			{
				if (Owner().mDamageInfo.mEffect == DamageEffect::Stun)
				{
					return SiblingTransition<Alive_Main_Stunned>();
				}

				return SiblingTransition<Alive_Main_Hurt>();
			}

			//@TODO: Something along these lines...
			//return Transition(Transition::InnerEntry, Owner().GetTargetState(GetStateTypeId(Alive_Main_Locomotion));
			return Owner().GetRootTransition();
		}
	};

	struct Alive_Main_Stunned : EnemyState
	{
		GameTimeType mElapsedTime;

		virtual void OnEnter()
		{
			mElapsedTime = 0;
			ASSERT(Owner().mDamageInfo.mEffect == DamageEffect::Stun);
			Owner().mDamageInfo.Reset();

			PlayAnim(BaseAnim::Idle);
		}

		virtual Transition GetTransition()
		{
			// Not crazy about this, probably should have single outer state
			// that transitions us to Hurt
			DamageInfo& dmgInfo = Owner().mDamageInfo;
			if (dmgInfo.IsSet() && dmgInfo.mEffect != DamageEffect::Stun)
			{
				return SiblingTransition<Alive_Main_Hurt>();
			}

			if (mElapsedTime > SEC_TO_FRAMES(2.0))
			{
				return SiblingTransition<Alive_Main_Locomotion>();
			}

			return NoTransition();
		}

		virtual void Update(GameTimeType deltaTime)
		{
			mElapsedTime += deltaTime;

			// Keep resetting timer if re-stunned. Note that re-stunning happens a few times even
			// on the initial stun since the boomerang continues to collide with the enemy for a
			// few frames. This is ok, not much we can do about it.
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

	struct Alive_Main_Hurt : EnemyState
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

		virtual Transition GetTransition()
		{
			return SiblingTransition<Alive_Main_Locomotion>();
		}
	};	

	struct Dead : EnemyState
	{
		virtual void OnEnter()
		{
			PlayGlobalAnim(BaseAnim::Spawn); // Play same anim as spawn
			//@TODO: Tell someone we're dead
		}

		virtual void Update(GameTimeType deltaTime)
		{
			if (IsAnimFinished())
			{
				Owner().OnDead();
			}
		}
	};

}; // struct EnemyStates

#endif // ENEMY_STATE_H
