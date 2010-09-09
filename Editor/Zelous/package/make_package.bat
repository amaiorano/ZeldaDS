@echo off

rem This script creates a Zelous package which can be distributed to users to build maps without
rem requiring the typical developer setup.

setlocal

set ZELOUS_ROOT=%NDSGAMEROOT%\Editor\Zelous
set GAME_ROOT=%NDSGAMEROOT%\Game\ZeldaDS
set TOOLS_ROOT=%NDSGAMEROOT%\Tools

set PACKAGE_PATH=%ZELOUS_ROOT%\package

set COPY_CMD=robocopy
set COPY_OPTS=/A-:R
set COPY_RECURSE_OPTS=/E

set MAKE_WRITABLE_CMD=chmod +w


:START

echo.
echo *** Setting up msdev environment
call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
if %ERRORLEVEL% neq 0 goto :ERROR

echo.
echo *** Building Zelous
devenv %ZELOUS_ROOT%\Zelous.sln /build Debug
if %ERRORLEVEL% neq 0 goto :ERROR

echo.
echo *** Building ZeldaDS
pushd %GAME_ROOT% 
call %GAME_ROOT%\build.bat build RELEASE
if %ERRORLEVEL% neq 0 goto :ERROR
popd

echo.
echo *** Copying Zelous (editor) to package
%COPY_CMD% %ZELOUS_ROOT%\out\Zelous %PACKAGE_PATH%\Zelous %COPY_OPTS% %COPY_RECURSE_OPTS%

echo.
echo *** Copying ZeldaDS (game) to package
%COPY_CMD% %GAME_ROOT% %PACKAGE_PATH%\Zelous\game ZeldaDS.arm* %COPY_OPTS%
%COPY_CMD% %GAME_ROOT%\nitrofiles %PACKAGE_PATH%\Zelous\game\nitrofiles %COPY_OPTS% %COPY_RECURSE_OPTS%

echo.
echo *** Copying desmume (emulator) to package
%COPY_CMD% %TOOLS_ROOT%\desmume %PACKAGE_PATH%\Zelous\desmume desmume.exe %COPY_OPTS% %COPY_RECURSE_OPTS%

echo.
echo *** Copying base (scripts and config) to package
%COPY_CMD% %PACKAGE_PATH%\base %PACKAGE_PATH%\Zelous %COPY_OPTS% %COPY_RECURSE_OPTS%

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
pause
