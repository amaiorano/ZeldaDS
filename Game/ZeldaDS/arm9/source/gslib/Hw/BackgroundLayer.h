#ifndef GS_BACKGROUND_RENDERER_H
#define GS_BACKGROUND_RENDERER_H

#include "gslib/Core/Core.h"

//@LAME: Want to forward declare PrintConsole, but it's a typedef'd anonymous struct, so I can't forward declare it and
// am forced to include the header here :(
//#include <nds/arm9/console.h>
struct PrintConsole;

class BackgroundLayer
{
public:
	BackgroundLayer();
	~BackgroundLayer();

	// Tile layer functions
	void InitTiled(int bgId, uint8 metaTileSizeX, uint8 metaTileSizeY);
	void LoadTilesImage(const void* pImage, uint16 sizeBytes);
	void DrawTile(uint16 metaTileIndex, uint16 metaTileMapX, uint16 metaTileMapY);

	// Console layer functions
	void InitConsole(PrintConsole* pConsole); // Transfers ownership of pointer (this class will delete it)
	bool IsTextLayer() const;
	void ActivateTextLayer(); // Call so subsequent iprintfs render to this layer
	void SetTextWindow(int tileX, int tileY, int numTilesX, int numTilesY);
	void ClearText();

	// 3D layer functions
	void Init3d(int bgId);

	void SetEnabled(bool enabled);

	void SetPriority(int priority);

	// Need to call GraphicsEngine::Update() for scroll values to take effect
	void SetScroll(int x, int y);

private:
	int mBgId;
	PrintConsole* mpConsole;

	uint8* mpTileMap;
	uint8 mMetaTileSizeX;
	uint8 mMetaTileSizeY;

	// Cached constants
	uint8 mNumHwTilesPerMetaTileX;
	uint8 mNumHwTilesPerMetaTileY;
};

#endif // GS_BACKGROUND_RENDERER_H
