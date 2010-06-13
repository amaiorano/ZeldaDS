#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "gslib/Core/Types.h"
#include "gslib/Math/Vector2.h"

class BoundingBox
{
public:
	Vector2I pos;
	int16 w, h;

	BoundingBox()
		: pos(InitZero), w(0), h(0)
	{
	}

	BoundingBox(const Vector2I& pos, int16 w, int16 h)
		: pos(pos), w(w), h(h)
	{
	}

	void Reset(int16 x, int16 y, int16 w, int16 h)
	{
		pos.x = x;
		pos.y = y;
		this->w = w;
		this->h = h;
	}

	int16 Left() const		{ return pos.x; }
	int16 Right() const		{ return pos.x + w; }
	int16 Top() const		{ return pos.y; }
	int16 Bottom() const	{ return pos.y + h; }

	// Returns true if this rect collides with rhs
	bool CollidesWith(const BoundingBox& rhs) const;

	// Returns true if this rect collides with rhs, along with the amount
	// to offset this rect so as not to collide with it.
	bool CollidesWith(const BoundingBox& rhs, Vector2I& offset) const;
};


#endif // BOUNDING_BOX_H
