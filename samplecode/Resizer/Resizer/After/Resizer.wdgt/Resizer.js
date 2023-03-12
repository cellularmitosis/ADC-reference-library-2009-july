/*

File: Resizer.js

Abstract: JavaScript logic for Resizer example widget

Version: 1.0

� Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

var gInfoButton, gDoneButton;
var gFrontHeight, gFrontWidth;

function setup()
{
    gInfoButton = new AppleInfoButton(document.getElementById("infoButton"), document.getElementById("front"), "black", "black", showPrefs);
	gDoneButton = new AppleGlassButton(document.getElementById("doneButton"), "Done", hidePrefs);
}

function showPrefs()
{
	
	// store the front height and width values to restore on return to front
	gFrontHeight = window.innerHeight;
	gFrontWidth = window.innerWidth;

	
	// Two AppleRect objects store the starting and finshing rectangle sizes
	var startingRect = new AppleRect (0, 0, window.innerWidth, window.innerHeight);
	var finishingRect = new AppleRect (0, 0, 320, 150);
	
	// The RectAnimation specifies the range of values and the handler to call at the animator's interval
	var currentRectAnimation = new AppleRectAnimation( startingRect, finishingRect, rectHandler );

	// The Animator is the timer for the animation
	var currentAnimator = new AppleAnimator (500, 13);
	
	// Associate the animator and animation
	currentAnimator.addAnimation(currentRectAnimation);
	
	// Set a handler to be called when the animation is done (in this case, flip the widget to its back)
	currentAnimator.oncomplete = flipToBack;
	
	// Once everything is set up, start the animation
	currentAnimator.start();
	
}

function flipToBack()
{
    var front = document.getElementById("front");
    var back = document.getElementById("back");
 
	if (window.widget) {
		widget.prepareForTransition("ToBack");
 	}
		
    front.style.display="none";
    back.style.display="block";
 
    if (window.widget){
        setTimeout ('widget.performTransition();', 0);
		widget.setCloseBoxOffset(13,14);
    }
}

// The rectHandler is called each interval by the Animator.  It passes in the animation values at
// this point in the animation.  The values are used to resize parts of the interface and the
// widget as a whole

function rectHandler( rectAnimation, currentRect, startingRect, finishingRect )
{
	document.getElementById("middle").style.height = (currentRect.bottom-40);
	document.getElementById("bottom").style.top = (currentRect.bottom-13);
	window.resizeTo(currentRect.right, currentRect.bottom);
}


function hidePrefs()
{
    var front = document.getElementById("front");
    var back = document.getElementById("back");
	
	if (window.widget) {
		widget.prepareForTransition("ToFront");
 	}

    back.style.display="none";
    front.style.display="block";
	
    if (window.widget) {
        setTimeout ('widget.performTransition();', 0);
		widget.setCloseBoxOffset(7,9);
	}
	
	// Once the flip to the front is complete, flipToFront resizes the widget back to its former size.
	setTimeout("flipToFront()", 750);
		
}


// flipToFront uses another AppleRectAnimation to resize the widget to its previous size

function flipToFront()
{
	var startingRect = new AppleRect (0, 0, 320, 150);
	var finishingRect = new AppleRect (0, 0, gFrontWidth, gFrontHeight);

	var currentRectAnimation = new AppleRectAnimation( startingRect, finishingRect, rectHandler );

	var currentAnimator = new AppleAnimator (500, 13);
	currentAnimator.addAnimation(currentRectAnimation);
	currentAnimator.start();	
}



// Standard widget resize code follows...

var growboxInset;
 
function mouseDown(event)
{
 
    document.addEventListener("mousemove", mouseMove, true);
    document.addEventListener("mouseup", mouseUp, true);
 
    growboxInset = {x:(window.innerWidth - event.x), y:(window.innerHeight - event.y)};
 
    event.stopPropagation();
    event.preventDefault();
}
 
function mouseMove(event)
{
	// checks if the reported event data is legit or not
	if((event.x == -1 ) ) { break; }
	
	var x = event.x + growboxInset.x;
    var y = event.y + growboxInset.y;
 
	if(x < 113) 	// an arbitrary minimum width
		x = 113;

	if(y < 74) 		// an arbitrary minimum height
		y = 74;

	document.getElementById("middle").style.height = (y-40);
	document.getElementById("bottom").style.top = (y-13);
    window.resizeTo(x,y);
 
    event.stopPropagation();
    event.preventDefault();
}
 
function mouseUp(event)
{
    document.removeEventListener("mousemove", mouseMove, true);
    document.removeEventListener("mouseup", mouseUp, true); 
 
    event.stopPropagation();
    event.preventDefault();
}
