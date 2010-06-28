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
void SceneGraph::RemoveMarkedNodesInList(List& list, bool deleteNode)
{
	typedef typename List::iterator Iterator;
	typedef typename List::value_type ValueType;
	
	for (Iterator iter = list.begin(); iter != list.end(); /*++iter*/)
	{
		Iterator currIter = iter;
		++iter;

		ValueType pNode = *currIter;

		if (pNode->mRemoveNodePostUpdate)
		{
			pNode->mRemoveNodePostUpdate = false;
			RemoveNode(*pNode);
			list.erase(currIter);
			if (deleteNode)
			{
				delete pNode;
			}
		}
	}
}

void SceneGraph::RemoveNodesPostUpdate()
{
	RemoveMarkedNodesInList(mPlayerList, false);
	//@HACK: Fact that SceneGraph owns enemies is hard-coded. We
	// need to communicate whether SceneGraph owns certain nodes,
	// or just have it own all nodes.
	RemoveMarkedNodesInList(mEnemyList, true);
	RemoveMarkedNodesInList(mPlayerWeaponList, false);
	RemoveMarkedNodesInList(mEnemyWeaponList, false);
}
