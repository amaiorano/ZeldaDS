@echo off
rem Use to reconcile offline files for all files in this project.

setlocal
set P4_RECONCILE_CMD=%NDSGAMEROOT%\Tools\bin\p4_reconcile_offline_files.bat

echo.
echo Reconciling offline files for:

echo Assets...
cd /d %NDSGAMEROOT%\Assets\Graphics
rm -f *.h *.s
cd /d %NDSGAMEROOT%\Assets\Audio
rm -f *.h *.bin
call %P4_RECONCILE_CMD% %NDSGAMEROOT%\Assets

echo Docs...
call %P4_RECONCILE_CMD% %NDSGAMEROOT%\Docs

echo Tools...
call %P4_RECONCILE_CMD% %NDSGAMEROOT%\Tools
cd /d %NDSGAMEROOT%\Tools\desmume
p4 revert desmume.ini

echo Game...
cd /d %NDSGAMEROOT%\Game
call UpdateP4.bat nopause

echo Editor...
cd /d %NDSGAMEROOT%\Editor\Zelous
rem TODO: Add UpdateP4.bat for Zelous
rem call UpdateP4.bat nopause

echo Done!
pause
