=============================================================================
QuickTime for Java SDK                              Updated: 15 Sept 1999

Read Me Notes to "SoundMeter" Demo and Sample Code

=============================================================================
This code shows using the SoundMediaHandler to meter the Bands Frequency levels and to change bass and treble.

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
Media Requirements: You need to open a media file with sound track in it. 
=============================================================================
Notes & Comments

The code shows the usage of SoundMediaHandler to get the Media's Equalizer bands and meter the frequency levels. It should be noted that the MediaEQSpectrumBands object should be first set to the frequencies you wish to meter. The media is played using the QTPlayer and the spectrum bands frequencies are printed out.
The Demo currently just meters the sound track of the media. One can use this to meter the music track and display a spectrum meter.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
