#include "GameHelpers.h"
#include "gslib/Core/Core.h"
#include "gslib/Hw/Constants.h"
#include "IPhysical.h"
#include "Camera.h"

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

	SpriteDir::Type GetRandomSpriteDir(SpriteDir::Type excludeDir)
	{
		if (excludeDir == SpriteDir::Unset)
		{
			return static_cast<SpriteDir::Type>(MathEx::Rand(SpriteDir::NumTypes - 1));
		}
		else
		{
			const int dir = (static_cast<int>(excludeDir) + MathEx::Rand(1, SpriteDir::NumTypes - 1)) % SpriteDir::NumTypes;
			return static_cast<SpriteDir::Type>(dir);
		}
	}

	bool IsPhysicalInScreenBounds(const IPhysical& physical, ScrollDir::Type* pDirToScroll)
	{
		const BoundingBox& worldBBox = physical.GetBoundingBox();
		BoundingBox screenBBox(Camera::Instance().WorldToScreen(worldBBox.pos), worldBBox.w, worldBBox.h);
		
		ScrollDir::Type scrollDir = ScrollDir::None;

		if (screenBBox.Left() < 0)						scrollDir = ScrollDir::Left;
		else if (screenBBox.Top() < 0)					scrollDir = ScrollDir::Up;
		else if (screenBBox.Right() > HwScreenSizeX)	scrollDir = ScrollDir::Right;
		else if (screenBBox.Bottom() > HwScreenSizeY)	scrollDir = ScrollDir::Down;

		if (pDirToScroll)
		{
			*pDirToScroll = scrollDir;
		}

		return scrollDir == ScrollDir::None;
	}

	SpriteDir::Type ScrollToSpriteDir(ScrollDir::Type scrollDir)
	{
		switch (scrollDir)
		{
		case ScrollDir::None:	return SpriteDir::None;
		case ScrollDir::Right:	return SpriteDir::Right;
		case ScrollDir::Left:	return SpriteDir::Left;
		case ScrollDir::Up:		return SpriteDir::Up;
		case ScrollDir::Down:	return SpriteDir::Down;
		}
		ASSERT(FALSE);
		return SpriteDir::Unset;
	}
}
