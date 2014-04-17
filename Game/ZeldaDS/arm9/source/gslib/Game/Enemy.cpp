#include "Enemy.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Core/Rtti.h"
#include "CharacterState.h"
#include "GameHelpers.h"
#include "Player.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "MovementModel.h"
#include "EnemyState.h"

Enemy::Enemy()
	: mpEnemyStateData(0)
{
}

void Enemy::InitStateMachine()
{
	Base::InitStateMachine();
	mpEnemyStateData = CheckedDynamicCast<EnemySharedStateData*>(mpSharedStateData);
	mStateMachine.Initialize<EnemyStates::Root>(this, "Enemy");	
}

SharedStateData* Enemy::CreateSharedStateData()
{
	return new EnemySharedStateData();
}

void Enemy::OnDead()
{
	SceneGraph::Instance().RemoveNodePostUpdate(this);
}

void Enemy::OnAddToScene()
{
	Base::OnAddToScene();

	//@TODO: This type of info will belong to the spawner eventually (perhaps passed into Init?)
	SetSpriteDir(GameHelpers::GetRandomSpriteDir());
}

void Enemy::Update(GameTimeType deltaTime)
{
	Base::Update(deltaTime);
	mLastFrameCollision.mIsSet = false;
}

void Enemy::OnCollision(const CollisionInfo& collisionInfo)
{
	mLastFrameCollision.mInfo = collisionInfo;
	mLastFrameCollision.mIsSet = true;

	if ( Player* pPlayer = DynamicCast<Player*>(collisionInfo.mpCollidingWith) )
	{
		if (mpEnemyStateData->mAttribCanDamage)
		{
			static DamageInfo dmgInfo; //@MT_UNSAFE
			dmgInfo.mAmount = 1;
			dmgInfo.mPushVector = -collisionInfo.mPushVector;
			pPlayer->OnDamage(dmgInfo);
		}
	}
}
