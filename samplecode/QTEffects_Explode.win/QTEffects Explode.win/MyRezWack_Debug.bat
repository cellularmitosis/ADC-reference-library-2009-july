REM *** batch program to embed our macintosh  ***
REM *** resources into our application file            ***

..\..\QTDevWin\Tools\RezWack -d .\Debug\QTEffects.exe -r ApplicationResources -o .\Debug\QT_Effects.exe -f

REM *** delete the original file (which does not have our resources) ***
del .\Debug\QTEffects.exe

REM *** now rename new file to previous name
ren .\Debug\QT_Effects.exe QTEffects.exe