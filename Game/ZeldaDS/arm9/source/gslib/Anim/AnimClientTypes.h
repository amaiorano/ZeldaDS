#ifndef ANIM_CLIENT_TYPES_H
#define ANIM_CLIENT_TYPES_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Types.h"

// Required types
typedef uint16 AnimTimeType; // Elapsed frames
typedef uint32 AnimAssetKey;

//typedef uint8* AnimPoseType;

struct AnimPoseType
{
	const uint8* pFrameGfx;
	uint16 frameSize;
};

// Represents collection of poses
class AnimPoses // Required class
{
public:
	void Init(const uint8* pFrameGfx, uint16 frameSize)
	{
		mpFrameGfx = pFrameGfx;
		mCurrPose.frameSize = frameSize;
	}

	const AnimPoseType& GetPose(int frameIndex) const // Required member function
	{
		mCurrPose.pFrameGfx = mpFrameGfx + (mCurrPose.frameSize * frameIndex);
		return mCurrPose;
	}

private:
	const uint8* mpFrameGfx; // Pointer to start of sprite frames in ram
	mutable AnimPoseType mCurrPose; // We don't actually store all Poses, we build the latest as needed
};

#endif // ANIM_CLIENT_TYPES_H
