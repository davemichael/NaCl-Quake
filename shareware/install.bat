@echo off
DEICE.EXE
if ERRORLEVEL == 1 GOTO END
RESOURCE.EXE
if ERRORLEVEL == 1 GOTO ERROR
DEL RESOURCE.EXE
echo.
echo Prepare to enter Quake!
pause
quake
goto END
:ERROR
echo Error installing Quake Shareware v1.06!
echo If you got a Write Error you have run out of hard drive space.
:END
echo.
