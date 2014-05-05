#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"
#include "ScrollingMgr.h"

class Boomerang;

class Player : public Character
{
public:
	typedef Character Base;	

	Player();

	// Character interface
	virtual void InitStateMachine();
	
	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo);

	// ISceneNode interface
	virtual void Update(GameTimeType deltaTime);
	virtual void Render(GameTimeType deltaTime);

private:
	friend struct PlayerStates;
	Vector2I mLastDamagePushVector;
	Boomerang* mpBoomerang;
	ScrollDir::Type mScrollDir;
};


#endif // PLAYER_H
