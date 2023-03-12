README - QTWiredSprites

This sample code creates a wired sprite movie containing one sprite track.
The sprite track contains six sprites: two penguins and four buttons.

The four buttons are initially invisible. When the mouse enters (or "rolls over") 
a button, it appears.

When the mouse is clicked inside a button, its image changes to its "pressed" image.
When the mouse is released, its image changes back to its "unpressed" image. If the 
mouse is released inside the button,  an action is triggered. The buttons perform 
the actions of go to beginning of movie, step backward, step forward, and go to end 
of movie.
	
The first penguin shows all of the buttons when the mouse enters it, and hides 
them when the mouse exits.
The first penguin is the only sprite that has properties that are overriden by 
the override sprite samples.
These samples override its matrix (in order to move it) and its image index 
(in order to make it "waddle").

When the mouse is clicked on the second penguin, it changes its image index to 
it's "eyes closed" image. When the mouse is released, it changes back to its 
normal image. This makes it appear to blink when clicked on.
When the mouse is released over the penguin, several actions are triggered. 
Both penguins' graphics states are toggled between copyMode and blendMode, 
and the movie's rate is toggled between zero and one.

The second penguin moves once per second. This occurs whether the movie's rate 
is currently zero or one, because it is being triggered by a gated idle event. 
When the penguin receives the idle event, it changes its matrix using an action 
which uses min, max, delta, and wraparound options.

The movie's looping mode is set to palindrome by a frame-loaded action.

For Windows builds using Microsoft Visual C++, the Makefile (QTWiredSprites.mak)
automatically calls Rez to create the resource file QTWiredSprites.qtr from the
input file QTWiredSprites.r. The .qtr file must reside in the same directory as the
application QTWiredSprites.exe. For final delivery of your product, you should
insert the .qtr file into the .exe file by using the tool RezWack.

Enjoy,
QuickTime Team

