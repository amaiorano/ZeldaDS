@echo off
setlocal
pushd %~dp0

set CONFIG=%2

if "%1"=="build" goto :BUILD
if "%1"=="clean" goto :CLEAN
if "%1"=="rebuild" goto :REBUILD

:ERROR
echo Invalid input param!
goto :END

:BUILD
make CONFIG=%CONFIG%
goto :POSTBUILD

:CLEAN
make clean CONFIG=%CONFIG%
goto :END

:REBUILD
make clean CONFIG=%CONFIG%
make CONFIG=%CONFIG%
goto :POSTBUILD

:POSTBUILD
make printsize CONFIG=%CONFIG%
goto :END

:END
popd
