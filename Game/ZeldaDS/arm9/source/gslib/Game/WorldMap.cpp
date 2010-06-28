#include "WorldMap.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Physics/BoundingBox.h"
#include "gslib/Anim/AnimAssetManager.h"
#include "gslib/Anim/AnimControl.h"
#include "gslib/Stream/BinaryFileStream.h"
#include "ScrollingMgr.h"
#include <cstdlib>
#include <limits>
#include <string.h>

namespace
{
	void RotateColors(std::vector<uint16>& paletteIndices)
	{
		uint16* pBgPalette = GraphicsEngine::GetBgPalette();

		uint16 tmp = pBgPalette[paletteIndices[0]];
		std::size_t i = 1;

		for ( ; i < paletteIndices.size(); ++i)
		{
			pBgPalette[paletteIndices[i-1]] = pBgPalette[paletteIndices[i]];
		}
		pBgPalette[paletteIndices[i-1]] = tmp;
	}
} // anonymous namespace


void TileSet::Init()
{
	for (uint16 i = 0; i < NUM_ARRAY_ELEMS(mTileData); ++i)
	{
		mTileData[i].Reset();
	}
}

void TileLayer::Init(uint16 numTilesX, uint16 numTilesY)
{
	mTileSet.Init();
	mTileMap.Reset(numTilesX, numTilesY);
}



WorldMap::WorldMap()
	: mNumScreensX(0)
	, mNumScreensY(0)
	, mNumTilesX(0)
	, mNumTilesY(0)
{
	// Would have preferred a compile-time assert, but max() is a function
	ASSERT_MSG(TileSet::MaxNumTiles < std::numeric_limits<TileIndexType>::max(), "TileIndexType not large enough to index tiles in TileSet");
}

void WorldMap::Init(uint16 numScreensX, uint16 numScreensY)
{
	mNumScreensX = numScreensX;
	mNumScreensY = numScreensY;
	mNumTilesX = mNumScreensX * GameNumScreenMetaTilesX;
	mNumTilesY = mNumScreensY * GameNumScreenMetaTilesY;

	for (uint16 layer=0; layer < NumLayers; ++layer)
	{
		mTileLayers[layer].Init(mNumTilesX, mNumTilesY);
	}

	mDataLayer.Reset(mNumTilesX, mNumTilesY);
}

void WorldMap::Shutdown()
{
	ASSERT(mAnimAssets.size() == mAnimControls.size());
	for (std::size_t i = 0; i < mAnimAssets.size(); ++i)
	{
		delete mAnimAssets[i];
		delete mAnimControls[i];
	}
	mAnimAssets.clear();
	mAnimControls.clear();
}

