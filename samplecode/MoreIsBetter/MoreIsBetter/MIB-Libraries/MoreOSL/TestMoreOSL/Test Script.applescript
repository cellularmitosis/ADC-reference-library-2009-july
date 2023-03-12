(*
	File:		Test Script.applescript

	Contains:	AppleScript to test TestMoreOSL.

	Written by:	DTS

	Copyright:	Copyright (c) 2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):

$Log: Test\040Script.applescript,v $
Revision 1.2  2001/11/07 15:57:41         
Tidy up headers, add CVS logs, update copyright.


*)

on testFilePath(x)
	tell application "Finder"
		name of startup disk & ":" & "TestMoreOSL Test File " & x
	end tell
end testFilePath

on createTestFile(x)
	set filePath to testFilePath(x)
	tell application "Finder"
		exists file filePath
	end tell
	if not result then
		tell application "TestMoreOSL"
			set newDoc to (make new document)
			save newDoc in file filePath
		end tell
	end if
end createTestFile

on deleteTestFile(x)
	set filePath to testFilePath(x)
	tell application "Finder"
		if exists file filePath then
			delete file filePath
		end if
	end tell
end deleteTestFile

on setupTestEnvironment()
	createTestFile(1)
	createTestFile(2)
	createTestFile(3)
	deleteTestFile(11)
	deleteTestFile(12)
	deleteTestFile(13)
	tell application "TestMoreOSL"
		close every window saving no
		set next unique ID to 128
		make new about window with properties {name:"A", position:{10, 300}}
		make new document with properties {name:"B"}
		make new document with properties {name:"C"}
		make new about window with properties {name:"D", position:{10, 440}}
		set docE to make new document with properties {name:"E"}
		set position of node display of docE to {300, 300}
		set frontmost of window "D" to true
		set frontmost of window "C" to true
		set frontmost of window "B" to true
		set frontmost of window "A" to true
	end tell
end setupTestEnvironment

on testPseudoCListCountAndIndex()
	-- Tests:
	-- ¥ ability of count handler to count lists (PseudoCListCount)
	-- ¥ ability of PseudoCListCount to count with type filtering
	-- ¥ ability of list pseudo-class accessor (PseudoClassOSLAccessorProc) to access cObject by index
	tell application "TestMoreOSL"
		set docList to {}
		repeat with doc in every document
			get contents of doc
			set docList to docList & result
		end repeat
		set t1 to docList = {document id 129, document id 130, document id 132}
		set c1 to count every window
		set c2 to count every window each item
		set c3 to count every window each window
		set c4 to count every window each boolean
		set c5 to count every window each node window
		set c6 to count every window each about window
	end tell
	if not t1 or c1 ­ 5 or c2 ­ 5 or c3 ­ 5 or c4 ­ 0 or c5 ­ 3 or c6 ­ 2 then
		log "¥¥¥ Failure -- testPseudoCListCountAndIndex" & docList
	end if
end testPseudoCListCountAndIndex

on testPseudoCListProperty()
	-- Tests:
	-- ¥ ability of list pseudo-class accessor (PseudoClassOSLAccessorProc) to break down list, call general accessor, and reassemble results
	tell application "TestMoreOSL"
		get name of every document
		if result ­ {"B", "C", "E"} then
			log "¥¥¥ Failure -- testPseudoCListProperty"
		end if
	end tell
end testPseudoCListProperty

on testReadWriteProperty()
	-- Tests:
	-- ¥ property get and set (MOSLGeneralGetter, MOSLGeneralSetter)
	-- ¥ ability of parameter handling to resolve object specifiers (MOSLCoerceObjDesc)
	tell application "TestMoreOSL"
		set oldName to name of window 1
		set name of window 1 to name of window 2
		set success to (name of window 1) = (name of window 2)
		set name of window 1 to oldName
		if not success then
			log "¥¥¥ Failure -- testReadWriteProperty"
		end if
	end tell
end testReadWriteProperty

