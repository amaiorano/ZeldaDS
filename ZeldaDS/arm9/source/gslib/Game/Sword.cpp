#include "Sword.h"
#include "gslib/Hw/Sprite.h"
#include "GameItems.h"
#include "Enemy.h"

void Sword::Init(SpriteDir::Type dir)
{
	mDirection = dir;
}

void Sword::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	gameObjectInfo.mGameActor = GameActor::Sword;
}

void Sword::OnAddToScene()
{
	Base::OnAddToScene();

	// No anim to play on the sword
	//mAnimControl.PlayAnim( MakeAnimAssetKey(GameActor::Boomerang, BaseAnim::ItemDefault, SpriteDir::None));
	mpSprite->SetAnimPose(GetGameItemPose(GameItem::Sword, mDirection));
}

void Sword::Update(GameTimeType deltaTime)
{
	//@HACK: Don't call base class Update() because we don't want it to update animation
	// as we don't set one! Should probably set a flag on GameObject like mPlayAnim or something.
	//Base::Update(deltaTime);
}

void Sword::OnCollision(const CollisionInfo& collisionInfo)
{
	if (Enemy* pEnemy = DynamicCast<Enemy*>(collisionInfo.mpCollidingWith))
	{
		// Stun attack (might kill the enemy, depends on the enemy)
		static DamageInfo dmgInfo;
		dmgInfo.mAmount = 10;
		dmgInfo.mPushVector = -collisionInfo.mPushVector;

		pEnemy->OnDamage(dmgInfo);
	}
}
