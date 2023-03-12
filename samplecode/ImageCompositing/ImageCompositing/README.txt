=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Image Compositing" Demo and Sample Code

=============================================================================
This demo program shows how to composit a presentation out of disparate media sources, applying compositing effects such as blend and transparency. Recording a movie from the activities of the Compositor is also shown.

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

Various different types of objects are added as members to the top level compositor:
(1) A Compositor made up of cycling image based Sprites (see Bouncing Sprites for explanation)
(2) A Movie which is added through the use of the MoviePresenter class. A blend effect is applied to this member
(3) An image which has a blend effect also applied to it (ImagePresenter)
(4) QTImageDrawer object. This object takes any drawing that is done using Java drawing APIs and draws the resultant image in the QT owned space. It has a transparent graphics mode applied to it.

The paint method of the QTImageDrawer Paintable object returns an array of rectangles that tell the drawer which areas of its drawing area were drawn. This optimises the amount of copying and can greatly increase performance when composited. The QTImageDrawer is presented using the Notifier class JavaTextDrawer which captures a single rendering of the image drawer and then presents this in the context of the Compositor. Whenever the user clicks the Text Colour button a new image drawer is done and the text colour is randomly generated. We only need to keep the image drawer around to do this single pass - once done we can throw it away and we notify the NotifyListener that the image data has changed.

Controllers are attached to the Compositor in the same way that they are for the Group Drawing demo - dragging and layering responders.

The sample code also shows the usage of the RecordMovie class - the user can enable recording, select the number of frames to record. As the compression of each frame can take longer than just the composit cycle the user can adjust the rate of the Compositor's playback. This does not effect the ultimate recorded output - which is recorded to playback at ten frame a second regardless of the time it takes to record each frame.

The sample also prints out some profiling information about the time it takes to composit the set frames per second and the time it took to actually composit the last frame.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
