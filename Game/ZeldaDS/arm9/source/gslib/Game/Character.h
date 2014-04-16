#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "GameObject.h"
#include "IDamageable.h"
#include "Health.h"
#include "gslib/Util/HsmUtil.h"

// Base class for game characters (players and enemies)
class Character : public GameObject, public IDamageable, public IHsmOwner
{
public:
	typedef GameObject Base;

	Character();
	virtual void Init(const Vector2I& initPos);
		
protected:
	virtual void InitStateMachine();
	virtual SharedStateData* CreateSharedStateData();

	virtual void OnDead() { }

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

public:
	// IDamageable interface
	virtual void OnDamage(const DamageInfo& damageInfo);

protected:
	hsm::StateMachine mStateMachine;
	struct CharacterSharedStateData* mpCharacterStateData;
	Vector2I mInitPos;
	DamageInfo mDamageInfo;
	Health mHealth;
};

#endif // GAME_CHARACTER_H
