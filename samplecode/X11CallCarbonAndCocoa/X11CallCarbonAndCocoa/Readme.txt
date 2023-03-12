Read Me About X11CallCarbonAndCocoa

1.0

*Description: This sample gives developers a demo of how to create a double clickable X11 application made for MacOSX.  Also, more importantly this program demonstrates how basic X11 code can call directly into native MacOSX Carbon, Cocoa and the low-level CoreFoundation.  

*Important Note for running: To run this program you must have X11 installed and have the application running.

*Notes for X11 programmers: Pay particular attention to the build settings placed in project builder.  Specifically the following settings were added to the default "Carbon Application" project to make the application able to incorporate and use X11 (note these flags are eventually passed onto command line gcc): -I /usr/X11R6/include -L/usr/X11R6/lib  -lXaw -lXext -lXmu -lXt -lX11.  Note you can see _exactly_ what is being passed to gcc by building in project builder and opening the build window once the build is complete.

*What the sample does by default: This sample will display a X11 native window with a group of buttons.  Each button when clicked will perform some native operation in either Carbon, Cocoa or Core Foundation (for example: Putting up alerts or producing system beeps).

*Packing List:
• X11CallCarbonAndCocoa.app — The prebuilt X11 application which can be launched simply by double clicking the application bundle.
• CarbonCode.h — Header containing the Carbon API's that X11 calls.
• CarbonCode.c — Carbon source file containing the implementation of all Carbon functions used in the program.
• CocoaCode.h — Header containing the Cocoa API's that X11 calls.
• CocoaCode.c — Cocoa source file containing the implementation of all Cocoa functions used in the program.
• CFCode.h — Header containing the Core Foundation API's that X11 calls.
• CFCode.c — Core Foundation source file containing the implementation of all Core Foundation functions used in the program.
• main.c — The main file in the program used to initalize X11 and get things up and running.
• X11Code.h — The header file containing the callable X11 functions in the program.  These are the functions which main calls to get the program up and running.
• X11Code.c - This file contains all the X11 code in the program.  It initializes X11 displays the dialog and even handles the button clicks which call into native Carbon, Cocoa and Core Foundation.
• X11CallCarbonAndCocoa.pbproj — The project builder project file.
• Readme.txt — This file.

*Sample Requirements:

For ProjectBuilder users: This project was built with ProjectBuilder Jaguar version as a standard tool.  This project relies on Carbon, CoreFoundation, Cocoa frameworks.  This project also relies on the X11 subsystem being installed (X11R6).

*Building the Sample:

Using Project Builder:  To build the sample simply open the Project builder file and hit the 'build' button.  Similarly the sample can be run simply by clicking the 'run' button.  Note when running the sample you do need to have the X11 application running ahead of time.*Credits and Version History:

If you find any problems with this sample or have any suggestions, mail <dts@apple.com> with “Attn: Chad Jones” as the first line of your mail.

Version 1.0 is the first release.

Chad Jones
Apple Developer Technical Support
Networking, Communications, Hardware

Feb 28, 2003

---