#ifndef GAME_HELPERS_H
#define GAME_HELPERS_H

#include "gslib/Math/Vector2.h"
#include "GameAnims.h"
#include "ScrollingMgr.h"

class IPhysical;

namespace GameHelpers
{
	Vector2I SpriteDirToUnitVector(SpriteDir::Type dir);
	SpriteDir::Type VectorToSpriteDir(const Vector2I vec);
	SpriteDir::Type GetRandomSpriteDir(SpriteDir::Type excludeDir = SpriteDir::Unset);
	bool IsPhysicalInScreenBounds(const IPhysical& physical, ScrollDir::Type* pDirToScroll = 0);

	SpriteDir::Type ScrollToSpriteDir(ScrollDir::Type scrollDir);
}

#endif // GAME_HELPERS_H
