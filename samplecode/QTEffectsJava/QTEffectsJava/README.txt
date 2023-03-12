=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "QTEffects" Demo and Sample Code

=============================================================================
This demo program shows how to use QuickTime's visual effects architecture as applied to two source images. The effects are applied in realtime - controlled by the user settings in the window.

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3
	- QTJava.zip
	- Swing

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Mac OS X
	- 10.0 or later

- Windows 95, 98, NT, 2000 or XP.
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code are found in the media directory of the QTJava SDK:

(1) house.jpg and icons.jpg

=============================================================================
Notes & Comments

To make an effect you need to build one in code - a routine is supplied in the sample code to show you how to build a SMPTE effect.

An effect can also be chosen by the user through QuickTime's choose effects dialog. This dialog is set in the sample code to only show effects that expect two sources, but this dialog can be set to show effects that can be applied to 0 or 1 source.

The following points apply to the use of the QTTransition.doTransition() method. This method is provided as a means of running a transition through setting parameters to control the render behaviour of the effect.

When you set the time button it will endevour to render the effect in the time that is specified - in this case the frames are the maximum number of frames that it will render. In order to run the effect in the time selected some frames will be dropped on machines that are not capable of rendering all of the frames in the specified frames.

You can also change the frames per second that the effect will render in given the time specified

When you set the frame button the time value is ignored but it is the time it would take to render the effect at specified fps.

Collect stats on the effect - remember though that these are only approximate figures, but you can also see the behaviour on mac and windows and the effect that different frame rates use.

An application can also directly control the rendering of each frame of an effect through setting the frame directly and redrawing the effect. The Transitions sample code shows an example of this.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
