/*
	File:		MoreToolbox.h

	Contains:	General toolbox utilities.

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

$Log: MoreToolbox.h,v $
Revision 1.10  2003/04/09 19:45:14         
Moved a bunch of low-level Gestalt routines from MoreToolbox to MoreOSUtils.

Revision 1.9  2003/03/28 16:15:38         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.8  2003/03/26 15:57:27         
Added MoreRunningOnClassic.

Revision 1.7  2002/11/09 00:13:28         
When using framework includes, explicitly include the frameworks we need. Added MoreRunningOnMacOSX. GetSystemVersion now returns a UInt32.

Revision 1.6  2001/11/07 15:56:01         
Tidy up headers, add CVS logs, update copyright.


         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>      2/9/99    PCG     lose QDGlobals and ShowWatchCursor
         <3>    11/11/98    PCG     fix headers
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <6>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <5>    10/23/98    PCG     add HavePowerManager
         <4>      9/9/98    PCG     re-work import and export pragmas
         <3>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <2>     7/24/98    PCG	    eliminate dependency on 'qd'
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <Quickdraw.h>
#endif

#include "MoreOSUtils.h"

#ifdef __cplusplus
	extern "C" {
#endif

pascal OSStatus		InitMac				(void);
pascal void			Beep				(void);
pascal Boolean		HaveAppleEvents		(void);

#ifdef __cplusplus
	}
#endif
