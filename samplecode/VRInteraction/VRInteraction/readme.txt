=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "VR Interaction" Demo and Sample Code

=============================================================================
This demo program shows the introspection of a Movie and the presentation of mixer channels to edit the playback volumes and balance of audio media.

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 501 
	- QTJava.zip
	- Swing

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code:

(1) User selected file with at least one audio (sound or music) media track
=============================================================================
Notes & Comments

This demo illustrates how to control the interaction properties of a QuickTime VR movie.

vrInteraction class:  This is the main class.  It opens the movie file, and installs VR callbacks.

MouseOverHotSpot class: 
On kQTVRHotSpotEnter (entering a hotspot) the execute method of the callback sets the pan/tilt speed to
either slow, normal or fast, calculated from the id of the hotspot.

On kQTVRHotSpotLeave (leaving a hotspot) the execute method of the callback resets the pan/tilt speed to normal
and resets the Field of View (FOV) to the value at the time the hotspot was entered.

TriggerHotSpotInterceptor class: 
On kQTVRTriggerHotSpotSelector (the hotspot is clicked) the execute method of the interceptor changes the FOV to zoom in,
for non 'link' (kQTVRHotSpotLinkType) hotspots.  If the zoom in constraint is reached the FOV is reset to the original value.

REQUIREMENTS:
VRInstance methods for Interaction Properties require QuickTime 5.0 and QTJava 5.0.1
checkQTJavaVersion method is used to check that the methods are available.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.

In this example, it is required that the removeInterceptProc method be used at exit to remove the VR callback.

- Java 1.3 : IllegalStateException : cannot dispose input context
There is a general override of the dispose method to deal with a IllegalStateException 

=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 2001 Apple Computer Inc. All rights reserved.
