README - QT3DTween

QT3DTween is a sample application that creates a QuickTime movie 
containing both a 3D track and a tween track. The 3D track is created 
by reading data from any 3DMF file. The tween track contains 
QuickDraw3D camera data. The camera data in the tween track 
specifies both an initial position for the camera and a final 
position for the camera. The result of the tween track can be 
seen as the movie is played: the objects in the scene will appear 
to move. 

The code is based on the QuickTime 3.0 APIs. Please note that 
all QuickTime atom data must be in big-endian format. The code 
makes use of the QuickTime 3.0 endian format conversion routines 
contained in the interface file Endian.h to do any required 
translations.

Note also the code contains very limited error checking. 
It's up to you to add the appropriate error-handling routines.

QT3DTween can be compiled and run under the MacOS and under Windows. 
The main tweening code is found in the file QT3DTween.c. 
The remaining files in this folder are part of the general 
Mac and Windows support code.

USING QT3DTween

Launch the application and select either "Use Camera Tweening" 
or "Use Rotation Tweening" (or both) from the Test menu. 
Then select the menu item "Create 3D Tween Movie". 
A window will appear asking you to select a 3DMF file 
(a sample 3DMF file is included, but you can choose any 3DMF file). 
Once a 3DMF file is selected, the selection window will go away; 
you'll then be asked to select a destination for the new movie. 
Open the newly created movie and play the movie.

Enjoy,
QuickTime Team

