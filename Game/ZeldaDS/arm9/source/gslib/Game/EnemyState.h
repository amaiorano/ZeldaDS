#ifndef ENEMY_SM_H
#define ENEMY_SM_H

#include "CharacterState.h"
#include "MovementModel.h"

// Contains base and shared enemy state machine stuff, included by state machine
// implementations of enemies

// Enemy shared state data
struct EnemySharedStateData : CharacterSharedStateData
{
	EnemySharedStateData()
		: mAttribCanDamage(false)
	{
	}

	Attribute<bool> mAttribCanDamage; // Can we damage player?
};

// Base class for enemy states
typedef CharacterStateBase<EnemySharedStateData, Enemy> EnemyStateBase;

// Shared states for all enemies
struct EnemySharedStates
{
	template <typename Base = EnemyStateBase>
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

		virtual void PerformStateActions(HsmTimeType deltaTime)
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


#endif // ENEMY_SM_H
