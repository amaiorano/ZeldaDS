#include "AnimAssetManager.h"
#include "gslib/Core/Core.h"
#include <map>

namespace
{
	typedef std::map<AnimAssetKey, AnimAsset*> AnimAssetMap;
	AnimAssetMap gAnimAssetMap;
}

namespace AnimAssetManager
{
	void AddAnimAsset(AnimAssetKey key, AnimAsset* pAnimAsset)
	{
		ASSERT_FORMATTED(!FindAnimAsset(key), ("AnimAsset with this key already added: %d", key));
		gAnimAssetMap[key] = pAnimAsset;
	}

	AnimAsset* FindAnimAsset(AnimAssetKey key)
	{
		AnimAssetMap::iterator iter = gAnimAssetMap.find(key);
		return (iter == gAnimAssetMap.end()? 0 : iter->second);
	}

	AnimAsset* GetAnimAsset(AnimAssetKey key)
	{
		AnimAsset* pAnimAsset = FindAnimAsset(key);
		ASSERT(pAnimAsset);
		return pAnimAsset;
	}
}
