setlocal
set GRIT=%NDSGAMEROOT%\Tools\grit\grit.exe

rem No easy way to tell grit to output to a specified folder, so we go into the target
rem folder, and run the commands relative to this folder
md output
cd output

rem TODO: Consider renaming 16x16 files to *_16x16.bmp and simplify this command 
ls ../*.bmp | grep "_32x32.bmp" -v | xargs -ixxx %GRIT% xxx -ff ../tiles16x16.grit

ls ../*_32x32.bmp | xargs -ixxx %GRIT% xxx -ff ../tiles32x32.grit
