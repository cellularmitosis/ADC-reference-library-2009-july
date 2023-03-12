javac src/*.java src/duke/*.java
echo Main-Class: DukeMovie> manifest
cd src
jar cmf ..\manifest ..\DukeMovie.jar *.class duke\*.class
cd ..
java -classpath "%CLASSPATH%;DukeMovie.jar" DukeMovie