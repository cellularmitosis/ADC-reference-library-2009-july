=============================================================================
QuickTime for Java SDK                              Updated: 1 April 2006

Read Me Notes to "QTSimple Applet" Demo and Sample Code

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
Media requirements for this Sample Code are found in the media directory in the classes directory where the HTML and class files are located:

(1) crossfad.gif

=============================================================================
Notes & Comments

QTJSimpleApplet.html is for use with this code - it will expect to find the AppletTag.js when run in a browser

The Sample code:
The applet methods are used in the following way:

(1) init()
The init method is used to get the resources for the program and set up the environment - including the creation of the QTCanvas. This QTCanvas is the object responsible for handling the integration from the java.awt side between Java and QuickTime. The QTCanvas also has parameters that control the resizing of the client that it presents. In this case we tell the canvas to centre the client within the space that it is given by the layout manager of the applet and to ensure that the client is only as big as its intial size (or smaller if the canvas is made smaller). If a QTException is thrown in the init method we set the qtContent to a java.awt.image (wrapped in ImageDrawer) of the QuickTime logo - thus if there is a problem the user will see the QTLogo instead of nothing.

(2) start()
In the start method we set the client of the QTCanvas. This QTCanvas.client is the QuickTime object (an object that implements the QTDrawable interface) that will draw to the area of the screen that the QTCanvas occupies. This is the QuickTime side of the integration between Java and QuickTime.

(3) stop()
The stop method is used to remove the client from the QTCanvas. It will be reset in the start method if the applet is restarted.

(4) destroy()
The destroy method is used to close the QT session. It is also a good idea to undo what was done in the init method - so we remove the QTCanvas from the applet.

This protocol enables the applet to be reloaded, suspended and resumed, for eg. the user leaving and returning to the page with the applet. The use of the init/destroy and start/stop methods are reciprocal in their activities.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used - it is called first in the init() method

It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly - it is called in the destroy() method

=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 - 2006 Apple Computer Inc. All rights reserved.
