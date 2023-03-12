(*

File: Document.applescript

Abstract: Implementation for ScriptView documents

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*)

on data representation theObject of type ofType
	(*Return the data that is to be stored in your document here.*)
end data representation

on load data representation theObject of type ofType with data withData
	(* The withData contains the data that was stored in your document that you provided in the "data representation" event handler. Return "true" if this was successful, or false if not.*)
	return true
end load data representation

on awake from nib theObject
	-- Make the new toolbar, giving it a unique identifier
	set documentToolbar to make new toolbar at end with properties {name:"document toolbar", identifier:"document toolbar identifier", allows customization:true, auto sizes cells:true, display mode:default display mode, size mode:default size mode}
	
	-- Setup the allowed and default identifiers.
	set allowed identifiers of documentToolbar to {"record item identifier", "stop item identifier", "run item identifier", "compile item identifier", "space item identifier", "eraser item identifier", "flexible space item identifer", "separator item identifier", "customize toolbar item identifer", "formatting item identifier", "new record identifier", "delete record identifier"}
	set default identifiers of documentToolbar to {"record item identifier", "stop item identifier", "run item identifier", "separator item identifier", "compile item identifier", "eraser item identifier", "separator item identifier", "new record identifier", "delete record identifier", "separator item identifier", "customize toolbar item identifer"}
	
	--  Assign the OSAScriptController to a variable we can use for toolbar properties
	set scriptView to text view "scriptView" of scroll view "scriptView" of split view "main" of theObject
	set scriptController to call method "controller" of scriptView
	
	-- Create the toolbar items, adding them to the toolbar.
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"compile item identifier", name:"compile item", label:"Compile", palette label:"Compile", tool tip:"Compile script", image name:"CompileScript", target:scriptController, action method:"compileScript:"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"stop item identifier", name:"stop item", label:"Stop", palette label:"Stop", tool tip:"Stop script", image name:"StopScript", target:scriptController, action method:"stopScript:"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"record item identifier", name:"record item", label:"Record", palette label:"Record", tool tip:"Record", image name:"RecordScriptImage", target:scriptController, action method:"recordScript:"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"run item identifier", name:"run item", label:"Run", palette label:"Run", tool tip:"Run script", image name:"RunScript", target:scriptController, action method:"runScript:"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"eraser item identifier", name:"eraser item", label:"Erase", palette label:"Eraser", tool tip:"Erase contents of script and results fields", image name:"eraser"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"new record identifier", name:"new record", label:"Add Record", palette label:"Add Record", tool tip:"Add Record", image name:"newRecord"}
	make new toolbar item at end of toolbar items of documentToolbar with properties {identifier:"delete record identifier", name:"delete record", label:"Delete Record", palette label:"Delete Record", tool tip:"Delete Record", image name:"deleteRecord"}
	
	-- Assign our toolbar to the window
	set toolbar of theObject to documentToolbar
end awake from nib
