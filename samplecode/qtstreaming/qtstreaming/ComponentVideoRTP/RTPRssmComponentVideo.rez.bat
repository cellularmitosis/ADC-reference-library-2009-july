@echo off
REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i ".\Sources" -i "..\..\QTDevWin\RIncludes" ".\RTPRssmComponentVideo.r" -o ".\RTPRssmComponentVideo.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\RTPRssmComponentVideo.dll" -r ".\RTPRssmComponentVideo.qtr" -o ".\RTPRssmComponentVideo.qtx" -f