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

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		Base::GetGameObjectInfo(gameObjectInfo);
		gameObjectInfo.mSpriteRenderGroupId = GameSpriteRenderGroup::Weapons;
	}

	// IPhysics interface
	virtual bool AllowWorldOverlap() const { return true; }

private:
	bool mIsPlayerWeapon;
};

#endif // WEAPON_H
