#include "BoundingBox.h"
#include "gslib/Math/MathEx.h"

namespace
{
	bool CollidesWith(const BoundingBox& lhs, const BoundingBox& rhs, Vector2I* pOffset)
	{
		// TODO: Optimize this function

		int16 sX1 = lhs.pos.x;
		int16 sX2 = lhs.pos.x + lhs.w;
		int16 dX1 = rhs.pos.x;
		int16 dX2 = rhs.pos.x + rhs.w;

		int16 sY1 = lhs.pos.y;
		int16 sY2 = lhs.pos.y + lhs.h;
		int16 dY1 = rhs.pos.y;
		int16 dY2 = rhs.pos.y + rhs.h;

		if (	(sX2 <= dX1 || sX1 >= dX2)
			||	(sY2 <= dY1 || sY1 >= dY2) )
		{
			return false; // No overlap
		}

		if (pOffset == 0)
			return true;

		// Return smaller of two X offsets
		int16 xOffset1 = dX1 - sX2;
		int16 xOffset2 = dX2 - sX1;
		if (-xOffset1 < xOffset2)
		{
			pOffset->x = xOffset1;
		}
		else
		{
			pOffset->x = xOffset2;
		}

		// Return smaller of two Y offsets
		int16 yOffset1 = dY1 - sY2;
		int16 yOffset2 = dY2 - sY1;
		if (-yOffset1 < yOffset2)
		{
			pOffset->y = yOffset1;
		}
		else
		{
			pOffset->y = yOffset2;
		}

		// Finally, return smaller of two offsets (or both if equal)
		if (MathEx::Abs(pOffset->x) < MathEx::Abs(pOffset->y))
		{
			pOffset->y = 0;
		}
		else if (MathEx::Abs(pOffset->y) < MathEx::Abs(pOffset->x))
		{
			pOffset->x = 0;
		}

		return true;
	}

} // anonymous namespace

bool BoundingBox::CollidesWith(const BoundingBox& rhs) const
{
	return ::CollidesWith(*this, rhs, 0);
}

bool BoundingBox::CollidesWith(const BoundingBox& rhs, Vector2I& offset) const
{
	return ::CollidesWith(*this, rhs, &offset);
}
