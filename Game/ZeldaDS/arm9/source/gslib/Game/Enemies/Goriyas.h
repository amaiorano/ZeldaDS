#ifndef ENEMY_GORIYAS_H
#define ENEMY_GORIYAS_H

#include "gslib/Game/Enemy.h"

class Goriyas : public Enemy
{
protected:
	void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		gameObjectInfo.mGameActor = GameActor::Goriyas;
	}

	virtual Transition& GetRootTransition();
};

#endif // ENEMY_GORIYAS_H
