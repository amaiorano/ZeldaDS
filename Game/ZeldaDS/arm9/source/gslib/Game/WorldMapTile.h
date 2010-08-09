#ifndef WORLD_MAP_TILE_H
#define WORLD_MAP_TILE_H

#include "GameObject.h"
#include "WorldMap.h"

//@TODO: Think of a better name for this stupid class
class WorldMapTile : public GameObject
{
public:
	typedef GameObject Base;

	WorldMapTile(GameTileLayer::Type tileLayer, const Vector2I& worldTilePos);

	// GameObject interface
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		Base::GetGameObjectInfo(gameObjectInfo);
		gameObjectInfo.mSpriteRenderGroupId = GameSpriteRenderGroup::AboveAll;
	}

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void Update(GameTimeType deltaTime);

private:
	AnimPoseType mAnimPose;
};

#endif // WORLD_MAP_TILE_H
