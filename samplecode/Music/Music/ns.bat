javac src/*.java
echo Main-Class: NotesToneTest> manifest
cd src
jar cmf ..\manifest ..\NotesToneTest.jar *.class
cd ..
java -classpath "%CLASSPATH%;NotesToneTest.jar" NotesToneTest