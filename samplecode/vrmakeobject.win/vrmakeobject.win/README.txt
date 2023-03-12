README - VRMakeObject

VRMakeObject is a simple application that converts a linear QuickTime
movie into a QuickTime VR object movie. The QuickTime VR movie created 
by VRMakeObject is either a version 2.0 QTVR movie or a version 1.0 
QTVR movie; you select the version using the Test menu.

VRMakeObject is distributed as sample code. It builds the QTVR file 
according to the documented file format. 

IMPORTANT: VRMakeObject assumes that the source QuickTime movie 
contains 468 frames, organized into 36 rows and 13 columns. 
These values are determined by constants in the file VRMakeObject.c. 
Of course, a real-world utility would allow the user to specify the 
relevant settings (presumably by displaying a dialog box). 
This is left as an exercise for the reader.

The essential code is found in the file VRMakeObject.c. 
This code compiles and runs on both MacOS and Windows platforms.


Enjoy,
QuickTime Team

