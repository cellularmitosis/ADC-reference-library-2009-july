=============================================================================
QuickTime for Java SDK                                  Created: 28 June 2000

Read Me Notes to "FrameStepper" Demo and Sample Code

=============================================================================
This demo program shows how to step frame-by-frame through the video track of
a QuickTime movie. It also demonstrates the use of the QuickTime graphics
exporters by allowing the user to save the current movie frame as a jpeg image.
=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 4
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code:

(1) QuickTime movie & jpeg image file are provided

=============================================================================
Notes & Comments

This sample application will display a movie (no controller) along with a background image in a window. Press the right arrow key (->) and the program advances the movie to the next video sample. Press the left arrow key (<-) and the program moves to the previous video sample. Press the <space> key and the program will save to disk the current video sample as a jpeg image.

This sample defines code that you can use to step frame-by-frame through a QuickTime
movie. Indeed, it illustrates *two* different methods for doing this: (1) using the Movie class methods to advance (or retreat) to interesting times in the movie; and (2) using the MovieController class methods to step forward or backward through a movie.

METHOD ONE: Use Movie class methods to step to interesting times in the movie. An interesting time is a time value in a movie, track, or media that meets certain search conditions that you specify. We'll use a very simple search condition: locate the next (or previous) sample in the movie's media. Once we have an interesting time, we display the sample at that time by calling the setTimeValue method. Here are the methods:

GoToNextVideoSample: display the sample that follows the current sample in a movie
GoToPrevVideoSample: display the sample that precedes the current sample in a movie
GoToFirstVideoSample: display the first video sample in a movie

METHOD TWO: Use the MovieController class methods to step through frames in the movie. This method uses the step function. Using this method, the code is considerably simpler. To implement this second method, we define three functions (which all operate on a movie controller that is associated with an open movie):

MCGoToNextVideoSample: display the sample that follows the current sample in a movie
MCGoToPrevVideoSample: display the sample that precedes the current sample in a movie
MCGoToFirstVideoSample: display the first video sample in a movie

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
