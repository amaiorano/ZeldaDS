rem This script builds the graphics data needed for ZeldaDS. After running it,
rem copy the files in the output directory to Game/ZeldaDS/nitrofiles/Graphics/
rem and also to Editor/Zelous/data

setlocal
pushd %~dp0

set GRIT=%NDSGAMEROOT%\Tools\grit\grit.exe
set TGT_PATH=output
set SRC_PATH=..

rem Delete and recreate output folder
rm -rf %TGT_PATH%
md %TGT_PATH%

rem No easy way to tell grit to output to a specified folder, so we go into the target
rem folder, and run the commands relative to this folder
cd %TGT_PATH%

rem TODO: Consider renaming 16x16 files to *_16x16.bmp and simplify this command 
rem ls ../*.bmp | grep "_32x32.bmp" -v | xargs -ixxx %GRIT% xxx -ff ../tiles16x16.grit
rem ls ../*_32x32.bmp | xargs -ixxx %GRIT% xxx -ff ../tiles32x32.grit

rem Export palette data (.pal)
%GRIT% %SRC_PATH%/characters.bmp -ff %SRC_PATH%/tiles_palette.grit -o sprite_palette
%GRIT% %SRC_PATH%/overworld_bg.bmp -ff %SRC_PATH%/tiles_palette.grit -o tile_palette

rem Export image data (.img)
%GRIT% %SRC_PATH%/items.bmp -ff %SRC_PATH%/tiles16x16.grit

%GRIT% %SRC_PATH%/characters.bmp -ff %SRC_PATH%/tiles16x16.grit
%GRIT% %SRC_PATH%/characters_32x32.bmp -ff %SRC_PATH%/tiles32x32.grit

%GRIT% %SRC_PATH%/overworld_bg.bmp -ff %SRC_PATH%/tiles16x16.grit
%GRIT% %SRC_PATH%/overworld_fg.bmp -ff %SRC_PATH%/tiles16x16.grit

%GRIT% %SRC_PATH%/dungeon_bg.bmp -ff %SRC_PATH%/tiles16x16.grit
%GRIT% %SRC_PATH%/dungeon_fg.bmp -ff %SRC_PATH%/tiles16x16.grit

rem Now remove the bin extension
rename *.bin *.

popd
