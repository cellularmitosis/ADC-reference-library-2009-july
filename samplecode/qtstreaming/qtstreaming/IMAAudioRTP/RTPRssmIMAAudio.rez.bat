@echo off
REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i ".\Sources" -i "..\..\QTDevWin\RIncludes" ".\RTPRssmIMAAudio.r" -o ".\RTPRssmIMAAudio.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\RTPRssmIMAAudio.dll" -r ".\RTPRssmIMAAudio.qtr" -o ".\RTPRssmIMAAudio.qtx" -f