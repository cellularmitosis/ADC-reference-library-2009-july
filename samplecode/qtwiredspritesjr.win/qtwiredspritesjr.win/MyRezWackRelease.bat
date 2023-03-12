REM *** batch program to embed our Macintosh 		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTWiredSpritesJr.r -o QTWiredSpritesJr.qtr
..\..\QTDevWin\Tools\RezWack -d .\Release\QTWiredSpritesJr.exe -r QTWiredSpritesJr.qtr -o .\Release\TEMP.exe -f
del .\Release\QTWiredSpritesJr.exe
REM *** now rename new file to previous name
ren .\Release\TEMP.exe QTWiredSpritesJr.exe