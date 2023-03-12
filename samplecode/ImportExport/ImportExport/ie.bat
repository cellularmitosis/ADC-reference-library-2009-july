javac src/*.java
echo Main-Class: ImportExport> manifest
cd src
jar cmf ..\manifest ..\ImportExport.jar *.class
cd ..
java -classpath "%CLASSPATH%;ImportExport.jar" ImportExport