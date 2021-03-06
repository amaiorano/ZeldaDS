#include "Camera.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Physics/BoundingBox.h"

Vector2I Camera::ScreenToWorld(const Vector2I& screenSpacePos) const
{
	return GetPosition() + screenSpacePos;
}

Vector2I Camera::WorldToScreen(const Vector2I& worldSpacePos) const
{
	return worldSpacePos - GetPosition();
}

bool Camera::IsWorldBBoxInScreenBounds(const BoundingBox& worldBBox) const
{
	BoundingBox screenBBox(WorldToScreen(worldBBox.pos), worldBBox.w, worldBBox.h);

	return (screenBBox.Left() >= 0
		&& screenBBox.Top() >= 0 
		&& screenBBox.Right() < HwScreenSizeX 
		&& screenBBox.Bottom() < HwScreenSizeY);
}
