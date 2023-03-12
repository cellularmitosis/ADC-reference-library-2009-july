REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i "..\..\QTDevWin\RIncludes" ".\Sources\RTPRssmIMAAudio.r" -o ".\Release\RTPRssmIMAAudio.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\Release\RTPRssmIMAAudio.dll" -r ".\Release\RTPRssmIMAAudio.qtr" -o ".\Release\RTPRssmIMAAudio.qtx" -f