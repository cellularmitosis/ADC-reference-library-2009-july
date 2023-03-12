(*
File:Processes.applescript

Abstract: AppleScripts used in the Processes example.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
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

Copyright © 2006 Apple Computer, Inc., All Rights Reserved
*)

on awake from nib theObject
	delay 2
	set documentToolbar to make new toolbar at end with properties {name:"document toolbar", identifier:"document toolbar identifier", allows customization:true, auto sizes cells:true, display mode:default display mode, size mode:default size mode}
	
	-- Setup the allowed and default identifiers.
	set allowed identifiers of documentToolbar to {"quit process identifier", "inspector item identifier", "reveal item identifier", "space item identifier", "update item identifier", "flexible space item identifer", "separator item identifier", "customize toolbar item identifer", "formatting item identifier", "show process identifier", "hide process identifier"}
	set default identifiers of documentToolbar to {"quit process identifier", "inspector item identifier", "separator item identifier", "show process identifier", "hide process identifier", "reveal item identifier", "separator item identifier", "flexible space item identifier", "space item identifier", "space item identifier", "space item identifier", "space item identifier", "space item identifier", "space item identifier", "update item identifier"}
	
	-- Create the toolbar items, adding them to the toolbar.
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"inspector item identifier", name:"inspector item", label:"Inspect", palette label:"Inspect", tool tip:"Inspect Process", image name:"Inspect"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"quit process identifier", name:"quit process", label:"Quit", palette label:"Quit", tool tip:"Quit Process", image name:"StopScript"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"reveal item identifier", name:"reveal item", label:"Reveal", palette label:"Reveal", tool tip:"Reveal application", image name:"reveal"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"update item identifier", name:"Update item", label:"Update", palette label:"Update", tool tip:"Update", image name:"update"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"show process identifier", name:"Show Process", label:"Show", palette label:"Show", tool tip:"Show Process", image name:"show"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"hide process identifier", name:"Hide Process", label:"Hide", palette label:"Hide", tool tip:"Hide Process", image name:"hide"}
	-- Assign our toolbar to the window
	set toolbar of theObject to documentToolbar
	
end awake from nib

