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


property targetData : missing value
property currentProcessSelection : {}
property idleVal : 60

on awake from nib theObject
	
	if name of theObject is "Processes" then
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
		
	else if name of theObject is "processesTable" then
		-- Make new data source for table view
		set targetData to make new data source at end of data sources with properties {name:"processesTable"}
		
		-- Target the data source
		tell targetData
			-- Create each of the data columns, including the sort information for each column  	
			make new data column at end of data columns with properties {name:"ProcessNameVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
			make new data column at end of data columns with properties {name:"frontmostVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
			make new data column at end of data columns with properties {name:"visibleVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
			make new data column at end of data columns with properties {name:"creatorTypeVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
			make new data column at end of data columns with properties {name:"fileTypeVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
			make new data column at end of data columns with properties {name:"scriptingVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
			make new data column at end of data columns with properties {name:"opensInClassicVal", sort order:ascending, sort type:alphabetical, sort case sensitivity:case sensitive}
		end tell
		
		-- Make this a sorted data source
		set sorted of targetData to true
		
		-- Set the "name" data column as the sort column
		set sort column of targetData to data column "ProcessNameVal" of targetData
		
		-- Set the data source of the table view to the new data source
		set data source of theObject to targetData
		
		-- Register for "file names" drag types
		tell theObject to register drag types {"file names"}
	end if
end awake from nib

-- on launch of the application get the processes
on launched theObject
	updateProcesses()
end launched

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

on clicked toolbar item theObject
	set theToolbarName to name of theObject
	if theToolbarName = "inspector item" then
		inpectProcesses()
	else if theToolbarName = "Hide Process" then
		if class of currentProcessSelection = list then
			repeat with i in currentProcessSelection
				tell application "System Events"
					set visible of application process i to false
				end tell
			end repeat
			updateProcesses()
		end if
	else if theToolbarName = "Show Process" then
		repeat with i in currentProcessSelection
			tell application "System Events"
				set visible of application process i to true
			end tell
		end repeat
		updateProcesses()
	else if theToolbarName = "Update item" then
		updateProcesses()
	else if theToolbarName = "reveal item" then
		repeat with i in currentProcessSelection
			tell application "System Events"
				set fileRef to file of process i as text
			end tell
			tell application "Finder" to activate
			tell application "Finder"
				set foo to make new Finder window
				set target of foo to file (fileRef)
			end tell
		end repeat
	else if theToolbarName = "quit process" then
		set theRowCount to count of currentProcessSelection
		if theRowCount = 1 then
			set theReply to display alert "Are you sure you want to quit the process?" default button "OK" alternate button "Cancel"
		else
			set theReply to display alert "Are you sure you want to quit the processes?" default button "OK" alternate button "Cancel"
		end if
		set theReply to button returned of theReply
		if theReply = "OK" then
			repeat with i in currentProcessSelection
				try
					-- we don't want to quit "System Events"
					if i does not contain "System Events" then
						tell application i to quit
						delay 0.5 -- let process quit before updating list below
					end if
				end try
			end repeat
			updateProcesses()
		end if
	end if
end clicked toolbar item

on selection changed theObject
	set currentProcessSelection to {}
	try
		set currentProcessSelectionTEMP to selected data rows of theObject
		repeat with i in currentProcessSelectionTEMP
			set end of currentProcessSelection to contents of data cell "ProcessNameVal" of i
		end repeat
	on error errMsg
		log errMsg
	end try
end selection changed

on idle theObject
	set popupName to title of popup button "idlepopupmenu" of window "Processes"
	if popupName is not equal to "Update manually" then
		updateProcesses()
	end if
	return idleVal
end idle

on choose menu item theObject
	if name of theObject = "idlepopupmenu" then
		set popupName to title of popup button "idlepopupmenu" of window "Processes"
		
		if popupName = "Update every minute" then
			set idleVal to 60
		else if popupName = "Update every 3 minutes" then
			set idleVal to 180
		else if popupName = "Update every 5 minutes" then
			set idleVal to 300
		else if popupName = "Update every 10 minutes" then
			set idleVal to 600
		end if
	end if
end choose menu item

on drop theObject drag info dragInfo
	set theDragSucceeded to false
	
	-- Get the list of data types on the pasteboard
	set dataTypes to types of pasteboard of dragInfo
	
	-- We are only interested in "file names" data types
	if "file names" is in dataTypes then
		
		-- Initialize the list of files to an empty list
		set theFiles to {}
		
		-- We want the data as a list of file names, so set the preferred type to "file names"
		set preferred type of pasteboard of dragInfo to "file names"
		
		-- Get the list of files from the pasteboard
		set theFiles to contents of pasteboard of dragInfo
		
		-- Make sure we have at least one item
		if (count of theFiles) > 0 then
			set containsApp to false
			-- repeat through files list and launch any file that is an application
			repeat with i in theFiles
				set TheFile to i as POSIX file
				set FileKind to kind of (info for TheFile)
				if FileKind = "Application" then
					set shortName to short name of (info for TheFile)
					tell application shortName to launch
					set containsApp to true
				end if
			end repeat
			if containsApp then
				updateProcesses()
				set theDragSucceeded to true
			end if
		end if
	end if
	
	return theDragSucceeded
end drop


-- update the processes list
on updateProcesses()
	set listcnt to getProcessCount()
	processLoop(listcnt)
end updateProcesses

--Get name of every process and return a list
on getProcessCount()
	try
		tell application "System Events"
			set processNameList to name of every process
			set listcnt to count of processNameList
		end tell
	end try
	set contents of text field "processCount" of window "Processes" to listcnt
	
	return listcnt
end getProcessCount

-- process the processes list
on processLoop(listcnt)
	set update views of targetData to false
	set progressIndicatorStepValue to (100 / listcnt)
	
	set processInfoRecord to {}
	set processRowNumber to 1
	repeat listcnt times
		set end of processInfoRecord to getProcessInfo(processRowNumber)
		set progressIndicatorVal to contents of progress indicator "progressindicator" of window "Processes"
		set contents of progress indicator "progressindicator" of window "Processes" to (progressIndicatorVal + progressIndicatorStepValue)
		set processRowNumber to processRowNumber + 1
	end repeat
	
	set content of targetData to processInfoRecord
	
	set contents of progress indicator "progressindicator" of window "Processes" to 0
	
	try
		-- clear any previous table view row selections, in order to prevent stale row data
		set selected rows of table view "processesTable" of scroll view "processesScrollView" of window "Processes" to {}
	end try
	
	set update views of targetData to true
end processLoop

-- Get individual process information
on getProcessInfo(processRowNumber)
	try
		tell application "System Events"
			set processInfo to {(name of process processRowNumber), (frontmost of process processRowNumber as string), (visible of process processRowNumber as string), (creator type of process processRowNumber), (file type of process processRowNumber), (has scripting terminology of process processRowNumber) as string, (Classic of process processRowNumber) as string}
		end tell
	on error
		-- If System Events encounters an error, try again after a delay
		delay 0.5
		try
			tell application "System Events"
				set processInfo to {(name of process processRowNumber), (frontmost of process processRowNumber as string), (visible of process processRowNumber as string), (creator type of process processRowNumber), (file type of process processRowNumber), (has scripting terminology of process processRowNumber) as string, (Classic of process processRowNumber) as string}
			end tell
		on error
			-- If the second try also fails, return placeholder data and return
			set processInfo to {"unknown", "unknown", "unknown", "unknown", "unknown", "unknown", "unknown"}
		end try
	end try
	return processInfo
end getProcessInfo

-- Bring up a inspector window with additional process information
on inpectProcesses()
	repeat with i in currentProcessSelection
		if (i as Unicode text) is not equal to "Processes" then
			set theID to make new document at end with properties {name:i}
			set theID to id of theID
			try
				tell application "System Events"
					set PropertiesRecord to properties of process i
					set creatorCode to creator type of PropertiesRecord
					set unixidVal to unix id of PropertiesRecord
					set descriptionVal to description of PropertiesRecord
					set idVal to id of PropertiesRecord
					set HighLevelEventsVal to ((accepts remote events of PropertiesRecord) as Unicode text)
					set RemoteEventsVal to ((accepts high level events of PropertiesRecord) as Unicode text)
					set backgroundOnlyVal to ((background only of PropertiesRecord) as Unicode text)
					set HasScriptingTermsVal to ((has scripting terminology of PropertiesRecord) as Unicode text)
				end tell
				tell window of document id theID
					set contents of text field "nameField" of box "InspectorBox" to (name of PropertiesRecord)
					set contents of text field "displayedNameField" of box "InspectorBox" to (displayed name of PropertiesRecord)
					set contents of text field "FileRefField" of box "InspectorBox" to (file of PropertiesRecord)
					set contents of text field "FileTypeField" of box "InspectorBox" to (file type of PropertiesRecord)
					set contents of text field "creatorcodeField" of box "InspectorBox" to (creatorCode)
					set contents of text field "frontmostTextField" of box "InspectorBox" to ((frontmost of PropertiesRecord) as text)
					set contents of text field "unixidField" of box "InspectorBox" to (unixidVal)
					set contents of text field "descriptionField" of box "InspectorBox" to (descriptionVal)
					set contents of text field "idField" of box "InspectorBox" to (idVal)
					set contents of text field "HighLevelEventsField" of box "InspectorBox" to (HighLevelEventsVal)
					set contents of text field "RemoteEventsField" of box "InspectorBox" to (RemoteEventsVal)
					set contents of text field "backgroundOnlyField" of box "InspectorBox" to (backgroundOnlyVal)
					set contents of text field "HasScriptingTermsField" of box "InspectorBox" to (HasScriptingTermsVal)
				end tell
			end try
		end if
	end repeat
end inpectProcesses
