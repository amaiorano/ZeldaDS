#ifndef GS_GAME_RESOURCES_H
#define GS_GAME_RESOURCES_H

#include "ResourceMgr.h"

namespace GameResource
{
	enum Type
	{
		//Pal_Sprites,
		//Pal_Tiles,
		Temporary, // Temporary buffer for loading resources that get copied to vram, etc.
		Gfx_Characters16x16,
		Gfx_Characters32x32,
		Gfx_Items,
		//Gfx_BgTiles,
		//Gfx_FgTiles,

		NumTypes
	};

	const int BytesPerTile16x16 = (16*16);
	const int BytesPerTile32x32 = (32*32);

	const long SizeBytesPerType[] =
	{
		//512,
		//512,
		16 * 8 * BytesPerTile16x16, // Temporary: large enough to hold largest temp resource (tiles)
		14 * 13 * BytesPerTile16x16,
		4 * 2 * BytesPerTile32x32,
		4 * 12 * BytesPerTile16x16,
		//16 * 8 * BytesPerTile16x16,
		//16 * 8 * BytesPerTile16x16
	};
	CT_ASSERT(NUM_ARRAY_ELEMS(SizeBytesPerType) == NumTypes);
}

#endif // GS_GAME_RESOURCES_H
