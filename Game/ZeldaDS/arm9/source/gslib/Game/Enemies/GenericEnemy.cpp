#include "GenericEnemy.h"
#include "gslib/Game/EnemyState.h"

struct GenericEnemyStates
{
	struct Main : EnemyState
	{
		virtual Transition& EvaluateTransitions(HsmTimeType deltaTime)
		{
			return InnerEntryTransition<Move>();
		}
	};

	struct Move : EnemySharedStates::RandomMovement<>
	{
		typedef EnemySharedStates::RandomMovement<> Base;
	};

}; // struct GenericEnemyStates

GenericEnemy::GenericEnemy(GameActor::Type gameActor)
	: mGameActor(gameActor)
{
}

void GenericEnemy::Init(const Vector2I& initPos)
{
	Base::Init(initPos);
	//mStateMachine.SetDebugLevel(1);
}

void GenericEnemy::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	printf("Spawning generic enemy: %d\n", mGameActor);
	gameObjectInfo.mGameActor = mGameActor;

	// Determine whether we're directional by seeing if we have a Move animation in any direction but Down
	{
		AnimAsset* pAnimAsset = AnimAssetManager::FindAnimAsset(MakeAnimAssetKey(mGameActor, BaseAnim::Move, SpriteDir::Left));
		gameObjectInfo.mIsDirectional = (pAnimAsset != NULL);
		printf("mIsDirectional = %d\n", gameObjectInfo.mIsDirectional);
	}

	// Determine our dimensions by looking at our Idle anim (not exactly the "right" way, but will do)
	{
		AnimAsset* pAnimAsset = AnimAssetManager::FindAnimAsset(MakeAnimAssetKey(mGameActor, BaseAnim::Idle, SpriteDir::Down));
		ASSERT_MSG(pAnimAsset, "Should have Idle anim");
		const uint16 idlePoseFrameSize = pAnimAsset->mAnimPoses.GetPose(0).frameSize;
		switch (idlePoseFrameSize)
		{
		case 16*16:
			gameObjectInfo.mSpriteSize.x = 16;
			gameObjectInfo.mSpriteSize.y = 16;
			break;

		case 32*32:
			gameObjectInfo.mSpriteSize.x = 32;
			gameObjectInfo.mSpriteSize.y = 32;
			break;

		default:
			FAIL_MSG("Unexpected Idle anim frame size");
		}
		gameObjectInfo.mPhysicalSize = gameObjectInfo.mSpriteSize;
	}
}

Transition& GenericEnemy::GetRootTransition()
{
	return InnerEntryTransition<GenericEnemyStates::Main>();
}
