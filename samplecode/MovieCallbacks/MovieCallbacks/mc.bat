javac src/*.java
echo Main-Class: MovieCallbacks> manifest
cd src
jar cmf ..\manifest ..\MovieCallbacks.jar *.class
cd ..
java -classpath "%CLASSPATH%;MovieCallbacks.jar" MovieCallbacks