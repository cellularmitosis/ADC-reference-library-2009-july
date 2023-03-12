javac src/*.java
echo Main-Class: MovieTextFinder> manifest
cd src
jar cmf ..\manifest ..\MovieTextFinder.jar *.class
cd ..
java -classpath "%CLASSPATH%;MovieTextFinder.jar" MovieTextFinder