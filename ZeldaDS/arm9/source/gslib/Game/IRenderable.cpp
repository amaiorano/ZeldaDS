#include "IRenderable.h"
#include "gslib/Hw/Sprite.h"
#include "gslib/Hw/GraphicsEngine.h"
#include "gslib/Game/WorldMap.h"

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

// Call once per frame from child
void IRenderable::UpdateSpritePosition(const Vector2I& pos)
{
	mpSprite->Properties().pos = WorldMap::Instance().WorldToScreen(pos);
}
