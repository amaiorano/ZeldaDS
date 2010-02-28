#ifndef IRENDERABLE_H
#define IRENDERABLE_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Types.h"
#include "gslib/Math/Vector2.h"

class Sprite;

class IRenderable
{
protected:
	IRenderable();	
	virtual ~IRenderable();

	void Activate(uint16 width, uint16 height);
	void Deactivate();

	//@NOTE: Should probably have a Render() function that is called
	// once per frame on all IRenderables. Right now, child class is
	// responsible for calling UpdateSpritePosition(), which is not very clear...
	//@NOTE: Sprite position is in screen-space (hardware position)
	void UpdateSpritePosition(const Vector2I& pos);

public:
	Sprite& GetSprite() { ASSERT(mpSprite); return *mpSprite; }
	const Sprite& GetSprite() const { ASSERT(mpSprite); return *mpSprite; }

	// Convenient getters
	uint16 GetWidth() const;
	uint16 GetHeight() const;
	
protected:
	Sprite* mpSprite;
};

#endif // IRENDERABLE_H
