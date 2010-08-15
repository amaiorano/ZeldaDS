#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include "gslib/Core/Core.h"

class BackgroundLayer;
class Sprite;

namespace FadeScreenDir
{
	enum Type { In, Out };
}

//@TODO: Replace with proper Color class or something
#ifndef RGB15
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))
#endif

namespace GraphicsEngine
{
	// Inits video mode, sets up backgrounds, etc.
	void Init(uint16 metaTileSizeX, uint16 metaTileSizeY);
	bool IsInitialized();

	void SetAllBgsEnabled(bool enabled);

	void LoadBgPalette(const void* pPalette, uint16 sizeBytes);
	void LoadSubBgPalette(const void* pPalette, uint16 sizeBytes);
	void LoadSpritePalette(const uint16* pPalette, uint16 sizeBytes);
	void LoadSubSpritePalette(const uint16* pPalette, uint16 sizeBytes);

	// Sets font color for the libnds default font (which uses the 255th palette entry)
	void SetBgFontColor(uint16 color);
	void SetSubBgFontColor(uint16 color);

	uint16* GetBgPalette();
	uint16* GetSubBgPalette();

	BackgroundLayer& GetBgLayer(uint8 layer);
	BackgroundLayer& GetSubBgLayer(uint8 layer);

	// Must be used to create/destroy Sprite instances
	//@TODO: sprite pooling by priority groups
	Sprite* AllocSprite();
	void FreeSprite(Sprite*& pSprite);

	//@TODO: Add color param (write Color class?)
	void FadeScreen(FadeScreenDir::Type dir, GameTimeType timeToFade);
	bool IsFadingScreen();
	float GetFadeRatio();

	void Update(GameTimeType deltaTime);
	void Render(GameTimeType deltaTime);
	
	// Shouldn't be exposed, but required for hack in ScrollingMgr
	void WaitForVBlankAndPostVBlankUpdate();

} // namespace GraphicsEngine

#endif // GRAPHICS_ENGINE_H
