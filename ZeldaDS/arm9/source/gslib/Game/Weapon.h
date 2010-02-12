#ifndef WEAPON_H
#define WEAPON_H

#include "GameObject.h"

class Weapon : public GameObject
{
public:
	typedef GameObject Base;

	// IPhysics interface
	virtual bool AllowWorldOverlap() const { return true; }
};


#endif // WEAPON_H
