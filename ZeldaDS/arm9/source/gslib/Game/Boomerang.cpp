#include "Boomerang.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Game/WorldMap.h"
#include "gslib/Game/Player.h"
#include "gslib/Game/Enemy.h"
#include "gslib/Game/GameAnims.h"

const float BoomerangSpeed = 3.0f;

void Boomerang::Init(Player* pOwner, const Vector2I& launchDir)
{
	mpOwner = pOwner;

	mPositionF = pOwner->GetPosition();
	mSpeed = Normalized(launchDir) * BoomerangSpeed;
	
	mIsLeaving = true;
	mHasReturned = false;
}

void Boomerang::GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
{
	gameObjectInfo.mGameActor = GameActor::Boomerang;
	gameObjectInfo.mPhysicalSize.Reset(12, 12);  // Slightly smaller to bound actual boomerang
}

void Boomerang::OnAddToScene()
{
	Base::OnAddToScene();

	SetPosition(mPositionF + Vector2F(0.5f, 0.5f));
	mAnimControl.PlayAnim(MakeAnimAssetKey(GameActor::Boomerang, BaseAnim::ItemDefault, SpriteDir::None));
	mpSprite->SetAnimPose(mAnimControl.GetCurrPose());
}

void Boomerang::Update(GameTimeType deltaTime)
{
	const Vector2F charPos = mpOwner->GetPosition();

	mPositionF += mSpeed * deltaTime; // We keep everything in float vectors during motion

	if (mIsLeaving)
	{
		// Have we hit the edge of the screen yet?
		if ( !WorldMap::Instance().IsWorldBBoxInScreenBounds(GetBoundingBox()) )
		{
			mIsLeaving = false;
			mSpeed.Reset(0, 0); // Stop for one frame - we'll start returning to player next frame
		}
	}
	else
	{
		if ( (charPos - mPositionF).Length() < BoomerangSpeed )
		{
			mHasReturned = true;
		}

		mSpeed = (charPos - mPositionF).Normalize() * BoomerangSpeed;
	}

	IPhysical::Teleport(mPositionF + Vector2F(0.5f, 0.5f));

	Base::Update(deltaTime);
}

void Boomerang::OnCollision(const CollisionInfo& collisionInfo)
{
	if (Enemy* pEnemy = DynamicCast<Enemy*>(collisionInfo.mpCollidingWith))
	{		
		// Stun attack (might kill the enemy, depends on the enemy)
		static DamageInfo dmgInfo;
		dmgInfo.mEffect = DamageEffect::Stun;
		dmgInfo.mAmount = 1; // ???
		dmgInfo.mPushVector = -collisionInfo.mPushVector;

		pEnemy->OnDamage(dmgInfo);
	}

	// Go back to owner
	mIsLeaving = false;
}

