#include "EnemyFactory.h"

#include "gslib/Game/Enemies/Goriyas.h"
#include "gslib/Game/Enemies/Snake.h"

namespace EnemyFactory
{
	Enemy* CreateEnemy(GameActor::Type enemyType)
	{
		switch (enemyType)
		{
		case GameActor::Goriyas:	return new Goriyas();
		case GameActor::Snake:		return new Snake();
		default: break;
		}

		FAIL();
		return NULL;
	}

	Enemy* CreateRandomEnemy()
	{
		const GameActor::Type enemyType = rand() % 2 == 0? GameActor::Goriyas : GameActor::Snake;
		return CreateEnemy(enemyType);
	}
}
