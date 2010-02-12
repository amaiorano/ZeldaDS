#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "GameObject.h"
#include "IDamageable.h"

//@TODO: Move up shared stuff from Player/Enemy classes here

// Base class for game characters (players and enemies)
class Character : public GameObject, public IDamageable
{
public:
	typedef GameObject Base;
};

#endif // GAME_CHARACTER_H
