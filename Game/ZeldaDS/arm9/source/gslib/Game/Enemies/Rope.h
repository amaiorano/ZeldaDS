#ifndef ENEMY_ROPE_H
#define ENEMY_ROPE_H

#include "gslib/Game/Enemy.h"

class Rope : public Enemy
{
	typedef Enemy Base;

protected:
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		Base::GetGameObjectInfo(gameObjectInfo);
		gameObjectInfo.mGameActor = GameActor::Rope;
	}

	virtual hsm::Transition GetRootTransition();

private:
	friend struct RopeStates;
};


#endif // ENEMY_ROPE_H
