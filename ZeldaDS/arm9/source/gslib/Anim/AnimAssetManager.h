#ifndef ANIM_ASSET_MANAGER_H
#define ANIM_ASSET_MANAGER_H

#include "AnimClientTypes.h"
#include "AnimTimeline.h"

// Shareable animation asset (poses & key frames)
class AnimAsset
{
public:
	AnimPoses mAnimPoses;
	AnimTimeline mAnimTimeline;
};

// Stores (and owns) currently loaded animation assets
namespace AnimAssetManager
{
	void AddAnimAsset(AnimAssetKey key, AnimAsset* pAnimAsset);
	AnimAsset* FindAnimAsset(AnimAssetKey key);
	AnimAsset* GetAnimAsset(AnimAssetKey key);
};

#endif // ANIM_ASSET_MANAGER_H
