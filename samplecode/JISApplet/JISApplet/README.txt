=============================================================================
QuickTime for Java SDK                              Updated: 30 November 1998

Read Me Notes to "QTSimple Applet" Demo and Sample Code

=============================================================================
This demo program shows how to display any QuickTime content within a java.awt.Applet where the content is read using a java.io.InputStream

=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 
	- QTJava.zip
	- An Applet viewer - Apple Applet Runner or the sun appletviewer
	
- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Windows 95, 98, or NT::
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements for this Sample Code are found in the classes directory where the HTML and class files are located:

(1) crossfad.gif

=============================================================================
Notes & Comments

test.html is for use with this code - it will expect to find the AppletTag.js when run in a browser

See the SimpleApplet for description on the usage of the applet methods.

This is a demonstration of using the QTFactory.makeDrawable methods from a java.ioi.InputStream.

If you have a file then that is the preferred way of using the QTFactory as the file itself can be used to guide the importers in the correct selection and identification of the contained media format. However if you don't then you can use the InputStream as the source data. 

The input stream should have originated as a file at some point in its history. You need to describe the standard file extension as a guide to the importer to enable it to select the appropriate importer for the bytes that are read in from the stream. Some file formats are not able to be determined without this information. The imported data will be loaded into memory (first a byte array) and then copied into a QTHandle so this can be a quite expensive operation (in terms of memory used) for large media sources.

This code can be used to read media content that is transported within a zip archive. QuickTime (like Java itself) had no direct means of opening a file that is contained within a zip file.

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used - it is called first in the init() method

It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly - it is called in the destroy() method

=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
