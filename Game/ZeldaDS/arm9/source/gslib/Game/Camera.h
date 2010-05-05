#ifndef CAMERA_H
#define CAMERA_H

#include "gslib/Core/Singleton.h"
#include "gslib/Math/Vector2.h"
#include "ISceneNode.h"

class BoundingBox;

// Camera represents the current screen view
class Camera : public Singleton<Camera>, public ISceneNode
{
public:
	// Screen/World transform helpers
	Vector2I ScreenToWorld(const Vector2I& screenSpacePos) const;
	Vector2I WorldToScreen(const Vector2I& worldSpacePos) const;
	bool IsWorldBBoxInScreenBounds(const BoundingBox& worldBBox) const;

private:
	friend class Singleton<Camera>;
	Camera() {}
};

#endif // CAMERA_H
