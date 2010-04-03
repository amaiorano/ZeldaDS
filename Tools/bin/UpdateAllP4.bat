@echo off
rem Use to reconcile offline files for all files in this project.

setlocal
set P4_RECONCILE_CMD=%NDSGAMEROOT%\Tools\bin\p4_reconcile_offline_files.bat

echo.
echo Reconciling offline files for:

echo Assets...
call %P4_RECONCILE_CMD% %NDSGAMEROOT%\Assets

echo Docs...
call %P4_RECONCILE_CMD% %NDSGAMEROOT%\Docs

echo Tools...
call %P4_RECONCILE_CMD% %NDSGAMEROOT%\Tools

echo Game...
pushd %NDSGAMEROOT%\Game
call UpdateP4.bat nopause
popd

echo Editor...
pushd %NDSGAMEROOT%\Editor\Zelous
rem TODO: Add UpdateP4.bat for Zelous
rem call UpdateP4.bat nopause
popd

echo Done!
pause
