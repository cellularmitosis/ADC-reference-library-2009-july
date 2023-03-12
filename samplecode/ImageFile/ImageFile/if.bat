javac src/*.java
echo Main-Class: ImageFileDemo> manifest
cd src
jar cmf ..\manifest ..\ImageFileDemo.jar *.class
cd ..
java -classpath "%CLASSPATH%;ImageFileDemo.jar" ImageFileDemo
