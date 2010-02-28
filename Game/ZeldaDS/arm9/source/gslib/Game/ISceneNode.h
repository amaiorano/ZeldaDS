#ifndef ISCENE_NODE
#define ISCENE_NODE

#include "gslib/Core/Core.h"
#include "gslib/Math/Vector2.h"
#include "ITreeNode.h"

// SceneGraph node
class ISceneNode : public ITreeNode<ISceneNode>
{
public:
	ISceneNode();

	// Initialization/shutdown hooks for nodes
	virtual void OnAddToScene() { }
	virtual void OnRemoveFromScene() { }
	
	// Convenience function (just forwards to SceneGraph)
	//@TODO: Maybe remove this? Is it really that convenient?
	bool IsNodeInScene() const;

	// Node is updated and rendered once per frame when part of the scene
	virtual void Update(GameTimeType deltaTime) { }
	virtual void Render(GameTimeType deltaTime) { }

	// "Position" alone means world position (as opposed to local position)
	const Vector2I& GetPosition() const;
	const Vector2I& GetLocalPosition() const;

	void SetPosition(const Vector2I& pos);
	void SetLocalPosition(const Vector2I& pos);

protected:
	// ITreeNode interface
	virtual bool OnPreAttachChild(NodePtr pNode);
	virtual bool OnPreDetachChild(NodePtr pNode);

private:
	mutable Vector2I mLocalPos;
	mutable Vector2I mWorldPos; // Only used as storage in GetPosition()
	
	friend class SceneGraph;
	bool mRemoveNodePostUpdate;
};

#endif // ISCENE_NODE
