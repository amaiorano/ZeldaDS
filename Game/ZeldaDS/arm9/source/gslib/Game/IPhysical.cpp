#include "IPhysical.h"
#include "ISceneNode.h" // TransformOwner

IPhysical::IPhysical()
	: mWidth(0)
	, mHeight(0)
	, mVelocity(InitZero)
	, mImpulse(InitZero)
{
}

void IPhysical::Activate(uint16 width, uint16 height)
{
	mWidth = width;
	mHeight = height;
}

void IPhysical::Deactivate()
{
}

void IPhysical::Teleport(const Vector2I& pos)
{
	SetPhysicalPosition(pos);
	mVelocity.Reset(InitZero);
}

void IPhysical::SetVelocity(const Vector2I& vel)
{
	mVelocity = vel;
}

void IPhysical::AddImpulse(const Vector2I& impulse)
{
	mImpulse = impulse;
}

BoundingBox IPhysical::GetBoundingBox() const
{
	return BoundingBox(GetPhysicalPosition(), mWidth, mHeight);
}
