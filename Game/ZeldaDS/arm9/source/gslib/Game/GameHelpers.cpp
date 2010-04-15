#include "GameHelpers.h"
#include "gslib/Core/Core.h"

namespace GameHelpers
{
	Vector2I SpriteDirToUnitVector(SpriteDir::Type dir)
	{
		Vector2I v(InitZero);

		switch (dir)
		{
			case SpriteDir::Right:	v.x = 1;	break;
			case SpriteDir::Left:	v.x = -1;	break;
			case SpriteDir::Down:	v.y = 1;	break;
			case SpriteDir::Up:		v.y = -1;	break;
			default: FAIL(); break;
		}

		return v;
	}

	SpriteDir::Type VectorToSpriteDir(const Vector2I vec)
	{
		ASSERT(vec.x == 0 || vec.y == 0);
		if (vec.x > 0) return SpriteDir::Right;
		if (vec.x < 0) return SpriteDir::Left;
		if (vec.y > 0) return SpriteDir::Down;
		if (vec.y < 0) return SpriteDir::Up;

		FAIL();
		return SpriteDir::None;
	}
}
