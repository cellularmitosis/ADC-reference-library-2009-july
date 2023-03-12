README

Offscreen Sample Version 1.0
5/15/98
README - Offscreen


1. About Offscreen

Offscreen is a sample application that demonstrates how to use
the Quicktime for Windows 3.0 NewGWorldFromHBITMAP function. The code "wraps"
an offscreen GWorld around an existing DIB (created with the Win32 CreateDIBSection function)
using NewGWorldFromHBITMAP, and then by draws a movie frame-by-frame into the offscreen GWorld.

2. Specifics

The code first creates a memory device context and DIB (using CreateDIBSection)
to use with the NewGWorldFromHBITMAP function. If you don't create your own
memory device context and DIB, NewGWorldFromHBITMAP will do it for you.
Note if you do create your own DIB using CreateDIBSection, you may want to specify
a negative bitmap height parameter in the biHeight field in the BITMAPINFOHEADER structure
as this will designate the bitmap as "top-down" with the origin in the upper-left corner.
If you don't do this, your bitmap will be a "bottom-down" bitmap with origin in the lower-left corner,
and any movie you draw into a GWorld associated with this bitmap will show up as inverted.

Next, the code creates the offscreen GWorld using NewGWorldFromHBITMAP.
A movie is "drawn" to this newly created offscreen by first setting the movie's GWorld
via the Quicktime SetMovieGWorld function and then calling the UpdateMovies and MoviesTask functions.
Finally, a background image and other messages are drawn to the memory device context,
and the memory device context contents are drawn to the screen using the Win32 BitBlt function.

Note the code contains very limited error checking. It's up to you to add the
appropriate error-handling routines.

Offscreen currently can only be compiled and run under Windows 95/NT. Quicktime 3.0 is required.

2. Using Offscreen

Launch the application, and open any QuickTime movie using the File Open menu item.
Next, a window is displayed showing the first frame of the movie along with a background image.
Press the <enter> key to display the next frame of the movie (the movie will wrap once it
reaches the last frame)


Enjoy,
QuickTime Team