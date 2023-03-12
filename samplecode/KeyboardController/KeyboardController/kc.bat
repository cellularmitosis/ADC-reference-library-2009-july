javac src/*.java
echo Main-Class: KeyBoardController> manifest
cd src
jar cmf ..\manifest ..\KeyBoardController.jar *.class
cd ..
java -classpath "%CLASSPATH%;KeyBoardController.jar" KeyBoardController