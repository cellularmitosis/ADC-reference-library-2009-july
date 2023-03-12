REM *** batch program to embed our Macintosh 		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTActionTargets.r -o QTActionTargets.qtr
..\..\QTDevWin\Tools\RezWack -d .\Release\QTActionTargets.exe -r QTActionTargets.qtr -o .\Release\TEMP.exe -f
del .\Release\QTActionTargets.exe
REM *** now rename new file to previous name
ren .\Release\TEMP.exe QTActionTargets.exe