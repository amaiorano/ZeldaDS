@echo off
setlocal
pushd %~dp0

:START

set P4_RECONCILE_CMD=%NDSGAMEROOT%\Tools\bin\p4_reconcile_offline_files.bat
set DEL_FILES=rm -f
set DEL_FOLDER=rm -rf

echo.
echo Deleting temporary and generated files
%DEL_FOLDER% bin
%DEL_FOLDER% obj
call package\make_package.bat clean

echo.
echo Reconciling offline files...
call %P4_RECONCILE_CMD% %CD%

echo.
echo Reverting some files I want to keep local
dir /b /a-d *.suo | p4 -x - revert 
dir /b /a-d *.user | p4 -x - revert
dir /b /a-d *.wwdb | p4 -x - revert 


if "%1" neq "nopause" (
	echo Done!
	pause
)

:END
popd
