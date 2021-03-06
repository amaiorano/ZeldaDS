#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Hw/Constants.h"
#include "gslib/Math/Vector2.h"
#include "gslib/Anim/AnimTimeline.h"
#include "gslib/Util/Array2d.h"
#include "GameAnims.h"
#include <vector>

// Game-specific constants
const uint16 GameMetaTileSizeX				= 16; // Doesn't need to be fixed for all - can be just background-specific...
const uint16 GameMetaTileSizeY				= 16;
const uint16 GameNumBgMetaTilesX			= (HwBgSizeX / GameMetaTileSizeX);
const uint16 GameNumBgMetaTilesY			= (HwBgSizeY / GameMetaTileSizeY);
const uint16 GameNumScreenMetaTilesX		= (HwScreenSizeX / GameMetaTileSizeX);
const uint16 GameNumScreenMetaTilesY		= (HwScreenSizeY / GameMetaTileSizeY);

namespace GameTileLayer
{
	enum Type
	{
		Background,
		Foreground,

		NumTypes
	};
}

class BoundingBox;
class BackgroundLayer;
class GameEvent;

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

	TileData& GetTileData(uint16 tileIndex)
	{
		return const_cast<TileData&>( const_cast<const TileSet*>(this)->GetTileData(tileIndex) );
	}

	const TileData& GetTileData(uint16 tileIndex) const
	{
		ASSERT_MSG(tileIndex < MaxNumTiles, "TileSet::GetTileData() index out of range");
		return mTileData[tileIndex];
	}

private:
	TileData mTileData[MaxNumTiles];
};

typedef uint16 TileIndexType; //@TODO: Make this uint8 (TileSet::MaxNumTiles is < 256)
typedef Array2d<TileIndexType, uint16> TileMap;

// TileLayer represents a single layer of tiles. WorldMap aggregates one TileLayer
// per background layer.
struct TileLayer
{
	void Init(uint16 numTilesX, uint16 numTilesY);

	TileSet mTileSet;
	TileMap mTileMap;
};

namespace DrawScreenTilesMode
{
	enum Type
	{
		All,
		AnimatedOnly
	};
}

// The world map (should this be a singleton?)
class WorldMap : public Singleton<WorldMap>
{
public:
	~WorldMap();

	void LoadMap(const char* mapFile);
	void UnloadMap();

	// Useful helper to convert a tile position to a world position, and vice versa
	//@TODO: Remove 'Pos' from names
	static Vector2I TileToWorldPos(uint16 tileX, uint16 tileY) { return Vector2I(tileX * GameMetaTileSizeX, tileY * GameMetaTileSizeY); }
	static Vector2I TileToWorldPos(const Vector2I& tilePos) { return Vector2I(tilePos.x * GameMetaTileSizeX, tilePos.y * GameMetaTileSizeY); }
	static Vector2I TileToScreen(const Vector2I& tilePos) { return Vector2I(tilePos.x / GameNumScreenMetaTilesX, tilePos.y / GameNumScreenMetaTilesY); }
	static Vector2I WorldPosToTile(uint16 posX, uint16 posY) { return Vector2I(posX / GameMetaTileSizeX, posY / GameMetaTileSizeY); }
	static Vector2I WorldPosToTile(const Vector2I& worldPos) { return Vector2I(worldPos.x / GameMetaTileSizeX, worldPos.y / GameMetaTileSizeY); }
	static Vector2I WorldPosToScreen(const Vector2I& worldPos) { return TileToScreen(WorldPosToTile(worldPos)); }

	// Adds a shared clock for animated tiles and returns its index (pass index to EnableAnimTile())
	uint16 AddAnimTileSharedClock(int numFrames, AnimTimeType unitsPerFrame, AnimCycle::Type animCycle);

	// Turns on tile animation for the input tile
	void EnableAnimTile(uint16 layer, uint16 tileIndex, uint16 sharedClockIndex);

	// Turns on the rotation of colors at the input palette indices at every intervalTime
	void EnableColorRotation(uint16* paletteIndices, uint16 numPaletteIndices, GameTimeType intervalTime);

