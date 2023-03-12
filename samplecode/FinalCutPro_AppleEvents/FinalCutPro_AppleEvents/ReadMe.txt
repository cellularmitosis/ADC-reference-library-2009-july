Sample Code  for Final Cut Pro and Apple Events

Final Cut Pro version 5.1.2 provides support for Apple events that external applications 
can use to import and export XML from the application. This disk image contains an 
Xcode project for a sample application that demonstrates the use of these Apple events. 
To explore this sample application, follow 
these steps: 
1. Open project file in Xcode 2.3 or later. 
2. Build the application. The resulting appli- 
cation allows you to send Apple events to 
Final Cut Pro, which must be running to re- 
spond to the events. You can pick the event 
to send, the project to operate on, and 
other options. 
3.  You can use the sample code as the 
foundation for your own application. Alter- 
natively, you may want to start from scratch 
using only the header file 
FCP_AppleEvents.h.

Using the AEDebugSends and AEDebugReceives environment variables can be of great help in debugging your applications use of AppleEvents. More information can be found on the Apple Developer Connection web site:

http://developer.apple.com/

Copyright © 2006 by Apple Computer, Inc.  All Rights Reserved. 