#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

class SharedStateData;

class Enemy : public Character
{
public:
	typedef Character Base;	

	// Character interface
	virtual void InitStateMachine();

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

private:
	struct EnemySharedStateData* mpSharedStateData;

	friend class EnemyStates;
	friend class GoriyasStates; // ugh
};

#endif // ENEMY_H
