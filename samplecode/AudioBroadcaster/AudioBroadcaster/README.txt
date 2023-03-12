=============================================================================
QuickTime for Java SDK                              Updated: 7 December 2000

Read Me Notes to "Audio Broadcaster" Demo and Sample Code

=============================================================================
This demo program shows how to use the Broadcasting API to broadcast audio from a Java application
=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 5 
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code are found in the media directory of the QTJava SDK:

(1) A spd file specifying the settings for the broadcast such as the included multicasta.sdp file.

=============================================================================
Notes & Comments

The code creates a window with two buttons: "Start Broadcast" and "Configure Broadcast". Clicking the "Configure Broadcast"
button creates a standard file dialog. The user must specify an sdp file used to configure the broadcast. Once an sdp file
is chosen, a settings dialog is created allowing the user to specify the audio source, rate, codec, and 
packetizer to be used by the broadcaster. 

Once the broadcast settings are configured, clicking "Start Broadcast" will create a new broadcast session.

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
