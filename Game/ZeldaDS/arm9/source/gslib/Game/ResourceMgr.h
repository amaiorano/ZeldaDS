#ifndef GS_RESOURCE_MGR_H
#define GS_RESOURCE_MGR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"

class ResourceData
{
public:
	ResourceData() : mpData(0), mDataSizeBytes(0) { }

	const uint8* Data() const { return mpData; }

	template <typename ResourcePtrType>
	const ResourcePtrType DataAs() const { return (const ResourcePtrType)mpData; }

	long SizeBytes() const { return mDataSizeBytes; }

private:
	friend class ResourceMgr;
	const uint8* mpData;
	long mDataSizeBytes;
};

class ResourceMgr : public Singleton<ResourceMgr>
{
private:
	friend class Singleton<ResourceMgr>;
	ResourceMgr();

public:
	typedef uint16 ResourceId;
	static const int MaxNumResources = 10;

	~ResourceMgr();

	void AllocateResourceBuffer(ResourceId resourceId, uint32 sizeBytes);

	const ResourceData& LoadResource(ResourceId resourceId, const char* filename);
	//void UnloadResource(ResourceId resourceId);

	const ResourceData& GetResource(ResourceId resourceId);

private:
	struct ResourceBuffer : ResourceData
	{
		ResourceBuffer() : mBufferSizeBytes(0) { }
		bool IsAllocated() const { return mpData != 0; }
		bool IsLoaded() const { return mDataSizeBytes > 0; }

		long mBufferSizeBytes;		
	};

	ResourceBuffer mResourceMap[MaxNumResources];
};

#endif // GS_RESOURCE_MGR_H
