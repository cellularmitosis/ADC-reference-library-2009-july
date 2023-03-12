=============================================================================
QuickTime for Java SDK                              Updated: 1 April 2006

Read Me Notes to "QTStreaming Applet" Demo and Sample Code

=============================================================================
This demo program shows how to display any QuickTime content within a java.awt.Applet.

=============================================================================
The minimum build requirements for this Sample Code are:

- Common:
	- QuickTime 7 or later

- Mac OS X:
	- XCode 2.2 or later
	- Java 1.4.2 or later, 1.5.0 recommended

- Windows:
	- Windows 2000, XP or later
	- Java 1.4.2 or later, 1.5.0 recommended
	- JRE/JDK from Sun Microsystems, Inc. recommended
	- PATH environment variable includes java and javac

=============================================================================
Media requirements for this Sample Code:

A user supplied media

=============================================================================
Notes & Comments

QTStreamingApplet.html is for use with this code - it will expect to find the AppletTag.js when run in a browser.
		

Enter a url eg. http:///... to play a movie (or read in an image file) in the text box at the bottom of the applet

See the Simple Applet for comments about the code.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used - it is called first in the init() method

It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly - it is called in the destroy() method

=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