void WorldMap::LoadMap(const char* mapFile)
{
	//@TODO: Optimize map size by:
	// 1) Using uint8 to store tile indices
	// 2) RLE-compressing data

	BinaryFileStream bfs;
	bfs.Open(mapFile, "r");

	const uint16 numLayers = bfs.ReadInt<uint16>();	
	ASSERT(numLayers == NumLayers + 1); // Tile layers + collision layer
	
	for (uint16 layer=0; layer < numLayers; ++layer)
	{
		const uint16 numTilesX = bfs.ReadInt<uint16>();
		const uint16 numTilesY = bfs.ReadInt<uint16>();
		const int numElemsToRead = numTilesX * numTilesY;
		ASSERT(numTilesX == mNumTilesX);
		ASSERT(numTilesY == mNumTilesY);
		
		if (layer < 2)
		{
			ASSERT(numElemsToRead == GetTileMapLayer(layer).Size());
			bfs.ReadElems<uint16>(GetTileMapLayer(layer).RawPtr(), numElemsToRead);
		}
		else // Third layer is the data layer
		{
			ASSERT(numElemsToRead == mDataLayer.Size());
			bfs.ReadElems<DataLayerEntry>(mDataLayer.RawPtr(), numElemsToRead);
		}

		uint16 marker = bfs.ReadInt<uint16>();
		(void)marker;
		ASSERT(marker == 0xFFFF);
	}

	//@TODO: Read anim tile info from file
	{
		const int TilesPerRow = 16;

		// Ocean shore tiles
		uint16 sharedClockIndex = AddAnimTileSharedClock(4, 45, AnimCycle::Loop);
		for (int i = 0; i < 12; ++i)
		{
			EnableAnimTile(0, 5 * TilesPerRow + (i * 4), sharedClockIndex);
		}

		// Waterfall
		sharedClockIndex = AddAnimTileSharedClock(4, 10, AnimCycle::Loop);
		EnableAnimTile(0, 2 * TilesPerRow + 9, sharedClockIndex);
	}

	//@TODO: Read color rotation info from file
	{
		uint16 paletteIndices[] = {10, 11, 13, 14};
		EnableColorRotation(paletteIndices, NUM_ARRAY_ELEMS(paletteIndices), SEC_TO_FRAMES(0.2f));
	}

	//@TODO: Read enemy spawn info from file (packed with collision data)
	mDataLayer(1, 1).SpawnActorType = GameActor::Rope;
	mDataLayer(6, 3).SpawnActorType = GameActor::Goriya;
	mDataLayer(12, 5).SpawnActorType = GameActor::Rope;

	mDataLayer(GameNumScreenMetaTilesX+6, 4).SpawnActorType = GameActor::Rope;
	mDataLayer(GameNumScreenMetaTilesX+13, 8).SpawnActorType = GameActor::Goriya;
	mDataLayer(GameNumScreenMetaTilesX+5, 10).SpawnActorType = GameActor::Goriya;
}

uint16 WorldMap::AddAnimTileSharedClock(int numFrames, AnimTimeType unitsPerFrame, AnimCycle::Type animCycle)
{
	//@TODO: Create and use a simplified AnimControl that internally stores an AnimTimeline
	AnimAsset* pAnimAsset = new AnimAsset;
	pAnimAsset->mAnimTimeline.Populate(0, numFrames, unitsPerFrame, animCycle);
	mAnimAssets.push_back(pAnimAsset);

	AnimControl* pAnimControl = new AnimControl;
	pAnimControl->PlayAnim(pAnimAsset);
	mAnimControls.push_back(pAnimControl);

	return static_cast<uint16>(mAnimControls.size() - 1);
}

void WorldMap::EnableAnimTile(uint16 layer, uint16 tileIndex, uint16 sharedClockIndex)
{
	TileSet::TileData& data = mTileLayers[layer].mTileSet.GetTileData(tileIndex);
	ASSERT(!data.mIsAnimated);
	data.mIsAnimated = true;
	data.mAnimTimelineIndex = sharedClockIndex;
}

void WorldMap::EnableColorRotation(uint16* paletteIndices, uint16 numPaletteIndices, GameTimeType intervalTime)
{
	mColorRotElems.push_back(ColorRotElem());
	ColorRotElem& elem = mColorRotElems.back();
	
	for (int i = 0; i < numPaletteIndices; ++i)
	{
		elem.mPaletteIndices.push_back(paletteIndices[i]);
	}

	elem.mIntervalTime = intervalTime;
}

void WorldMap::Update(GameTimeType deltaTime)
{
	// Don't animate stuff on map while scrolling
	if (!ScrollingMgr::Instance().IsScrolling()) 
	{
		// Advance animated tile clocks
		for (std::size_t i = 0; i < mAnimControls.size(); ++i)
		{
			mAnimControls[i]->Update(deltaTime);
		}

		// Do color rotation
		for (std::size_t i = 0; i < mColorRotElems.size(); ++i)
		{
			ColorRotElem& elem = mColorRotElems[i];
			elem.mElapsedTime += deltaTime;
			if (elem.mElapsedTime >= elem.mIntervalTime)
			{
				elem.mElapsedTime = 0;
				RotateColors(elem.mPaletteIndices);
			}
		}
	}
}

