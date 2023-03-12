javac src/*.java
echo Main-Class: PlayTune> manifest
cd src
jar cmf ..\manifest ..\PlayTune.jar *.class
cd ..
java -classpath "%CLASSPATH%";PlayTune.jar PlayTune