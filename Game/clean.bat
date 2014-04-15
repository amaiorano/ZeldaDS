@echo off
setlocal
pushd %~dp0

set DEL_FILES=rm -f
set DEL_FOLDER=rm -rf

echo.
echo Cleaning...
@echo on
pushd ZeldaDS
call build.bat clean
call build.bat clean DEBUG
popd
@echo off

echo.
echo Deleting temporary and generated files
@echo on
%DEL_FOLDER% Debug
%DEL_FOLDER% Release
@echo off

popd
