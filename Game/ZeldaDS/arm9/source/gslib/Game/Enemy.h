#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

class Enemy : public Character
{
public:
	typedef Character Base;	

	Enemy();

	// Character interface
	virtual void InitStateMachine();

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

protected:
	virtual Transition& GetRootTransition() = 0;

private:
	struct EnemySharedStateData* mpSharedStateData;
	
	struct LastFrameCollision
	{
		LastFrameCollision() : mIsSet(false) { }
		bool mIsSet;
		CollisionInfo mInfo;
	} mLastFrameCollision;

	friend class EnemyStates;
	friend class GoriyasStates;
	friend class SnakeStates;
};

#endif // ENEMY_H
