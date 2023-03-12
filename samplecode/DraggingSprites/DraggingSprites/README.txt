=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Dragging Sprites" Demo and Sample Code

=============================================================================
This demo program shows how to define MouseResponders to customise the behaviour of MouseDrag actions and to define a custom Matrix transformations on a Sprite.

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

See the notes for the Bouncing Sprites sample for general construction of animation.

This code demonstrates the use of the SWController with Sprites as the target object to be dragged. It also uses the setActionable method on Actions which allows the program to trigger an action when the action is invoked (PlayNote on the first sprite when it bounces, PlayNote on any sprite when it is dragged to the side).

It also uses the makeTransparent call on the painted sprite to set a hot spot region for the hit testing of the sprite. Thus the green rect within the yellow rect is the "Hot spot". The makeTransparent call is used in this case to construct the hot spot. This call still requires a color to be set - in this case we specify black which is NOT a part of the sprite's image so there is not transparency. The specified region for this call becomes the hot spot.

It uses three drag actions:

All the sprites are draggable by just clicking on them - this is the action of the Drager object. Two sprites are scalable if you shift-click on it - this is the action of the ScaleAction object. The static yellow square is skewable by option(alt)-clicking on it.

After setting up the sprites the controllers are used. A controller controls objects that reside in its space.

The controllers are SWontrollers - a subclass of MouseController. A SWController knows how to extract a QuickTime sprite from a SpriteWorld if there is one where the user pressed the mouse.

All of the controllers are given Draggers as their responders.

The first controller has its wholespace set to true - thus any objects that are members of the Compositor will be dragged.

The second controller has its wholespace set to false - so you have to explicitly add objects to this controller for them to be controlled by it. This controller is given a Scaler responder which will rescale the sprite according to how far the mouse is moved. This scale action will only occur when the user is holding the shift key down when the mouseDown is received - these are the conditions under which the controller is activated.

The third controller also has its wholespace set to false - so you have to explicitly add objects to this controller for them to be controlled by it. This controller is given a Skewer which will skew the sprite according to how far the mouse is moved. This skew action's magnitude is controlled by the values supplied to the maker - larger values give a finer degree of control. The skew action will only occur when the user is holding the option/alt key down when the mouseDown is received.

The Scaler and Skewer are custom draggers that the application has defined.

One Sprite is also given a RotateAction that rotates the sprite in a circle. The rotate action is a subclass of quicktime.app.actions.MatrixAction. MatrixAction takes care of most of the details of manipulating its target - an Object that implements the Transformable interface (which the TwoDSprite does).

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
