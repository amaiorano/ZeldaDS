#include "IRenderable.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Hw/GraphicsEngine.h"

IRenderable::IRenderable()
	: mpSprite(0)
{
}

IRenderable::~IRenderable()
{
}

void IRenderable::Activate(uint16 width, uint16 height)
{
	mpSprite = GraphicsEngine::AllocSprite();
	mpSprite->Init(width, height, SpriteSize_16x16, SpriteColorFormat_256Color);
}

void IRenderable::Deactivate()
{
	GraphicsEngine::FreeSprite(mpSprite);
}

void IRenderable::UpdateSpritePosition(const Vector2I& pos)
{
	mpSprite->Properties().pos = pos;
}

uint16 IRenderable::GetWidth() const
{
	return mpSprite->GetWidth();
}

uint16 IRenderable::GetHeight() const
{
	return mpSprite->GetHeight();
}