on testExists()
	-- Tests:
	-- ¥ general support for exists event (MOSLGeneralExists)
	-- ¥ exists of list returns single Boolean (ProcessExistsResult)
	tell application "TestMoreOSL"
		set goodExample to exists window 5
		set badExample to exists window 6
		set secondGoodExample to exists every window
		set secondBadExample to exists {document 1, document 17}
		set thirdGoodExample to exists
	end tell
	if not goodExample or badExample or not secondGoodExample or secondBadExample or not thirdGoodExample then
		log "¥¥¥ Failure -- testExists"
	end if
end testExists

on testPseudoPropertyAccess()
	-- Tests:
	-- ¥ ability to access elements and properties of properties (PseudoCPropertyAccessor)
	tell application "TestMoreOSL"
		set resultsWindowName to name of node display of document 1
		set theResultsNames to name of every node of node display of document 1
	end tell
	if resultsWindowName ­ "B" or theResultsNames ­ {"root1", "root2", "root3"} then
		log "¥¥¥ Failure -- testPseudoPropertyAccess"
	end if
end testPseudoPropertyAccess

on testListFlattening()
	-- Test:
	-- ¥ test code that ensures we only return lists one level deep (RecursiveResolve, PseudoCListAccessor, AEAppendListToList)
	tell application "TestMoreOSL"
		name of every node of every node of node window 1
	end tell
	if result ­ {"root1child1", "root1child2", "root3child1", "root3child2", "root3child3", "root3child4"} then
		log "¥¥¥ Failure -- testListFlattening"
	end if
end testListFlattening

on testWhoseStringCompare()
	-- Tests:
	-- ¥ string compares in whose clauses (CompareDataDescriptors)
	-- ¥ whose clauses in nested specifiers
	tell application "TestMoreOSL"
		set name of about window 2 to "About"
		set greaterThanC to name of every window whose name > "C"
		set greaterThanOrEqualToC to name of every window whose name ³ "C"
		set lessThanC to name of every window whose name < "C"
		set lessThanOrEqualToC to name of every window whose name ² "C"
		set equalToC to name of every window whose name = "C"
		set containsA to name of every window whose name contains "A"
		set startsWithA to name of every window whose name starts with "A"
		set endsWithT to name of every window whose name ends with "T"
		set name of about window 2 to "D"
		set whoseNameList1 to name of (nodes whose name contains "2") of node 1 of node window 1
	end tell
	if greaterThanC ­ {"E"} Â
		or greaterThanOrEqualToC ­ {"C", "E"} Â
		or lessThanC ­ {"A", "B", "About"} Â
		or lessThanOrEqualToC ­ {"A", "B", "C", "About"} Â
		or equalToC ­ {"C"} Â
		or containsA ­ {"A", "About"} Â
		or startsWithA ­ {"A", "About"} Â
		or endsWithT ­ {"About"} Â
		or whoseNameList1 ­ {"root1child2"} then
		log "¥¥¥ Failure -- testWhoseStringCompare"
	end if
end testWhoseStringCompare

on testWhoseIntegerCompare()
	-- Tests:
	-- ¥ integer compares in whose clauses (CompareDataDescriptors)
	tell application "TestMoreOSL"
		set greaterThan130 to ID of every window whose ID > 130
	end tell
	tell application "TestMoreOSL"
		set lessThan130 to ID of every window whose ID < 130
	end tell
	tell application "TestMoreOSL"
		set equalTo130 to ID of every window whose ID = 130
	end tell
	if greaterThan130 ­ {131, 132} or lessThan130 ­ {128, 129} or equalTo130 ­ {130} then
		log "¥¥¥ Failure -- testWhoseIntegerCompare"
	end if
end testWhoseIntegerCompare

on testCompareTokens()
	-- Tests:
	-- ¥ tests code which compares object tokens (CompareTokens)
	-- ¥ includes tests for inherited equivalence
	tell application "TestMoreOSL"
		set t1 to document 1 = document 1
		set f1 to document 1 ­ document 1
		set f2 to document 1 = document 2
		set t2 to document 1 ­ document 2
		set f3 to document 1 = window 1
		set f4 to window 1 = about window 1
		set f5 to window 2 = node window 1
	end tell
	if not t1 or f1 or f2 or not t2 or f3 or not f4 or not f5 then
		log "¥¥¥ Failure -- testCompareTokens"
	end if
end testCompareTokens

