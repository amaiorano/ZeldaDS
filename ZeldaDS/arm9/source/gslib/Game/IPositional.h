#ifndef IPOSITIONAL_H
#define IPOSITIONAL_H

#include "gslib/Math/Vector2.h"

class IPositional
{
public:
	IPositional() : mPosition(InitZero) { }
	virtual ~IPositional() { }

	const Vector2I& GetPosition() const { return mPosition; }
	void SetPosition(const Vector2I& pos);

protected:
	Vector2I mPosition;
};

#endif // IPOSITIONAL_H
