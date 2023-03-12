=============================================================================
QuickTime for Java SDK                              Updated: 7 December 2000

Read Me Notes to "Drawable Broadcaster" Demo and Sample Code

=============================================================================
This demo program shows how to use the Broadcasting API to broadcast audio from a Java application
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

=============================================================================
Media requirements for this Sample Code are found in the media directory of the QTJava SDK:

(1) A spd file specifying the settings for the broadcast such as the included multicasta.sdp file.

=============================================================================
Notes & Comments

This sample creates a window with a QTCanvas and a single button "Start Broadcast". When the button 
is pressed, the user will be presented with a standard file dialog for choosing a sdp file. Once the user has 
chosen a file, the user will see a configuration dialog for configuring the broadcast. The settings dialog allos
the user to choose a broadcasting source (currently, only Sequence grabber sources may be chosen) such as a video
camera, cd, or microphone. The user may also choose settings like codec for compression, format, and packetizer
settings. Once the user clicks the start button, the broadcast will begin and the window will show status information
about the broadcast in progress.

Clicking the stop button stops the broadcast.

To receive a broadcast, the client must open the same sdp file used to create the broadcast in the QuickTime Player. 
Note that this feature requires QuickTime 5. The user must also be on the same network or at the address specified in
the SDP file. 
=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
