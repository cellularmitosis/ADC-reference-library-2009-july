README - QTGRAPHIMP

ABOUT QTGRAPHIMP

This sample code illustrates how to use QuickTime's graphics importer routines
to open and display image files. The graphics importer routines were introduced
in QuickTime version 2.5 as a new way to draw still images. The graphics import
routines (for example, GetGraphicsImporterForFile) use graphics import components
(of component type 'grip') to open and perform other operations on image files.
Essentially, you can use the graphics import routines to insulate your application
from the nitty gritty details of image file format and compression used in the
image.

In this sample code, we allow the user to open an image file; then we draw it into
a window on the screen. Your application, of course, will probably want to do more
interesting things with the image. We also allow the user to save an image using JPEG
compression.

Enjoy,
QuickTime Team
