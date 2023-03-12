/*
	File:		MoreWindows.cp

	Contains:	Window Manager utilities.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreWindows.cp,v $
Revision 1.9  2002/11/09 00:16:18         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.8  2001/11/07 15:56:04         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>    22/12/00    Quinn   Added a bunch of new APIs.
         <5>     20/3/00    Quinn   Added GetWindowList for non-Carbon clients.
         <4>      2/9/99    PCG     more TARGET_CARBON
         <3>     1/25/99    PCG     TARGET_CARBON
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <2>     7/24/98    PCG	    eliminate dependency on 'qd'
         <1>     6/16/98    PCG     initial checkin
*/

#include "MoreSetup.h"

#include "MoreWindows.h"

#include "MoreQuickDraw.h"
#include "MoreAppearance.h"

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Controls.h>
	#include <Menus.h>
	#include <LowMem.h>
#endif

pascal OSStatus MoveWindowToAlertPosition (WindowRef window)
{
	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( ) || IsWindowVisible (window)))
	{
		assert(false);
		err = paramErr;
	}
	else
	{
		Rect screenRect = (**GetMainDevice ( )).gdRect;
		screenRect.top += GetMBarHeight ( );
		InsetRect (&screenRect,4,4);

		Rect contRect;

#if ! TARGET_API_MAC_CARBON

		if (!HaveAppearance ( ) || IsWindowVisible (window))
			contRect = (**(((WindowPeek)window)->contRgn)).rgnBBox;
		else

#endif

		{
			RgnHandle contRgn = NewRgn ( );

			if (!contRgn)
				err = QDError ( );
			else
			{
				err = GetWindowRegion (window,kWindowContentRgn,contRgn);
				GetRegionBounds (contRgn,&contRect);
				DisposeRgn (contRgn);
				assert(noErr == QDError ( ));
			}
		}

		if (!err)
		{
			Point windLoc;

			windLoc.v	= screenRect.top + ((screenRect.bottom - screenRect.top) / 3);
			windLoc.h	= screenRect.left + ((screenRect.right - screenRect.top) / 2);

			windLoc.v	-= (contRect.bottom - contRect.top) / 2;
			windLoc.h	-= (contRect.right - contRect.left) / 2;

			MoveWindow (window, windLoc.h, windLoc.v, true);
		}
	}

	return err;
}

pascal OSErr MoreNewWindow (	const Rect *			boundsRect,
								ConstStr255Param 		title,
								short 					theProc,
								Boolean 				goAwayFlag,
								long 					refCon,
								WindowRef				*window			)
{
	OSErr err = noErr;

	if (!title)
		title = "\p";

	if (HaveColorQuickDraw ( ))
		*window = NewCWindow (NULL,boundsRect,title,false,theProc,(WindowRef)kFirstWindowOfClass,goAwayFlag,refCon);
	else
		*window = NewWindow (NULL,boundsRect,title,false,theProc,(WindowRef)kFirstWindowOfClass,goAwayFlag,refCon);

	if (!*window)
		err = nilHandleErr;
	else if (HaveAppearance ( ))
	{
		ControlRef dontCare;
		err = CreateRootControl (*window,&dontCare);
		if (err) DisposeWindow (*window);
	}

	return err;
}

#if ! TARGET_API_MAC_CARBON

	extern pascal WindowRef GetWindowList(void)
	{
		return LMGetWindowList();
	}

#endif

extern pascal Boolean WindowHasEmbeddingHierarchy(WindowPtr window)
{
	ControlHandle junkControl;
	
	return HaveAppearance() && (GetRootControl(window, &junkControl) == noErr);
}

