#include "GameObject.h"
#include "gslib/Game/Camera.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void GameObject::OnAddToScene()
{
	GameObjectInfo goi;
	GetGameObjectInfo(goi);

	IAnimatedRenderable::Activate(goi.mSpriteRenderGroupId, goi.mSpriteSize.x, goi.mSpriteSize.y, goi.mGameActor, goi.mIsDirectional);
	IPhysical::Activate(goi.mPhysicalSize.x, goi.mPhysicalSize.y);
}

void GameObject::OnRemoveFromScene()
{
	IPhysical::Deactivate();
	IAnimatedRenderable::Deactivate();
}

void GameObject::Update(GameTimeType deltaTime)
{
	//@TODO: template method here? or child class knows to call Base::Update() last?
	IAnimatedRenderable::AdvanceClockAndSetAnimPose(deltaTime);
}

void GameObject::Render(GameTimeType deltaTime)
{
	IAnimatedRenderable::UpdateSpritePosition(GetScreenPosition());
}

Vector2I GameObject::GetScreenPosition() const
{
	return Camera::Instance().WorldToScreen(GetPosition());
}
