@echo off
REM
REM QDrawMHdlr.rez.bat
REM
REM Construct the resource file for Windows builds and add it to the existing DLL file.
REM
REM Written by Tim Monroe
REM 25 Jan 1999
REM
QuickTimeSDK\QTDevWin\Tools\Rez -i "QuickTimeSDK\QTDevWin\RIncludes" -i . QDrawHandler.r -o QDrawHandler.qtr
QuickTimeSDK\QTDevWin\Tools\RezWack -d QDrawMHdlr.dll -r QDrawHandler.qtr -o QDrawMHdlr.qtx