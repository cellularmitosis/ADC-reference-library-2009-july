SlideShowImporter Sample Code

Version 1.0
6/13/02


DESCRIPTION

This is an example of a movie import component that accepts as input an XML file, and constructs a movie from the elements in this file. The XML file consists of XML elements of the form:

/*
    An XML Element, i.e.
        <element attr="value" attr="value" ...> [contents] </element>
    or
        <element attr="value" attr="value" .../>
*/

Elements can be images, effect types, and audio files.

Each image element consists of an image file attribute and a duration attribute. Each effect element consists of an effect type attribute and a duration attribute. Each audio element consists of an audio-only movie file attribute.

The component parses each element in the file using the QuickTime XML parsing component, line by line, and constructs a movie from these elements. For example, given the following sample elements: 

	<image src="sky01.jpg" dur="1" /><effect type = "dissolve" dur = "1" />

the program will construct a track media item from the image file "sky01.jpg", and add this media item to the video track of the movie (NOTE: in this sample, all our images are the same size - if you use different size images they will be scaled appropriately by QuickTime). The duration of this media item will be 1 as specified by the duration attribute. After adding the image element to the movie, the program will add the next element, the dissolve effect. After all images & effects are added this will result in a movie file which displays a series of images, with an effect as a transition between each image. In addition, an audio-only movie file will be played for the length of the movie as background music.


USING THE SAMPLE

After placing the built component file in the appropriate folder on your system (see below for more information), launch the QuickTime Player application and navigate to either the "maynard jumps from plane" or "other samples" folder. These folders contain the XML files plus all the related image and audio files.

In the "maynard jumps from plane" folder, open (or import) the file "maynard.qtsl" using QuickTime Player. Or if in the "othere samples" folder, open (or import) the "slideshow.qtsl" file. Opening either of these .qtsl XML files will invoke the movie import component and a movie file will be constructed from the elements in the XML file.

BUILD REQUIREMENTS

-Mac-
CodeWarrior 6, IDE 4.1, or
Project Builder Version 1.1.1 (December 2001 Developer Tools)

-Windows-
Microsoft Visual C++ 6

RUNTIME REQUIREMENTS

-Mac-
QuickTime 5, Macintosh OS 9, CarbonLib 1.3 or better for Carbon builds, or
QuickTime 5, Macintosh OS X

-Windows-
QuickTime 5, Windows 98, ME, 2000, XP

BUILDING THE SAMPLE

Macintosh

The sample builds two targets for Macintosh using the CodeWarrior development environment: 

SlideShowImporter PPC - standalone PPC movie import component for Traditional Mac OS. Place the target in your System Folder:Extensions folder.

SlideShowCarb.component - standalone Carbon sequence grabber panel component for Mac OS X. Place the target in your /Library/QuickTime/ folder.

The sample builds two targets for Macintosh using the Project Builder development environment: 

SlideShowDyLib.component - a shared library movie import component for Mac OS X. Place the target in your /Library/QuickTime/ folder.

SlideShowImport.component - a bundle containing the movie import component for Mac OS X.

Warnings you can safely ignore

When you see the following warnings when building this sample they can be safely ignored. They are only mentioned here so no one thinks they're doing anything wrong. 

Link Warning : A 'cfrg'(0) resource was found.  It will override some project settings.

Warning: Ignored duplicate resource 'cfrg' (0) in 'SlideShowImport.component'.

Link Warning : entry-point 'SlideShowImportComponentDispatchRD' is not a descriptor


Windows

The sample builds two target for Windows:

SlideShowImport Release - release build, standalone movie import component for Windows 98, ME, 2000 or XP. Place the target in your \Windows\System(32)\QuickTime\ folder.

SlideShowImport Debug - debug build, standalone movie import component for Windows 98, ME, 2000 or XP. Place the target in your \Windows\System(32)\QuickTime\ folder.

You will need to install the QuickTime 5 or later Windows SDK (called QTDevWin) and set up access paths to directories where VC++ can find the QuickTime includes, resource includes, libraries and build tools. This can be done from the VC++ 'Tools->Options...' menu. Select the 'Directories' tab then add the correct paths for each.

This project has been set up assuming your project can locatge the QTDevWin folder (from the Windows SDK) as follows:

..\QTDevWin\foo\your project folder here\


Include File paths added:
     ..\..\QTDevWin\CIncludes\
     ..\..\QTDevWin\ComponentIncludes\
     ..\..\QTDevWin\RIncludes

Library File path added:
     ..\..\QTDevWin\Libraries\

ADDITIONAL NOTES (Windows builds)

QuickTimeComponents.h interface file

Note also we have provided a modified copy of the QuickTimeComponents.h interface file. There's a bug in the current QuickTime 5 for Windows QuickTimeComponents.h interface file in it's use of the GLUE2 macro. Use the modified copy included in the project instead.

FEEDBACK

Send feedback to: <http://developer.apple.com/contact/feedback.html>
