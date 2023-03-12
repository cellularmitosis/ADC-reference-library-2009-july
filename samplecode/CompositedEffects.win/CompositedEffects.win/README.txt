=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Composited Effects" Demo and Sample Code

=============================================================================
This demo program shows the construction of a Composited image containing the layering of an image file, the ripple effect, an animation and some Java text. Over this is placed a Movie which is drawn in front of the composited image.

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

(1) ShipX.pct files (where X is a number indicative of a frame order)
(2) Water.pct
(3) jumps.mov

=============================================================================
Notes & Comments

This demo program shows how to use the Compositor to combine multiple QTDrawable objects including a CompositableEffect (the ripple codec) and Java Graphics into a single image, which is then blitted on screen.

The composited image (which itself contains drawing objects that are layered) is then added to a DirectGroup behind a movie. The DirectGroup does not provide the compositing services of the Compositor (that is no overdrawing of transparencies, alpha blending, etc) are available with a DirectGroup, but it does allow for its member objects to be layered. Thus the movie can draw in front of the Compositor unheeded.

The demo also shows the usage of the QTImageDrawer object using the Java-drawing APIs to draw the "Java Text" which is then given a transparency so that only the text characters themselves are drawn by QuickTime.

The ripple effect is layered to apply on top of the background image, and its bounds is set to only the top part of the image. It thus applies a ripple effect only to the back image, not to sprites or text that are drawn in front of it. The ripple effect codec works by moving the pixels around on the destination QDGraphics that it is set to. By placing it in a Compositor the application can control which part of an image it ripples - in this case the water picture that is behind it.

By using the background layer of the Compositor you get a better rendering speed of any background image that a compositor may have. To use this capability you create the background buffer when creating the Compositor and then add the member(s) to the back most layer - they draw into this buffer and the SpriteWorld Compositor is able to optimise the drawing of this layer when doing its composit cycle.

The sample code also demonstrates the timing hierarchy that is built using the DisplaySpaces of the Compositor and DirectGroup. The top DirectGroup is the master time base for all of its members - the rate at which it's time base is set (the top text box) determines the overall rate of its members. The members can have their own rates which become offset based on the rates of their container groups. To start the demo set the top rate to 1.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
