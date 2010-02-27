#ifndef ANIM_CONTROL_H
#define ANIM_CONTROL_H

#include "AnimClientTypes.h"
class AnimAsset;

// Used to play animations
class AnimControl
{
public:
	AnimControl();

	void PlayAnim(AnimAssetKey key, int playbackRate = 1);
	void PlayAnim(AnimAsset* pCurrAnimAsset, int playbackRate = 1);
	
	void SetFrameIndex(int frameIndex);
	int GetFrameIndex() const; // Don't confuse this with GetCurrPoseIndex()

	void Update(AnimTimeType deltaTime);

	// Returns true for exactly one frame when looping anim cycles or non-looping anim ends
	bool HasAnimCycled() const;

	const AnimPoseType& GetCurrPose() const;
	const int GetCurrPoseIndex() const;

private:
	void Reset();

	AnimAsset* mpCurrAnimAsset;
	int mPlaybackRate;
	AnimTimeType mElapsedTime;
	int mCurrKeyFrameIndex;
	bool mAnimCycled;
};


#endif // ANIM_CONTROL_H
