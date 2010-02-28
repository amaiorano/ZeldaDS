#include "SceneGraph.h"
#include "Player.h"
#include "Enemy.h"
#include "Boomerang.h" //@TODO: REPLACE WITH "Weapon.h"
#include <algorithm>


void SceneGraph::AddNode(ISceneNode& node)
{
	ASSERT_MSG(!node.mRemoveNodePostUpdate, "Probably got scheduled for remove then added in same frame");

	if (IPhysical* pPhysical = DynamicCast<IPhysical*>(&node)) // Hmm...
	{
		mPhysicalList.push_back(pPhysical);
	}

	mSceneNodeList.push_back(&node);
	node.OnAddToScene();
}

void SceneGraph::AddNode(Player& node)
{
	mPlayerList.push_back(&node);
	AddNode(static_cast<ISceneNode&>(node));
}

void SceneGraph::AddNode(Enemy& node)
{
	mEnemyList.push_back(&node);
	AddNode(static_cast<ISceneNode&>(node));
} 

void SceneGraph::AddNode(Weapon& node)
{
	//@TODO: Check here if it's a player or enemy weapon and add to the right list?
	mPlayerWeaponList.push_back(&node);
	AddNode(static_cast<ISceneNode&>(node));
}

void SceneGraph::RemoveNodePostUpdate(ISceneNode& node)
{
	ASSERT(!node.mRemoveNodePostUpdate);
	node.mRemoveNodePostUpdate = true;
}

void SceneGraph::RemoveNode(ISceneNode& node)
{
	if (IPhysical* pPhysical = DynamicCast<IPhysical*>(&node)) // Hmm...
	{
		REMOVE_FROM_CONTAINER(PhysicalList, mPhysicalList, pPhysical);
	}

	REMOVE_FROM_CONTAINER(SceneNodeList, mSceneNodeList, &node);

	//mSceneNodesJustRemoved.push_back(&node);
	node.OnRemoveFromScene();
}

//void SceneGraph::RemoveNode(Player& node)
//{
//	REMOVE_FROM_CONTAINER(PlayerList, mPlayerList, &node);
//	RemoveNode(static_cast<ISceneNode&>(node));
//}
//
//void SceneGraph::RemoveNode(Enemy& node)
//{
//	REMOVE_FROM_CONTAINER(EnemyList, mEnemyList, &node);
//	RemoveNode(static_cast<ISceneNode&>(node));
//}
//
//void SceneGraph::RemoveNode(Weapon& node)
//{
//	REMOVE_FROM_CONTAINER(WeaponList, mPlayerWeaponList, &node);
//	RemoveNode(static_cast<ISceneNode&>(node));
//}

bool SceneGraph::IsNodeInScene(const ISceneNode& node) const
{
	return std::find(mSceneNodeList.begin(), mSceneNodeList.end(), &node) != mSceneNodeList.end();
}

void SceneGraph::Update(GameTimeType deltaTime)
{
	SceneNodeList::iterator iter = mSceneNodeList.begin();
	for ( ; iter != mSceneNodeList.end(); ++iter)
	{
		(*iter)->Update(deltaTime);
	}

	RemoveNodesPostUpdate();
}

void SceneGraph::Render(GameTimeType deltaTime)
{
	SceneNodeList::iterator iter = mSceneNodeList.begin();
	for ( ; iter != mSceneNodeList.end(); ++iter)
	{
		(*iter)->Render(deltaTime);
	}
}

template <typename List>
void SceneGraph::RemoveMarkedNodesInList(List& list)
{
	typedef typename List::iterator Iterator;
	
	for (Iterator iter = list.begin(); iter != list.end(); /*++iter*/)
	{
		Iterator currIter = iter;
		++iter;

		if ((*currIter)->mRemoveNodePostUpdate)
		{
			(*currIter)->mRemoveNodePostUpdate = false;
			RemoveNode(**currIter);
			list.erase(currIter);
		}
	}
}

template <typename List>
void Test(List& list)
{
	typedef typename List::iterator ListIter;

	ListIter iter = list.begin();
}

void SceneGraph::RemoveNodesPostUpdate()
{
	Test(mPlayerList);

	RemoveMarkedNodesInList(mPlayerList);
	RemoveMarkedNodesInList(mEnemyList);
	RemoveMarkedNodesInList(mPlayerWeaponList);

	//for (PlayerList::iterator iter = mPlayerList.begin(); iter != mPlayerList.end(); /*++iter*/)
	//{
	//	PlayerList::iterator currIter = iter;
	//	++iter;

	//	if ((*currIter)->mRemoveNodePostUpdate)
	//	{
	//		(*currIter)->mRemoveNodePostUpdate = false;
	//		RemoveNode(**currIter);
	//		mPlayerList.erase(currIter);
	//	}
	//}

	//for (EnemyList::iterator iter = mEnemyList.begin(); iter != mEnemyList.end(); /*++iter*/)
	//{
	//	EnemyList::iterator currIter = iter;
	//	++iter;

	//	if ((*currIter)->mRemoveNodePostUpdate)
	//	{
	//		(*currIter)->mRemoveNodePostUpdate = false;
	//		RemoveNode(**currIter);
	//		mEnemyList.erase(currIter);
	//	}
	//}

	//for (WeaponList::iterator iter = mPlayerWeaponList.begin(); iter != mPlayerWeaponList.end(); /*++iter*/)
	//{
	//	WeaponList::iterator currIter = iter;
	//	++iter;

	//	if ((*currIter)->mRemoveNodePostUpdate)
	//	{
	//		(*currIter)->mRemoveNodePostUpdate = false;
	//		RemoveNode(**currIter);
	//		mPlayerWeaponList.erase(currIter);
	//	}
	//}
}
