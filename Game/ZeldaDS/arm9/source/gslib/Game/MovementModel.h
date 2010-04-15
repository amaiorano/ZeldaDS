#ifndef MOVEMENT_MODEL_H
#define MOVEMENT_MODEL_H

#include "gslib/Math/Vector2.h"
#include "GameAnims.h"
#include "GameHelpers.h"

namespace MovementModel
{
	// Moves character as expected: along input direction at input speed
	void MoveDefault(const Vector2I& currPos, const SpriteDir::Type& currDir, uint16 moveSpeed,
		Vector2I& newVelocity, SpriteDir::Type& newDir
		);

	// Moves the character along an 8x8 grid just like in the original Zelda (see code for more detail)
	void MoveGrid8x8(
		const Vector2I& currPos, const SpriteDir::Type& currDir, uint16 moveSpeed,
		Vector2I& newVelocity, SpriteDir::Type& newDir
		);

} // namespace MovementModel

#endif // MOVEMENT_MODEL_H
