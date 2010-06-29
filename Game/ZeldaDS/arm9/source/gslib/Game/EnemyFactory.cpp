#include "EnemyFactory.h"

#include "gslib/Game/Enemies/Goriya.h"
#include "gslib/Game/Enemies/Rope.h"

namespace EnemyFactory
{
	Enemy* CreateEnemy(GameActor::Type enemyType)
	{
		switch (enemyType)
		{
		case GameActor::Goriya:		return new Goriya();
		case GameActor::Rope:		return new Rope();
		default: break;
		}

		FAIL();
		return NULL;
	}

	Enemy* CreateRandomEnemy()
	{
		const GameActor::Type enemyType = rand() % 2 == 0? GameActor::Goriya : GameActor::Rope;
		return CreateEnemy(enemyType);
	}
}
