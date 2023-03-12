=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Group Drawing" Demo and Sample Code

=============================================================================
This demo program shows how to select group QuickTime drawing capable objects into the same display space of the QTCanvas

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

(1) ShipX.pct files (where X is a number indicative of a frame order)
(2) The house.jpg picture file
(3) The jumps.mov movie

=============================================================================
Notes & Comments

The DirectGroup allows other QTDrawable objects to be added to the same QTCanvas - much in the same way that lightweight components work with java.awt

The DirectGroup requires that the contained members to draw directly to the screen. The layering of the members are achieved through the clipping of the members that are behind others - disallowing their ability to draw where members are in front of them.

There are also Controllers attached to the Space:
(1) shift-click on any object to drag it around - the picture is restrained to the bounds of the DirectGroup. The other two objects can be dragged to within a single pixel showing.
(2) option-click to send an object to the back layer.

The spaceships (see Bouncing and Dragging Sprite samples) are also draggable and are contained within their space. Thus this sample also shows the ability to embed spaces within spaces with the contained space retained their own sense of a Space for their members.

The blue border around the picture after you make the window bigger is the background color of the DirectGroup. You could restrain the resize of the canvas to the picture's size (new QTCanvas (kInitialsize....))

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
