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
rem Pipe output of stderr and stdout to sed to format errors for Visual Studio. Unfortunately sed overwrites the return
rem status of make, so we tee the output to a temp file and grep it to see if any errors occured.
make CONFIG=%CONFIG% 2>&1 |tee %TEMP%\zdsbuildoutput.txt | sed -e 's@\(.[a-zA-Z]\+\):\([0-9]\+\):\([0-9]\+\):@\1(\2,\3):@g'
grep "error:" "%TEMP%\zdsbuildoutput.txt"
if not %errorlevel%==0 goto :POSTBUILD
goto :END

:CLEAN
make clean CONFIG=%CONFIG%
goto :END

:REBUILD
make clean CONFIG=%CONFIG%
goto :BUILD

:POSTBUILD
make printsize CONFIG=%CONFIG%
goto :END

:END
popd
