#ifndef BOOMERANG_H
#define BOOMERANG_H

#include "gslib/Core/Core.h"
#include "Weapon.h"

class Sprite;
class Player;

class Boomerang	: public Weapon
{
public:
	typedef Weapon Base;

	//@TODO: Should NOT store a bald pointer here... what if owner is deleted?
	void Init(Player* pOwner, const Vector2I& launchDir);

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
	Player* mpOwner; // Not sure I like this...

	Vector2F mPositionF;
	Vector2F mSpeed;

	bool mIsLeaving;
	bool mHasReturned;
};

#endif // BOOMERANG_H
