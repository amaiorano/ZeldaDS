#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "gslib/Core/Core.h"
#include "ISceneNode.h"
#include "IAnimatedRenderable.h"
#include "IPhysical.h"

namespace GameSpriteRenderGroup
{
	enum Type
	{
		AboveAll,
		Heroes,
		Weapons,
		Enemies,

		NumTypes
	};
}

// Root class for all game objects
class GameObject : public ISceneNode, public IAnimatedRenderable, public IPhysical
{
public:
	GameObject();
	virtual ~GameObject();

	// ISceneNode interface
	virtual void OnAddToScene();
	virtual void OnRemoveFromScene();
	virtual void Update(GameTimeType deltaTime);
	virtual void Render(GameTimeType deltaTime);

	// Returns GameObject's screen position relative to the Camera
	Vector2I GetScreenPosition() const;

protected:
	// IPhysical interface
	virtual void SetPhysicalPosition(const Vector2I& pos) { ISceneNode::SetPosition(pos); }
	virtual const Vector2I& GetPhysicalPosition() const { return ISceneNode::GetPosition(); }

	// Child class may override GetGameObjectInfo() to populate with child-specific values
	struct GameObjectInfo
	{
		GameObjectInfo() 
			: mSpriteRenderGroupId(GameSpriteRenderGroup::AboveAll)
			, mSpriteSize(16, 16)
			, mPhysicalSize(16, 16)
			, mGameActor(GameActor::None)
			, mIsDirectional(true)
		{
		}

		uint16 mSpriteRenderGroupId;
		Vector2I mSpriteSize;
		Vector2I mPhysicalSize;
		GameActor::Type mGameActor;
		bool mIsDirectional;
	};
	virtual void GetGameObjectInfo(GameObjectInfo& gameObjectInfo)
	{
		// Default does nothing, leaving default values in gameObjectInfo
	}
};

#endif // GAME_OBJECT_H
