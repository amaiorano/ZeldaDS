@echo off

setlocal
set GNU_TO_MSDEV_CMD=sed -e "s/\(\.*\):\([0-9]\+\):/\1(\2):/"
set CONFIG=%2

if "%1"=="build" goto :BUILD
if "%1"=="clean" goto :CLEAN
if "%1"=="rebuild" goto :REBUILD

:ERROR
echo Invalid input param!
goto :END

:BUILD
make CONFIG=%CONFIG% 2>&1 | %GNU_TO_MSDEV_CMD%
goto :END

:CLEAN
make clean
goto :END

:REBUILD
make clean
make CONFIG=%CONFIG% 2>&1 | %GNU_TO_MSDEV_CMD%


:END
