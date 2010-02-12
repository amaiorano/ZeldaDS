#include "Sprite.h"
#include <nds/dma.h>

Sprite::Sprite()
	: mWidth(0)
	, mHeight(0)
	, mpOamState(0)
	, mId(~0)
	, mpSpriteGfxMem(0)	
{
}

Sprite::~Sprite()
{
	if (mpSpriteGfxMem)
	{
		oamClear(&oamMain, mId, 1); // Seems we need to clear (disable) the sprite manually or it continues to render
		oamFreeGfx(&oamMain, mpSpriteGfxMem);
	}
}

void Sprite::Init(uint16 width, uint16 height, SpriteSize spriteSize, SpriteColorFormat spriteColorFormat, bool bMainScreen)
{
	ASSERT_MSG(!mpOamState, "Cannot Sprite::Init twice!");

	mWidth = width;
	mHeight = height;
	mSpriteSize = spriteSize;
	mSpriteColorFormat = spriteColorFormat;
	mpOamState = bMainScreen? &oamMain : &oamSub;

	//@TODO: validate SpriteSize against width and height (if we can do that, can probably just get rid of passing in SpriteSize...)
	ASSERT(mSpriteColorFormat == SpriteColorFormat_16Color || mSpriteColorFormat == SpriteColorFormat_256Color);

	//@TODO: Move getting a unique sprite id into a SpriteManager
	static volatile uint16 sSpriteId = 0;
	mId = sSpriteId;
	++sSpriteId;

	// Alloc a single block for our gfx - we will copy the current frame to vram when we need to
	mpSpriteGfxMem = oamAllocateGfx(mpOamState, mSpriteSize, mSpriteColorFormat);
	ASSERT(mpSpriteGfxMem);

	PreVBlankUpdate(); // Call once to set to default values
}

void Sprite::PreVBlankUpdate()
{
	//@TODO: lazy update on dirty flag
	UpdateOamShadow();
}

void Sprite::PostVBlankUpdate()
{
	//@TODO: lazy update on dirty flag
	if (mProperties.show)
	{
		UpdateAnimPose();
	}
}

void Sprite::UpdateOamShadow()
{
	// Update OAM shadow with latest property values
	oamSet(&oamMain, mId, mProperties.pos.x, mProperties.pos.y, mProperties.priority, 0, mSpriteSize, mSpriteColorFormat,
		mpSpriteGfxMem, -1, false, !mProperties.show, mProperties.horzFlip, mProperties.vertFlip, mProperties.mosaic);
}

void Sprite::UpdateAnimPose()
{
	DBG_STATEMENT(const uint16 frameSize = (mSpriteColorFormat == SpriteColorFormat_16Color? mWidth*mHeight/2 : mWidth*mHeight));
	ASSERT_MSG(mTargetAnimPose.frameSize == frameSize, "Likely reasons: haven't called SetAnimPose() yet, or are passing in an invalid pose");

	dmaCopy(mTargetAnimPose.pFrameGfx, mpSpriteGfxMem, mTargetAnimPose.frameSize);
}
