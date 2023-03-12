=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "QTTest Applet" Demo and Sample Code

=============================================================================
This demo program shows how to test if both QT and QTJava are installed correctly

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 
	- QTJava.zip
	- An Applet viewer
	
- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
No Media requirements for this Sample Code
=============================================================================
Notes & Comments

1. test.html is for use with this code - it will expect to find the AppletTag.js when run in a browser

2. When run the applet will print a message and display the QTLogo if QT and QTJava are installed		

3. See comments in code about this applet

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used - it is called first in the init() method

It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly - it is called in the destroy() method

=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
