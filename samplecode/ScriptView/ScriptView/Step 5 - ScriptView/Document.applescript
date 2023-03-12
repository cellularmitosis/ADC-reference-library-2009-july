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
	set theWindow to front window of theObject
	set theDataSource to data source of table view "scriptTable" of scroll view "scriptTable" of split view "main" of theWindow
	set theTableViewData to contents of every data cell of every data row of theDataSource
	set theSortColumn to sort column of theDataSource
	set theScriptSource to contents of text view "scriptView" of scroll view "scriptView" of split view "main" of theWindow
	set theWindowBounds to bounds of front window
	set theData to {theScriptDatabase:theTableViewData, theSortColumnName:name of theSortColumn, theScriptSource:theScriptSource, theWindowBounds:theWindowBounds}
	
	return theData
end data representation

on load data representation theObject of type ofType with data withData
	(* The withData contains the data that was stored in your document that you provided in the "data representation" event handler. Return "true" if this was successful, or false if not.*)
	set theWindow to front window of theObject
	
	set theDataSource to data source of table view "scriptTable" of scroll view "scriptTable" of split view "main" of theWindow
	
	set sort column of theDataSource to data column (theSortColumnName of withData) of theDataSource
	set contents of text view "scriptView" of scroll view "scriptView" of split view "main" of theWindow to (theScriptSource of withData)
	set bounds of front window to (theWindowBounds of withData)
	
	append the theDataSource with (theScriptDatabase of withData)
	return true
end load data representation

on awake from nib theObject
	set theName to name of theObject
	
	if theName = "scriptable" then
		-- Create the data source
		set theDataSource to make new data source at end of data sources with properties {name:"scriptTable"}
		
		-- Create each of the data columns, including the sort information for each column
		make new data column at end of data columns of theDataSource with properties {name:"scriptDate", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		make new data column at end of data columns of theDataSource with properties {name:"scriptName", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		make new data column at end of data columns of theDataSource with properties {name:"scriptSource", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		make new data column at end of data columns of theDataSource with properties {name:"scriptResult", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		make new data column at end of data columns of theDataSource with properties {name:"scriptComments", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		
		-- Make this a sorted data source
		set sorted of theDataSource to true
		
		-- Set the "name" data column as the sort column
		set sort column of theDataSource to data column "scriptDate" of theDataSource
		
		-- Set the data source of the table view to the new data source
		set data source of theObject to theDataSource
	else
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
	end if
end awake from nib

property theRowVar : ""

on clicked toolbar item theObject
	set theToolbarName to name of theObject
	if theToolbarName = "new record" then
		addRecord()
	else if theToolbarName = "delete record" then
		deleteRecord()
	else if theToolbarName = "eraser item" then
		set contents of text view "scriptView" of scroll view "scriptView" of split view "main" of front window to ""
		set contents of text view "resultView" of scroll view "resultView" of split view "main" of front window to ""
	end if
end clicked toolbar item

on selection changed theObject
	try
		set theRowVar to selected data row of theObject
		set theDataSource to data source of theObject
		set contents of text view "resultView" of scroll view "resultView" of split view "main" of front window to ""
		set ScriptSourcetVar to contents of data cell "scriptSource" of theRowVar
		set contents of text view "scriptView" of scroll view "scriptView" of split view "main" of front window to ScriptSourcetVar
	end try
end selection changed

on column clicked theObject table column tableColumn
	-- Get the data source of the table view
	set theDataSource to data source of theObject
	
	-- Get the identifier of the clicked table column
	set theColumnIdentifier to identifier of tableColumn
	
	-- Get the current sort column of the data source
	set theSortColumn to sort column of theDataSource
	
	-- If the current sort column is not the same as the clicked column then switch the sort column
	if (name of theSortColumn) is not equal to theColumnIdentifier then
		set the sort column of theDataSource to data column theColumnIdentifier of theDataSource
	else
		-- Otherwise change the sort order
		if sort order of theSortColumn is ascending then
			set sort order of theSortColumn to descending
		else
			set sort order of theSortColumn to ascending
		end if
	end if
	
	-- We need to update the table view (so it will be redrawn)
	update theObject
end column clicked

on addRecord()
	set modified of front document to true
	try
		set scriptDate to (current date) as string
		set scriptName to ""
		set scriptResult to contents of text view "resultView" of scroll view "resultView" of split view "main" of front window
		set scriptSource to contents of text view "scriptView" of scroll view "scriptView" of split view "main" of front window
		set scriptComments to ""
		set the rowData to {scriptDate:scriptDate, scriptName:scriptName, scriptSource:scriptSource, scriptResult:scriptResult, scriptComments:scriptComments}
		tell me to addToDataSource(rowData)
	end try
end addRecord

on deleteRecord()
	set modified of front document to true
	try
		set tableDataSource to data source of table view "scriptTable" of scroll view "scriptTable" of split view "main" of front window
		tell tableDataSource
			delete theRowVar
		end tell
		set selected rows of table view "scriptTable" of scroll view "scriptTable" of split view "main" of front window to {}
	end try
end deleteRecord

on addToDataSource(rowData)
	set tableDataSource to data source of table view "scriptTable" of scroll view "scriptTable" of split view "main" of front window
	try
		tell tableDataSource
			set theTableRow to make new data row at end of data rows
			set contents of data cell "scriptName" of theTableRow to scriptName of rowData
			set contents of data cell "scriptSource" of theTableRow to scriptSource of rowData
			set contents of data cell "scriptResult" of theTableRow to scriptResult of rowData
			set contents of data cell "scriptComments" of theTableRow to scriptComments of rowData
			set contents of data cell "scriptDate" of theTableRow to scriptDate of rowData
			set NumOfRows to number of data rows
		end tell
	on error errMsg
		display dialog errMsg
	end try
end addToDataSource
