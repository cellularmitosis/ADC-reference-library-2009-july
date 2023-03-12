OpenGL Screen Saver

Purpose:
    The purpose of this sample is to demonstrate how to integrate OpenGL drawing into a standard Cocoa screen saver on Mac OS X. The process illustrates of how to tie in an external view (OpenGL in this case, but could be Quicktime, Quickdraw, etc) into the framework. For the purposes of this sample, the Cocoa OpenGL sample was used as-is as the testbed NSOpenGLView for the application. No other modifications were made to the sample - it literally "dropped" right in.

    Please note that the command interface to Cocoa OpenGL will not work with the screen saver. As you might expect, pressing a key or moving the mouse deactivates the screen saver. 

    To test the screen saver, place it in /Library/Screen Savers on the system volume.

Requirements:
    Mac OS X Panther 10.3
    XCode 1.0