on testGetDataCoercion()
	-- Tests:
	-- ¥ get data event handling of coercions (MOSLGeneralGetData)
	-- ¥ treatment of class "number" as equivalent to class "integer" (MOSLCoerceObjDesc)
	tell application "TestMoreOSL"
		set oldName to name of window 1
		set name of window 1 to "123"
		set newNameAsInteger to name of window 1 as integer
		set newNameAsNumber to name of window 1 as number
		set name of window 1 to oldName
	end tell
	if class of newNameAsInteger ­ integer or newNameAsInteger ­ 123 Â
		and class of newNameAsNumber ­ integer or newNameAsNumber ­ 123 then
		log "¥¥¥ Failure -- testGetDataCoercion"
	end if
end testGetDataCoercion

on testCount()
	-- Tests:
	-- ¥ count event handler (MOSLGeneralCount)
	-- ¥ count with specific direct object
	-- ¥ count with element type (cObject, specific type, and default)
	tell application "TestMoreOSL"
		set objectCount to count
		set windowCount to count windows
		set documentCount to count documents
		set rootNodeCount to count node window 1 each node
		set child1NodeCount to count node 1 of node window 1 each node
	end tell
	if objectCount ­ 5 or windowCount ­ 5 Â
		or documentCount ­ 3 or rootNodeCount ­ 3 or child1NodeCount ­ 2 then
		log "¥¥¥ Failure -- testCount"
	end if
end testCount

on testFormAbsPos()
	tell application "TestMoreOSL"
		set winList to every window
		set lastWinID to ID of window 5
		set lastWinID2 to ID of last window
		set lastWinID3 to ID of window -1
		set firstWinID to ID of window 1
		set firstWinID2 to ID of first window
		set middleWinID to ID of middle window
		set middleDocID to ID of middle document
		set middleAboutWinID to ID of middle about window
		set hitList to {0, 0, 0, 0, 0}
		repeat with i from 1 to 10
			get index of some window
			set item result of hitList to 1
		end repeat
		set hitCount to 0
		repeat with i in hitList
			if contents of i = 1 then
				set hitCount to hitCount + 1
			end if
		end repeat
	end tell
	if length of winList ­ 5 Â
		or lastWinID ­ lastWinID2 Â
		or lastWinID2 ­ lastWinID3 Â
		or firstWinID ­ firstWinID2 Â
		or middleWinID ­ 130 Â
		or middleDocID ­ 130 Â
		or middleAboutWinID ­ 128 Â
		or hitCount < 3 then
		log "¥¥¥ Failure -- testFormAbsPos"
	end if
end testFormAbsPos

on testResultActions()
	-- Tests:
	-- ¥ events that returns lists do return lists (kRADefault)
	-- ¥ events that return nothing return nothing even when called with list of direct objects (kRANone)
	global gNeedToResetTestEnvironment
	
	set a to alias (testFilePath(1))
	tell application "TestMoreOSL"
		-- kRACountList is already covered by testPseudoCListCountAndIndex
		-- kRACollapseBooleanList is already covered by testExists
		
		-- kRADefault
		
		set x to every document
		set goodRADefault to (x = {document id 129, document id 130, document id 132})
		
		-- kRANone
		
		try
			set x to open {a, a, a}
			set goodRANone to false
		on error msg number num
			set goodRANone to (num = -2763)
		end try
	end tell
	if not goodRADefault or not goodRANone then
		log "¥¥¥ Failure -- testResultActions"
	end if
	set gNeedToResetTestEnvironment to true
end testResultActions

