=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Tween Camera" Demo and Sample Code

=============================================================================
This demo program shows how to apply camera actions to a 3D model (a 3D media track) in a movie using the Tween Media type of QuickTime.

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

(1) jet.mov

=============================================================================
Notes & Comments

This demo presents an example of adding a tween track to a movie with a 3D track.  

The essential steps are creating the tween track and media, creating the tween sample, adding the tween sample to the tween media, adding the tween media to the tween track, creating a new input map for the 3D handler, and attaching the input map to the 3D media handler.  Something to watch out for: endian problems with the QTByteObjects being passed as arguments to insertChild.  This demo will overwrite the original source file, so keep a copy around if you need to get it back.  Also, note that you need to close the little window that pops up to actually exit the program properly.  This should work on Macs as well as under NT.

Why endian problems:
Movies are big endian file formats. Ints, shorts, floats on Win32 systems are little endian (yes even in Java). Whilst QuickTime can take care of many of the endian issues when reading and writing movies, the one area it cannot do this is with atom containers the user adds to a movie directly. The requirement for the user is to ensure that the data that is added to AtomContainers THAT ARE ADDED TO MOVIES be in big endian formats - thus the need to flip the endian ordering in this sample code. This is not a requirement when just using atom containers - for instance the effects demos use atom containers and do not flip as these containers are not being added to movies. This is well documented in the QuickTime documentation - please see that doc for more information.

The QTUtils.endianX calls will flip the endian order - but they will flip only when the application is running on a Win platform. Thus the calls can be used with impunity by an application on both Mac and Win. 

Known issues:

The movies created by this program on Macs don't run properly when transfered  to a PC. To fix this, open the movie on the PC, go to Edit -> Enable tracks, turn off the 3D track, and turn it back on. We're still sorting out why this happens.

SPECIAL NOTE:
this will overwrite the original movie - it does not make a copy.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
