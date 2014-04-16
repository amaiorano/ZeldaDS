#ifndef ENEMY_GENERIC_ENEMY_H
#define ENEMY_GENERIC_ENEMY_H

#include "gslib/Game/Enemy.h"

class GenericEnemy : public Enemy
{
public:
	typedef Enemy Base;

	GenericEnemy(GameActor::Type gameActor);
	virtual void Init(const Vector2I& initPos);

protected:
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);
	virtual hsm::Transition GetRootTransition();

private:
	GameActor::Type mGameActor; //@LAME: Once again, caching a variable we already store in a base data member
};


#endif // ENEMY_GENERIC_ENEMY_H
