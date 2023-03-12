=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Time Slaving" Demo and Sample Code

=============================================================================
This demo program shows how the effects of using timing information to control the Scrolling Java Text object and how slaving its TimeBase to a movie alters the behaviour.

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

(1) jumps.mov

=============================================================================
Notes & Comments

The sample open up with the Scrolling Java Text object (QTImageDrawer). Setting the playback rate to 1 will start scrolling the text. The scrolling text object is set up to positing the scrolling text based on the incoming time values, the scale of 110 is the number of pixels that the text scrolls. To get a smooth scroll the rate can be set to a slower rate (0.1, ...) and a negative rate will reverse the direction of the scroll. Changing the scale will change the scrolling behaviour with smaller number having the text jump to its next pixel which is a pixel more than 1 pixel from where it is.

When the make movie button is pressed the scrolling text's time base is slaved to the time base of the movie. As the movie is initially stopped the text will stop scrolling. Playing the movie will start the text scrolling again (provided that the rate of the text is still != 0). Changing the rate to a negative value will NOT change the scrolling behaviour once it is slaved (only starting (rate != 0) and stopping (rate == 0)). Why? The text is positioned solely on the incoming time value and the rate values of the master time base determine the time values. This is imbedded in the calculations of the text positioning - where you can see that the rate parametre of the tickle method is disregarded.

Playing the movie backwards (apple click on the back frame button) will make the text scroll backwards as the master time base is now feeding in time values that decrease in value.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
