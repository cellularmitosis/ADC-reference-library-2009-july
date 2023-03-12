/*
	File:		MoreLists.c

	Contains:	List Manager utilities.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreLists.c,v $
Revision 1.4  2002/11/08 23:33:46         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.3  2001/11/07 15:53:01         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>    22/12/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreLists.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <StringCompare.h>
#endif

// MIB interfaces

#include "MoreAppearance.h"
#include "MoreEvents.h"

/////////////////////////////////////////////////////////////////

#if !ACCESSOR_CALLS_ARE_FUNCTIONS

	extern pascal ListBounds *GetListDataBounds(ListRef list, ListBounds *bounds)
	{
		*bounds = (**list).dataBounds;
		return bounds;
	}
	
	extern pascal ListBounds *GetListVisibleCells(ListRef list, ListBounds *visible)
	{
		*visible = (**list).visible;
		return visible;
	}

#endif

extern pascal void 		LSetSelectWhite(Boolean setIt, Cell theCell, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LSetSelect(setIt, theCell, lHandle);
		err = MoreSetThemeDrawingState(state, true);
		assert(err == noErr);
	}
	assert(err == noErr);
}

extern pascal void 		LAutoScrollWhite(ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LAutoScroll(lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}

extern pascal void 		LScrollWhite(SInt16 dCols, SInt16 dRows, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LScroll(dCols, dRows, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}

extern pascal void 		LUpdateWhite(RgnHandle theRgn, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LUpdate(theRgn, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}

extern pascal void 		LDelRowWhite(SInt16 count, SInt16 rowNum, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LDelRow(count, rowNum, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}

extern pascal SInt16 	LAddRowWhite(SInt16 count, SInt16 rowNum, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;
	SInt16 result;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		result = LAddRow(count, rowNum, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
	return result;
}

extern pascal Boolean	LClickWhite(Point pt, SInt16 modifiers, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;
	Boolean result;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		result = LClick(pt, modifiers, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
	return result;
}

extern pascal void 		LActivateWhite(Boolean act, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LActivate(act, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}

extern pascal void		LSizeWhite(SInt16 listWidth, SInt16 listHeight, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LSize(listWidth, listHeight, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}

extern pascal void		LSetCellWhite(const void *dataPtr, SInt16 dataLen, Cell theCell, ListHandle lHandle)
{
	OSStatus err;
	MoreThemeDrawingState state;

	err = MoreGetThemeDrawingState(&state);
	if (err == noErr) {
		err = MoreNormalizeThemeDrawingState();
		assert(err == noErr);
		LSetCell(dataPtr, dataLen, theCell, lHandle);
		err = MoreSetThemeDrawingState(state, true);
	}
	assert(err == noErr);
}


// Global variables for the LDoKey routine.

static Str255 		gCharsTypedSoFar;
static UInt32 		gTimeOfLastCharTyped;
static ListHandle 	gListHandleOfLastCharTyped;

extern pascal void 		LSetNoSelection(ListHandle listH)
	// See comment in interface part.
{
	Cell listCell;

	listCell.v = 0;
	listCell.h = 0;
	while ( LGetSelect(true, &listCell, listH) ) {
		LSetSelectWhite(false, listCell, listH);
		listCell.v = listCell.v + 1;
		listCell.h = 0;
	}
}

extern pascal void 		LSelectAll(ListHandle listH)
{
	Cell listCell;
	SInt16 row;
	ListBounds bounds;

	(void) GetListDataBounds(listH, &bounds);
	for (row = 0; row < bounds.bottom; row++) {
		listCell.v = row;
		listCell.h = 0;
		LSetSelectWhite(true, listCell, listH);
	}
}

extern pascal void 		LSetSingleSelection(ListHandle listH, SInt16 row)
	// See comment in interface part.
{
	Cell listCell;

	listCell.h = 0;
	listCell.v = row;
	LSetSelectWhite(true, listCell, listH);
	listCell.v = 0;
	listCell.h = 0;
	while ( LGetSelect(true, &listCell, listH) ) {
		if (listCell.v != row) {
			LSetSelectWhite(false, listCell, listH);
		}
		listCell.v = listCell.v + 1;
		listCell.h = 0;
	}
	LAutoScrollWhite(listH);
}

static void LGetUniqueEntryName(ListHandle listH, const Cell *listCell, GetListCellTextProcType getCellText, Str255 result)
	/*	This function calls getCellText and then returns a �uniquified� version of the
		cell text.  What that means is that it returns the cell text followed by
		a chr(0) followed by the the vertical co-ordinate of the cell encoded
		as two characters.  This is useful because it allows functions that
		need to distinguish between two cells even if they have the same
		name to function, eg tabbing.
	*/
{
	UInt32 count;

	assert(getCellText != NULL);

	result[0] = 0;
	getCellText(listH, listCell, result);
	count = result[0];
	if (count < 253) {			// We�re going to add three chars, but not if we�d exceed the buffer size.
		result[count] = 0;
		result[count + 1] = (listCell->v / 256);
		result[count + 2] = (listCell->v % 256);
		result[0] += 3;
	}
}

