#include "ResourceMgr.h"
#include "gslib/Stream/BinaryFileStream.h"

ResourceMgr::ResourceMgr()
{
}

ResourceMgr::~ResourceMgr()
{
	FreeAllResourceBuffers();
}

void ResourceMgr::AllocateResourceBuffer(ResourceId resourceId, uint32 sizeBytes)
{
	ResourceBuffer& resourceBuffer = mResourceMap[resourceId];
	ASSERT(!resourceBuffer.IsAllocated());
	
	resourceBuffer.mpData = new uint8[sizeBytes];
	resourceBuffer.mBufferSizeBytes = sizeBytes;
	resourceBuffer.mDataSizeBytes = 0;
	
	ASSERT(resourceBuffer.IsAllocated());
}

void ResourceMgr::FreeAllResourceBuffers()
{
	for (uint16 i = 0; i < NUM_ARRAY_ELEMS(mResourceMap); ++i)
	{
		ResourceBuffer& resourceBuffer = mResourceMap[i];

		delete [] resourceBuffer.mpData;
		resourceBuffer.mBufferSizeBytes = 0;
		resourceBuffer.mDataSizeBytes = 0;
	}
}

const ResourceData& ResourceMgr::LoadResource(ResourceId resourceId, const char* filename)
{
	ResourceBuffer& resourceBuffer = mResourceMap[resourceId];
	//ASSERT(!resourceBuffer.IsLoaded());

	BinaryFileStream bfs;
	bfs.Open(filename, "r");

	// Do we want to allow loading resources that are smaller than the allocated block?
	ASSERT(bfs.SizeBytes() <= resourceBuffer.mBufferSizeBytes);

	bfs.ReadElems(resourceBuffer.mpData, bfs.SizeBytes());
	resourceBuffer.mDataSizeBytes = bfs.SizeBytes();

	ASSERT(resourceBuffer.IsLoaded());
	return resourceBuffer;
}

//void ResourceMgr::UnloadResource(ResourceId resourceId)
//{
//	ResourceBuffer& resourceBuffer = mResourceMap[resourceId];
//	ASSERT(resourceBuffer.IsLoaded());
//	resourceBuffer.mDataSizeBytes = 0;
//	ASSERT(!resourceBuffer.IsLoaded());
//}

const ResourceData& ResourceMgr::GetResource(ResourceId resourceId)
{
	ResourceBuffer& resourceBuffer = mResourceMap[resourceId];
	ASSERT(resourceBuffer.IsLoaded());
	return resourceBuffer;
}
