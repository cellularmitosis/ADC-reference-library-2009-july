javac src/*.java
echo Main-Class: QTStreamingApplet> manifest
cd src
jar cmf ..\manifest ..\QTStreamingApplet.jar *.class
cd ..
copy resources\QTStreamingApplet.html .
copy resources\AppletTag.js .
copy resources\QuickTimeJava.jp .
appletviewer -J-classpath -J"%CLASSPATH%";QTStreamingApplet.jar -J-Djava.security.policy=QuickTimeJava.jp QTStreamingApplet.html