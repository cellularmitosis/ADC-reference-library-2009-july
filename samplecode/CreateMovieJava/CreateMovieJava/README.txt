=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "Create Movies" Demo and Sample Code

=============================================================================
This demo program shows how to select create a movie from the results of using Java drawing capabilites and a sound file. It roughly translates the sample code in the QuickTime SDK - the Inside Macintosh:QuickTime examples of Chapter 2 (listings 2-5 through 2-12, starting at page 2-47 of the unbound edition).

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

(1) sound.datafork

=============================================================================
Notes & Comments

To compress image data you need to have some kind of QDGraphics object. The sample code draws into an offscreen QDGraphics (GWorld). This GWorld is the source image that is then compressed. The resultant compressed data is then added to the movie. This code is seen in the addVideoSample method.

This code also reuses the same QTHandle for compressing the data into and then adding the data into the movie. The compress call takes an EncodedImage class - we use a RawEncodedImage object that references the same memory as is allocated in the QTHandle imageHandle. So the data is compressed to the imageHandle then the data is added to the media from the imageHandle. We create the imageHandle to be the maximum size that would be required to hold the resultant compressed data and we reuse that handle from frame to frame. We can do this because the vidMedia.addSample method will copy the data and the ImageDescription that is returned by the compress call tells this addSample method how much of the incoming imageHandle is valid data - at this point we don't care what is in the rest of the imageHandle.

What we do in the CreateMovie constructor:
The QTImageDrawer is used to get the drawing results from the NumberPainter object. The NumberPainter object draws into a java.awt.Image object using those drawing commands that is derived from a java.awt.Graphics object. This Image object is an offscreen Image object.

The QTImageDrawer creates that offscreen Image object from the QTCanvas that it is attached to. The image it creates remains unchanged in size and composition once it is created - so the NumberPainter can do less work once it has done an intial paint, where it paints all of the image. In subsequent calls it only paints the new number and returns a Rectangle that tells the QTImageDrawer which area it changed of its pixel data.

Why do you have to create a QTCanvas in the first place, and add it to a Frame? Good question, to which there is no sensible answer, but what seems like an arbitrary and highly inconvenient and unnecessary restriction in the AWT.

The java.awt does NOT allow the creation of an offscreen image at all unless you create it from a java.awt.Component.createImage call. AND you can only do this when the Component has been added to a Frame (or Applet). The native graphics environment must be initialised (this is why we call addNotify()), but we do not need to actually show the frame - we just need the native graphics environment of it to be initialised. We show the frame for the convenience and visual feedback for the sample code itself - it is not required and the makeMovie() method could be called straight after the constructor without showing the number frame at all.

The QTImageDrawer object must remain attached to the QTCanvas (which is done with the setClient call). If you remove the QTImageDrawer from the canvas the offscreen image it created will be thrown away and the NumberPainter has no offscreen image to paint to (or Graphics). However we don't need to see the results of this painting so we can set the destination QDGraphics of the QTImageDrawer to the offscreen GWorld that we then compress and add to the movie.

If java allowed the creation of offscreen Images without the restriction of having them attached to a visual display component then the construction of the Frame and QTCanvas would not be required.

So the NumberPainter paints its numbers into an offscreen java.awt.Image which is derived from an onscreen java Canvas which you need never see.


The QTImageDrawer takes the pixels that the NumberPainter paints and blits them to its destination QDGraphics (the setGWorld call) which is offscreen. We set the QTImageDrawer to draw to an offscreen QDGraphics and after each draw the QDGraphics is compressed and the resultant compressed image data is added to the movie. 


Sound Data in Data Fork

The example sound provided with QTJava is in the file "sound.dataFork".  It is the original resource from the C example except that the 'snd ' resource data was copied into the data fork. This file is found in the media directory.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
