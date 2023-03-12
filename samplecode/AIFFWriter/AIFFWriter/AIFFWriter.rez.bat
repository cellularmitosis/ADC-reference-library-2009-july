@echo off
REM
REM AIFFWriter.rez.bat
REM
REM Construct the resource file for Windows builds and add it to the existing DLL file.
REM
REM Written by Tim Monroe
REM 25 Jan 1999
REM
QuickTimeSDK\QTDevWin\Tools\Rez -i "QuickTimeSDK\QTDevWin\RIncludes" -i . AIFFWriter.r -o AIFFWriter.qtr
QuickTimeSDK\QTDevWin\Tools\RezWack -d AIFFWriter.dll -r AIFFWriter.qtr -o AIFFWriter.qtx