static Boolean LGetSelectedCellCommon(ListHandle listH, Cell *listCell, GetListCellTextProcType getCellText,
														Boolean first)
	/*	This function finds the alphabetically first or last cell (depending
		on the value of first) in the currently selected cells of the list.  It
		returns false if there are no selected cells.
	*/
{
	Boolean result;
	Str255 cellText;
	Str255 bestText;
	SInt16 indexOfBestText;

	assert(getCellText != NULL);

	// Establish some pre-conditions.

	result = false;
	listCell->h = 0;
	listCell->v = 0;
	indexOfBestText = 0;
	if (first) {
		bestText[0] = 0;
		bestText[1] = 255;
		bestText[2] = 255;
	} else {
		bestText[0] = 0;
	}
	
	// Loop through the selected cells, looking for the best text (ie the
	// alphabetically first or last).

	while ( LGetSelect(true, listCell, listH) ) {
		result = true;
		getCellText(listH, listCell, cellText);
		if ( (first && (CompareString(cellText, bestText, NULL) < 0)) ||
					(!first && (CompareString(cellText, bestText, NULL) > 0)) )  {
			indexOfBestText = listCell->v;
			BlockMoveData(cellText, bestText, cellText[0] + 1);
		}
		listCell->v += 1;
	}
	
	// Finish up.
	listCell->h = 0;
	listCell->v = indexOfBestText;
	return result;
}

static Boolean LGetFirstSelectedCell(ListHandle listH, Cell *listCell, GetListCellTextProcType getCellText)
	/*	This function finds the alphabetically first cell in the currently
		selected cells of the list.  It returns false if there are no selected cells.
	*/
{
	assert(getCellText != NULL);
	return LGetSelectedCellCommon(listH, listCell, getCellText, true);
}

static Boolean LGetLastSelectedCell(ListHandle listH, Cell *listCell, GetListCellTextProcType getCellText)
	/*	This function finds the alphabetically last cell in the currently
		selected cells of the list.  It returns false if there are no selected cells.
	*/
{
	assert(getCellText != NULL);
	return LGetSelectedCellCommon(listH, listCell, getCellText, false);
}

