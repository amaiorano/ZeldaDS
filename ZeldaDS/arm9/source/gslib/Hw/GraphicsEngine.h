#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <nds/ndstypes.h>

class BackgroundLayer;
class Sprite;

namespace GraphicsEngine
{
	// Inits video mode, sets up backgrounds, etc.
	void Init(uint16 metaTileSizeX, uint16 metaTileSizeY);

	void LoadBgPalette(const void* pPalette, uint16 sizeBytes);
	void LoadSubBgPalette(const void* pPalette, uint16 sizeBytes);
	void LoadSpritePalette(const uint16* pPalette, uint16 sizeBytes);
	void LoadSubSpritePalette(const uint16* pPalette, uint16 sizeBytes);

	// Sets font color for the libnds default font (which uses the 255th palette entry)
	void SetBgFontColor(uint16 color);
	void SetSubBgFontColor(uint16 color);

	BackgroundLayer& GetBgLayer(uint8 layer);
	BackgroundLayer& GetSubBgLayer(uint8 layer);

	// Must be used to create/destroy Sprite instances
	//@TODO: sprite pooling by priority groups
	Sprite* AllocSprite();
	void FreeSprite(Sprite*& pSprite);

	void PreVBlankUpdate();
	void WaitForVBlank();
	void PostVBlankUpdate();

} // namespace GraphicsEngine

#endif // GRAPHICS_ENGINE_H
