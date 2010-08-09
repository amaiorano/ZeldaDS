#include "IAnimatedRenderable.h"
#include "gslib/Hw/Sprite.h"

IAnimatedRenderable::IAnimatedRenderable()
	: mGameActor(GameActor::None)
	, mSpriteDir(SpriteDir::None)
	, mIsDirectional(true)
{
}

IAnimatedRenderable::~IAnimatedRenderable()
{
}

void IAnimatedRenderable::Activate(uint16 spriteRenderGroupId, uint16 width, uint16 height, GameActor::Type gameActor, bool isDirectional)
{
	IRenderable::Activate(spriteRenderGroupId, width, height);
	mGameActor = gameActor;
	mIsDirectional = isDirectional;
	mSpriteDir = SpriteDir::Down;
}

//void UpdateAnimationState(GameTimeType deltaTime)
void IAnimatedRenderable::AdvanceClockAndSetAnimPose(GameTimeType deltaTime)
{
	mAnimControl.Update(deltaTime);
	mpSprite->SetAnimPose(mAnimControl.GetCurrPose());
}

void IAnimatedRenderable::PlayAnim(BaseAnim::Type anim)
{
	ASSERT(mGameActor != GameActor::None);
	mAnimControl.PlayAnim( MakeAnimAssetKey(mGameActor, anim, mIsDirectional? mSpriteDir : SpriteDir::None) );
}

void IAnimatedRenderable::PlayGlobalAnim(BaseAnim::Type anim)
{
	// Check for actor-specific override
	AnimAssetKey key = MakeAnimAssetKey(mGameActor, anim, SpriteDir::None);
	if ( !AnimAssetManager::FindAnimAsset(key) )
	{
		key = MakeAnimAssetKey(GameActor::None, anim, SpriteDir::None);
	}

	mAnimControl.PlayAnim(key);
}

bool IAnimatedRenderable::IsAnimFinished()
{
	return mAnimControl.HasAnimCycled();
}
