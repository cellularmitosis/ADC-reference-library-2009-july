=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Play Movie" Demo and Sample Code

=============================================================================
This demo program shows how to display any QuickTime content within a java.awt display space using the QTCanvas. It also demonstrates the use of the different resize options of the QTCanvas (with the QTCanvas' alignment set to centre it in the display space).

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 Streaming for URL DataRefs
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Mac OS X
	- 10.0 or later

- Windows 95, 98, NT, 2000 or XP.
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code:

(1) QuickTime movie files of the user's choice

=============================================================================
Notes & Comments

The user is shown an empty movie - choose to view either a file with the Open menu item or enter an URL with the Open URL menu item. If the selection is made the movie and its controller will be presented to the user. Closing the window or choosing the Quit menu item will quit the application.

The window is the size of the movie and resizing the window will resize the movie. The QTCanvas is set to allow any size and is the central component in a java.awt.BorderLayout of its parent Frame.

When a window is iconified we stop the task thread and when de-iconified we restart it. The taskThread is a thread that the QTPlayer attaches itself to so that processing time is regularly allotted to QuickTime to enable it to render the movie.

The OpenURL command will create a DataRef from the entered string - a couple of examples that are generally available are printed out to the console window when the program begins. Any URL (file, http, rtsp) can be entered in the Open URL window - QuickTime handles the creation of the appropriate DataHandler for the specified protocol. If the protocol is unknown to QuickTime an invalidDataHandler error is thrown. If the entered string does not point to a valid movie, an invalidDataRef error is thrown when the movie is created.

The PresentMovie command will put the screen into full screen mode and display the movie in the centre of the screen using the performance resize characteristics of the QTCanvas.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
