REM *** batch program to embed our Macintosh 		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTCapture.r -o QTCapture.qtr
..\..\QTDevWin\Tools\RezWack -d .\Release\QTCapture.exe -r QTCapture.qtr -o .\Release\TEMP.exe -f
del .\Release\QTCapture.exe
REM *** now rename new file to previous name
ren .\Release\TEMP.exe QTCapture.exe