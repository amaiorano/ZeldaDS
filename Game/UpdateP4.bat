@echo off
setlocal
set DEL_FILES=rm -f
set DEL_FOLDER=rm -rf

echo.
echo Cleaning...
pushd ZeldaDS
call build.bat clean
popd

echo.
echo Deleting temporary and generated files
rem %DEL_FILES% *.suo
rem %DEL_FILES% *.user
%DEL_FILES% *.ncb
%DEL_FILES% *.wwdb
%DEL_FOLDER% Debug
%DEL_FOLDER% Release

echo.
echo Reconciling offline files...
call util\p4_reconcile_offline_files.bat %CD%

echo.
echo Reverting some files I want to keep local
dir /b /a-d *.suo | p4 -x - revert 
dir /b /a-d *.user | p4 -x - revert
