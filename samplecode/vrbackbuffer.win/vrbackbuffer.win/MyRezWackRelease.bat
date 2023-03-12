REM *** batch program to embed our Macintosh 'PICT' ***
REM *** resources into our application file         ***
..\..\QTDevWin\Tools\RezWack -d .\Release\VRBackBuffer.exe -r VRBackBuffer.qtr -o .\Release\TEMP.exe -f
del .\Release\VRBackBuffer.exe
REM *** now rename new file to previous name
ren .\Release\TEMP.exe VRBackBuffer.exe