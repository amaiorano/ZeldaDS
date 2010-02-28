#include "IAnimatedRenderable.h"
#include "gslib/Hw/Sprite.h"

IAnimatedRenderable::IAnimatedRenderable()
	: mGameActor(GameActor::None)
	, mSpriteDir(SpriteDir::None)
{
}

IAnimatedRenderable::~IAnimatedRenderable()
{
}

void IAnimatedRenderable::Activate(uint16 width, uint16 height, GameActor::Type gameActor)
{
	IRenderable::Activate(width, height);
	mGameActor = gameActor;
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
	mAnimControl.PlayAnim( MakeAnimAssetKey(mGameActor, anim, mSpriteDir) );
}

void IAnimatedRenderable::PlayGlobalAnim(BaseAnim::Type anim)
{
	mAnimControl.PlayAnim( MakeAnimAssetKey(GameActor::None, anim, SpriteDir::None) );
}

bool IAnimatedRenderable::IsAnimFinished()
{
	return mAnimControl.HasAnimCycled();
}
