javac src/*.java
echo Main-Class: PlaySound> manifest
cd src
jar cmf ..\manifest ..\PlaySound.jar *.class
cd ..
java -classpath "%CLASSPATH%";PlaySound.jar PlaySound