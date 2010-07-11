#ifndef BOOMERANG_H
#define BOOMERANG_H

#include "gslib/Core/Core.h"
#include "Weapon.h"

class Sprite;
class Character;

class Boomerang	: public Weapon
{
public:
	typedef Weapon Base;

	Boomerang(bool isPlayerWeapon);

	//@TODO: Should NOT store a bald pointer here... what if owner is deleted?
	void Init(Character* pOwner, const Vector2I& launchDir);

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

	//@TODO: virtual bool Item::IsComplete() const
	bool HasReturned() const { return mHasReturned; }

private:
	Character* mpOwner; // Not sure I like this...

	Vector2F mPositionF;
	Vector2F mSpeed;

	bool mIsLeaving;
	bool mHasReturned;
};



#endif // BOOMERANG_H
