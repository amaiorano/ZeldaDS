#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "GameObject.h"
#include "IDamageable.h"
#include "Health.h"
#include "gslib/Hsm/HsmStateMachine.h"

// Base class for game characters (players and enemies)
class Character : public GameObject, public IDamageable
{
public:
	typedef GameObject Base;

	virtual void Init(const Vector2I& initPos);
	virtual void InitStateMachine() = 0;

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

	// IDamageable interface
	virtual void OnDamage(const DamageInfo& damageInfo);

protected:
	StateMachine mStateMachine;
	struct CharacterSharedStateData* mpSharedStateData;
	Vector2I mInitPos;
	DamageInfo mDamageInfo;
	Health mHealth;
};

#endif // GAME_CHARACTER_H
