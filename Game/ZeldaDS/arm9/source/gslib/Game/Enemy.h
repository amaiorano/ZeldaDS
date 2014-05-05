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
	virtual hsm::Transition GetRootTransition() = 0;

	struct LastFrameCollision
	{
		LastFrameCollision() : mIsSet(false) { }
		bool mIsSet;
		CollisionInfo mInfo;
	} mLastFrameCollision;

	friend struct EnemyStates;
	hsm::StateValue<bool> mSvCanDealDamage; // Can we damage player?
};

#endif // ENEMY_H
