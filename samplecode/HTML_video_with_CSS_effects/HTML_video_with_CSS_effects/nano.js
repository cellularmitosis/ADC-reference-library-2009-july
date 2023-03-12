/*
Important:

This is sample code demonstrating API, technology or techniques in development.
Although this sample code has been reviewed for technical accuracy, it is not 
final. Apple is supplying this information to help you plan for the adoption of 
the technologies and programming interfaces described herein. This information 
is subject to change, and software implemented based on this sample code should 
be tested with final operating system software and final documentation. Newer 
versions of this sample code may be provided with future seeds of the API or 
technology. For information about updates to this and other developer 
documentation, view the New & Updated sidebars in subsequent documentation seeds.
*/

/*
File: nano.js
Abstract: Defines JavaScript functionality for the iPodNano sample.
Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc.
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/


var movies = 
[
	"media/lasttime_180x102.mp4",
	"media/lasttime_180x102.mp4",
	"media/lasttime_180x102.mp4",
	"media/lasttime_180x102.mp4",
	"media/lasttime_180x102.mp4"
];

var nanoDivs = null;
var currentNano = null;
var stackClosed = true;

// we use the same video for all five iPods so set each to a different time
function videoPlayable(evt) 
{
	var video = evt.target;
	var time = (Math.random() * video.duration);

	video.currentTime = time;
	video.removeEventListener("canshowcurrentframe", videoPlayable);
}


// the document has loaded, set everything up
function initialize(evt) 
{
	var ndx, count;
	var nanoContainer = document.getElementById('nanoStack');

	// cache the nano divs
	nanoDivs = [];
	var childNodes = nanoContainer.childNodes;
	for ( ndx = 0, count = childNodes.length; ndx < count; ndx++) 
	{
		var child = childNodes[ndx];
		if ( "nano" == child.className )
			nanoDivs[nanoDivs.length] = child;
	}
	
	// setup each of the video elements
	var videos = document.getElementsByTagName("video");
	for ( ndx = 0, count = videos.length; ndx < count; ndx++ ) 
	{
		var video = videos[ndx];

		if ( ndx > 0 ) 
		{
			// listen for the only ready state event we are guaranteed to see (it is fired
			//  as part of the "load" processing. see section 3.12.10.6 of the HTML5 spec)
			video.addEventListener("canshowcurrentframe", videoPlayable, true);
			video.volume = 0.0;
		}

		video.autoplay = true;
		video.playCount = 10000;
		video.src = movies[ndx];
	}
}

// toggle the nanos between stacked and expanded
function toggleNanoStack() 
{
	var ndx, count;
	var nanoContainer = document.getElementById('nanoStack');

	if ( "closed" == nanoContainer.className ) 
	{
		// open the container 
		stackClosed = false;
		nanoContainer.className = "open";
		
		for ( ndx = 0, count = nanoDivs.length; ndx< count; ndx++) 
		{
			var nano = nanoDivs[ndx];
			if ( 0 == ndx)
				currentNano = nano;
			nano.addEventListener("click",nanoClicked);
		}
	}
	else 
	{
		// close the container
		stackClosed = true;
		nanoContainer.className = "closed";
		
		for ( ndx = 0, count = nanoDivs.length; ndx < count; ndx++) 
		{
			var nano = nanoDivs[ndx];
			nano.className = "nano";
			nano.removeEventListener("click", nanoClicked);
		}
		setCurrentNano(nanoDivs[0]);
	}
}

// fade audio down in the previously focused movie, up in the new one
function fadeAudio(previous, current) 
{
	 var previousVideo = previous.getElementsByTagName("video")[0];
	 var currentVideo = current.getElementsByTagName("video")[0];
	 var steps = 10;
	 var increment = 1.0 / steps;
	 var frequency = (1000 * .7) / steps;

	 var _fadeAudio = function() 
	 {
		 var newVolume;

		 newVolume = currentVideo.volume + increment;
		 currentVideo.volume = (newVolume <= 0.5) ? newVolume : 0.5;

		 newVolume = previousVideo.volume - increment;
		 previousVideo.volume = (newVolume >= 0.0) ? newVolume : 0.0;

		 if ( 0 == previousVideo.volume ) 
		 {
			 if ( arguments.callee.interval )
			 {
				 clearInterval(arguments.callee.interval);
				 arguments.callee.interval = null;
			 }
		 }
	 }
	 _fadeAudio.interval = setInterval(_fadeAudio, frequency);
}

// bring one of the nanos to the front of the stack
function setCurrentNano(target) 
{
	var targetVideo = target.getElementsByTagName("video")[0];
	var previousNano = currentNano;

	if ( "nano selected" != target.className ) 
	{
		// clicked on a non-focused nano, start playing, and bring to front if expanded
		targetVideo.play();
						
		if ( !stackClosed )
			target.className = "nano selected";
		if ( currentNano && (target != currentNano) )
			currentNano.className = "nano";

		currentNano = target;
	}
	else if ( currentNano == target )
	{
		// already selected, push it back into the pack and select the first one as
		//  it is now the only one fully visible
		target.className = "nano";
		currentNano = nanoDivs[0];
	}

	fadeAudio(previousNano, currentNano);
}

// nano click handler, only active when stack is expanded
function nanoClicked(evt) 
{
	var target = evt.target;

	if ( "VIDEO" == target.tagName.toUpperCase() ) 
		target = target.parentNode;
	if ( "DIV" != target.tagName.toUpperCase() ) 
		return;
	
	setCurrentNano(target);
}

function videoForControl(element) 
{
	return element.parentNode.getElementsByTagName("video")[0];
}

function playPause(element) 
{
	var video = videoForControl(element);            
	if ( video.paused )
		video.play();
	else
		video.pause();
}

function jumpForward(element)
{
	videoForControl(element).currentTime += 10;
}

function jumpBackward(element)
{
	videoForControl(element).currentTime -= 10;
}

window.addEventListener("load", initialize);
