#include "GraphicsEngine.h"
#include "gslib/Core/Core.h"
#include "gslib/Math/Lerper.h"
#include "Constants.h"
#include "BackgroundLayer.h"
#include "Sprite.h"
#include <nds/arm9/console.h>
#include <nds/arm9/sprite.h>
#include <nds/bios.h>
#include <nds/interrupts.h>
#include <vector>
#include <algorithm>
#include <string.h>

//@TODO: Tuck all this code away behind some 3d renderer class
#include <nds/arm9/videoGl.h>
//@NOTE: Using these macros generates very slow code. Unfortunately,
// not sure how to improve this as each vertex is a 4.12 fixed point
// value, so range is [0,15] for mantissa (can't use screen coords
// as orthographic dimenions)
const float OrthoWidth = 3.0f;
const float OrthoHeight = 2.0f;
#define VERTX(v) ((v) * OrthoWidth / HwScreenSizeX)
#define VERTY(v) ((v) * OrthoHeight / HwScreenSizeY)
#define VERTEX2(x, y) glVertex3f(VERTX(x), VERTY(y), -1)

void DrawQuad(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color, uint16 alpha = 31)
{
	glEnable(GL_BLEND);
	glPolyFmt(POLY_ALPHA(alpha) | POLY_CULL_BACK);

	// Draw slightly larger quad to deal with imprecision issues on DS
	glBegin(GL_QUAD);
		glColor(color);
		VERTEX2(x, y);
		VERTEX2(x, y+h);
		VERTEX2(x+w, y+h);
		VERTEX2(x+w, y);
	glEnd();
}

namespace
{
	BackgroundLayer gBgLayers[4];
	BackgroundLayer gSubBgLayers[1];
	bool gEnabled3d = false;
	Lerper<float, GameTimeType> gFadeLerper; // out (black) = 0, in = 1

	class SpriteManager
	{
	public:
		Sprite* AllocSprite()
		{
			Sprite* pSprite = new Sprite();
			ASSERT(pSprite);
			mSprites.push_back(pSprite);
			return pSprite;
		}

		void FreeSprite(Sprite*& pSprite)
		{
			SpriteList::iterator iter = std::find(mSprites.begin(), mSprites.end(), pSprite);
			ASSERT(iter != mSprites.end());
			mSprites.erase(iter);
			delete pSprite;
			pSprite = NULL;
		}

		typedef std::vector<Sprite*> SpriteList;
		SpriteList mSprites;

	} gSpriteManager;

	void Init3d()
	{
		glInit();
		//vramSetBankD(VRAM_D_TkEXTURE); // Reserve some memory for textures
		
		// Setup the rear plane
		glClearColor(31, 31, 31, 0);
		glClearPolyID(63); // 3d backdrop must have a unique polygon ID for AA to work
		glClearDepth(GL_MAX_DEPTH);
		
		// Set our viewport to be the same size as the screen
		glViewport(0, 0, HwScreenSizeX-1, HwScreenSizeY-1);
		
		// Set projection matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, OrthoWidth, OrthoHeight, 0.0f, 0.1f, 100.0f);

		// Setup BG0 to blend on top of other layers
		REG_BLDCNT = BLEND_SRC_BG0 | BLEND_DST_BG2 | BLEND_DST_BG3 | BLEND_DST_SPRITE;

		// Switch to modelview and reset
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		gEnabled3d = true;
	}

	void DrawFullScreenQuad(float blendRatio)
	{
		uint16 alpha = static_cast<uint16>(blendRatio * 31);

		DrawQuad(0, 0, HwScreenSizeX+1, HwScreenSizeY, RGB15(0, 0, 0), alpha);
/*
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glPolyFmt(POLY_ALPHA(alpha) | POLY_CULL_BACK);

		// Draw slightly larger quad to deal with imprecision issues on DS
		glBegin(GL_QUADS);
			glColor(RGB15(0, 0, 0));
			VERTEX2(0, 0);
			VERTEX2(0, HwScreenSizeY);
			VERTEX2(HwScreenSizeX+1, HwScreenSizeY);
			VERTEX2(HwScreenSizeX+1, 0);
		glEnd();
*/
	}

	void PreVBlankUpdate()
	{
		// Let sprites update OAM shadow/copy
		SpriteManager::SpriteList::iterator iter = gSpriteManager.mSprites.begin();
		for ( ; iter != gSpriteManager.mSprites.end(); ++iter)
		{
			(*iter)->PreVBlankUpdate();
		}
	}

	void WaitForVBlank()
	{
		if (gEnabled3d)
		{
			glFlush(0);
		}

		swiWaitForVBlank();
	}

	void PostVBlankUpdate()
	{
		// Let sprites update their anim pose buffer
		SpriteManager::SpriteList::iterator iter = gSpriteManager.mSprites.begin();
		for ( ; iter != gSpriteManager.mSprites.end(); ++iter)
		{
			(*iter)->PostVBlankUpdate();
		}

		// Update actual sprite OAM from shadow/copy
		oamUpdate(&oamMain);

		// Push bg values (i.e. scroll location)
		bgUpdate();
	}
}

