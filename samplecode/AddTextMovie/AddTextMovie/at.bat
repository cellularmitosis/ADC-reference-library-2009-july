javac src/*.java
echo Main-Class: AddText> manifest
cd src
jar cmf ..\manifest ..\AddText.jar *.class
cd ..
java -classpath "%CLASSPATH%;AddText.jar" AddText
