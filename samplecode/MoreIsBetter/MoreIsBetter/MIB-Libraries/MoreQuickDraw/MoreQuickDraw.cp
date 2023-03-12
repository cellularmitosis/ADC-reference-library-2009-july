/*
	File:		MoreQuickDraw.cp

	Contains:	QuickDraw utilities.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

$Log: MoreQuickDraw.cp,v $
Revision 1.12  2002/11/25 18:06:06         
Eliminate "implicit arithmetic conversion" warnings.

Revision 1.11  2002/11/08 23:59:30         
Convert nil to NULL. Convert MoreAssertQ to assert. When using framework includes, explicitly include the frameworks we need.

Revision 1.10  2001/11/07 15:54:54         
Tidy up headers, add CVS logs, update copyright.


         <9>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <8>    22/12/00    Quinn   Made HaveColorQuickDraw autoinitialise. Added
                                    GetPortVisibleRegion and MoreSetMagicHiliteMode.
         <7>    24/11/00    Quinn   Implement GetQDGlobalsScreenBits for non-Carbon clients.
         <6>    23/11/00    Quinn   2548029 Need to call CloseCPort in DisposePort.
         <5>     20/3/00    Quinn   Implement CreateNewPort and DisposePort for non-Carbon clients.
         <4>      2/9/99    PCG     more TARGET_CARBON
         <3>     1/22/99    PCG     TARGET_CARBON
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <2>     8/28/98    PCG     add IsColorGrafPort
         <1>     6/16/98    PCG     initial checkin
*/

#include "MoreSetup.h"

#include "MoreQuickDraw.h"

#if !MORE_FRAMEWORK_INCLUDES
	#include <LowMem.h>
	#include <Gestalt.h>
#endif

static long gQuickDrawVersion = 0;

pascal Boolean HaveColorQuickDraw (void)
{
	if (gQuickDrawVersion == 0) {
		(void) InitMoreQuickDraw();
	}
	// the "features" selector is wrong; test the version instead
	return gQuickDrawVersion > gestaltOriginalQD;
}

pascal OSErr InitMoreQuickDraw (void)
{
	OSErr err = Gestalt (gestaltQuickdrawVersion,&gQuickDrawVersion);

	if (err == gestaltUndefSelectorErr)
	{
		gQuickDrawVersion = gestaltOriginalQD;
		err = noErr;
	}

	return err;
}

pascal Boolean IsColorGrafPort (GrafPtr port)
{
#if TARGET_API_MAC_CARBON
	// Carbon can have non-color ports (when you print to an 
	// older printer driver on traditional Mac OS), so this 
	// routine lives on!
	return IsPortColor( (CGrafPtr) port );
#else
	// stolen from {CommonSystem}:Toolbox:ToolboxUtils:CommonHeaders:ColorUtils.h
	return ((CGrafPtr) port)->portVersion < 0;
#endif
}

pascal OSErr ShowWatchCursor (void)
{
	OSErr err = noErr;

	CursHandle watchFob = GetCursor (watchCursor);

	if (!watchFob)
		err = nilHandleErr;
	else
	{
		Cursor preservedArrow;
		GetQDGlobalsArrow (&preservedArrow);
		SetQDGlobalsArrow (*watchFob);
		InitCursor ( );
		SetQDGlobalsArrow (&preservedArrow);
	}

	return err;
}

#if !TARGET_API_MAC_CARBON

pascal QDGlobalsPtr GetQDGlobalsPtr (void)
{
	return QDGlobalsPtr (* (Ptr*) LMGetCurrentA5 ( ) - 0xCA);
}

#endif

pascal void SetArrowCursor (void)
{
#if TARGET_API_MAC_CARBON
	Cursor arrow;
	SetCursor (GetQDGlobalsArrow (&arrow));
#else
	SetCursor (&(qd.arrow));
#endif
}

#if !TARGET_API_MAC_CARBON

	extern pascal CGrafPtr CreateNewPort(void)
	{
		CGrafPtr result;
		
		result = (CGrafPtr) NewPtr(sizeof(CGrafPort));
		if (result != NULL) {
			OpenCPort(result);
		}
		return result;
	}
	
	extern pascal void DisposePort(CGrafPtr port)
	{
		CloseCPort(port);
		
		DisposePtr( (Ptr) port );
		assert(MemError() == noErr);
	}

	extern pascal BitMap *GetQDGlobalsScreenBits(BitMap *screenBits)
	{
		*screenBits = qd.screenBits;
		return screenBits;
	}

	extern pascal RgnHandle GetPortVisibleRegion(CGrafPtr port, RgnHandle visRgn)
	{
		CopyRgn(port->visRgn, visRgn);
		return visRgn;
	}
	
#endif

extern void MoreSetMagicHiliteMode(void)
{
	if ( HaveColorQuickDraw() ) {
		LMSetHiliteMode( (UInt8) (LMGetHiliteMode() & 0x7F) );
	}
}
