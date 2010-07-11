#include "SceneGraph.h"
#include "Player.h"
#include "Enemy.h"
#include "Boomerang.h" //@TODO: REPLACE WITH "Weapon.h"
#include <algorithm>

void SceneGraph::AddNode(ISceneNode* pNode)
{
	ASSERT(pNode);
	ASSERT_MSG(!pNode->mRemoveNodePostUpdate, "Probably got scheduled for remove then added in same frame");

	//@LAME: Because we multiply inherit ISceneNode and IPhysical, we need to do this here
	if (IPhysical* pPhysical = DynamicCast<IPhysical*>(pNode))
	{
		mPhysicalList.push_back(pPhysical);
	}

	mSceneNodeList.push_back(pNode);
	pNode->OnAddToScene();
}

void SceneGraph::AddNode(Player* pNode)
{
	mPlayerList.push_back(pNode);
	AddNode(static_cast<ISceneNode*>(pNode));
}

void SceneGraph::AddNode(Enemy* pNode)
{
	mEnemyList.push_back(pNode);
	AddNode(static_cast<ISceneNode*>(pNode));
} 

void SceneGraph::AddNode(Weapon* pNode)
{
	if (pNode->IsPlayerWeapon())
	{
		mPlayerWeaponList.push_back(pNode);
	}
	else
	{
		mEnemyWeaponList.push_back(pNode);
	}
	AddNode(static_cast<ISceneNode*>(pNode));
}

void SceneGraph::RemoveNodePostUpdate(ISceneNode* pNode)
{
	ASSERT(pNode);
	ASSERT(!pNode->mRemoveNodePostUpdate);
	pNode->mRemoveNodePostUpdate = true;
}

void SceneGraph::RemoveNode(ISceneNode* pNode)
{
	if (IPhysical* pPhysical = DynamicCast<IPhysical*>(pNode))
	{
		REMOVE_FROM_CONTAINER(PhysicalList, mPhysicalList, pPhysical);
	}

	REMOVE_FROM_CONTAINER(SceneNodeList, mSceneNodeList, pNode);

	pNode->OnRemoveFromScene();
}

bool SceneGraph::IsNodeInScene(const ISceneNode* pNode) const
{
	ASSERT(pNode);
	return std::find(mSceneNodeList.begin(), mSceneNodeList.end(), pNode) != mSceneNodeList.end();
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
	typedef typename List::value_type ValueType;
	
	for (Iterator iter = list.begin(); iter != list.end(); /*++iter*/)
	{
		Iterator currIter = iter;
		++iter;

		ValueType pNode = *currIter;

		if (pNode->mRemoveNodePostUpdate)
		{
			pNode->mRemoveNodePostUpdate = false;
			RemoveNode(pNode);
			list.erase(currIter);
			delete pNode;
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
