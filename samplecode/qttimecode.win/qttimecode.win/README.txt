README - QTTimeCode

QTTimeCode is a sample application that illustrates how to use the timecode media handler.  
You can use QTTimeCode to add a timecode track to a movie, to extract
the current timecode value from a movie, to extract the source
information about a timecode track, and to toggle the display of a
movie's timecode track.

The main timecode track is contained in the files QTTimeCode.c
and QTTimeCode.h. 

For Windows builds using Microsoft Visual C++, the Makefile (QTTimeCode.mak)
automatically calls Rez to create the resource file QTTimeCode.qtr from the
input file QTTimeCode.r. The .qtr file must reside in the same directory as the
application QTTimeCode.exe. For final delivery of your product, you should
insert the .qtr file into the .exe file by using the tool RezWack.


Enjoy,
QuickTime Team

