@echo off
REM
REM MyRezWackDebug.bat
REM
REM Construct the resource file for Windows builds and add it to the existing application file.
REM
REM
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" AudioConvert.r -o AudioConvert.qtr
..\..\QTDevWin\Tools\RezWack -d .\Debug\AudioConvert.exe -r AudioConvert.qtr -o .\Debug\TEMP.exe
del .\Debug\AudioConvert.exe
ren .\Debug\TEMP.exe AudioConvert.exe