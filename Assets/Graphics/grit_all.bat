setlocal
set GRIT=%NDSGAMEROOT%\Tools\grit\grit.exe

ls *.bmp | xargs -ixxx %GRIT% xxx -ff tiles16x16.grit

%GRIT% rhino.bmp -ff tiles32x32.grit
