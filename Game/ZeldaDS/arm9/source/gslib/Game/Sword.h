#ifndef SWORD_H
#define SWORD_H

#include "gslib/Core/Core.h"
#include "Weapon.h"

class Sword : public Weapon
{
public:
	typedef Weapon Base;

	Sword();

	void Init(SpriteDir::Type dir);

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

private:
	SpriteDir::Type mDirection;
};

#endif //SWORD_H
