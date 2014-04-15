@echo off
setlocal
pushd %~dp0

:START

set DEL_FILES=rm -f
set DEL_FOLDER=rm -rf

echo.
echo Deleting temporary and generated files
@echo on
%DEL_FOLDER% bin
%DEL_FOLDER% obj
call package/make_package.bat clean
@echo off

:END
popd

