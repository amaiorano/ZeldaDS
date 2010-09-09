setlocal
set TARGET=ZeldaDS

ndstool	-c %TARGET%.nds -7 %TARGET%.arm7 -9 %TARGET%.arm9 -d nitrofiles
