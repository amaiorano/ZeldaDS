#ifndef ENEMY_GORIYA_H
#define ENEMY_GORIYA_H

#include "gslib/Game/Enemy.h"

class Boomerang;

class Goriya : public Enemy
{
public:
	typedef Enemy Base;

	Goriya();

protected:
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		Base::GetGameObjectInfo(gameObjectInfo);
		gameObjectInfo.mGameActor = GameActor::Goriya;
	}

	virtual hsm::Transition GetRootTransition();
	virtual void OnDead();
	virtual void Update(GameTimeType deltaTime);

	friend struct GoriyaStates;
	Boomerang* mpBoomerang;
};

#endif // ENEMY_GORIYA_H
