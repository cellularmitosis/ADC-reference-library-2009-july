javac src/*.java
echo Main-Class: QTSimpleApplet> manifest
jar cmf manifest QTSimpleApplet.jar -C src QTSimpleApplet.class
copy resources\QTJSimpleApplet.html .
copy resources\Sample.mov .
copy resources\AppletTag.js .
copy resources\QuickTimeJava.jp .
appletviewer -J-classpath -J"%CLASSPATH%";QTSimpleApplet.jar -J-Djava.security.policy=QuickTimeJava.jp QTJSimpleApplet.html