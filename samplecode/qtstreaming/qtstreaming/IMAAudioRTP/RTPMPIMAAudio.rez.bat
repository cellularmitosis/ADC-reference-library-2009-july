@echo off
REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i ".\Sources" -i "..\..\QTDevWin\RIncludes" ".\RTPMPIMAAudio.r" -o ".\RTPMPIMAAudio.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\RTPMPIMAAudio.dll" -r ".\RTPMPIMAAudio.qtr" -o ".\RTPMPIMAAudio.qtx" -f