on testFormRange()
	-- Tests:
	-- ¥ basic formRange handling
	-- ¥ that reversed ranges are treated the same as forward ranges
	-- ¥ ranges with absolute ordinals as their bounds
	-- ¥ ranges with boundary objects of different types
	-- ¥ ranges with boundary objects not compatible with desired class
	-- ¥ ranges with requesting objects not of the base class
	-- ¥ ranges with boundary objects in different containers
	-- ¥ ranges with a property as a boundary object
	-- ¥ ranges whose container is a list of objects
	-- ¥ nested ranges
	-- ¥ combinations of formRange and formTest
	-- ¥ empty results yield an error
	tell application "TestMoreOSL"
		set winNames to name of windows 1 through 3
		set winNames2 to name of windows 1 through 3
		set winNames3 to name of windows from first window to middle window
		set winNames4 to name of windows last window through 1
		set winNames5 to name of windows from node window 1 to about window 2
		set winNames6 to name of node windows from node window 1 to about window 2
		set winNames7 to name of about windows from node window 1 to about window 2
		set winNames8 to name of windows 1 through about window 2
		try
			nodes 1 through (node 1 of node 3) of node window 1
			set rangeErr1 to 0
		on error msg number errNum
			set rangeErr1 to errNum
		end try
		try
			nodes 1 through (name) of node window 1
			set rangeErr2 to 0
		on error msg number errNum
			set rangeErr2 to errNum
		end try
		try
			documents from window 1 to window 5
		on error msg number errNum
			set rangeErr2a to errNum
		end try
		set nodeNames to name of nodes 1 through 1 of every node window
		set nodeNames2 to name of nodes 1 through 1 of every node of node window 1
		set l to nodes 1 through 1 of nodes 1 through 1 of node window 1
		set r to node 1 of node 1 of node window 1
		set mustBeTrue to l = {r}
		set nodeNames3 to name of items 1 through 2 of (nodes of node window 1)
		set nodeNames4 to name of nodes 1 through 2 of (node windows whose name does not contain "C")
		set nodeNames5 to name of (nodes 1 through 2 whose name contains "root") of node window 1
		try
			node windows 1 thru (node 1 of node window 1)
			set rangeErr3 to 0
		on error msg number errNum
			set rangeErr3 to errNum
		end try
		
		try
			about windows from window 3 to window 3
			set rangeErr4 to 0
		on error msg number errNum
			set rangeErr4 to errNum
		end try
		
	end tell
	if winNames ­ {"A", "B", "C"} Â
		or winNames2 ­ {"A", "B", "C"} Â
		or winNames3 ­ {"A", "B", "C"} Â
		or winNames4 ­ {"A", "B", "C", "D", "E"} Â
		or winNames5 ­ {"B", "C", "D"} Â
		or winNames6 ­ {"B", "C"} Â
		or winNames7 ­ {"D"} Â
		or winNames8 ­ {"A", "B", "C", "D"} Â
		or rangeErr1 ­ 5306 Â
		or rangeErr2 ­ 5307 Â
		or rangeErr2a ­ 5308 Â
		or nodeNames ­ {"root1", "root1", "root1"} Â
		or nodeNames2 ­ {"root1child1", missing value, "root3child1"} Â
		or not mustBeTrue Â
		or nodeNames3 ­ {"root1child1", "root1child2", missing value, "root3child1", "root3child2"} Â
		or nodeNames4 ­ {"root1", "root2", "root1", "root2"} Â
		or nodeNames5 ­ {"root1", "root2"} Â
		or rangeErr3 ­ 5308 Â
		or rangeErr4 ­ -1728 then
		log "¥¥¥ Failure -- testFormRange"
	end if
end testFormRange

on testDataCompare()
	-- Tests:
	-- ¥ testing typeQDPoint for equality, but not relating
	-- ¥ testing typeQDRectangle for equality
	-- ¥ comparing FSSpecs for equality, but not relating
	-- ¥ comparing Booleans, including typeTrue and typeFalse
	-- ¥ no relating Booleans
	-- ¥ comparing typeType for equality, but not relating
	
	set fss to alias (testFilePath(1))
	
	tell application "TestMoreOSL"
		set winPos to position of node window id 130
		set winPosList to every window whose position is winPos
		set t1 to winPosList = {node window id 130}
		
		set winBounds to bounds of node window id 130
		set winBoundsList to every window whose bounds is winBounds
		set t2 to winBoundsList = {node window id 130}
		
		try
			every window whose position > {10, 10}
			set t3 to false
		on error msg number errNum
			set t3 to errNum = 5303
		end try
		
		set newDoc to make new document with properties {file:fss}
		
		set t4 to (index of every document whose file valid is true and location on disk is fss) = {1}
		
		try
			index of every document whose file valid is true and location on disk > fss
			set t5 to false
		on error msg number errNum
			set t5 to errNum = 5303
		end try
		
		close newDoc
		
		set t6 to (name of every window whose resizable is true) = {"B", "C", "E"}
		set t7 to (name of every window whose resizable is false) = {"A", "D"}
		set x to true
		set t8 to (name of every window whose resizable is x) = {"B", "C", "E"}
		try
			index of every window whose resizable > false
			set t9 to false
		on error msg number errNum
			set t9 to errNum = 5303
		end try
		
		set t10 to (name of every window whose best type is node window) = {"B", "C", "E"}
		try
			name of every window whose best type > node window
			set t11 to false
		on error msg number errNum
			set t11 to errNum = 5303
		end try
		
	end tell
	if not t1 or not t2 or not t3 or not t4 Â
		or not t5 or not t6 or not t7 or not t8 Â
		or not t9 or not t10 or not t11 then
		log "¥¥¥ Failure -- testFormRange"
	end if
