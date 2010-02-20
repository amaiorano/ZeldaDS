#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/Vector2.h"
#include <vector>

// Game-specific constants
const uint16 GameMetaTileSizeX				= 16; // Doesn't need to be fixed for all - can be just background-specific...
const uint16 GameMetaTileSizeY				= 16;
const uint16 GameNumBgMetaTilesX			= (HwBgSizeX / GameMetaTileSizeX);
const uint16 GameNumBgMetaTilesY			= (HwBgSizeY / GameMetaTileSizeY);
const uint16 GameNumScreenMetaTilesX		= (HwScreenSizeX / GameMetaTileSizeX);
const uint16 GameNumScreenMetaTilesY		= (HwScreenSizeY / GameMetaTileSizeY);

class BoundingBox;

class WorldMap : public Singleton<WorldMap>
{
public:
	typedef std::vector< std::vector<uint8> > Array2D;

	void Init(uint16 numScreensX, uint16 numScreensY);
	void TEMP_LoadRandomMap();

	uint16 GetNumScreensX() const	{ return mNumScreensX; }
	uint16 GetNumScreensY() const	{ return mNumScreensY; }
	uint16 GetNumTilesX() const		{ return mNumTilesX; }
	uint16 GetNumTilesY() const		{ return mNumTilesY; }

	Array2D& GetTileMapLayer(uint16 layer) { return mTileMap[layer]; }

	// Copy a screen's tiles from the map to a screen quadrant on the target bg layers
	void DrawScreenTiles(const Vector2I& srcScreen, const Vector2I& tgtScreen);

	// Returns true and sets bbox if a collision tile is set at input world pos
	bool GetTileBoundingBoxIfCollision(const Vector2I& worldPos, BoundingBox& bbox);

private:
	friend class Singleton<WorldMap>;
	WorldMap();

	uint16 mNumScreensX, mNumScreensY;
	uint16 mNumTilesX, mNumTilesY;

	static const int NumLayers = 2;
	Array2D mTileMap[NumLayers];

	Array2D mCollisionMap; //@TODO: store 2d array of DataTile
};

#endif // WORLD_MAP_H
