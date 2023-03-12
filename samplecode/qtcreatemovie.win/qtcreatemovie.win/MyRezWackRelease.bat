REM *** batch program to embed our macintosh 'snd ' ***
REM *** resource into our application file          ***
..\..\QTDevWin\Tools\RezWack -d .\Release\QTCreateMovie.exe -r SoundResources -o .\Release\QTCreateAMovie.exe -f
del .\Release\QTCreateMovie.exe
REM *** now rename new file to previous name
ren .\Release\QTCreateAMovie.exe QTCreateMovie.exe