#ifndef ENEMY_FACTORY_H
#define ENEMY_FACTORY_H

#include "GameAnims.h"

class Enemy;

namespace EnemyFactory
{
	Enemy* CreateEnemy(GameActor::Type enemyType);
	Enemy* CreateRandomEnemy();
}

#endif // ENEMY_FACTORY_H
