=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "QT to Java Image" Demo and Sample Code

=============================================================================
This demo program shows the usage of the GraphicsImporterDrawer is used to produce pixels to create a java.awt.Image

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Mac OS X
	- 10.0 or later

- Windows 95, 98, NT, 2000 or XP.
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code:

Any image file that can be imported using the GraphicsImporter

=============================================================================
Notes & Comments

This demo works with GraphicsImporter to produce pixels for the creation of a java.awt.Image. The sample code has some notes about other possible image sources that can be used in this manner.  

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
