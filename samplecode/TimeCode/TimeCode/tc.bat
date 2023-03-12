javac src/*.java
echo Main-Class: TimeCode> manifest
cd src
jar cmf ..\manifest ..\TimeCode.jar *.class
cd ..
java -classpath "%CLASSPATH%";TimeCode.jar TimeCode