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
%DEL_FILES% *.ncb
%DEL_FILES% *.suo
%DEL_FILES% *.user
%DEL_FILES% *.wwdb
%DEL_FOLDER% Debug
%DEL_FOLDER% Release

echo.
echo Reconciling offline files...
call util\p4_reconcile_offline_files.bat %CD%
pause
