/*

File: Voices.js

Abstract: JavaScript logic for Voices sample widget.
	This milestone uses synchronous (blocking)
	widget.system calls to speak text.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

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

var currentlyBeingSpoken = null;

// setup() is called when the body of the widget is loaded.  Here, it introduces the widget to the user.


function setup()
{
	if(window.widget) {
		currentlyBeingSpoken = widget.system("/usr/bin/osascript -e 'say \"Welcome to Voices!\" using \"Fred\"'" , doneSpeaking);
	}
}

// voiceChanged() is called when the Voices popup menu changes.  It changes the faked menu text and announces the new voice to the user. 

function voiceChanged(elem)
{
	var chosenVoice = elem.options[elem.selectedIndex].value;
	document.getElementById("voiceMenuText").innerText = chosenVoice;
	
	if(window.widget) {
		if( currentlyBeingSpoken != null) {
			currentlyBeingSpoken.cancel();
			done();
		}
		currentlyBeingSpoken = widget.system("/usr/bin/osascript -e 'say \"Hi, I`m " + chosenVoice + ".\" using \"" + chosenVoice + "\"'" , doneSpeaking);	
	}

}

// speak() gets the input text, sanitizes it for use with osascript, and speaks it to the user using the currently selected voice.

function speak(event)
{
	document.getElementById("speakButton").src = "Images/Button_down.png";
	
	var textToSpeak = document.getElementById('textInput').value;
	var regExForSingleQuote = new RegExp ('\'', 'g') ;
	var textToSpeak = textToSpeak.replace(regExForSingleQuote, '`') ;
	var regExForDoubleQuote = new RegExp ('"', 'g') ;
	var textToSpeak = textToSpeak.replace(regExForDoubleQuote, '') ;

	var voiceMenu = document.getElementById('voicePopup');
	var chosenVoice = voiceMenu.options[voiceMenu.selectedIndex].value;

	var speakButtonText = document.getElementById("speakButtonText");
	speakButtonText.innerText = "Stop";
	
	var theButton = document.getElementById("theButton");
	theButton.onmousedown = null;
	theButton.onmousedown = cancel;
	
	if(window.widget) {
		if( currentlyBeingSpoken != null) {
			currentlyBeingSpoken.cancel();
		}
		currentlyBeingSpoken = widget.system("/usr/bin/osascript -e 'say \"" + textToSpeak + "\" using \"" + chosenVoice + "\"'" , done);	
	}
}

// cancel() is called with the user wants to stop the widget from speaking

function cancel(event)
{
	if(window.widget) {
		currentlyBeingSpoken.cancel();
	}
	
	document.getElementById("speakButton").src = "Images/Button_down.png";

	done();
}

// done() is used as an end handler for widget.system and is called when the user cancels speaking.  It does cleanup of the interface. 

function done()
{
	var speakButtonText = document.getElementById("speakButtonText");
	speakButtonText.innerText = "Speak";

	var theButton = document.getElementById("theButton");

	theButton.onmousedown = null;
	theButton.onmousedown = speak;
	
	doneSpeaking();
}

// doneSpeaking() is the end handler for the two introductions and is called at the end of done().  It clears out the global keeping track of the currently spoken command.

function doneSpeaking()
{
	currentlyBeingSpoken = null;
}

// buttonUpOut() switches the button image to the unclicked version.

function buttonUpOut()
{
	document.getElementById("speakButton").src = "Images/Button.png";
}

// removed() is called when the widget is removed from the Dashboard; it stops any speaking that might be happening

function removed()
{
	if( currentlyBeingSpoken != null) {
		currentlyBeingSpoken.cancel();
	}
}

// here we register for widget events that interest us.

if(window.widget)
{
	widget.onremove = removed;
}