=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Transitions" Demo and Sample Code

=============================================================================
This demo shows how to use the QuickTime effects architecture applied to a character in an animation scene.

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

(1) Ship.pct, stars.jpg and water.pct in the pics directory

=============================================================================
Notes & Comments

This code shows how a Transition effect could be built and applied to a character in a realistic animation of a UFO encounter. Here a fading effect has been applied during the transition to the spaceship image which fades in and out as per the rate and the number of frames set for the transition. The transition is added to the Compositor using the QTEffectPresenter class.

The ripple effect has been applied to the water image (it is added in front of the water image taking up the same location) using the CompositableEffect class. 

A controller is added to the compositor to control the drawing of the transition effect. The controller object implements the TicklishController and subclasses the PeriodicAction class that has a doAction method which gets invoked on every tickle call. The doAction call is overidden to set the current frame and redraw the TransitionEffect. The source and the destination images of the transition effect are swaped when the no. of set frames are reached. The transition's controller then rests for a few seconds before it is woken up again and reapplied. The incoming time values to the doAction (called by PeriodicActin.tickle) method are used to calculate the rest and transition - ensuring that as the rate of playback changes the transition controller will react to these changes.

The example shows the usage of effects and effects presenters. The source and the destination images are applied the kCrossFadeTransitionType effect that makes them fade as per the number of frames set for the transition. 

The QTEffectPresenter is used to embbed the QTTransition effect and present it to the compositor which draws it to the canvas. Note that the QTTransition effect cannot be directly added to the compositor instead its given to the QTEffecPresenter which is added to the compositor. If a filter were being applied it has the same limitations as a transition with regards to adding it to a compositor - it must be added using the QTEffectPresenter.

The ripple effect can be directly applied to the Compositor and is added using the CompositableEffects class.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
