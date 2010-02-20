#ifndef SPRITE_H
#define SPRITE_H

#include "gslib/Core/Core.h"
#include "gslib/Math/Vector2.h"
#include "gslib/Anim/AnimClientTypes.h"
#include <nds/arm9/sprite.h> // Required for SpriteSize and SpriteColorFormat enums

struct AnimPoseType;

// Represents a hardware-renderable sprite
class Sprite
{
public:
	Sprite();
	~Sprite();	

	void Init(uint16 width, uint16 height, SpriteSize spriteSize, SpriteColorFormat spriteColorFormat, bool bMainScreen = true);

	struct OamProperties
	{
		OamProperties() : pos(InitZero), priority(2), show(true), horzFlip(false), vertFlip(false), mosaic(false)
		{
		}

		Vector2I pos;
		uint16 priority; // The bg layer priority to render above
		bool show;
		bool horzFlip;
		bool vertFlip;
		bool mosaic;
	};

	OamProperties& Properties() { return mProperties; }
	const OamProperties& Properties() const { return mProperties; }

	uint16 GetWidth() const		{ return mWidth; }
	uint16 GetHeight() const	{ return mHeight; }

	void SetAnimPose(const AnimPoseType& pose) { mTargetAnimPose = pose; }

	void PreVBlankUpdate();
	void PostVBlankUpdate();

private:
	void UpdateOamShadow(); // Should be called pre vblank so that when oamUpdate() is called, new values will be reflected
	void UpdateAnimPose(); // Should be called during vblank (to avoid tearing)

	uint16 mWidth, mHeight;
	SpriteSize mSpriteSize;
	SpriteColorFormat mSpriteColorFormat;
	OamState* mpOamState;

	uint16 mId; // [0,127]
	uint16* mpSpriteGfxMem;			// Points to sprite memory
	OamProperties mProperties;		// OAM local cache
	AnimPoseType mTargetAnimPose;	// AnimPose local cache
};

#endif // SPRITE_H