extern pascal OSStatus MoreGetWindowRegion(WindowRef window, WindowRegionCode inRegionCode, RgnHandle ioWinRgn)
	/*	OK, this is a mess.  We have to handle the following cases:
		o Carbon -- Just call GetWindowRegion
		o PowerPC -- Test for availability of GetWindowRegion, call it if it's available, otherwise use compatibility code.
		o 68K -- Always use compatibility code.
	*/
{
	OSStatus err;

	#if TARGET_API_MAC_CARBON
		err = GetWindowRegion(window, inRegionCode, ioWinRgn);
	#else

		// Beware the wacky syntax!

		#if TARGET_CPU_PPC
			if ( (void *) GetWindowRegion != (void *) kUnresolvedCFragSymbolAddress) {
				err = GetWindowRegion(window, inRegionCode, ioWinRgn);
			} else
		#endif
			{
				RgnHandle tmpRgn;
				
				tmpRgn = NULL;
				switch (inRegionCode) {
					case kWindowStructureRgn:
						tmpRgn = ((WindowPeek) window)->strucRgn;
						break;
					case kWindowContentRgn:
						tmpRgn = ((WindowPeek) window)->contRgn;
						break;
					default:
						; // do nothing
				}
				if (tmpRgn == NULL) {
					assert(false);
					err = -1;
				} else {
					CopyRgn(tmpRgn, ioWinRgn);
					err = noErr;
				}
			}
	#endif
	return err;
}

static RgnHandle gTmpContentRgn = NULL;
static RgnHandle gTmpStructureRgn = NULL;
static RgnHandle gTmpVisibleRgn = NULL;

extern pascal RgnHandle MoreGetWindowContentRegion(WindowRef window)
{
	if (gTmpContentRgn == NULL) {
		gTmpContentRgn = NewRgn();
		// If we can�t allocate a region the system is heading for a 
		// crash very soon!
		assert(gTmpContentRgn != NULL);
	}
	if (gTmpContentRgn != NULL) {
		OSStatus err;

		err = MoreGetWindowRegion(window, kWindowContentRgn, gTmpContentRgn);
		assert(err == noErr);
	}
	return gTmpContentRgn;
}

extern pascal RgnHandle MoreGetWindowStructureRegion(WindowRef window)
{
	if (gTmpStructureRgn == NULL) {
		gTmpStructureRgn = NewRgn();
		assert(gTmpStructureRgn != NULL);
	}
	if (gTmpStructureRgn != NULL) {
		OSStatus err;

		assert(gTmpStructureRgn != NULL);
		err = MoreGetWindowRegion(window, kWindowStructureRgn, gTmpStructureRgn);
		assert(err == noErr);
	}
	return gTmpStructureRgn;
}

extern pascal RgnHandle MoreGetWindowVisibleRegion(WindowRef window)
{
	if (gTmpVisibleRgn == NULL) {
		gTmpVisibleRgn = NewRgn();
		assert(gTmpVisibleRgn != NULL);
	}
	if (gTmpVisibleRgn != NULL) {
		assert(gTmpVisibleRgn != NULL);
		(void) GetPortVisibleRegion(GetWindowPort(window), gTmpVisibleRgn);
	}
	return gTmpVisibleRgn;
}

extern pascal Boolean MoreTitleBarOnScreen(WindowRef window)
{
	Boolean 	result;
	RgnHandle 	titleBarRegion;

	result = true;
	titleBarRegion = NewRgn();
	if (titleBarRegion != NULL) {

		// First calculate the title bar region by subtracting the content
		// region away from the structure region.

		CopyRgn(MoreGetWindowStructureRegion(window), titleBarRegion);
		DiffRgn(titleBarRegion, MoreGetWindowContentRegion(window), titleBarRegion);
		
		// Now intersect the title bar region with the grey region, ie the region
		// describing the extent of the desktop and return true if the intersection
		// is not empty.
		
		SectRgn(titleBarRegion, GetGrayRgn(), titleBarRegion);
		result = ! EmptyRgn(titleBarRegion);
		DisposeRgn(titleBarRegion);
	}
	return result;
}

extern pascal void MoreGetWindowGlobalRect(WindowRef theWindow, Rect *windowRect)
	// See comment in interface part.
{
	GrafPtr oldPort;
		
	GetWindowPortBounds(theWindow, windowRect);

	// Transform the windowRect to global coordinates.
	
	GetPort(&oldPort);
	SetPort(GrafPtr(GetWindowPort(theWindow)));
	LocalToGlobal( (Point *) &windowRect->top);
	LocalToGlobal( (Point *) &windowRect->bottom);
	SetPort(oldPort);
}
