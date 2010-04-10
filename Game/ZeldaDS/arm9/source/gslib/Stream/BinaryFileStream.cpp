#include "BinaryFileStream.h"
#include <stdio.h>
#include <string.h>

//@TODO: Prefetch doesn't work when reading more bytes than the prefetch buffer size
//#define ENABLE_PREFETCH

namespace
{
	template <typename T>
	void Swap(T& lhs, T& rhs)
	{
		T tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}
}

BinaryFileStream::BinaryFileStream()
: mpFile(0)
, mFileEndian(FileDefaultEndian)
, mSizeBytes(0)
, mAtEof(false)
, mPrefetchBuffer(0)
, mPrefetchCurr(0)
, mPrefetchEnd(0)
{
#if defined(ENABLE_PREFETCH)
	mPrefetchBuffer = new uint8[PREFETCH_BUFFER_SIZE];
#endif
}

BinaryFileStream::~BinaryFileStream()
{
#if defined(ENABLE_PREFETCH)
	delete [] mPrefetchBuffer;
#endif

	Close();
}

void BinaryFileStream::Open(const char* filePath, const char* mode, Endian::Type fileEndian)
{
	Close();

	mpFile = fopen(filePath, mode);
	ASSERT(mpFile);

	// Get size of file
	fseek(mpFile, 0, SEEK_END);
	mSizeBytes = ftell(mpFile);
	fseek(mpFile, 0, SEEK_SET);

	mFileEndian = fileEndian;
}

void BinaryFileStream::Close()
{
	if (mpFile)
	{
		fclose(mpFile);
		mpFile = 0;
	}
}

bool BinaryFileStream::AtEof() const
{
	return mAtEof;
}

size_t BinaryFileStream::ReadPrefetchedData(uint8* pBuffer, uint32 numBytes)
{
	size_t bytesRead = 0;

	const size_t numPrefetchedBytes = mPrefetchEnd - mPrefetchCurr;
	if (numBytes > numPrefetchedBytes)
	{
		// Copy what we've got left in the buffer, then fetch another chunk
		if (numPrefetchedBytes > 0)
		{
			memcpy(pBuffer, &mPrefetchBuffer[mPrefetchCurr], numPrefetchedBytes);
			numBytes -= numPrefetchedBytes;
			bytesRead += numPrefetchedBytes;
		}

		const size_t prefetchBytesRead = fread(mPrefetchBuffer, 1, PREFETCH_BUFFER_SIZE, mpFile);
		mPrefetchCurr = 0;
		mPrefetchEnd = prefetchBytesRead; // Might be 0
	}

	if (numBytes > 0)
	{
		// Copy from the prefetch buffer
		memcpy(pBuffer, &mPrefetchBuffer[mPrefetchCurr], numBytes);
		mPrefetchCurr += numBytes;
		bytesRead += numBytes;
		ASSERT(mPrefetchCurr <= mPrefetchEnd);
	}

	return bytesRead;
}

void BinaryFileStream::ReadBytes(uint8* pBuffer, uint32 numBytes, bool isIntegralType)
{
#if defined(ENABLE_PREFETCH)
	size_t bytesRead = ReadPrefetchedData(pBuffer, numBytes);
#else
	size_t bytesRead = fread(pBuffer, 1, numBytes, mpFile);
#endif
	(void)bytesRead;
	ASSERT(bytesRead == numBytes); // Hit EOF sooner than expected
	
	mAtEof = ftell(mpFile) == mSizeBytes;

	// Endian swap based on size of types
	if (isIntegralType && mFileEndian != PlatformEndian)
	{
		// Validate integral type by size
		ASSERT(numBytes == 1 || numBytes == 2 || numBytes == 4 || numBytes == 8);
		switch (numBytes)
		{
		case 1:
			break;

		case 2:
			Swap(pBuffer[0], pBuffer[1]);
			break;

		case 4:
			Swap(pBuffer[0], pBuffer[3]);
			Swap(pBuffer[1], pBuffer[2]);
			break;

		case 8:
			Swap(pBuffer[0], pBuffer[7]);
			Swap(pBuffer[1], pBuffer[6]);
			Swap(pBuffer[2], pBuffer[5]);
			Swap(pBuffer[3], pBuffer[4]);
			break;

		default:
			FAIL_MSG("Not an integral type!");
		}
	}
}
