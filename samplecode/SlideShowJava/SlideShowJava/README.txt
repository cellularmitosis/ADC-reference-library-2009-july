=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Slide Show" Demo and Sample Code

=============================================================================
This demo program shows how to use the ImageViewer object to present a sequence of images one at a time using mouse clicks from the user.

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
Media requirements for this Sample Code are found in the media directory of the QTJava SDK:

(1) ShipX.pct in the images directory

=============================================================================
Notes & Comments

SlideShow.java -- Source file that subclasses an existing QTDrawable object to create a simple SlideShow presentation from a sequence of Images. It defines the addedTo method to register its interest in Mouse events - the mouse down is used to go to the next image, when the alt/option key is down the show presents the previous image in its sequence.

ViewerDemo.java	-- The source file containing the Main Class.

Possible extensions:
The SlideShow could be added to the Compositor for a smooth redraw - it  ensures that when the next/previous image is set the user won't see a flash of the redrawing of the changed image.

An alternative to using the Compositor would be to have an alternate version of this SlideShow which applied effects when transitioning to another image.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
