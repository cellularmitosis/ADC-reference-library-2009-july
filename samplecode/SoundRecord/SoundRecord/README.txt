=============================================================================
QuickTime for Java SDK                                 Updated: 1 April 2006

Read Me Notes to "Sound Record" Demo and Sample Code

=============================================================================
This code shows using the SequenceGrabber for simple audio recording

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
No media requirements for this Sample Code.
=============================================================================
Notes & Comments

Settings - allows the user to specify the source, compression and format for the recording

Record - create a file to save the recorded information in and start recording

Stop - stop recording and show window with movie in it

You need to dispose of your connection to the Audio Channel before playing back the recorded sound, (otherwise you won't hear your recorded sound). This is a limitation of the Windows sound architecture.

It is important that your code call setDataOutput() before any other sequence grabber call's e.g prepare(). Failure to do so will result in your program silently aborting.

You do not need to create a moviefile to save the recorded data as the sequence grabber will automatically create a movie file for you based on the file you set in the setDataOutput method.

It is important to note that the Java Garbage Collector may hold onto previously created files, and not free them up when expected. The call QTUtils.reclaimMemory() in the SoundRecord.java code signals the garbage collector to run, however this is not a guarantee that the GC will immediately be invoked. The side affect of this behaviour is that you may occassionaly see a message that a file is use if you try to save a recording the second time using the same filename.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 - 2006 Apple Computer Inc. All rights reserved.
