=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "QTSimple Applet" Demo and Sample Code

=============================================================================
This demo program shows how to control a QuickTime movie within a browser using JavaScript.

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 
	- QTJava.zip
	- Internet Explorer 4 on Windows
 	- It will work with the Mozilla builds of Netscape Navigator on both MacOS and Win using their OJI plugin.

- MacOS
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code are found in the classes directory where the HTML and class files are located:

(1) jumps.mov

=============================================================================
Notes & Comments

See the SimpleApplet comments for general comments about the structure of QTJava applets.

The demo works with IE on Windows - due to an incomplete implementation of LivveConnect in Netscape V4 and on MacOS browsers that are currently shipping. It will work with the Mozilla builds of Netscape Navigator on both MacOS and Win using their OJI plugin.

Load the MovieDemo.html in the browser - it will load the movie which can be controlled through the Java Script buttons.

This applet uses the Java Plugin to allow the JavaSoft JRE to be the runtime java VM rather than the browser's built in Java.

QTJava applets are run in IE using the Java Plugin through a conversion of the applet tag to an object tag for IE and an embed tag for Netscape.

The original applet tag is:
<APPLET CODE=JScriptApplet.class WIDTH=200 HEIGHT=200 ALIGN=baseline NAME=JScriptApplet>
	<param name=file value="jumps.mov">
</APPLET>

The resulting tag is:
		<OBJECT classid="clsid:8AD9C840-044E-11D1-B3E9-00805F499D93"
			ID="JScriptApplet"
         	WIDTH="200" HEIGHT="200"
         	ALIGN="baseline"
     		CODEBASE="http://java.sun.com/products/plugin/1.1/jinstall-11-win32.cab#Version=1,1,0,0">
     		<PARAM NAME="code" VALUE="JScriptApplet.class">
     		<PARAM NAME="type" VALUE="application/x-java-applet;version=1.1">
    		<PARAM NAME="file" VALUE="jumps.mov">
    		<COMMENT> <!-- This wont' work in Netscape version 4 or less --> 
    			<EMBED TYPE="application/x-java-applet;version=1.1"
    				CODE="JScriptApplet.class"
    				NAME="JScriptApplet"
    				WIDTH="200" HEIGHT="200"
    				ALIGN="baseline"
    				FILE="jumps.mov"
    				PLUGINSPAGE="http://java.sun.com/products/plugin/1.1/plugin-install.html">
					<NOEMBED>
    		</COMMENT>
               			No JDK 1.1 support for APPLET!!
               		</NOEMBED>
               	</EMBED>
    	</OBJECT>


So the Applet tag names the applet (which is how JScript would talk to it)

In the OBJECT tag the value of the NAME tag becomes the ID tag.

In the EMBED tag (if it worked!) the NAME tag is the NAME tag.

In the accompanying html file you will see the approach that is used to send JScript messages to the applet.

An applet tagged version of this html page (not provided) should work with the OJI plugin of the Mozilla builds of Netscape Navigator on MacOS and Win32 systems.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used - it is called first in the init() method

It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly - it is called in the destroy() method

=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
