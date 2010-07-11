#include "ISceneNode.h"
#include "SceneGraph.h"

ISceneNode::ISceneNode()
	: mLocalPos(InitZero)
	, mWorldPos(InitZero)
	, mRemoveNodePostUpdate(false)
{
}

bool ISceneNode::IsNodeInScene() const
{
	return SceneGraph::Instance().IsNodeInScene(this);
}

const Vector2I& ISceneNode::GetPosition() const
{
	if (ISceneNode* pParent = GetParent())
	{
		mWorldPos = mLocalPos + pParent->GetPosition();
		return mWorldPos;
	}
	
	return mLocalPos;
}

const Vector2I& ISceneNode::GetLocalPosition() const
{
	return mLocalPos;
}


void ISceneNode::SetPosition(const Vector2I& pos)
{
	// If we have a parent, we don't move it, we just update
	// our local pos relative to our parent
	if (ISceneNode* pParent = GetParent())
	{
		mLocalPos = pos - pParent->GetPosition();
	}
	else
	{
		mLocalPos = pos;
	}
}

void ISceneNode::SetLocalPosition(const Vector2I& pos)
{
	mLocalPos = pos;
}

bool ISceneNode::OnPreAttachChild(NodePtr pNode)
{
	// Make it so that the child remains where it was in the world
	pNode->SetLocalPosition(pNode->GetPosition() - GetPosition());
	return true;
}

bool ISceneNode::OnPreDetachChild(NodePtr pNode)
{
	// Node is about to become part of the world, so let's leave it exactly where it is
	pNode->SetLocalPosition(pNode->GetPosition());
	return true;
}
