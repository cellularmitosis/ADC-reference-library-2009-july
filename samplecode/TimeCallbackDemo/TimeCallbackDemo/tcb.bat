javac src/*.java
echo Main-Class: TimeCallbackDemo> manifest
cd src
jar cmf ..\manifest ..\TimeCallbackDemo.jar *.class
cd ..
java -classpath "%CLASSPATH%";TimeCallbackDemo.jar TimeCallbackDemo