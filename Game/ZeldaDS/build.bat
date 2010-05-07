@echo off

setlocal
set CONFIG=%2

if "%1"=="build" goto :BUILD
if "%1"=="clean" goto :CLEAN
if "%1"=="rebuild" goto :REBUILD

:ERROR
echo Invalid input param!
goto :END

:BUILD
make CONFIG=%CONFIG%
goto :END

:CLEAN
make clean CONFIG=%CONFIG%
goto :END

:REBUILD
make clean CONFIG=%CONFIG%
make CONFIG=%CONFIG%

:END
