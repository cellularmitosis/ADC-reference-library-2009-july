REM *** batch program to embed our Macintosh		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTActionTargets.r -o QTActionTargets.qtr
..\..\QTDevWin\Tools\RezWack -d .\Debug\QTActionTargets.exe -r QTActionTargets.qtr -o .\Debug\TEMP.exe -f
del .\Debug\QTActionTargets.exe
REM *** now rename new file to previous name
ren .\Debug\TEMP.exe QTActionTargets.exe