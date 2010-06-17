#ifndef WEAPON_H
#define WEAPON_H

#include "GameObject.h"

class Weapon : public GameObject
{
public:
	typedef GameObject Base;

	void Init(bool isPlayerWeapon) { mIsPlayerWeapon = isPlayerWeapon; }
	bool IsPlayerWeapon() const { return mIsPlayerWeapon; }

	// IPhysics interface
	virtual bool AllowWorldOverlap() const { return true; }

private:
	bool mIsPlayerWeapon;
};


#endif // WEAPON_H
