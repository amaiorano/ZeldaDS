#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"
#include "gslib/Hsm/HsmStateMachine.h"

class SharedStateData;

class Player : public Character
{
public:
	typedef Character Base;	

	void Init(const Vector2I& initPos);
	
	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);
	virtual void Render(GameTimeType deltaTime);

	// IDamageable interface
	virtual void OnDamage(const DamageInfo& damageInfo);

private:
	StateMachine mStateMachine;
	SharedStateData* mpSharedStateData; // Alias for convenience

	friend class PlayerStates;
	Vector2I mInitPos;
	DamageInfo mDamageInfo;
	bool mInvincible;
	uint16 mInvincibleElapsedTime;
};


#endif // PLAYER_H
