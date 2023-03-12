@echo off
REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i ".\Sources" -i "..\..\QTDevWin\RIncludes" ".\RTPMPComponentVideo.r" -o ".\RTPMPComponentVideo.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\RTPMPComponentVideo.dll" -r ".\RTPMPComponentVideo.qtr" -o ".\RTPMPComponentVideo.qtx" -f