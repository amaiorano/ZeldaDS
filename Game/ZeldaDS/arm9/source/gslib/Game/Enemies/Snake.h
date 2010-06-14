#ifndef ENEMY_SNAKE_H
#define ENEMY_SNAKE_H

#include "gslib/Game/Enemy.h"

class Snake : public Enemy
{
protected:
	void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		gameObjectInfo.mGameActor = GameActor::Snake;
	}

	virtual Transition& GetRootTransition();
};


#endif // ENEMY_SNAKE_H
