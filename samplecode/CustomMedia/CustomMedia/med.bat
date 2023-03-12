javac src/*.java src/com/vr/*.java
echo Main-Class: TestVRMedia> manifest
cd src
jar cmf ..\manifest ..\TestVRMedia.jar *.class .\com\vr\*.class
cd ..
java -classpath "%CLASSPATH%;TestVRMedia.jar" TestVRMedia