namespace GraphicsEngine
{
	void Init(uint16 metaTileSizeX, uint16 metaTileSizeY)
	{
		// The default instance utilizes the sub display, approximatly 15KiB of vram C starting
		//at tile base 0 and 2KiB of map at map base 30. 
		//consoleDemoInit();
		//consoleDebugInit(DebugDevice_NOCASH); // Output to no$gba console

		// Set video mode
		//videoSetMode(MODE_5_2D);
		videoSetMode(MODE_5_3D);

		// Map vram for backgrounds
		vramSetBankA(VRAM_A_MAIN_BG_0x06000000); // Map 128k to vram
		vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
		vramSetBankC(VRAM_C_MAIN_BG_0x06040000);
		//vramSetBankD(VRAM_D_MAIN_BG_0x06060000);

		// Map vram for sprites
		vramSetBankE(VRAM_E_MAIN_SPRITE); // 64k @ 0x06400000
		//vramSetBankF(VRAM_F_MAIN_SPRITE_0x06400000); // 16k
		//vramSetBankG(VRAM_G_MAIN_SPRITE_0x06404000); // 16k more

		// Notes about SpriteMapping "boundary size":
		// The libnds oam functions take care of managing the sprite memory for us; we just need to specify
		// the right values to oamInit(), and the rest is taken care of behind the scenes.
		// To determine the SpriteMapping value, first figure out how much memory you need for all your sprite
		// gfx (loaded), then select a mapping that will allow you to address those tiles in sprite gfx memory.
		// Ex: 32x32 8bit sprites require 1024 bytes per sprite, so if you want to address 128 unique sprite images,
		// you need 128 * 1024 = 128k, so select at least SpriteMapping_1D_128, and make sure to map 128k of vram for sprites.

		oamInit(&oamMain, SpriteMapping_1D_64, false); // Stride is 64 bytes, so can address 64k of vram (64 unique 8bit 32x32 sprites, or 128 4bit 32x32 sprites, etc.)


		// Compute how much memory our tile maps require (these can be global constants...)
		{
			//const int tileMapBytes = (HwBgNumTilesX * HwBgNumTilesY * 2); // How much memory is required for the tile map of a single background
			//const int numTileMap2kBlocks = tileMapBytes / (2*1024); // How many 2k blocks required for the tile map of a single background
			//printf("tileMapBytes : %d\n", tileMapBytes);
			//printf("numTileMap2kBlocks : %d\n", numTileMap2kBlocks);

			// We running in mode 5 with 512x512 backgrounds with 2 byte tile indices
			// That means the tile maps are (512x512)/(8x8)*2 = 8k per background (4 tile map slots each)
			//ASSERT(numTileMap2kBlocks == 4);
		}

		// Create sub console text layer
		{
			// This is pretty much what consoleDemoInit() does, except we allocate and manage our own PrintConsole
			videoSetModeSub(MODE_0_2D);
			vramSetBankC(VRAM_C_SUB_BG);
			PrintConsole* pSubConsole = new PrintConsole();
			consoleInit(pSubConsole, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
			gSubBgLayers[0].InitConsole(pSubConsole);
			ASSERT(GetSubBgLayer(0).IsTextLayer());
		}

		// NOTE:
		// tileMapBase is the 2k offset into vram where tile map will be placed (range is [0,31])
		// tileGfxBase is the 16k offset into vram where tile gfx will be placed (range is [0,31])

		// We're running 512x512 bgs with 2 byte indices => each tile map therefore requires 8k or 4 slots ((512x512)/(8x8)*2 = 8k)
		// and we allow ourselves tile gfx of 64k max (4 slots) per bg -> 16 slots for all bgs.
		// So with 4 bgs, that's 8k => 4 slots * 4 = 16 slots (out of 32), so tile gfx must start at slot 2
		// Altogether, that means we need a total of 2 (maps) + 16 (gfx) 16k slots = 18 * 16k = 288k (3 128k ram banks)

		// 4 slots * 16k = 64k for tile image data per map, and 16x16x2 = 512 bytes per tile image,
		// so NumTilesPerMap = 64k / 512 bytes = 128

		// BG0
		{
			// 3D
			{
				Init3d();
				int bg0 = 0;
				gBgLayers[0].Init3d(bg0);
			}

			// 2D tiled
			{
				//const int tileMapBase = 0;
				//const int tileGfxBase = 10;//2;
				//int bg0 = bgInit(0, BgType_Text8bpp, TextBgInitSize, tileMapBase, tileGfxBase);
				//gBgLayers[0].InitTiled(bg0, metaTileSizeX, metaTileSizeY);
			}

			// 2D console
			{
				// Default font: 256 characters, 8x8 per char, 4 bits per char -> 256 * 8*8 / 2 = 8k for gfx
				//PrintConsole* pBg0Console = new PrintConsole();
				//consoleInit(pBg0Console, 0, BgType_Text4bpp, BgSize_T_256x256, 0, 2, true, true);
				//gBgLayers[0].InitConsole(pBg0Console);
				//ASSERT(GetBgLayer(0).IsTextLayer());
			}
		}

		// BG1
		{
			const int tileMapBase = 4;
			const int tileGfxBase = 6;
			int bg1 = bgInit(1, BgType_Text8bpp, TextBgInitSize, tileMapBase, tileGfxBase);
			ASSERT(bg1 >= 0);
			gBgLayers[1].InitTiled(bg1, metaTileSizeX, metaTileSizeY);
		}

		// BG2
		{
			const int tileMapBase = 8;
			const int tileGfxBase = 10;
			int bg2 = bgInit(2, BgType_ExRotation, ExRotBgInitSize, tileMapBase, tileGfxBase);
			ASSERT(bg2 >= 0);
			gBgLayers[2].InitTiled(bg2, metaTileSizeX, metaTileSizeY);
		}

		// BG3
		{
			const int tileMapBase = 12;
			const int tileGfxBase = 14;
			int bg3 = bgInit(3, BgType_ExRotation, ExRotBgInitSize, tileMapBase, tileGfxBase);
			ASSERT(bg3 >= 0);
			gBgLayers[3].InitTiled(bg3, metaTileSizeX, metaTileSizeY);
		}

		// By default, all layers have priority 0, which is weird and useless, so we set them
		// to a reasonable default: BG0 above BG1, BG1 above BG2, etc.
		for (uint16 i = 0; i < NUM_ARRAY_ELEMS(gBgLayers); ++i)
		{
			gBgLayers[i].SetPriority(i); // bg0 -> 0, bg1 -> 1, etc.
		}
		for (uint16 i = 0; i < NUM_ARRAY_ELEMS(gSubBgLayers); ++i)
		{
			gSubBgLayers[i].SetPriority(i);
		}

		// Initially all bgs hidden
		SetAllBgsEnabled(false);

		gFadeLerper.Reset(0.0f, 1.0f, 1.0f, 1.0f, 0.0f);
	}

	void SetAllBgsEnabled(bool enabled)
	{
		for (uint16 i = 0; i < NUM_ARRAY_ELEMS(gBgLayers); ++i)
		{
			gBgLayers[i].SetEnabled(enabled);
		}
	}

	void LoadBgPalette(const void* pPalette, uint16 sizeBytes)
	{
		ASSERT(sizeBytes == 256 * 2); // For Mode 5, we expect a 16 bit palette of 256 colors
		memcpy(BG_PALETTE, pPalette, sizeBytes); //@TODO: use dmaCopy?
	}

	void LoadSubBgPalette(const void* pPalette, uint16 sizeBytes)
	{
		memcpy(BG_PALETTE_SUB, pPalette, sizeBytes); //@TODO: use dmaCopy?
	}

	void LoadSpritePalette(const uint16* pPalette, uint16 sizeBytes)
	{
		ASSERT(sizeBytes == 512); // Only supporting 8 bit sprites for now...
		dmaCopy(pPalette, SPRITE_PALETTE, sizeBytes);
	}

	void LoadSubSpritePalette(const uint16* pPalette, uint16 sizeBytes)
	{
		FAIL(); // Not implemented yet
	}

	void SetBgFontColor(uint16 color)
	{
		BG_PALETTE[255] = color;
	}

	void SetSubBgFontColor(uint16 color)
	{
		BG_PALETTE_SUB[255] = color;
	}

	uint16* GetBgPalette()
	{
		return BG_PALETTE;
	}

	uint16* GetSubBgPalette()
	{
		return BG_PALETTE_SUB;
	}

	BackgroundLayer& GetBgLayer(uint8 layer)
	{
		ASSERT(layer >= 0 && layer <= 3);
		return gBgLayers[layer];
	}

	BackgroundLayer& GetSubBgLayer(uint8 layer)
	{
		ASSERT(layer==0); // Currently only supporting Bg 0 text layer
		return gSubBgLayers[0];
	}

	Sprite* AllocSprite()
	{
		return gSpriteManager.AllocSprite();
	}

	void FreeSprite(Sprite*& pSprite)
	{
		gSpriteManager.FreeSprite(pSprite);
	}

	void FadeScreen(FadeScreenDir::Type dir, GameTimeType timeToFade)
	{
		gFadeLerper.Reset(dir == FadeScreenDir::Out? -1.0f : 1.0f, timeToFade);
	}

	bool IsFadingScreen()
	{
		return !gFadeLerper.IsDone();
	}

	float GetFadeRatio()
	{
		return gFadeLerper.GetCurr();
	}

	void Update(GameTimeType deltaTime)
	{
		gFadeLerper.Update(deltaTime);
	}

	void Render(GameTimeType deltaTime)
	{
		// Draw the quad as long as we're not fully faded in
		const float fadeRatio = gFadeLerper.GetCurr();
		if (fadeRatio < 1.0f)
		{
			DrawFullScreenQuad(1.0f - fadeRatio);
		}

		PreVBlankUpdate();
		WaitForVBlankAndPostVBlankUpdate();
	}

	void WaitForVBlankAndPostVBlankUpdate()
	{
		WaitForVBlank();
		PostVBlankUpdate();
	}

} // namespace GraphicsEngine