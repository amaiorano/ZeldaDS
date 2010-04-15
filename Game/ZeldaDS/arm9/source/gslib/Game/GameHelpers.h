#ifndef GAME_HELPERS_H
#define GAME_HELPERS_H

#include "gslib/Math/Vector2.h"
#include "GameAnims.h"

namespace GameHelpers
{
	Vector2I SpriteDirToUnitVector(SpriteDir::Type dir);
	SpriteDir::Type VectorToSpriteDir(const Vector2I vec);
}

#endif // GAME_HELPERS_H
