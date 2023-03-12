javac src/*.java
echo Main-Class: CreatePictFile> manifest
cd src
jar cmf ..\manifest ..\CreatePictFile.jar *.class
cd ..
java -classpath "%CLASSPATH%;CreatePictFile.jar" CreatePictFile