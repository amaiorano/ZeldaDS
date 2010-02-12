#include "AnimTimeline.h"

AnimTimeline::AnimTimeline()
{
	Reset();
}

void AnimTimeline::Reset()
{
	mKeyFrames.clear();
	mLooping = false;
}

void AnimTimeline::Populate(int firstFrameIndex, int numFrames, AnimTimeType unitsPerFrame, AnimCycle::Type animCycle)
{
	Reset();

	int currFrameIndex = firstFrameIndex;

	for (int i=0; i<numFrames; ++i, ++currFrameIndex)
	{
		AddKeyFrame( AnimTimeline::KeyFrame(currFrameIndex, unitsPerFrame) );
	}

	// For ping-pong, add n-1 frames backwards
	if (animCycle == AnimCycle::PingPong)
	{
		currFrameIndex -= 2;
		for (int i=0; i<numFrames-2; ++i, --currFrameIndex)		
		{
			AddKeyFrame( AnimTimeline::KeyFrame(currFrameIndex, unitsPerFrame) );
		}
	}

	SetLooping( animCycle != AnimCycle::Once );
}

void AnimTimeline::AddKeyFrame(const AnimTimeline::KeyFrame& keyFrame)
{
	mKeyFrames.push_back(keyFrame);
}

void AnimTimeline::AddKeyFrame(int frameIndex, AnimTimeType frameDuration)
{
	return AddKeyFrame(KeyFrame(frameIndex, frameDuration));
}
