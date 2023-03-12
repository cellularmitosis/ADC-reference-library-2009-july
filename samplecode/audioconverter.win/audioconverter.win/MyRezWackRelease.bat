@echo off
REM
REM MyRezWackRelease.bat
REM
REM Construct the resource file for Windows builds and add it to the existing application file.
REM
REM
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . AudioConvert.r -o AudioConvert.qtr
..\..\QTDevWin\Tools\RezWack -d .\Release\AudioConvert.exe -r AudioConvert.qtr -o .\Release\TEMP.exe
del .\Release\AudioConvert.exe
ren .\Release\TEMP.exe AudioConvert.exe