#ifndef ANIM_TIMELINE_H
#define ANIM_TIMELINE_H

#include <vector>
#include "AnimClientTypes.h"

namespace AnimCycle
{
	enum Type
	{
		PingPong,	// 0,1,2,1,0,1,2,...
		Loop,		// 0,1,2,0,1,2,0,...
		Once,		// 0,1,2,2,2,2,2,...
	};
}

// Collection of key frames
class AnimTimeline
{
public:
	struct KeyFrame
	{
		KeyFrame(int in_frameIndex, AnimTimeType in_frameDuration) : frameIndex(in_frameIndex), frameDuration(in_frameDuration)
		{
		}

		int frameIndex;
		AnimTimeType frameDuration;
	};

	typedef std::vector<KeyFrame> KeyFrames;


	AnimTimeline();
	void Reset();

	// Helper to automatically add frames for a simple animation.
	void Populate(int firstFrameIndex, int numFrames, AnimTimeType unitsPerFrame, AnimCycle::Type animCycle);

	// Can be used to manually populate timeline
	void AddKeyFrame(const AnimTimeline::KeyFrame& keyFrame);
	void AddKeyFrame(int frameIndex, AnimTimeType frameDuration);

	// Sets whether anim is looping (default is false)
	void SetLooping(bool looping)		{ mLooping = looping; }
	bool IsLooping() const				{ return mLooping; }

	KeyFrames& GetKeyFrames()			{ return mKeyFrames; }
	KeyFrame& GetKeyFrame(int index)	{ return mKeyFrames[index]; }

private:
	KeyFrames mKeyFrames;
	bool mLooping;
};


#endif // ANIM_TIMELINE_H
