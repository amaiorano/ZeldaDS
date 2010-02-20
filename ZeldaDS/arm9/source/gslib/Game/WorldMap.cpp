#include "WorldMap.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Hw/BackgroundLayer.h"
#include "gslib/Math/MathEx.h"
#include "gslib/Physics/BoundingBox.h"
#include "ScrollingMgr.h"
#include <cstdlib>

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
		Array2D& tileMapLayer = GetTileMapLayer(layer);

		tileMapLayer.resize(mNumTilesX);
		for (uint16 x=0; x < tileMapLayer.size(); ++x)
		{
			tileMapLayer[x].resize(mNumTilesY);
		}
	}

	mCollisionMap.resize(mNumTilesX);
	for (uint16 x=0; x < mCollisionMap.size(); ++x)
	{
		mCollisionMap[x].resize(mNumTilesY);
	}
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
}

void WorldMap::DrawScreenTiles(const Vector2I& srcScreen, const Vector2I& tgtScreen)
{
	BackgroundLayer& tgtBgLayer2 = GraphicsEngine::GetBgLayer(2);
	BackgroundLayer& tgtBgLayer3 = GraphicsEngine::GetBgLayer(3);

	Array2D& bgTileMap = GetTileMapLayer(0);
	Array2D& fgTileMap = GetTileMapLayer(1);

	uint16 srcCurrTileX = srcScreen.x * GameNumScreenMetaTilesX;
	uint16 srcCurrTileY = srcScreen.y * GameNumScreenMetaTilesY;

	uint16 tgtCurrTileX = tgtScreen.x * GameNumScreenMetaTilesX;
	uint16 tgtCurrTileY = tgtScreen.y * GameNumScreenMetaTilesY;

	for (uint16 y = 0; y < GameNumScreenMetaTilesY; ++y)
	{
		for (uint16 x = 0; x < GameNumScreenMetaTilesX; ++x)
		{
			uint16 srcTile = bgTileMap[srcCurrTileX + x][srcCurrTileY + y];
			tgtBgLayer3.DrawTile(srcTile, tgtCurrTileX + x, tgtCurrTileY + y);

			srcTile = fgTileMap[srcCurrTileX + x][srcCurrTileY + y];
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
