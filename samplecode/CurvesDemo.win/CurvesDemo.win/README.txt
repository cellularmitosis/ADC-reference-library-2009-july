=============================================================================
QuickTime for Java SDK                              Updated: 13 Sept 1999

Read Me Notes to "CurvesDemo" Demo and Sample Code

=============================================================================
This demo program shows how to use the Curve Codec in QuickTime to draw a vector Graphic image. 
=============================================================================
The minimum runtime requirements for this Sample Code are:

- Common
	- Sun Compliant Java Runtime Environment 1.1
	- QuickTime 3 
	- QTJava.zip

- MacOS:
	- System 8 or later
	- Macintosh Runtime for Java (MRJ) 2.1

- Mac OS X
	- 10.0 or later

- Windows 95, 98, NT, 2000 or XP.
	- JRE/JDK from Sun Microsystems, Inc. recommended

=============================================================================
Media requirements - none
=============================================================================
Notes & Comments
In the QTVector Demo we build the image Data by constructing raw atoms representing the point data. QuickTime 4.0 added
API's that simplify this tremendously. We demonstrate the same example with use of these API calls. It should be noted that all the Endian 
flipping of the data is done internally by the QTJava classes otherwise its explicitly mentioned in the docs.

A vector image is created via an IntEncodedImage object. The IntEncodedImage contains the meta instructions which are then interpreted 
by the curve codec. It is possible to build a list of IntEncodedImages which can then be passed  to an ImagePresenter by using the 
setImageData call on the imagePresenter class.
 

=============================================================================
General Comments

- QTSession.open and close:

A QTSession.open will perform a gestalt check to ensure that QuickTime is present and is initialized. This is a required call before any QuickTime Java classes can be used.

When the user closes the window the program will quit, first calling QTSession.close to terminate QuickTime. It is necessary for programs to call QTSession.close if they have previously called QTSession.open in order to shut down QuickTime properly.


=============================================================================

QuickTime and QuickTime for Java are trademarks of Apple Computer, Inc.
(c) 1998 Apple Computer Inc. All rights reserved.
