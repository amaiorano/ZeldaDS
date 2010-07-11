#ifndef WEAPON_H
#define WEAPON_H

#include "GameObject.h"

class Weapon : public GameObject
{
	typedef GameObject Base;

protected:
	Weapon(bool isPlayerWeapon)
		: mIsPlayerWeapon(isPlayerWeapon)
	{
	}

public:
	bool IsPlayerWeapon() const { return mIsPlayerWeapon; }

	// IPhysics interface
	virtual bool AllowWorldOverlap() const { return true; }

private:
	bool mIsPlayerWeapon;
};

#endif // WEAPON_H
