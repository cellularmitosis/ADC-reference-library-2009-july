REM *** batch program to embed our Macintosh 		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTWiredActions.r -o QTWiredActions.qtr
..\..\QTDevWin\Tools\RezWack -d .\Release\QTWiredActions.exe -r QTWiredActions.qtr -o .\Release\TEMP.exe -f
del .\Release\QTWiredActions.exe
REM *** now rename new file to previous name
ren .\Release\TEMP.exe QTWiredActions.exe