@echo off

rem This script creates a Zelous package which can be distributed to users to build maps without
rem requiring the typical developer setup.

setlocal
pushd %~dp0

set ZELOUS_ROOT=%NDSGAMEROOT%/Editor/Zelous
set GAME_ROOT=%NDSGAMEROOT%/Game/ZeldaDS
set TOOLS_ROOT=%NDSGAMEROOT%/Tools
set PACKAGE_PATH=%ZELOUS_ROOT%/package
set VC_VARS_CMD="C:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/vcvarsall.bat"

set DEL_FILES_CMD=rm -f
set DEL_FOLDER_CMD=rm -rf
set MAKE_FOLDER_CMD=mkdir
set COPY_DIR_CMD=robocopy
set COPY_DIR_OPTS=/A-:R
set COPY_DIR_RECURSE_OPTS=/E
set MAKE_WRITABLE_CMD=chmod +w


:START

echo.
echo *** Cleaning
%DEL_FILES_CMD% Zelous.zip
%DEL_FOLDER_CMD% Zelous

if "%1"=="clean" goto :END

echo.
echo *** Setting up msdev environment
@echo on
call %VC_VARS_CMD%
@echo off
if %ERRORLEVEL% neq 0 goto :ERROR

echo.
echo *** Building Zelous
@echo on
devenv %ZELOUS_ROOT%/Zelous.sln /build Debug
@echo off
if %ERRORLEVEL% neq 0 goto :ERROR

echo.
echo *** Building ZeldaDS
pushd %GAME_ROOT% 
call %GAME_ROOT%/build.bat build RELEASE
if %ERRORLEVEL% neq 0 goto :ERROR
popd

echo.
echo *** Copying Zelous executable and data to package
@echo on
%MAKE_FOLDER_CMD% Zelous
%COPY_DIR_CMD% %ZELOUS_ROOT%/bin/Debug/ %PACKAGE_PATH%/Zelous Zelous.exe %COPY_DIR_OPTS%
%COPY_DIR_CMD% %ZELOUS_ROOT%/data %PACKAGE_PATH%/Zelous %COPY_DIR_OPTS% %COPY_DIR_RECURSE_OPTS%
@echo off

echo.
echo *** Copying ZeldaDS (game) to package
@echo on
%COPY_DIR_CMD% %GAME_ROOT% %PACKAGE_PATH%/Zelous/game ZeldaDS.arm* %COPY_DIR_OPTS%
%COPY_DIR_CMD% %GAME_ROOT%/nitrofiles %PACKAGE_PATH%/Zelous/game/nitrofiles %COPY_DIR_OPTS% %COPY_DIR_RECURSE_OPTS%
@echo off

echo.
echo *** Copying desmume (emulator) to package
@echo on
%COPY_DIR_CMD% %TOOLS_ROOT%/desmume %PACKAGE_PATH%/Zelous/desmume desmume.exe %COPY_DIR_OPTS% %COPY_DIR_RECURSE_OPTS%
@echo off

echo.
echo *** Copying base (scripts and config) to package
@echo on
%COPY_DIR_CMD% %PACKAGE_PATH%/base %PACKAGE_PATH%/Zelous %COPY_DIR_OPTS% %COPY_DIR_RECURSE_OPTS%
@echo off

echo.
rem echo *** Zipping up package
rem TODO: ZIP

echo.
echo *** Success!
goto :END

:ERROR
echo.
echo *** Error encountered (ERRORLEVEL is %ERRORLEVEL%), bailing...

:END
popd
