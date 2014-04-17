#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"

class SharedStateData;

class Player : public Character
{
public:
	typedef Character Base;	

	Player();

	// Character interface
	virtual void InitStateMachine();
	virtual SharedStateData* CreateSharedStateData();
	
	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void Update(GameTimeType deltaTime);
	virtual void Render(GameTimeType deltaTime);

private:
	struct PlayerSharedStateData* mpPlayerStateData;

	friend class PlayerStates;
	Vector2I mLastDamagePushVector;
};


#endif // PLAYER_H
