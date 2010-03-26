#include "BinaryFileStream.h"
#include <stdio.h>

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
, mAtEof(0)
{
}

BinaryFileStream::~BinaryFileStream()
{
	Close();
}

void BinaryFileStream::Open(const char* filePath, const char* mode, Endian::Type fileEndian)
{
	if (mpFile)
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

void BinaryFileStream::ReadBytes(uint8* pBuffer, uint32 numBytes, bool isIntegralType)
{
	size_t bytesRead = fread(pBuffer, 1, numBytes, mpFile);
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
