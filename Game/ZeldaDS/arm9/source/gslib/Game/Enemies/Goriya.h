#ifndef ENEMY_GORIYA_H
#define ENEMY_GORIYA_H

#include "gslib/Game/Enemy.h"

class Goriya : public Enemy
{
	typedef Enemy Base;

protected:
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		Base::GetGameObjectInfo(gameObjectInfo);
		gameObjectInfo.mGameActor = GameActor::Goriya;
	}

	virtual SharedStateData* CreateSharedStateData();
	virtual Transition GetRootTransition();
	virtual void OnDead();
	virtual void Update(GameTimeType deltaTime);
};

#endif // ENEMY_GORIYA_H
