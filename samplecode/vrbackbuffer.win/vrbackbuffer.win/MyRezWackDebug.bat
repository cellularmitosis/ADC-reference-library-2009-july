REM *** batch program to embed our Macintosh 'PICT' ***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\RezWack -d .\Debug\VRBackBuffer.exe -r VRBackBuffer.qtr -o .\Debug\TEMP.exe -f
del .\Debug\VRBackBuffer.exe
REM *** now rename new file to previous name
ren .\Debug\TEMP.exe VRBackBuffer.exe