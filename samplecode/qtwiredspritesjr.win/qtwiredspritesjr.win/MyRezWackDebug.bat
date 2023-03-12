REM *** batch program to embed our Macintosh		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTWiredSpritesJr.r -o QTWiredSpritesJr.qtr
..\..\QTDevWin\Tools\RezWack -d .\Debug\QTWiredSpritesJr.exe -r QTWiredSpritesJr.qtr -o .\Debug\TEMP.exe -f
del .\Debug\QTWiredSpritesJr.exe
REM *** now rename new file to previous name
ren .\Debug\TEMP.exe QTWiredSpritesJr.exe