	void Update(GameTimeType deltaTime);

	uint16 GetNumScreensX() const	{ return mNumScreensX; }
	uint16 GetNumScreensY() const	{ return mNumScreensY; }
	uint16 GetNumTilesX() const		{ return mNumTilesX; }
	uint16 GetNumTilesY() const		{ return mNumTilesY; }
	uint16 GetTileSetGroupIndex() const { return mTileSetGroupIndex; }

	// Copy a screen's tiles from the map to a screen quadrant on the target bg layers
	void DrawScreenTiles(const Vector2I& srcScreen, const Vector2I& tgtScreen, DrawScreenTilesMode::Type mode = DrawScreenTilesMode::All);

	// Returns true and sets bbox if a collision tile is set at input world pos
	bool GetTileBoundingBoxIfCollision(const Vector2I& worldPos, BoundingBox& bbox); //@TODO: Make const

	GameEvent* GetGameEventIfExists(const Vector2I& worldPos) const;

	uint16 GetTileIndex(uint16 layer, const Vector2I& worldTilePos);
	BackgroundLayer& GetBgLayer(GameTileLayer::Type layer);

	// Returns the player spawn data for the currently loaded map
	struct PlayerSpawnData
	{
		Vector2I mPos;
	};
	const PlayerSpawnData& GetPlayerSpawnData() const { return mPlayerSpawnData; }

	struct SpawnData
	{
		GameActor::Type mGameActor;
		Vector2I mPos;
	};
	typedef std::vector<SpawnData> SpawnDataList;
	void GetSpawnDataForScreen(const Vector2I& screen, SpawnDataList& spawnDataList);

private:
	friend class Singleton<WorldMap>;
	WorldMap();

	void FindPlayerSpawnData();
	TileMap& GetTileMapLayer(uint16 layer) { return mTileLayers[layer].mTileMap; }
	uint16 GetTileIndexToDraw(uint16 layer, uint16 tileX, uint16 tileY, bool& tileIsAnimated) const;

	PlayerSpawnData mPlayerSpawnData;

	uint16 mNumScreensX, mNumScreensY;
	uint16 mNumTilesX, mNumTilesY;

	static const int NumLayers = GameTileLayer::NumTypes;

	uint16 mTileSetGroupIndex;

	// Shared anim controls for animated tiles
	std::vector<class AnimAsset*> mAnimAssets;
	std::vector<class AnimControl*> mAnimControls;

	struct ColorRotElem
	{
		ColorRotElem() : mIntervalTime(0), mElapsedTime(0) { }
		std::vector<uint16> mPaletteIndices;
		GameTimeType mIntervalTime;
		GameTimeType mElapsedTime;
	};
	std::vector<ColorRotElem> mColorRotElems;

	TileLayer mTileLayers[NumLayers];

	//@TODO: Rename to mDataLayer and interpret 16 bits for different types of data
	//
	// Stuff that needs to be stored:
	// - collision: none, solid to all, solid to chars (but not to boomerang, for example)
	// - door: map to load (index into list of maps, stored separately in map data)
	// - enemy spawner: enemy type to load (GameActor::Type), spawn at location, or adjacent offscreen location
	// - hidden stairwell: map to load (temp map?), how to uncover (fire, push statue enemy off)
	// - hidden overworld bomb door: similar to stairwell, uncovered by bomb
	// - hidden dungeon bomb door: no map to load, just replace tile and remove collisions
	// - raft dock: if have raft, raft to location
	//
	// 2 bits: collision
	// 6 bits: spawner actor type
	// 1 bit : has associated event
	// etc.
	struct DataLayerEntry
	{
		uint16 Collision : 2;
		uint16 SpawnActorType : 6;
		uint16 HasEvent : 1;
		uint16 Unused : 7;
	};
	CT_ASSERT( sizeof(DataLayerEntry) == sizeof(uint16) );

	Array2d<DataLayerEntry, uint16> mDataLayer;

	typedef std::vector<GameEvent*> GameEventList;
	GameEventList mGameEvents;
};

#endif // WORLD_MAP_H
