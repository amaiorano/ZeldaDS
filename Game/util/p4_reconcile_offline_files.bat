@echo off
rem Author: Antonio Maiorano
rem Date: January 2009
rem Required: Perforce command line, cygwin (cut)
rem See http://kb.perforce.com/UserTasks/WorkingDisconnected

rem setlocal

if "%1"=="" goto :USAGE

goto :GET_P4_RECONCILE_PATH

:USAGE
echo.
echo Usage: p4_reconcile_offline_files [root path]
echo.
goto :END


:GET_P4_RECONCILE_PATH
pushd %1

rem Run p4 where to see if the input path is under the client root
p4 where
if %ERRORLEVEL% neq 0 goto :USAGE

FOR /F "usebackq delims=`" %%i IN (`p4 where ^| cut -d ' ' -f 1`) DO set P4_RECONCILE_PATH=%%i
if %ERRORLEVEL% neq 0 goto :USAGE

if "%P4_RECONCILE_PATH%"=="" goto :USAGE

:DO_RECONCILIATION

echo ==========================================================================
echo Opening for "delete" any old files no longer present:
echo ==========================================================================
p4 diff -sd %P4_RECONCILE_PATH% | p4 -x - delete

echo ==========================================================================
echo Opening for edit any files that have changed:
echo ==========================================================================
p4 diff -se %P4_RECONCILE_PATH% | p4 -x - edit

echo ==========================================================================
echo Opening for add any new files
echo ==========================================================================
dir /s /b /a-d | p4 -x - add

echo ==========================================================================
echo Reverting unchanged files
echo ==========================================================================
p4 revert -a %P4_RECONCILE_PATH%

:END
popd
