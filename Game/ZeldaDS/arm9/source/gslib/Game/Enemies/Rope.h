#ifndef ENEMY_ROPE_H
#define ENEMY_ROPE_H

#include "gslib/Game/Enemy.h"

class Rope : public Enemy
{
protected:
	void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		gameObjectInfo.mGameActor = GameActor::Rope;
	}

	virtual Transition& GetRootTransition();
};


#endif // ENEMY_ROPE_H
