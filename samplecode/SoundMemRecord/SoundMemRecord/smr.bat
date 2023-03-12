javac src/*.java
echo Main-Class: SoundMemRecord> manifest
cd src
jar cmf ..\manifest ..\SoundMemRecord.jar *.class
cd ..
java -classpath "%CLASSPATH%";SoundMemRecord.jar SoundMemRecord