void WorldMap::DrawScreenTiles(const Vector2I& srcScreen, const Vector2I& tgtScreen, DrawScreenTilesMode::Type mode)
{
	BackgroundLayer& tgtBgLayer2 = GraphicsEngine::GetBgLayer(2);
	BackgroundLayer& tgtBgLayer3 = GraphicsEngine::GetBgLayer(3);

	// Get upper-left tile coords in world space
	uint16 srcCurrTileX = srcScreen.x * GameNumScreenMetaTilesX;
	uint16 srcCurrTileY = srcScreen.y * GameNumScreenMetaTilesY;

	// Get upper-left tile coords in screen space
	uint16 tgtCurrTileX = tgtScreen.x * GameNumScreenMetaTilesX;
	uint16 tgtCurrTileY = tgtScreen.y * GameNumScreenMetaTilesY;

	const bool drawOnlyAnimTiles = (mode == DrawScreenTilesMode::AnimatedOnly);
	bool tileIsAnimated = false;
	uint16 srcTile = 0;

	for (uint16 y = 0; y < GameNumScreenMetaTilesY; ++y)
	{
		for (uint16 x = 0; x < GameNumScreenMetaTilesX; ++x)
		{
			srcTile = GetTileIndexToDraw(0, srcCurrTileX + x, srcCurrTileY + y, tileIsAnimated);
			if (!drawOnlyAnimTiles || tileIsAnimated)
				tgtBgLayer3.DrawTile(srcTile, tgtCurrTileX + x, tgtCurrTileY + y);

			srcTile = GetTileIndexToDraw(1, srcCurrTileX + x, srcCurrTileY + y, tileIsAnimated);
			if (!drawOnlyAnimTiles || tileIsAnimated)
				tgtBgLayer2.DrawTile(srcTile, tgtCurrTileX + x, tgtCurrTileY + y);
		}
	}
}


bool WorldMap::GetTileBoundingBoxIfCollision(const Vector2I& worldPos, BoundingBox& bbox)
{
	Vector2I tilePos(worldPos.x / GameMetaTileSizeX, worldPos.y / GameMetaTileSizeY);

	if ( mDataLayer(tilePos.x, tilePos.y).Collision == 0 )
		return false;

	//@NOTE: height is halved so we get the fake 3D feel from Zelda
	bbox.Reset(tilePos.x * GameMetaTileSizeX, tilePos.y * GameMetaTileSizeY, GameMetaTileSizeX, GameMetaTileSizeY / 2);
	return true;
}

void WorldMap::GetSpawnDataForScreen(const Vector2I& screen, SpawnDataList& spawnDataList)
{
	uint16 firstTileX = screen.x * GameNumScreenMetaTilesX;
	uint16 firstTileY = screen.y * GameNumScreenMetaTilesY;

	for (uint16 y = 0; y < GameNumScreenMetaTilesY; ++y)
	{
		const uint16 currTileY = firstTileY + y;
		for (uint16 x = 0; x < GameNumScreenMetaTilesX; ++x)
		{
			const uint16 currTileX = firstTileX + x;

			const uint16 actorType = mDataLayer(currTileX, currTileY).SpawnActorType;
			if (actorType > 0)
			{
				spawnDataList.push_back(SpawnData());
				SpawnData& spawnData = spawnDataList.back();
				spawnData.mGameActor = static_cast<GameActor::Type>(actorType);
				spawnData.mPos.Reset(currTileX * GameMetaTileSizeX, currTileY * GameMetaTileSizeY);
			}			
		}
	}
}

uint16 WorldMap::GetTileIndexToDraw(uint16 layer, uint16 x, uint16 y, bool& tileIsAnimated) const
{
	const TileLayer& tileLayer = mTileLayers[layer];

	uint16 tileIdx = tileLayer.mTileMap(x, y);

	const TileSet::TileData& data = tileLayer.mTileSet.GetTileData(tileIdx);
	if (data.mIsAnimated)
	{
		tileIdx += mAnimControls[data.mAnimTimelineIndex]->GetCurrPoseIndex();
	}

	tileIsAnimated = data.mIsAnimated;
	return tileIdx;
}
