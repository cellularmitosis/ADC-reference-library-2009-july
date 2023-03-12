=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Java Drawing" Demo and Sample Code

=============================================================================
This demo program shows the usage of the QTImageDrawer to allow QuickTime
to draw the results of painting into a Java offscreen Image and its associated Java Graphics.
=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code are found in the media directory of the QTJava SDK:

(1) T3.gif in the duke directory

=============================================================================
Notes & Comments

This demo program shows how to use the QTImageDrawer and its associated Paintable object to paint an image and some text using Java's imaging services (awt.Image and awt.Graphics). 

The QTImageDrawer uses Java's PixelGrabber class to get the pixels that have been drawn by the Paintable's paint method and uses QuickTime's image services to draw the results of this to the screen.

The QTImageDrawer is set as a client of the QTCanvas so that the results of this are presented to the user.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
