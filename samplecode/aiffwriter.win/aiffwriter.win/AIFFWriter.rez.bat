@echo off
REM
REM AIFFWriter.rez.bat
REM
REM Construct the resource file for Windows builds and add it to the existing DLL file.
REM
REM Written by Tim Monroe
REM 08 March 1999
REM
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . AIFFWriter.r -o .\Release\AIFFWriter.qtr
..\..\QTDevWin\Tools\RezWack -d .\Release\AIFFWriter.dll -r .\Release\AIFFWriter.qtr -o .\Release\AIFFWriter.qtx

..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . AIFFWriter.r -o .\Debug\AIFFWriter.qtr
..\..\QTDevWin\Tools\RezWack -d .\Debug\AIFFWriter.dll -r .\Debug\AIFFWriter.qtr -o .\Debug\AIFFWriter.qtx