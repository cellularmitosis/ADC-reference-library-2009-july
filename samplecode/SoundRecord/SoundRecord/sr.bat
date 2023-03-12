javac src/*.java
echo Main-Class: SoundRecord> manifest
cd src
jar cmf ..\manifest ..\SoundRecord.jar *.class
cd ..
java -classpath "%CLASSPATH%";SoundRecord.jar SoundRecord