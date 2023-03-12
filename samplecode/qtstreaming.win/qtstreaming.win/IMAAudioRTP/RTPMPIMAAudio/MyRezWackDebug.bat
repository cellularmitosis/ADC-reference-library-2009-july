REM *** batch program to create and embed our Macintosh resources ***
REM *** into the dll file, thereby creating a final qtx file ***
..\..\QTDevWin\Tools\Rez -i ".\Headers" -i "..\..\QTDevWin\RIncludes" ".\Sources\RTPMPIMAAudio.r" -o ".\Debug\RTPMPIMAAudio.qtr" -f
..\..\QTDevWin\Tools\Rezwack -d ".\Debug\RTPMPIMAAudio.dll" -r ".\Debug\RTPMPIMAAudio.qtr" -o ".\Debug\RTPMPIMAAudio.qtx" -f