@echo off
REM
REM AudioConvert.rez.bat
REM
REM Construct the resource file for Windows builds and add it to the existing EXE file.
REM
REM
QuickTimeSDK\QTDevWin\Tools\Rez -i "QuickTimeSDK\QTDevWin\RIncludes" -i . AudioConvert.r -o AudioConvert.qtr
QuickTimeSDK\QTDevWin\Tools\RezWack -d AudioConvert.exe -r AudioConvert.qtr -o TEMP.exe
del AudioConvert.exe
ren TEMP.exe AudioConvert.exe