WebKitDOMElementPlugIn
v1.0

This sample builds a Web Kit plug-in which acts as a draggable "magnifying glass" for the content on the hosting web page.  It uses the Objective-C DOM interface to modify its position as the mouse is dragged.  A clean demonstration of the WebPlugIn protocol methods necessary for creating a Web Kit plug-in is included.  For more on writing plug-ins and accessing the DOM with Objective-C, see "Web Kit Objective-C Programming Guide" and "Web Kit Plug-In Programming Topics" in the ADC Reference Library.

To install the built plug-in, place it in /Library/Internet Plug-Ins or ~/Library/Internet Plug-Ins and relaunch your favorite Web Kit-based browser.  After  installing the plug-in and loading the test page, drag the compass around to magnify the tiny text on the page.