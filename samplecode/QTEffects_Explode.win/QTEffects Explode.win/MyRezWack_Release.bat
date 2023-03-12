REM *** batch program to embed our macintosh  ***
REM *** resources into our application file   ***

..\..\QTDevWin\Tools\RezWack -d .\Release\QTEffects.exe -r ApplicationResources -o .\Release\tQTEffects.exe -f

REM *** delete the original file (which does not contain our resources) ***
del .\Release\QTEffects.exe

REM *** now rename new file to previous name
ren .\Release\tQTEffects.exe QTEffects.exe