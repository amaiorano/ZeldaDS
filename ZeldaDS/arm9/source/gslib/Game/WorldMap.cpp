#include "WorldMap.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Physics/BoundingBox.h"
#include "gslib/Anim/AnimAssetManager.h"
#include "gslib/Anim/AnimControl.h"
#include "ScrollingMgr.h"
#include <cstdlib>

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
	vector2<uint16>::resize(mTileMap, numTilesX, numTilesY);
}



WorldMap::WorldMap()
	: mNumScreensX(0)
	, mNumScreensY(0)
	, mNumTilesX(0)
	, mNumTilesY(0)
{
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

	mCollisionMap.resize(mNumTilesX);
	for (uint16 x=0; x < mCollisionMap.size(); ++x)
	{
		mCollisionMap[x].resize(mNumTilesY);
	}
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

void WorldMap::TEMP_LoadRandomMap()
{
	for (uint16 y = 0; y < mNumTilesY; ++y)
	{
		for (uint16 x = 0; x < mNumTilesX; ++x)
		{
			GetTileMapLayer(0)[x][y] = 0;
			GetTileMapLayer(1)[x][y] = 0;
			mCollisionMap[x][y] = 0;

			// randomly place a bush or rock tile
			if (MathEx::Rand(10) == 5)
			{
				int tile = 0;
				switch (MathEx::Rand(2))
				{
				case 0: tile = 20; break;
				case 1: tile = 23; break;
				case 2: tile = 40; break;
				}

				GetTileMapLayer(1)[x][y] = tile;

				mCollisionMap[x][y] = 1;
			}
		}
	}

	const int TilesPerRow = 22;

	TileMap& tileMap0 = GetTileMapLayer(0);
	tileMap0[0][0] = TilesPerRow*2 + 6;
	tileMap0[0][1] = TilesPerRow*3 + 6;
	tileMap0[0][2] = TilesPerRow*4 + 6;

	tileMap0[2][2] = 16;

	//TEST
	uint16 sharedClockIndex = AddAnimTileSharedClock(4, 45, AnimCycle::Loop);
	//EnableAnimTile(0, 0, sharedClockIndex);
	EnableAnimTile(0, TilesPerRow*2 + 6, sharedClockIndex);
	EnableAnimTile(0, TilesPerRow*3 + 6, sharedClockIndex);
	EnableAnimTile(0, TilesPerRow*4 + 6, sharedClockIndex);

	sharedClockIndex = AddAnimTileSharedClock(4, 10, AnimCycle::Loop);
	EnableAnimTile(0, 16, sharedClockIndex);

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
	TileSet::TileData& data = mTileLayers[layer].mTileSet.mTileData[tileIndex];
	ASSERT(!data.mIsAnimated);
	data.mIsAnimated = true;
	data.mAnimTimelineIndex = sharedClockIndex;
}

void WorldMap::Update(GameTimeType deltaTime)
{
	// Advance animated tile clocks
	for (std::size_t i = 0; i < mAnimControls.size(); ++i)
	{
		mAnimControls[i]->Update(deltaTime);
	}
}

void WorldMap::DrawScreenTiles(const Vector2I& srcScreen, const Vector2I& tgtScreen)
{
	BackgroundLayer& tgtBgLayer2 = GraphicsEngine::GetBgLayer(2);
	BackgroundLayer& tgtBgLayer3 = GraphicsEngine::GetBgLayer(3);

	// Get upper-left tile coords in world space
	uint16 srcCurrTileX = srcScreen.x * GameNumScreenMetaTilesX;
	uint16 srcCurrTileY = srcScreen.y * GameNumScreenMetaTilesY;

	// Get upper-left tile coords in screen space
	uint16 tgtCurrTileX = tgtScreen.x * GameNumScreenMetaTilesX;
	uint16 tgtCurrTileY = tgtScreen.y * GameNumScreenMetaTilesY;

	for (uint16 y = 0; y < GameNumScreenMetaTilesY; ++y)
	{
		for (uint16 x = 0; x < GameNumScreenMetaTilesX; ++x)
		{
			uint16 srcTile = GetTileIndexToDraw(0, srcCurrTileX + x, srcCurrTileY + y);
			tgtBgLayer3.DrawTile(srcTile, tgtCurrTileX + x, tgtCurrTileY + y);

			srcTile = GetTileIndexToDraw(1, srcCurrTileX + x, srcCurrTileY + y);
			tgtBgLayer2.DrawTile(srcTile, tgtCurrTileX + x, tgtCurrTileY + y);
		}
	}
}

bool WorldMap::GetTileBoundingBoxIfCollision(const Vector2I& worldPos, BoundingBox& bbox)
{
	Vector2I tilePos(worldPos.x / GameMetaTileSizeX, worldPos.y / GameMetaTileSizeY);

	if ( !mCollisionMap[tilePos.x][tilePos.y] )
		return false;

	//@NOTE: height is halved so we get the fake 3D feel from Zelda
	bbox.Reset(tilePos.x * GameMetaTileSizeX, tilePos.y * GameMetaTileSizeY, GameMetaTileSizeX, GameMetaTileSizeY / 2);
	return true;
}

uint16 WorldMap::GetTileIndexToDraw(uint16 layer, uint16 x, uint16 y) const
{
	const TileLayer& tileLayer = mTileLayers[layer];

	uint16 tileIdx = tileLayer.mTileMap[x][y];

	const TileSet::TileData& data = tileLayer.mTileSet.mTileData[tileIdx];
	if (data.mIsAnimated)
	{
		tileIdx += mAnimControls[data.mAnimTimelineIndex]->GetCurrPoseIndex();
	}

	return tileIdx;
}
