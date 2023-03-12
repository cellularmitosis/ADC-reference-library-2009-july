REM *** batch program to embed our Macintosh		***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\Rez -i "..\..\QTDevWin\RIncludes" -i . QTCapture.r -o QTCapture.qtr
..\..\QTDevWin\Tools\RezWack -d .\Debug\QTCapture.exe -r QTCapture.qtr -o .\Debug\TEMP.exe -f
del .\Debug\QTCapture.exe
REM *** now rename new file to previous name
ren .\Debug\TEMP.exe QTCapture.exe