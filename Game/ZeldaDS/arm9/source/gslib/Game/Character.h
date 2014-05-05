#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "GameObject.h"
#include "IDamageable.h"
#include "Health.h"
#include "hsm/StateMachine.h"

// Base class for game characters (players and enemies)
class Character : public GameObject, public IDamageable
{
public:
	typedef GameObject Base;

	Character();
	virtual void Init(const Vector2I& initPos);
		
protected:
	virtual void InitStateMachine() {}

	virtual void OnDead() { }

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

public:
	// IDamageable interface
	virtual void OnDamage(const DamageInfo& damageInfo);

protected:
	hsm::StateMachine mStateMachine;
	hsm::StateValue<bool> mSvCanTakeDamage; // Can we take damage?

	Vector2I mInitPos;
	DamageInfo mDamageInfo;
	Health mHealth;
};

#endif // GAME_CHARACTER_H
