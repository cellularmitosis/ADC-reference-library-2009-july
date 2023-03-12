REM *** batch program to embed our Macintosh dialog ***
REM *** resources into our application file          ***
..\..\QTDevWin\Tools\RezWack -d .\Release\QTStreamSplicer.exe -r QTStreamSplicer.qtr -o .\Release\TEMP.exe -f
del .\Release\QTStreamSplicer.exe
REM *** now rename new file to previous name
ren .\Release\TEMP.exe QTStreamSplicer.exe