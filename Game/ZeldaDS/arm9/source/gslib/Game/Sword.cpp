#include "Sword.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Core/Rtti.h"
#include "GameItems.h"
#include "Enemy.h"

Sword::Sword()
	: Base(true)
	, mDirection(SpriteDir::None)
{
}

void Sword::Init(SpriteDir::Type dir)
{
	mDirection = dir;
}

void Sword::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	Base::GetGameObjectInfo(gameObjectInfo);
	gameObjectInfo.mGameActor = GameActor::Sword;
}

void Sword::OnAddToScene()
{
	Base::OnAddToScene();

	// No anim to play on the sword
	//GetAnimControl().PlayAnim( MakeAnimAssetKey(GameActor::Boomerang, BaseAnim::ItemDefault, SpriteDir::None));
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
		static DamageInfo dmgInfo;
		dmgInfo.mEffect = DamageEffect::Hurt;
		dmgInfo.mAmount = 10;
		dmgInfo.mPushVector = -collisionInfo.mPushVector;

		pEnemy->OnDamage(dmgInfo);
	}
}
