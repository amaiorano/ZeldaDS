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
	: mSvCanDealDamage(false)
{
}

void Enemy::InitStateMachine()
{
	Base::InitStateMachine();
	mStateMachine.Initialize<EnemyStates::Root>(this, "Enemy");
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
		if (mSvCanDealDamage)
		{
			static DamageInfo dmgInfo; //@MT_UNSAFE
			dmgInfo.mAmount = 1;
			dmgInfo.mPushVector = -collisionInfo.mPushVector;
			pPlayer->OnDamage(dmgInfo);
		}
	}
}