end testDataCompare

on testDocHandling()
	-- Tests:
	-- ¥ ability to open files and lists of files
	-- ¥ saving untitled documents
	-- ¥ saving modified documents
	-- ¥ save as
	-- ¥ making documents with a file property
	global gNeedToResetTestEnvironment
	
	set a1 to alias (testFilePath(1))
	set a2 to alias (testFilePath(2))
	set a3 to alias (testFilePath(3))
	set fileNameList to {"TestMoreOSL Test File 1", "TestMoreOSL Test File 2", "TestMoreOSL Test File 3"}
	
	set p11 to (testFilePath(11))
	set p12 to (testFilePath(12))
	set p13 to (testFilePath(13))
	
	tell application "TestMoreOSL"
		
		-- Tests opening of files and lists of files.
		
		open a1
		open {a2, a3}
		set frontmost of node display of every document to true -- reverse window list
		set r1 to fileNameList = (name of every document whose file valid is true)
		
		-- Tests saving on unsaved documents.
		
		set unsavedDoc to item 1 of (every document whose file valid is false)
		set oldUnsavedDocName to name of unsavedDoc
		save unsavedDoc in file p11
		set savedDoc to unsavedDoc
		set r2 to name of savedDoc ­ oldUnsavedDocName
		set a11 to alias p11 -- tests "get data" on data object
		set r3 to {savedDoc} = (every document whose file valid is true and location on disk is a11)
		
		-- Setting the modified bit then saving to clear it.
		
		set modified of savedDoc to true
		save savedDoc
		set r4 to not (modified of savedDoc)
		
		-- Saving in a different document.
		
		save savedDoc in file p12
		file of savedDoc
		set r5 to (result as string) = p12
		
		-- Make new document with file property.
		
		set newDoc to make new document with properties {file:a1}
		file of savedDoc
		set r6 to (result as string) = (a1 as string)
	end tell
	if not r1 or not r2 or not r3 or not r4 or not r5 or r6 then
		log "¥¥¥ Failure -- testDocHandling"
	end if
	set gNeedToResetTestEnvironment to true
end testDocHandling

on testMissingValue()
	-- Tests:
	-- ¥ missing values in the list accessor
	-- ¥ single missing values
	tell application "TestMoreOSL"
		class of node 1 of every node of node window 1
		set r1 to result = {node, missing value, node}
		name of node 1 of every node of node window 1
		set r2 to result = {"root1child1", missing value, "root3child1"}
		set r3 to (file of document 1) = missing value
	end tell
	if not r1 or not r2 or not r3 then
		log "¥¥¥ Failure -- testMissingValue"
	end if
end testMissingValue

on testGetSetBulkProperties()
	-- Tests:
	-- ¥ ability to get all properties
	-- ¥ ability to set all properties and have the changed ones take effect
	-- ¥ error when attempting to change a read-only property via properties
	tell application "TestMoreOSL"
		set oldName to name of window 1
		
		set winProps to properties of window 1
		set name of winProps to "Test"
		set properties of window 1 to winProps
		set r1 to (name of window 1) = (name of winProps)
		set modal of winProps to not modal of winProps
		try
			set properties of window 1 to winProps
			set errNum1 to 0
		on error errMsg number errNum
			set errNum1 to errNum
		end try
		
		set name of window 1 to oldName
	end tell
	if not r1 or errNum ­ -10006 then
		log "¥¥¥ Failure -- testGetSetBulkProperties"
	end if
