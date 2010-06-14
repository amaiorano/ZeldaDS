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


#endif // ENEMY_SM_H
