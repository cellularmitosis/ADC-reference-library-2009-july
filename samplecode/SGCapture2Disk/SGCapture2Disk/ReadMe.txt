=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "SGCapture2Disk" Demo and Sample Code

=============================================================================
This code shows how to use the SGDrawable class to display video within a QTCanvas. It also shows how to use the Sequence Grabber to capture a the incoming video to a movie file.

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
No Media requirements for this Sample Code.
=============================================================================
Notes & Comments

Microsoft Windows Caveats: Has not been tested under windows due to a lack of Video in boards that support Quick Time 3.0

Note: You need to set the client in the WindowOpened method for the Sequence grabber as the QDGraphics needs to be visible when it is set. (Quick Time requirement).

You do not need to create a moviefile to save the recorded data as the sequence grabber will automatically create a movie file for you based on the file you set in the setDataOutput method.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
