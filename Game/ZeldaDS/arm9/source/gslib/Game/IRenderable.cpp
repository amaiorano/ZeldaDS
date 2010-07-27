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
	ASSERT(mpSprite == 0);
	mpSprite = GraphicsEngine::AllocSprite();
	
	SpriteSize spriteSize = SpriteSize_16x16;
	if (width == 16 && height == 16)
	{
		spriteSize = SpriteSize_16x16;
	}
	else if (width == 32 && height == 32)
	{
		spriteSize = SpriteSize_32x32;
	}
	else
	{
		FAIL_MSG("Unhandled sprite size");
	}
	
	mpSprite->Init(width, height, spriteSize, SpriteColorFormat_256Color);
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