end testGetSetBulkProperties

on testDirectObjectRequirements()
	-- Tests:
	-- ¥ direct object optional
	-- ¥ direct object required
	-- ¥ direct object illegal
	-- ¥Êdirect object bad OK is tested as part of testExists
	tell application "TestMoreOSL"
		-- optional
		set r1 to (count) = (count items)
		-- required
		try
			close
			set r2 to false
		on error errMsg number errNum
			set r2 to (errNum = 5300)
		end try
		-- illegal
		-- Can't seem to trigger this case, AppleScript seems to munge
		-- or ignore any parameters I supply to events that have no direct
		-- object (eg dir obj to "quit" is compiled out by AppleScript,
		-- dir obj to "make" is munged to "new" parameter).
	end tell
	if not r1 or not r2 then
		log "¥¥¥ Failure -- testDirectObjectRequirements"
	end if
end testDirectObjectRequirements

global gNeedToResetTestEnvironment

set testNames to {"testWhoseIntegerCompare", Â
	"testWhoseStringCompare", Â
	"testListFlattening", Â
	"testPseudoPropertyAccess", Â
	"testExists", Â
	"testReadWriteProperty", Â
	"testPseudoCListCountAndIndex", Â
	"testPseudoCListProperty", Â
	"testCompareTokens", Â
	"testGetDataCoercion", Â
	"testCount", Â
	"testResultActions", Â
	"testFormAbsPos", Â
	"testFormRange", Â
	"testDataCompare", Â
	"testDocHandling", Â
	"testMissingValue", Â
	"testGetSetBulkProperties", Â
	"testDirectObjectRequirements"}

choose from list testNames Â
	with prompt Â
	"Select the test(s) you wish to run." default items testNames Â
	OK button name Â
	"Run" with multiple selections allowed without empty selection allowed
set testsToRun to result
if testsToRun = false then
	error number -128
end if
set doingAllTests to (testsToRun = testNames)
tell application "TestMoreOSL"
	if doingAllTests then
		set debug to 0
	else
		set debug to 9
	end if
end tell
set gNeedToResetTestEnvironment to true
repeat with thisTest in testsToRun
	if gNeedToResetTestEnvironment then
		setupTestEnvironment()
		set gNeedToResetTestEnvironment to false
	end if
	set thisTestName to contents of thisTest
	if false or doingAllTests then
		log "Doing test Ò" & thisTestName & "Ó"
	end if
	if thisTestName is "testWhoseIntegerCompare" then
		testWhoseIntegerCompare()
	else if thisTestName is "testWhoseStringCompare" then
		testWhoseStringCompare()
	else if thisTestName is "testListFlattening" then
		testListFlattening()
	else if thisTestName is "testPseudoPropertyAccess" then
		testPseudoPropertyAccess()
	else if thisTestName is "testExists" then
		testExists()
	else if thisTestName is "testReadWriteProperty" then
		testReadWriteProperty()
	else if thisTestName is "testPseudoCListCountAndIndex" then
		testPseudoCListCountAndIndex()
	else if thisTestName is "testPseudoCListProperty" then
		testPseudoCListProperty()
	else if thisTestName is "testCompareTokens" then
		testCompareTokens()
	else if thisTestName is "testGetDataCoercion" then
		testGetDataCoercion()
	else if thisTestName is "testCount" then
		testCount()
	else if thisTestName is "testFormAbsPos" then
		testFormAbsPos()
	else if thisTestName is "testResultActions" then
		testResultActions()
	else if thisTestName is "testFormRange" then
		testFormRange()
	else if thisTestName is "testDataCompare" then
		testDataCompare()
	else if thisTestName is "testDocHandling" then
		testDocHandling()
	else if thisTestName is "testMissingValue" then
		testMissingValue()
	else if thisTestName is "testGetSetBulkProperties" then
		testGetSetBulkProperties()
	else if thisTestName is "testDirectObjectRequirements" then
		testDirectObjectRequirements()
	else
		log "¥¥¥ Bad test selection"
	end if
end repeat
