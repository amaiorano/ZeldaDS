#include "BoundingBox.h"
#include "gslib/Math/MathEx.h"

bool BoundingBox::CollidesWith(const BoundingBox& rhs, Vector2I& offset) const
{
	// TODO: Optimize this function

	int16 sX1 = pos.x;
	int16 sX2 = pos.x + w;
	int16 dX1 = rhs.pos.x;
	int16 dX2 = rhs.pos.x + rhs.w;

	int16 sY1 = pos.y;
	int16 sY2 = pos.y + h;
	int16 dY1 = rhs.pos.y;
	int16 dY2 = rhs.pos.y + rhs.h;

	if (	(sX2 <= dX1 || sX1 >= dX2)
		||	(sY2 <= dY1 || sY1 >= dY2) )
	{
		return false; // No overlap
	}

	// Return smaller of two X offsets
	int16 xOffset1 = dX1 - sX2;
	int16 xOffset2 = dX2 - sX1;
	if (-xOffset1 < xOffset2)
	{
		offset.x = xOffset1;
	}
	else
	{
		offset.x = xOffset2;
	}

	// Return smaller of two Y offsets
	int16 yOffset1 = dY1 - sY2;
	int16 yOffset2 = dY2 - sY1;
	if (-yOffset1 < yOffset2)
	{
		offset.y = yOffset1;
	}
	else
	{
		offset.y = yOffset2;
	}

	// Finally, return smaller of two offsets (or both if equal)
	if (MathEx::Abs(offset.x) < MathEx::Abs(offset.y))
	{
		offset.y = 0;
	}
	else if (MathEx::Abs(offset.y) < MathEx::Abs(offset.x))
	{
		offset.x = 0;
	}

	return true;
}
