/*
	File:		MoreLists.h

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

$Log: MoreLists.h,v $
Revision 1.4  2002/11/08 23:33:27         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.3  2001/11/07 15:53:06         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>    22/12/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <Lists.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#if !ACCESSOR_CALLS_ARE_FUNCTIONS
	extern pascal ListBounds *GetListDataBounds(ListRef list, ListBounds *bounds);
	extern pascal ListBounds *GetListVisibleCells(ListRef list, ListBounds *visible);
#endif

// "White" routines.  These routines are wrappers around the 
// standard List Manager calls that set the port�s background to 
// white before calling through to the List Manager and then reset 
// drawing state upon return.  The upshot is that your list draws 
// on a white background rather than on the grey background 
// that�s been standard since Mac OS 8.

extern pascal void 		LSetSelectWhite(Boolean setIt, Cell theCell, ListHandle lHandle);
extern pascal void 		LAutoScrollWhite(ListHandle lHandle);
extern pascal void 		LScrollWhite(SInt16 dCols, SInt16 dRows, ListHandle lHandle);
extern pascal void 		LUpdateWhite(RgnHandle theRgn, ListHandle lHandle);
extern pascal void 		LDelRowWhite(SInt16 count, SInt16 rowNum, ListHandle lHandle);
extern pascal SInt16 	LAddRowWhite(SInt16 count, SInt16 rowNum, ListHandle lHandle);
extern pascal Boolean	LClickWhite(Point pt, SInt16 modifiers, ListHandle lHandle);
extern pascal void 		LActivateWhite(Boolean act, ListHandle lHandle);
extern pascal void		LSizeWhite(SInt16 listWidth, SInt16 listHeight, ListHandle lHandle);
extern pascal void		LSetCellWhite(const void *dataPtr, SInt16 dataLen, Cell theCell, ListHandle lHandle);

// Simple selection accessors.  These assume that you�re using 
// list with N rows and 1 column.

extern pascal void 		LSetNoSelection(ListHandle listH);
extern pascal void 		LSelectAll(ListHandle listH);
extern pascal void 		LSetSingleSelection(ListHandle listH, SInt16 row);

extern pascal SInt16 	LSelectedLine(ListHandle listH);
extern pascal Boolean 	LIsEmpty(ListHandle listH);

// The LDoKey routine takes a procedural parameter that is uses to fetch
// the text associated with an item in the list so that it can implement
// its "select by typing" function.  Note that this doesn�t actually have 
// to be the real text stored in the cell, simply a sorting key for the 
// list as a whole.

typedef pascal void (*GetListCellTextProcType)(ListHandle listH, const Cell *listCell, Str255 cellText);

extern pascal void 		LDoKey(ListHandle listH, const EventRecord *event, GetListCellTextProcType getCellText);
	// This routine processes a key event associated with a list, including
	// "select by typing".  It assumes that you�re using a list with N rows 
	// and 1 column.  You can disable select by typing by passing NULL to
	// getCellText.

#ifdef __cplusplus
}
#endif
