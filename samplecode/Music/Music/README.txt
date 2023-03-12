=============================================================================
QuickTime for Java SDK                                 Updated: 1 April 2006

Read Me Notes to "Music" Demo and Sample Code

=============================================================================
This demo program shows a simple usage of QuickTime's music components.

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
No Media requirements

=============================================================================
Notes & Comments

This demo shows how to use the Musical Instruments package that's part of QuickTime using a NoteAllocator component.

The user is prompted to pick an instrument - the result of that pick is then printed out - its ToneDescription.

The program then makes a new NoteChannel to play a note using the Nylon String Guitar sound

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 - 2006 Apple Computer Inc. All rights reserved.
