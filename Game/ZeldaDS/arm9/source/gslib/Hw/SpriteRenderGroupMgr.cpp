#include "SpriteRenderGroupMgr.h"
#include <algorithm>

SpriteRenderGroupMgr::SpriteRenderGroupMgr()
{
}

bool SpriteRenderGroupMgr::IsInitialized() const
{
	return mGroupIdToData.size() > 0;
}

void SpriteRenderGroupMgr::Init(const SpriteRenderGroup* pGroups, uint16 numGroups)
{
	ASSERT(pGroups != 0 && numGroups > 0);

	mGroupIdToData.clear();

	uint16 firstGroupId = 0;
	for (uint16 i = 0; i < numGroups; ++i)
	{
		ASSERT_MSG(pGroups[i].mGroupId == i, "GroupId should be 0-based and sequential");

		GroupData& groupData = mGroupIdToData[i];
		groupData.mFirstId = firstGroupId;
		groupData.mAllocatedIds.resize(pGroups[i].mNumSprites, false);

		firstGroupId += pGroups[i].mNumSprites;
	}

	// firstGroupId is also the total number of sprites at this point
	ASSERT_MSG(firstGroupId <= MaxNumSpriteIds, "SpriteRenderGroups must divide up max 128 Sprite Ids");
}

uint16 SpriteRenderGroupMgr::AllocSpriteId(uint16 groupId)
{
	ASSERT_MSG(mGroupIdToData.size() > 0, "Must Init SpriteRenderGroupMgr");
	ASSERT_MSG(mGroupIdToData.find(groupId) != mGroupIdToData.end(), "Invalid Group Id");

	GroupData& groupData = mGroupIdToData[groupId];
	GroupData::AllocatedIdList& allocatedIds = groupData.mAllocatedIds;
	
	GroupData::AllocatedIdList::iterator iter = std::find(allocatedIds.begin(), allocatedIds.end(), false);
	ASSERT_MSG(iter != allocatedIds.end(), "No more Sprite Ids available to allocate in this group");
	
	*iter = true; // Set to allocated

	// Compute and return unique sprite id
	const uint16 offset = (iter - allocatedIds.begin());
	const uint16 spriteId = groupData.mFirstId + offset;
	//printf("Allocated Sprite -- group id: %d, sprite id: %d (offset: %d)\n", groupId, spriteId, offset);
	return spriteId;
}

void SpriteRenderGroupMgr::FreeSpriteId(uint16 spriteId)
{
	const uint32 groupId = GetGroupIdFromSpriteId(spriteId);
	ASSERT_MSG(mGroupIdToData.find(groupId) != mGroupIdToData.end(), "Invalid Group Id");

	GroupData& groupData = mGroupIdToData[groupId];
	GroupData::AllocatedIdList& allocatedIds = groupData.mAllocatedIds;

	const uint16 offset = spriteId - groupData.mFirstId;
	ASSERT(allocatedIds[offset]);
	allocatedIds[offset] = false;
	//printf("Freed Sprite -- group id: %d, sprite id: %d (offset: %d)\n", groupId, spriteId, offset);
}

uint16 SpriteRenderGroupMgr::GetGroupIdFromSpriteId(uint16 spriteId)
{
	GroupIdToDataMap::iterator iter = mGroupIdToData.begin();
	for ( ; iter != mGroupIdToData.end(); ++iter)
	{
		const uint32 firstSpriteId = iter->second.mFirstId;
		const uint32 lastSpriteId = firstSpriteId + iter->second.mAllocatedIds.size() - 1;

		if (spriteId >= firstSpriteId && spriteId <= lastSpriteId)
			return iter->first;
	}
	
	FAIL_MSG("Sprite Id not in any Group!");
	return ~0;
}
