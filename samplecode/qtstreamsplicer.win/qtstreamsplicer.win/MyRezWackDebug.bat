REM *** batch program to embed our Macintosh dialog ***
REM *** resources into our application file          ***
..\..\QTDevWin\Tools\RezWack -d .\Debug\QTStreamSplicer.exe -r QTStreamSplicer.qtr -o .\Debug\TEMP.exe -f
del .\Debug\QTStreamSplicer.exe
REM *** now rename new file to previous name
ren .\Debug\TEMP.exe QTStreamSplicer.exe