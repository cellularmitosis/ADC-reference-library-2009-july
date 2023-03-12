=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1999

Read Me Notes to "Media Presenter" Demo and Sample Code

=============================================================================
This demo program shows how to rotate and scale any QuickTime presented content and keep that content postioned at top,left of display area.

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
Media requirements for this Sample Code are any media of user's choice

=============================================================================
Notes & Comments

The code treats a QTPlayer object differently than an image in that the QTPlayer/MovieController will correctly deal with matrices and bounds whereas Images (GraphicsImporterDrawer) requires more work on the Applications behalf to deal with this.

Sample code has more comments.
=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
