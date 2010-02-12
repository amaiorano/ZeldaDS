#ifndef HW_CONSTANTS_H
#define HW_CONSTANTS_H

#include "gslib/Core/Types.h"
#include <nds/arm9/background.h>

// Hardware constants
const uint16 HwTileSizeX				= 8;
const uint16 HwTileSizeY				= 8;
const uint16 HwScreenSizeX				= 256;
const uint16 HwScreenSizeY				= 192;
const uint16 HwTextBgQuadrantNumTilesX	= 32;
const uint16 HwTextBgQuadrantNumTilesY	= 32;

//@TODO: These are not really hardware constants, more like game constants.
// Get rid of these because, in any case, we want backgrounds to support
// different dimensions (so push some down in BackgroundLayer as variables and
// others into WorldMap.h as game constants)
const BgSize ExRotBgInitSize	= BgSize_ER_512x512; // -> Remove
const BgSize TextBgInitSize		= BgSize_T_512x512; // -> Remove
const uint16 HwBgSizeX			= 512; // -> Game constant GameWorldMapBgSizeX
const uint16 HwBgSizeY			= 512; // -> Game constant GameWorldMapBgSizeY

const uint16 HwBgNumTilesX		= (HwBgSizeX / HwTileSizeX); // -> If we care, make game constant GameWorldMapNumBgTilesX AND variable in BackgroundLayer
const uint16 HwBgNumTilesY		= (HwBgSizeY / HwTileSizeY); // -> If we care, make game constant GameWorldMapNumBgTilesY AND variable in BackgroundLayer

// In Text modes, tile maps are divided into 32x32 tile quadrants
const uint16 HwTextBgNumQuadrantsX = (HwBgNumTilesX / HwTextBgQuadrantNumTilesX); // -> Variable in BackgroundLayer
const uint16 HwTextBgNumQuadrantsY = (HwBgNumTilesY / HwTextBgQuadrantNumTilesY); // -> Variable in BackgroundLayer


#endif // HW_CONSTANTS_H
