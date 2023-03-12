=============================================================================
QuickTime for Java SDK                                 Updated: 1 April 2006

Read Me Notes to "Time Code" Demo and Sample Code

=============================================================================
This demo program shows how to add and remove TimeCode tracks to a movie.
=============================================================================
The minimum build requirements for this Sample Code are:

- Common:
	- QuickTime 7 or later

- Mac OS X:
	- XCode 2.2 or later
	- Java 1.4.2 or later, 1.5.0 recommended

- Windows:
	- Windows 2000, XP or later
	- Java 1.4.2 or later, 1.5.0 recommended
	- JRE/JDK from Sun Microsystems, Inc. recommended
	- PATH environment variable includes java and javac

=============================================================================
Media requirements for this Sample Code - user supplied movie

=============================================================================
Notes & Comments

This code uses the TimeCode media services of QuickTime to add TimeCode readouts to a movie.

There are occasional redraw problems with the MovieController when the time code track is removed from the movie whilst the movie is actually playing (stopping the movie allows the MovieController to redraw properly). Ideally the movie should be stopped when removing the TimeCode track.

The code that would save the new time code track to the movie is commented out but present.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 - 2006 Apple Computer Inc. All rights reserved.
