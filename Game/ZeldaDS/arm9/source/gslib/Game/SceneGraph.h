#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "ISceneNode.h"
#include <list>

class WorldMap;
class IPhysical;
class Player;
class Enemy;
class Weapon;

typedef std::list<ISceneNode*> SceneNodeList;
typedef std::list<IPhysical*> PhysicalList;
typedef std::list<Player*> PlayerList;
typedef std::list<Enemy*> EnemyList;
typedef std::list<Weapon*> WeaponList;

class SceneGraph : public Singleton<SceneGraph>
{
public:
	void SetWorldMap(WorldMap& worldMap) { mpWorldMap = &worldMap; }

	void AddNode(ISceneNode& node);
	void AddNode(Player& node);
	void AddNode(Enemy& node);
	void AddNode(Weapon& node);

	void RemoveNodePostUpdate(ISceneNode& node);

private:
	void RemoveNode(ISceneNode& node);
	//void RemoveNode(Player& node);
	//void RemoveNode(Enemy& node);
	//void RemoveNode(Weapon& node);
public:

	bool IsNodeInScene(const ISceneNode& node) const;

	WorldMap& GetWorldMap()				{ ASSERT(mpWorldMap); return *mpWorldMap; }

	SceneNodeList& GetSceneNodeList()	{ return mSceneNodeList; }
	PhysicalList& GetPhysicalList()		{ return mPhysicalList; }
	PlayerList& GetPlayerList()			{ return mPlayerList; }
	EnemyList& GetEnemyList()			{ return mEnemyList; }
	WeaponList& GetPlayerWeaponList()	{ return mPlayerWeaponList; }

	// Updates/Renders all scene nodes
	void Update(GameTimeType deltaTime);
	void Render(GameTimeType deltaTime);

private:
	friend class Singleton<SceneGraph>;
	SceneGraph() {}

	template <typename List>
	void RemoveMarkedNodesInList(List& list);
	void RemoveNodesPostUpdate();

	WorldMap* mpWorldMap;
	SceneNodeList mSceneNodeList;
	PhysicalList mPhysicalList;

	PlayerList mPlayerList;
	EnemyList mEnemyList;
	WeaponList mPlayerWeaponList;
};

#endif // SCENE_GRAPH_H
