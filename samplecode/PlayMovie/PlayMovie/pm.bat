javac src/*.java
echo Main-Class: PlayMovie> manifest
cd src
jar cmf ..\manifest ..\PlayMovie.jar *.class
cd ..
java -classpath "%CLASSPATH%";PlayMovie.jar PlayMovie