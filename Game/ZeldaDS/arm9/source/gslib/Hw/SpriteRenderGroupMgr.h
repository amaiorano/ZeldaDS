#ifndef SPRITE_RENDER_GROUP_MGR_H
#define SPRITE_RENDER_GROUP_MGR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include <vector>
#include <map>

// Sprite Render Groups are used to divide the limited number of sprite ids into groups,
// where groups with lower values render above groups with larger values.
// On the Nintedo DS, you can allocate up to 128 sprites per screen, so make sure the total
// number of sprites in all groups does not exceed 128.

const int MaxNumSpriteIds = 128;

struct SpriteRenderGroup
{
	// Unique group id from a 0-based sequential list (usually an enum value) where smaller values render above larger values
	uint16 mGroupId;

	// Number of sprites in the render group
	uint16 mNumSprites;
};

class SpriteRenderGroupMgr : public Singleton<SpriteRenderGroupMgr>
{
	friend class Singleton<SpriteRenderGroupMgr>;
	SpriteRenderGroupMgr();

public:
	void Init(const SpriteRenderGroup* pGroups, uint16 numGroups);
	bool IsInitialized() const;

	uint16 AllocSpriteId(uint16 groupId);
	void FreeSpriteId(uint16 spriteId);

	uint16 GetGroupIdFromSpriteId(uint16 spriteId);

private:
	struct GroupData
	{
		uint16 mFirstId; // Effectively the "global" index of this group in the master meta-array
		typedef std::vector<bool> AllocatedIdList;
		AllocatedIdList mAllocatedIds;
	};
	typedef std::map<uint16, GroupData> GroupIdToDataMap;
	GroupIdToDataMap mGroupIdToData;
};

#endif // SPRITE_RENDER_GROUP_MGR_H
