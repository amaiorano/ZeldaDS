#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"

class SharedStateData;

class Enemy : public Character
{
public:
	typedef Character Base;	

	void Init(const Vector2I& initPos, GameActor::Type enemyType);

	// Character interface
	virtual void InitStateMachine();

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void Update(GameTimeType deltaTime);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

private:
	struct EnemySharedStateData* mpSharedStateData;
	GameActor::Type mGameActor; //@NOTE: already exists in IAnimatedRenderable
	
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
