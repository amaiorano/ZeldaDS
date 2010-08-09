#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

struct EnemySharedStateData;

class Enemy : public Character
{
public:
	typedef Character Base;	

	Enemy();

	// Character interface
	virtual void InitStateMachine();
	virtual void OnDead();

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		Base::GetGameObjectInfo(gameObjectInfo);
		gameObjectInfo.mSpriteRenderGroupId = GameSpriteRenderGroup::Enemies;
	}

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

protected:
	// Child class must override if extending EnemySharedStateData
	virtual EnemySharedStateData* CreateSharedStateData();

	virtual Transition& GetRootTransition() = 0;

	EnemySharedStateData* mpSharedStateData;
	
	struct LastFrameCollision
	{
		LastFrameCollision() : mIsSet(false) { }
		bool mIsSet;
		CollisionInfo mInfo;
	} mLastFrameCollision;

	friend class EnemyStates;
	//@TODO: Figure out how to get rid of these friendships
	friend class GoriyaStates;
	friend class RopeStates;
};

#endif // ENEMY_H
