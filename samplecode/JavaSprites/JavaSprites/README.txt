=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Java Sprites" Demo and Sample Code

=============================================================================
This demo program shows how to capture the results of java drawing and make a QuickTime sprite out of the result.

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

(1) Tx.gif in the duke directory (where X is a number indicative of a frame order)

=============================================================================
Notes & Comments

In the capture of the java drawing to quicktime it is very important that the images be prepared - otherwise when the draw command is given they won't draw and QuickTime won't be given the correct image data

This is NOT a demo of how to create sprites. QT has a far greater capability to deal with image formats than Java and the other sprite sample code (for example Bouncing Sprites) will deal with sprite images that are of any image file format known to QT.

If you were using this code to add the captured image data from Java and wished to add the image to a movie as a video track then the QTHandle.fromEncodedImage can be used to copy the data into a handle which the Media calls expect for their data - see the CreateMovies sample code.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
