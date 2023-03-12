javac src/*.java src/ip/*.java
echo Main-Class: ImageProducing> manifest
cd src
jar cmf ..\manifest ..\ImageProducing.jar *.class ip\*.class
cd ..
java -classpath "%CLASSPATH%;ImageProducing.jar" ImageProducing