#ifndef ENEMY_H
#define ENEMY_H

#include "Character.h"
#include "gslib/Hsm/HsmStateMachine.h"

class SharedStateData;

class Enemy : public Character
{
public:
	typedef Character Base;	

	void Init(const Vector2I& initPos);

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

	// IPhysical interface
	virtual void OnCollision(const CollisionInfo& collisionInfo);

	// IDamageable interface
	virtual void OnDamage(const DamageInfo& damageInfo);

private:
	StateMachine mStateMachine;

	friend class EnemyStates;
	friend class GoriyasStates; // ugh

	Vector2I mInitPos;
	DamageInfo mDamageInfo;
};

#endif // ENEMY_H
