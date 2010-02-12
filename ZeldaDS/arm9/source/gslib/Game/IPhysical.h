#ifndef IPHYSICAL_H
#define IPHYSICAL_H

#include "gslib/Core/Core.h"
#include "gslib/Math/Vector2.h"
#include "gslib/Physics/BoundingBox.h"

class IPhysical;

struct CollisionInfo
{
	CollisionInfo() : mpCollidingWith(0), mPushVector(InitZero) { }

	IPhysical* mpCollidingWith; // If NULL, means we collided with the world
	Vector2I mPushVector; // How much to move this IPhysical so it no longer overlaps mpCollidingWith
};

// An IPhysical is something that moves and collides
class IPhysical
{
public:
	void Activate(uint16 width, uint16 height);
	void Deactivate();

	void Teleport(const Vector2I& pos);
	void SetVelocity(const Vector2I& vel);
	void AddImpulse(const Vector2I& impulse);

	BoundingBox GetBoundingBox() const;
	const Vector2I& GetVelocity() const { return mVelocity; }

	virtual bool AllowWorldOverlap() const { return false; }

protected:
	friend class PhysicsHelpers;

	// Child class must provide and set world position. Note that only objects
	// that set velocities or that don't allow world overlap will get their
	// positions modified.
	virtual void SetPhysicalPosition(const Vector2I& pos) = 0;
	virtual const Vector2I& GetPhysicalPosition() const = 0;

	// Hook called by PhysicsSimulator when collision occurs.
	// Note that this function is only called on one of the two actors that collide,
	// putting the responsibility of affecting the collidee on the collider;
	// OR it's called when actor collides with the world.
	virtual void OnCollision(const CollisionInfo& collisionInfo) { /*FAIL_MSG("Unhandled OnCollision call");*/ }

private:
	uint16 mWidth, mHeight;
	Vector2I mVelocity;
	Vector2I mImpulse; // Added to position (not time-based)
};

#endif // IPHYSICAL_H
