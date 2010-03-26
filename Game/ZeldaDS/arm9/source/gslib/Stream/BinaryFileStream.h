#ifndef GS_BINARYFILESTREAM_H
#define GS_BINARYFILESTREAM_H

#include "gslib/Core/Core.h"

namespace Endian
{
	enum Type { Little, Big };
}

#if defined (PLATFORM_NDS)
	const Endian::Type PlatformEndian = Endian::Little;
#else
	#error "PlatformEndian not defined for current platform"
#endif

// Game-specific: default endianness of binary files
const Endian::Type FileDefaultEndian = Endian::Little;

// Stream for reading/writing binary files that can handle endian differences
// between file and platform
class BinaryFileStream
{
public:
	BinaryFileStream();
	~BinaryFileStream();

	void Open(const char* filePath, const char* mode, Endian::Type fileEndian = FileDefaultEndian);
	void Close();

	bool AtEof() const;

	// Reads and returns sizeof(T) integral, endian swaps if file and platform endianness don't match
	template <typename T>
	T ReadInt()
	{
		T value;
		ReadBytes(reinterpret_cast<uint8*>(&value), sizeof(T), true);
		return value;
	}

	// Reads sizeof(T) * numElems bytes into buffer, doesn't endian swap
	template <typename T>
	void ReadBytes(T& buffer, int numElems = 1)
	{
		ReadBytes(reinterpret_cast<uint8*>(&buffer), sizeof(T) * numElems, false);
	}

private:
	void ReadBytes(uint8* pBuffer, uint32 numBytes, bool isIntegralType);

	FILE* mpFile;
	Endian::Type mFileEndian;
	long mSizeBytes;
	bool mAtEof;
};


#endif // GS_BINARYFILESTREAM_H
