#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/Vector2.h"
#include "gslib/Anim/AnimTimeline.h"
#include <vector>

// Game-specific constants
const uint16 GameMetaTileSizeX				= 16; // Doesn't need to be fixed for all - can be just background-specific...
const uint16 GameMetaTileSizeY				= 16;
const uint16 GameNumBgMetaTilesX			= (HwBgSizeX / GameMetaTileSizeX);
const uint16 GameNumBgMetaTilesY			= (HwBgSizeY / GameMetaTileSizeY);
const uint16 GameNumScreenMetaTilesX		= (HwScreenSizeX / GameMetaTileSizeX);
const uint16 GameNumScreenMetaTilesY		= (HwScreenSizeY / GameMetaTileSizeY);

class BoundingBox;

// Define a useful vector2 type: a vector of vectors
template <typename T>
struct vector2
{
	typedef std::vector< std::vector<T> > type;

	static void resize(type& vec2, std::size_t sizeX, std::size_t sizeY)
	{
		vec2.resize(sizeX);
		typename type::iterator iter = vec2.begin();
		for ( ; iter != vec2.end(); ++iter)
			(*iter).resize(sizeY);
	}
};


// WorldMap contains an array of TileLayer, which contains a TileMap and a TileSet.
// TileMap is a 2D array of tile indices to render. TileSet contains the actual
// list of TileData that is indexed by TileMap, and which contains tile-specific
// metadata (i.e. animated, etc.)


// TileSet is a set of tiles (image, info). There is one TileSet per TileLayer.
struct TileSet
{
	void Init();

	static const int MaxNumTiles = 128; // See GraphicsEngine::Init() for computation

	struct TileData
	{
		TileData() { Reset(); }
		void Reset() { mIsAnimated = false; mAnimTimelineIndex = 0; }

		bool mIsAnimated;
		uint8 mAnimTimelineIndex; // Shared AnimTimeline index
	};

	TileData mTileData[MaxNumTiles];
};


typedef vector2<uint16>::type TileMap;

// TileLayer represents a single layer of tiles. WorldMap aggregates one TileLayer
// per background layer.
struct TileLayer
{
	void Init(uint16 numTilesX, uint16 numTilesY);

	TileSet mTileSet;
	TileMap mTileMap;
};



// The world map (should this be a singleton?)
class WorldMap : public Singleton<WorldMap>
{
public:
	void Init(uint16 numScreensX, uint16 numScreensY);
	void Shutdown();

	void TEMP_LoadRandomMap();

	// Adds a shared clock for animated tiles and returns its index (pass index to EnableAnimTile())
	uint16 AddAnimTileSharedClock(int numFrames, AnimTimeType unitsPerFrame, AnimCycle::Type animCycle);

	// Turns on tile animation for the input tile
	void EnableAnimTile(uint16 layer, uint16 tileIndex, uint16 sharedClockIndex);

	void Update(GameTimeType deltaTime);

	uint16 GetNumScreensX() const	{ return mNumScreensX; }
	uint16 GetNumScreensY() const	{ return mNumScreensY; }
	uint16 GetNumTilesX() const		{ return mNumTilesX; }
	uint16 GetNumTilesY() const		{ return mNumTilesY; }

	// Copy a screen's tiles from the map to a screen quadrant on the target bg layers
	void DrawScreenTiles(const Vector2I& srcScreen, const Vector2I& tgtScreen);

	// Returns true and sets bbox if a collision tile is set at input world pos
	bool GetTileBoundingBoxIfCollision(const Vector2I& worldPos, BoundingBox& bbox);

private:
	friend class Singleton<WorldMap>;
	WorldMap();

	TileMap& GetTileMapLayer(uint16 layer) { return mTileLayers[layer].mTileMap; }
	uint16 GetTileIndexToDraw(uint16 layer, uint16 x, uint16 y) const;

	uint16 mNumScreensX, mNumScreensY;
	uint16 mNumTilesX, mNumTilesY;

	static const int NumLayers = 2;

	// Shared anim controls for animated tiles
	std::vector<class AnimAsset*> mAnimAssets;
	std::vector<class AnimControl*> mAnimControls;

	TileLayer mTileLayers[NumLayers];
	vector2<int>::type mCollisionMap; //@TODO: store 2d array of DataTile
};

#endif // WORLD_MAP_H
