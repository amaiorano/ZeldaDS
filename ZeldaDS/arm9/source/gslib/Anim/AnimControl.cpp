#include "AnimControl.h"
#include "gslib/Core/Core.h"
#include "AnimAssetManager.h"

AnimControl::AnimControl()
{
	Reset();
}

void AnimControl::Reset()
{
	mpCurrAnimAsset = 0;
	mPlaybackRate = 1;
	mElapsedTime = 0;
	mCurrKeyFrameIndex = 0;
	mAnimCycled = false;
}

void AnimControl::PlayAnim(AnimAssetKey key, int playbackRate)
{
	Reset();
	mpCurrAnimAsset = AnimAssetManager::GetAnimAsset(key);
	mPlaybackRate = 1;
}

void AnimControl::SetFrameIndex(int frameIndex)
{
	ASSERT(frameIndex >= 0 && frameIndex < (int)mpCurrAnimAsset->mAnimTimeline.GetKeyFrames().size());
	mCurrKeyFrameIndex = frameIndex;
}

int AnimControl::GetFrameIndex() const
{
	return mCurrKeyFrameIndex;
}

void AnimControl::Update(AnimTimeType deltaTime)
{
	const AnimTimeline::KeyFrames& keyFrames = mpCurrAnimAsset->mAnimTimeline.GetKeyFrames();
	const AnimTimeType currFrameDuration = keyFrames[mCurrKeyFrameIndex].frameDuration;

	// Handle playback rate of 0
	if (currFrameDuration == 0.0f || mPlaybackRate == 0)
		return;

	mAnimCycled = false;

	// Track how much of the current frame has elapsed
	mElapsedTime += deltaTime * mPlaybackRate;

	if (mElapsedTime > currFrameDuration)
	{
		// If anim is non-looping, we hang on the last frame, returning true only ONCE
		const bool onLastFrame = (size_t)(mCurrKeyFrameIndex+1) == keyFrames.size();
		if (onLastFrame && !mpCurrAnimAsset->mAnimTimeline.IsLooping())
		{
			mPlaybackRate = 0; // Effectively hangs anim
			mAnimCycled = true;
			return;
		}

		// To keep things simple, we only advance one frame per Update() call
		mCurrKeyFrameIndex = (mCurrKeyFrameIndex + 1) % (int)keyFrames.size();
		mElapsedTime = (mElapsedTime - currFrameDuration); // Start with excess time for next frame
		mAnimCycled = (mCurrKeyFrameIndex == 0);
	}
}

bool AnimControl::HasAnimCycled() const
{
	return mAnimCycled;
}

const AnimPoseType& AnimControl::GetCurrPose() const
{
	const AnimPoseType& pose = mpCurrAnimAsset->mAnimPoses.GetPose( mpCurrAnimAsset->mAnimTimeline.GetKeyFrame(mCurrKeyFrameIndex).frameIndex );
	ASSERT_MSG(pose.pFrameGfx, "Retrieving invalid pose! Make sure to call PlayAnim() before GetCurrPose()");
	return pose;
}
