/*
	File:		MoreQuickDraw.h

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

$Log: MoreQuickDraw.h,v $
Revision 1.11  2003/03/28 16:15:25         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.10  2002/11/08 23:58:37         
When using framework includes, explicitly include the frameworks we need.

Revision 1.9  2001/11/07 15:54:56         
Tidy up headers, add CVS logs, update copyright.


         <8>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <7>    22/12/00    Quinn   Fixed previous checkin comment and changed name of
                                    MagicHiliteMode to MoreSetMagicHiliteMode.
         <6>    22/12/00    Quinn   Added GetPortVisibleRegion and MoreSetMagicHiliteMode.
         <5>    24/11/00    Quinn   Implement GetQDGlobalsScreenBits for non-Carbon clients.
         <4>     20/3/00    Quinn   Implement CreateNewPort and DisposePort for non-Carbon clients.
         <3>      2/9/99    PCG     more TARGET_CARBON
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <5>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <4>      9/9/98    PCG     re-work import and export pragmas
         <3>     8/28/98    PCG     add IsColorGrafPort
         <2>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <ApplicationServices/ApplicationServices.h>
#else
	#include <Quickdraw.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

pascal OSErr	InitMoreQuickDraw	(void);
pascal Boolean	HaveColorQuickDraw	(void);
pascal Boolean	IsColorGrafPort		(GrafPtr);
pascal OSErr	ShowWatchCursor		(void);
pascal void		SetArrowCursor		(void);

#if !TARGET_API_MAC_CARBON
	pascal QDGlobalsPtr GetQDGlobalsPtr (void);
#	define GetQDGlobalsArrow(x) do { *(x)=GetQDGlobalsPtr()->arrow; } while (false)
#	define SetQDGlobalsArrow(x) do { GetQDGlobalsPtr()->arrow=*(x); } while (false)
#	define GetRegionBounds(rgn,rect) do { *(rect) = (**(rgn)).rgnBBox; } while (false)

	extern pascal CGrafPtr CreateNewPort(void);
	extern pascal void DisposePort(CGrafPtr port);

	extern pascal BitMap *GetQDGlobalsScreenBits(BitMap *screenBits);

	extern pascal RgnHandle GetPortVisibleRegion(CGrafPtr port, RgnHandle visRgn);

#endif

extern void MoreSetMagicHiliteMode(void);

#ifdef __cplusplus
	}
#endif
