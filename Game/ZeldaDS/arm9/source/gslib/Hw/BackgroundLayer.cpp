#include "BackgroundLayer.h"
#include "gslib/Hw/Constants.h"
#include <nds/bios.h>
#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <string.h>

// Represent a 16bit tile map entry (for text and extended rotation backgrounds)
typedef union __TileMapEntry16 // We use a C-style declaration so we can make this type volatile
{
	struct
	{
		uint16 metaTileIndex : 10;
		uint16 hFlip : 1;
		uint16 vFlip : 1;
		uint16 palette : 4;

	} __attribute__ ((packed));

	uint16 value;
} volatile TileMapEntry16;

//CASSERT(sizeof(TileMapEntry16)==sizeof(uint16));

namespace
{
	PrintConsole* gpCurrConsole = 0;

	//@NOTE: Implementation-dependent! This function should be provided by libnds
	bool IsMainBg(int bgId) { return bgId < 4; }
	bool IsSubBg(int bgId) { return bgId >= 4; }

} // anonymous namespace


BackgroundLayer::BackgroundLayer()
	: mBgId(-1)
	, mpConsole(0)
	, mpTileMap(0)
	, mMetaTileSizeX(0)
	, mMetaTileSizeY(0)
	, mNumHwTilesPerMetaTileX(0)
	, mNumHwTilesPerMetaTileY(0)
{
}

BackgroundLayer::~BackgroundLayer()
{
	delete mpConsole;
}

void BackgroundLayer::InitTiled(int bgId, uint8 metaTileSizeX, uint8 metaTileSizeY)
{
	ASSERT(mBgId == -1);
	mBgId = bgId;

	mpTileMap = (uint8*)bgGetMapPtr(mBgId);
	mMetaTileSizeX = metaTileSizeX;
	mMetaTileSizeY = metaTileSizeY;

	mNumHwTilesPerMetaTileX = (mMetaTileSizeX / HwTileSizeX);
	mNumHwTilesPerMetaTileY = (mMetaTileSizeY / HwTileSizeY);

	// Fill tile map with 0 (the default tile)
	const uint16 tileIndexSizeBytes = 2;
	memset(mpTileMap, 0, (HwBgNumTilesX * HwBgNumTilesY * tileIndexSizeBytes)); //@TODO: use dmaCopy?
}

void BackgroundLayer::LoadTilesImage(const void* pImage, uint16 sizeBytes)
{
	memcpy(bgGetGfxPtr(mBgId), pImage, sizeBytes); //@TODO: use dmaCopy?
}

static void DrawHwTile(TileMapEntry16* pTileMap, uint16 hwTileMapX, uint16 hwTileMapY, uint16 hwTileIndex)
{
	pTileMap[hwTileMapY * HwBgNumTilesX + hwTileMapX].metaTileIndex = hwTileIndex;
}

static void DrawHwTileTextBg(TileMapEntry16* pTileMap, uint16 hwTileMapX, uint16 hwTileMapY, uint16 hwTileIndex)
{
	const uint16 quadrant = (hwTileMapY / HwTextBgQuadrantNumTilesY) * HwTextBgNumQuadrantsX + (hwTileMapX / HwTextBgQuadrantNumTilesX);
	const uint16 offset = quadrant * (HwTextBgQuadrantNumTilesX * HwTextBgQuadrantNumTilesY);

	pTileMap[hwTileMapY * HwTextBgQuadrantNumTilesX + hwTileMapX + offset].metaTileIndex = hwTileIndex;
}

// All coordinates are in meta-tile space
void BackgroundLayer::DrawTile(uint16 metaTileIndex, uint16 metaTileMapX, uint16 metaTileMapY)
{
	TileMapEntry16* pTileMap = reinterpret_cast<TileMapEntry16*>(mpTileMap); //@TODO: Support 2 types of indices?

	const uint16 hwTileMapX = metaTileMapX * mNumHwTilesPerMetaTileX;
	const uint16 hwTileMapY = metaTileMapY * mNumHwTilesPerMetaTileY;

	uint16 currHwTileIndex = metaTileIndex * (mNumHwTilesPerMetaTileX * mNumHwTilesPerMetaTileY);

	for (uint16 y = 0; y < mNumHwTilesPerMetaTileY; ++y)
	{
		for (uint16 x = 0; x < mNumHwTilesPerMetaTileX; ++x)
		{
			//@TODO: Determine whether text mode at Init and store func ptr
			//@TODO: Optimize this code! Super expensive, especially textmode call
			//if (mBgId < 2)
			if (bgIsText(mBgId))
			{
				DrawHwTileTextBg(pTileMap, hwTileMapX + x, hwTileMapY + y, currHwTileIndex);
			}
			else
			{
				DrawHwTile(pTileMap, hwTileMapX + x, hwTileMapY + y, currHwTileIndex);
			}
			++currHwTileIndex;
		}
	}
}

void BackgroundLayer::InitConsole(PrintConsole* pConsole)
{
	ASSERT(mBgId == -1);
	ASSERT(pConsole);
	mpConsole = pConsole;
	InitTiled(mpConsole->bgId, 8, 8);
}

bool BackgroundLayer::IsTextLayer() const
{
	return mpConsole != 0;
}

void BackgroundLayer::ActivateTextLayer()
{
	ASSERT(IsTextLayer());
	gpCurrConsole = mpConsole;
	consoleSelect(gpCurrConsole);
}

void BackgroundLayer::ClearText()
{
	ASSERT(IsTextLayer());
	// Unfortunately, there is no consoleClear() that takes in a PrintConsole*, so we have to
	// cache the current one ourselves (gpCurrConsole), and make sure to switch back to it when
	// we're done activating and clearing this one.
	consoleSelect(mpConsole);
	consoleClear();
	consoleSelect(gpCurrConsole);
}

void BackgroundLayer::Init3d(int bgId)
{
	ASSERT(mBgId == -1);
	mBgId = bgId;
	// Nothing to do for now...
}

void BackgroundLayer::SetEnabled(bool enabled)
{
	ASSERT(mBgId != -1);
	enabled? bgShow(mBgId) : bgHide(mBgId);
}

void BackgroundLayer::SetPriority(int priority)
{
	ASSERT(mBgId != -1);
	ASSERT(priority >= 0 && priority <= 3);
	bgSetPriority(mBgId, priority);
}

void BackgroundLayer::SetScroll(int x, int y)
{
	bgSetScroll(mBgId, x, y);
}
