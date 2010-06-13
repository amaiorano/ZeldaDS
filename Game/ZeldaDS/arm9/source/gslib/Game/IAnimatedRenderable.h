#ifndef IANIMATED_RENDERABLE_H
#define IANIMATED_RENDERABLE_H

#include "IRenderable.h"
#include "gslib/Anim/AnimControl.h"
#include "gslib/Game/GameAnims.h"

class IAnimatedRenderable : public IRenderable
{
protected:
	IAnimatedRenderable();
	virtual ~IAnimatedRenderable();

	void Activate(uint16 width, uint16 height, GameActor::Type gameActor);

	//void UpdateAnimationState(GameTimeType deltaTime)
	void AdvanceClockAndSetAnimPose(GameTimeType deltaTime);

	// Should be called after AdvanceClockAndSetAnimPose()
	using IRenderable::UpdateSpritePosition;

public:
	void PlayAnim(BaseAnim::Type anim);
	void PlayGlobalAnim(BaseAnim::Type anim);
	bool IsAnimFinished();

	AnimControl& GetAnimControl() { return mAnimControl; }
	const AnimControl& GetAnimControl() const { return mAnimControl; }

	void SetSpriteDir(SpriteDir::Type spriteDir) { mSpriteDir = spriteDir; }
	SpriteDir::Type GetSpriteDir() const { return mSpriteDir; }

	void SetGameActor(GameActor::Type gameActor) { mGameActor = gameActor; }
	GameActor::Type GetGameActor() const { return mGameActor; }

private:
	AnimControl mAnimControl;
	GameActor::Type mGameActor;
	SpriteDir::Type mSpriteDir;
};

#endif // IANIMATED_RENDERABLE_H
