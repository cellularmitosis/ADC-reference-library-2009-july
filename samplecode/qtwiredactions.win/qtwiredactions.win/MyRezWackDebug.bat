REM *** batch program to embed our Macintosh		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTWiredActions.r -o QTWiredActions.qtr
..\..\QTDevWin\Tools\RezWack -d .\Debug\QTWiredActions.exe -r QTWiredActions.qtr -o .\Debug\TEMP.exe -f
del .\Debug\QTWiredActions.exe
REM *** now rename new file to previous name
ren .\Debug\TEMP.exe QTWiredActions.exe