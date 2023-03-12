REM *** batch program to embed our macintosh 'snd ' ***
REM *** resource into our application file          ***
..\..\QTDevWin\Tools\RezWack -d .\Debug\QTCreateMovie.exe -r SoundResources -o .\Debug\QTCreateAMovie.exe -f
del .\Debug\QTCreateMovie.exe
REM *** now rename new file to previous name
ren .\Debug\QTCreateAMovie.exe QTCreateMovie.exe