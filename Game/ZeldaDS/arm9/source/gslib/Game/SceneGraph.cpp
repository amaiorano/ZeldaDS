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
	if (node.IsPlayerWeapon())
	{
		mPlayerWeaponList.push_back(&node);
	}
	else
	{
		mEnemyWeaponList.push_back(&node);
	}
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

	node.OnRemoveFromScene();
}

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

void SceneGraph::RemoveNodesPostUpdate()
{
	RemoveMarkedNodesInList(mPlayerList);
	RemoveMarkedNodesInList(mEnemyList);
	RemoveMarkedNodesInList(mPlayerWeaponList);
	RemoveMarkedNodesInList(mEnemyWeaponList);
}