static Boolean LSelectFirstCommon(ListHandle listH, ConstStr255Param markerText, GetListCellTextProcType getCellText,
												Boolean before, Boolean orEqual)
	/*	This function selects the first cell alphabetically before (or after, depending
		on the value of "before") the markerText.  If returns true if it managed to do this,
		false otherwise.  The orEqual value determines whether an
		equal value is considered to be before the otherwise best value.
	*/
{
	Boolean result;
	SInt16 	row;
	SInt16 	indexOfBestText;
	Cell 	listCell;
	Str255 	bestText;
	Str255 	cellText;
	SInt16 	comp1;
	SInt16 	comp2;
	ListBounds bounds;

	assert(getCellText != NULL);

	// Establish some pre-conditions.
	result = false;
	indexOfBestText = 0;
	if (before) {
		bestText[0] = 0;
	} else {
		bestText[0] = 0;
		bestText[1] = 255;
		bestText[2] = 255;
	}
	
	/*	Iterate through all the cells, looking for best text.  Best is defined
		as the phone that�s alphabetically before (or after, depending on
		the value of before) the markerText.
	*/
	(void) GetListDataBounds(listH, &bounds);
	for (row = 0; row < bounds.bottom; row++) {
		listCell.h = 0;
		listCell.v = row;
		getCellText(listH, &listCell, cellText);

		/*	OK, so this needs some explaining (-:
			comp1 and comp2 just cache the value of the comparisons between
			markerText, cellText and bestText.
			
			If before is true, we�re looking for the cell immediately before
			the markerText.  This means that the markerText must be
			greater than (ie "comp1 > 0") or equal to (ie "| (comp1 == 0)")
			the cellText, and the cellText must be greater than (ie "comp2 > 0")
			the bestText we�ve found so far.
			
			If before is false, we�re looking for the cell immediately after
			the markerText.  This means that the markerText must be
			less than (ie "comp1 < 0") or equal to (ie "| (comp1 == 0)")
			the cellText, and the cellText must be less than (ie "comp2 < 0")
			the bestText we�ve found so far.
			
			*phew*
		*/
		comp1 = CompareString(markerText, cellText, NULL);
		comp2 = CompareString(cellText,   bestText, NULL);
		if ((		before && (((comp1 > 0) || ((comp1 == 0) && orEqual)) && (comp2 > 0))) ||
			(!before & (((comp1 < 0) || ((comp1 == 0) && orEqual)) && (comp2 < 0))) ) {
			BlockMoveData(cellText, bestText, cellText[0] + 1);
			indexOfBestText = listCell.v;
			result = true;
		}
	}
	
	// Now set the selection to the cell we found.
	if (result) {
		LSetSingleSelection(listH, indexOfBestText);
	}
	return result;
}

static Boolean LSelectFirstBefore(ListHandle listH, Str255 beforeThis, GetListCellTextProcType getCellText)
	/*	This function selects the first cell alphabetically before
		the beforeThis text.  If returns true if it managed to do this,
		false otherwise.  The orEqual value determines whether an
		equal value is considered to be before the otherwise best value.
	*/
{
	assert(getCellText != NULL);
	return LSelectFirstCommon(listH, beforeThis, getCellText, true, false);
}

static Boolean LSelectFirstAfter(ListHandle listH, Str255 afterThis, GetListCellTextProcType getCellText, Boolean orEqual)
	/*	This function selects the first cell alphabetically after
		the afterThis text.  If returns true if it managed to do this,
		false otherwise.  The orEqual value determines whether an
		equal value is considered to be before the otherwise best value.
	*/
{
	assert(getCellText != NULL);
	return LSelectFirstCommon(listH, afterThis, getCellText, false, orEqual);
}

static void LDownArrow(ListHandle listH)
	/* Find the last selected cell and select the cell after it. */
{
	Cell listCell;
	SInt16 indexOfCellToSelect;
	ListBounds bounds;

	listCell.h = 0;
	listCell.v = 0;
	indexOfCellToSelect = 0;
	while ( LGetSelect(true, &listCell, listH) ) {
		listCell.v = listCell.v + 1;
		indexOfCellToSelect = listCell.v;
	}
	(void) GetListDataBounds(listH, &bounds);
	if (indexOfCellToSelect >= bounds.bottom) {
		indexOfCellToSelect = bounds.bottom - 1;
	}
	LSetSingleSelection(listH, indexOfCellToSelect);
	LAutoScrollWhite(listH);
}

static void LUpArrow(ListHandle listH)
	/* Find the first selected cell and select the cell before it. */
{
	Cell listCell;
	ListBounds bounds;

	listCell.h = 0;
	listCell.v = 0;
	(void) GetListDataBounds(listH, &bounds);
	if ( !LGetSelect(true, &listCell, listH) ) {
		listCell.v = bounds.bottom;
	}
	if (listCell.v > 0) {
		listCell.v = listCell.v - 1;
	}
	LSetSingleSelection(listH, listCell.v);
	LAutoScrollWhite(listH);
}

