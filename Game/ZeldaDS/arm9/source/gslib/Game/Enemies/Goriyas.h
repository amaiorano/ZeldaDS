#ifndef ENEMY_GORIYAS_H
#define ENEMY_GORIYAS_H

#include "gslib/Game/Enemy.h"

class Goriyas : public Enemy
{
	typedef Enemy Base;

protected:
	void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		gameObjectInfo.mGameActor = GameActor::Goriyas;
	}

	virtual EnemySharedStateData* CreateSharedStateData();
	virtual Transition& GetRootTransition();
	virtual void OnDead();
	virtual void Update(GameTimeType deltaTime);
};

#endif // ENEMY_GORIYAS_H
