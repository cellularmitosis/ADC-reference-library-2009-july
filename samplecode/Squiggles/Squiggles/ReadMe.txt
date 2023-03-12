Squiggles

"Squiggles" is a Cocoa Document-based Application that shows custom drawing and event-handling in a custom subclass of NSView.
This sample is part of the WWDC 2008 Cocoa session: "Cocoa Fundamentals" Session 348.


Sample Requirements
The supplied Xcode project was created using Xcode v3.0 running under Mac OS X 10.5 or later.


Using the Sample
Build and run the sample using Xcode. Press and drag the mouse in the main view to create one or more "squiggles". 
Drag the rotation slider to apply a rotational tranform repeatedly when drawing.
Observe the multi-document support inherited by our controller from NSDocument: New Document, Save, Open, Undo, etc.
	
Changes from Previous Versions
n/a


Packaging List

SquiggleView.h
SquiggleView.m
	The primary View in this sample. This is typical of Cocoa progams that find they need to do custom drawing and event handling. This class overrides the designated initializer -initWithFrame: to initialize its attributes, -drawRect: to respond to requests to draw, and two NSResponder methods -mouseDown: and -mouseDragged: to handle the events we care about.
	
Squiggle.h
Squiggle.m
	This is the lowest level Model object in this application. It uses ObjectiveC 2 properties to provide access to the path, thickness, and color of a single squiggle drawn by the user. It conforms to the NSCoding protocol to supporting archiving by simply implementing -initWithCoder: and -encodeWithCoder:.

MyDocument.h
MyDocument.m
	This functions as the Controller object for each open document in the sample application. It maintains the linkage between the model: an NSArray of Squiggle objects and the user interface archived in the "MyDocument" nib file. 
	
MyDocument.nib
	An Interface Builder "nib" file with the archived window, rotation slider, outlet and target/action connections, and a placeholder for our custom SquiggleView to be created at runtime. This will be loaded each time a document is created or opened. Its "File's Owner" is the instance of the MyDocument class that will control this document and its user interface.

MainMenu.nib
	An Interface Builder "nib" file with only the main menu in it. It will be loaded when the application is first launched, and its "File's Owner" is the instance of NSApplication.

Feedback and Bug Reports
Please send all feedback about this sample by connecting to the Contact ADC page.
Please submit any bug reports about this sample to the Bug Reporting page.


Developer Technical Support
The Apple Developer Connection Developer Technical Support (DTS) team is made up of highly qualified engineers with development expertise in key Apple technologies. Whether you need direct one-on-one support troubleshooting issues, hands-on assistance to accelerate a project, or helpful guidance to the right documentation and sample code, Apple engineers are ready to help you.  Refer to the Apple Developer Technical Support page.

Copyright (C) 2008 Apple Inc. All Rights Reserved.