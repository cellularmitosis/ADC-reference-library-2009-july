=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Bouncing Sprites" Demo and Sample Code

=============================================================================
This demo program shows how to construct a simple animation and apply compositing (graphics mode) effects to the sprites.

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

=============================================================================
Notes & Comments

The code will look for a directory of images in the java class path called
images which has a collection of images entitled "ShipX.pct" where X is a number indicating the order of the image in the sequence that describes the sprite

Clicking on the button "Click Here To Make Sprites Jump" will cause the sprites to jump to the other half of the window (Canvas). Note that as the Sprites jump between canvases the direction of the sprite's animation is reversed. This behavior is set by altering the rate of the Compositor's Timer.

The Compositor uses QuickTime's SpriteWorld and Sprite objects to create a composited image. The "Sprite" is constructed from a sequence of images and then behaviour's are attached to a particular sprite through the use of TicklishControllers.

A Space is a collection of objects (called members) which has a Timer that provides the basic timing services of the Space. Controllers are objects that are attached to a Space that exercise some control over the behaviour of members of that Space. 

TicklishControllers are a special type of controller which are controlled by the Space's Timer. They represent a controller that executes over time, the scale and period of a Ticklish object determining how often this object is tickled.

In this sample the SimpleActionList (which implements the TicklishController) is used to contain the list of actions that is applied to the Sprites. Two types of actions are applied - NextImageAction which uses an ImageSequencer to iterate through a Sequence of images and BounceAction which moves its target around a set number of pixels each time it is invoked, bouncing it off the walls of its containing DisplaySpace.

All of the Sprites have been recompressed to remove QDColor.blue from their image (this is the background colour in the original PICT files). The third Sprite uses one of these image files but after applying the transparency it also applies a graphics mode of blend to the TwoDSprite presenter that will draw this image.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
