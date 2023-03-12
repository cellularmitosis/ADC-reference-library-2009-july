README - QTSprites

This sample code creates a sprite movie containing one sprite track. 
The sprite track contains three sprites: a space ship, an icon, and a 
world. Optionally, the track also contains a fourth sprite, a background 
picture. The space ship and icon move about as the sprite movie plays. 
See the file QTSprites.c for further details.

This sample code also shows how to test for mouse clicks ("hits") on a
sprite. Open a movie containing a sprite track and then click on a sprite;
its visibility state is toggled between visible and invisible. 

For Windows builds using Microsoft Visual C++, the Makefile (QTSprites.mak)
automatically calls Rez to create the resource file QTSprites.qtr from the
input file QTSprites.r. The .qtr file must reside in the same directory as the
application QTSprites.exe. For final delivery of your product, you should
insert the .qtr file into the .exe file by using the tool RezWack.

Enjoy,
QuickTime Team