static void LTab(ListHandle listH, GetListCellTextProcType getCellText, Boolean shift)
	/* Handle Tab and shift-Tab keys in the list. */
{
	Cell 	listCell;
	Boolean done;
	Str255 	selectedCellText;

	if (getCellText != NULL) {
		if ( !shift ) {
			/*	Tab -- If there are selected cells then attempt to select the first
				cell after the last selected cell.  If we can�t or there were no 
				selected cells, then select the first cell alphabetically.
			*/
			done = false;
			if (LGetLastSelectedCell(listH, &listCell, getCellText)) {
				LGetUniqueEntryName(listH, &listCell, getCellText, selectedCellText);
				if (LSelectFirstAfter(listH, selectedCellText, getCellText, false)) {
					done = true;
				}
			}
			if ( !done ) {
				(void) LSelectFirstAfter(listH, "\p", getCellText, false);
			}
		} else {
			/*	shift-Tab -- If there are no selected cells then attempt to select the
				cell before the first selected cell.  If we can�t or there were no 
				selected cells, then select the last cell alphabetically.
			*/
			done = false;
			if (LGetFirstSelectedCell(listH, &listCell, getCellText)) {
				getCellText(listH, &listCell, selectedCellText);
				if (LSelectFirstBefore(listH, selectedCellText, getCellText)) {
					done = true;
				}
			}
			if ( !done ) {
				(void) LSelectFirstBefore(listH, "\p\xFF", getCellText);
			}
		}
	}
}

static void LOtherKey(ListHandle listH, GetListCellTextProcType getCellText,
										UInt8 typedChar, UInt32 eventTicks)
	/*	This routine handles the pressing of a normaly key in a list
		by selecting the cell best associated with the text typed so far.
	*/
{
	if ((getCellText != NULL) & (typedChar >= ' ')) {
		if (eventTicks - gTimeOfLastCharTyped > 60) {
			gCharsTypedSoFar[0] = 0;
		}
		gTimeOfLastCharTyped = eventTicks;
		gCharsTypedSoFar[0] += 1;
		gCharsTypedSoFar[gCharsTypedSoFar[0]] = typedChar;
		if ( !LSelectFirstAfter(listH, gCharsTypedSoFar, getCellText, true) ) {
			(void) LSelectFirstBefore(listH, "\p\xFF", getCellText);
		}
	}
}

extern pascal void 		LDoKey(ListHandle listH, const EventRecord *event, GetListCellTextProcType getCellText)
	/* See comment in interface part. */
{
	UInt32 		eventTicks;
	UInt8		typedChar;
	ListBounds 	bounds;
	ListBounds 	visible;

	eventTicks = event->when;
	typedChar  = event->message & charCodeMask;

	// First up, if we�ve changed lists or typed a control character,
	// we reset the globals that track the current typing state.

	if ((gListHandleOfLastCharTyped != listH) || (typedChar < ' ')) {
		gTimeOfLastCharTyped 	   = 0;
		gListHandleOfLastCharTyped = listH;
	}
	
	// Now dispatch the various characters type.
	(void) GetListDataBounds(listH, &bounds);
	(void) GetListVisibleCells(listH, &visible);
	switch (typedChar) {
		// Handle the trivial scrolling around keys.
		case kHomeCharCode:
			LScrollWhite(0, -bounds.bottom, listH);
			break;
		case kEndCharCode:
			LScrollWhite(0, bounds.bottom, listH);
			break;
		case kPageUpCharCode:
			LScrollWhite(0, -(visible.bottom - visible.top - 2), listH);
			break;
		case kPageDownCharCode:
			LScrollWhite(0, (visible.bottom - visible.top - 2), listH);
			break;
	
		// Handle up and down arrows.
		case kDownArrowCharCode:
			LDownArrow(listH);
			break;
		case kUpArrowCharCode:
			LUpArrow(listH);
			break;
			
		// Tab and shift-Tab and other keys are trickier.
		case kTabCharCode:
			LTab(listH, getCellText, ((event->modifiers & shiftKey) != 0));
			break;
		default:
			LOtherKey(listH, getCellText, typedChar, eventTicks);
			break;
	}
}

extern pascal SInt16 	LSelectedLine(ListHandle listH)
	// See comment in interface part.
{
	SInt16 result;
	Cell   listCell;

	SetPt(&listCell, 0, 0);
	if (LGetSelect(true, &listCell, listH)) {
		result = listCell.v;
	} else {
		result = -1;
	}
	return result;
}

extern pascal Boolean 	LIsEmpty(ListHandle listH)
	// See comment in interface part.
{
	ListBounds bounds;

	(void) GetListDataBounds(listH, &bounds);
	return bounds.bottom <= bounds.top;
}
