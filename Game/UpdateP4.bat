@echo off
setlocal
pushd %~dp0

set P4_RECONCILE_CMD=%NDSGAMEROOT%\Tools\bin\p4_reconcile_offline_files.bat
set DEL_FILES=rm -f
set DEL_FOLDER=rm -rf

echo.
echo Cleaning...
pushd ZeldaDS
call build.bat clean
call build.bat clean DEBUG
popd

echo.
echo Deleting temporary and generated files
%DEL_FOLDER% Debug
%DEL_FOLDER% Release

echo.
echo Reconciling offline files...
call %P4_RECONCILE_CMD% %CD%

cd Game
echo.
echo Reverting some files I want to keep local
dir /b /a-d *.suo | p4 -x - revert 
dir /b /a-d *.user | p4 -x - revert
dir /b /a-d *.wwdb | p4 -x - revert 
dir /b /a-d *.ncb | p4 -x - revert

popd

if "%1" neq "nopause" (
	echo Done!
	pause
)
