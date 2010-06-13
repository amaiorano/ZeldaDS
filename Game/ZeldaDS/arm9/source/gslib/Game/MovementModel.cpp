#include "MovementModel.h"

namespace MovementModel
{
	void MoveDefault(const Vector2I& currPos, const SpriteDir::Type& currDir, uint16 moveSpeed,
		Vector2I& newVelocity, SpriteDir::Type& newDir
		)
	{
		newVelocity = GameHelpers::SpriteDirToUnitVector(currDir) * moveSpeed;
		newDir = currDir;
	}

	void MoveGrid8x8(
		const Vector2I& currPos, const SpriteDir::Type& currDir, uint16 moveSpeed,
		Vector2I& newVelocity, SpriteDir::Type& newDir
		)
	{
		// This code moves the character along an 8x8 grid just like in the original Zelda.
		// We can move continuously along one of the grid lines, and when changing direction,
		// we must shift to the nearest grid line in the new direction.

		const Vector2I gridCell(8, 8);

		// Compute offset to nearest grid line
		///@TODO: This currently is biased towards the bottom/right grid line when at the exact middle pixel.
		//        We should bias towards the grid line that we're facing in this case.
		uint16 nearestGridLine = 0;
		Vector2I offset(InitZero);

		// Only offset towards nearest grid line in direction we're moving
		switch (currDir)
		{
		case SpriteDir::Left: case SpriteDir::Right:
			nearestGridLine = static_cast<uint16>( (currPos.y + gridCell.y/2) / gridCell.y ) * gridCell.y;
			offset.y = nearestGridLine - currPos.y;
			break;

		case SpriteDir::Up: case SpriteDir::Down:
			nearestGridLine = static_cast<uint16>( (currPos.x + gridCell.x/2) / gridCell.x ) * gridCell.x;
			offset.x = nearestGridLine - currPos.x;
			break;

		default:
			FAIL();
			break;
		}

		if (offset.Length() > 0)
		{
			// Make sure we don't overshoot our offset this frame (@NOTE: relying on frame-based movement)
			if (moveSpeed > offset.Length())
				moveSpeed = offset.Length();

			// Now move along the offset vector - note that we rely on the fixed frame rate (can't overshoot in one frame)
			newVelocity = Normalized(offset) * moveSpeed;
			newDir = GameHelpers::VectorToSpriteDir(offset); // Stick to direction we're moving in
		}
		else
		{
			MoveDefault(currPos, currDir, moveSpeed, newVelocity, newDir);
		}
	}

} // namespace MovementModel
