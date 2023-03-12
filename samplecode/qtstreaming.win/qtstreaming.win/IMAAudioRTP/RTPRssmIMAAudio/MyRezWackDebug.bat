REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i "..\..\QTDevWin\RIncludes" ".\Sources\RTPRssmIMAAudio.r" -o ".\Debug\RTPRssmIMAAudio.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\Debug\RTPRssmIMAAudio.dll" -r ".\Debug\RTPRssmIMAAudio.qtr" -o ".\Debug\RTPRssmIMAAudio.qtx" -f