/*
	File:		MoreOpenAndSave.h

	Contains:	presents API like Standard File but implemented in terms
				of Navigation Services

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

$Log: MoreOpenAndSave.h,v $
Revision 1.10  2003/03/28 16:15:15         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.9  2002/11/08 23:39:37         
Tidied up framework style include error checking.

Revision 1.8  2001/11/07 15:53:47         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>     15/2/01    Quinn   Complain if built using PB on Mac OS X and explain what the
                                    problem is.
         <5>      1/3/00    Quinn   All entry points now take routine pointers rather than UPPs. 
                                    This is required because Carbon has eliminated the routines
                                    necessary to call most Standard File UPPs, and has no generic
                                    CallUniversalProc from which I can build a replacement.
         <4>     1/22/99    PCG     TARGET_CARBON
         <3>      1/7/99    PCG     fix indentation, EnableDisableNav, WillUseNav,
                                    GetFirstCustomItemIndex
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <5>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <4>      9/9/98    PCG     re-work import and export pragmas
         <3>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <2>     7/11/98    PCG     add header
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
    // To build MoreOpenAndSave you need "StandardFile.h", which is not included 
    // in the default Mac OS X developer install [2632000].  If you need to build 
    // MoreOpenAndSave on Mac OS X please let us <dts@apple.com> know and I will
    // remove this limitation (one way or another).
    #error MoreOpenAndSave does not support framework includes
#else
	#include <StandardFile.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#if ! TARGET_API_MAC_CARBON

	pascal Boolean
MOASI_EnableDisableNav
	(Boolean);

#endif

	pascal Boolean
MOASI_WillUseNav
	(void);

	pascal DialogItemIndexZeroBased
MOASI_GetFirstCustomItemIndex
	(void);

	pascal OSErr
MOASI_CustomPutFile
	(	volatile	ConstStr255Param						prompt,				// same as CustomPutFile
		volatile	ConstStr255Param						defaultName,		// same as CustomPutFile
					StandardFileReply *			volatile,						// same as CustomPutFile
		volatile	short									ditlResID,			// 'DITL' resource ID to APPEND to the existing dialog
		volatile	DlgHookYDProcPtr,											// slightly different from CustomGetFile
		volatile	ModalFilterYDProcPtr,										// slightly different
					void *						volatile	yourDataPtr		);	// same as CustomPutFile

	pascal OSErr
MOASI_CustomGetFile
	(	volatile	FileFilterYDProcPtr,										// same as CustomGetFile
		volatile	short									numTypes,			// same as CustomGetFile
		volatile	ConstSFTypeListPtr,											// same as CustomGetFile
					StandardFileReply *			volatile,						// same as CustomGetFile
		volatile	short									ditlResID,			// 'DITL' resource ID to APPEND to the existing dialog
		volatile	DlgHookYDProcPtr,											// slightly different from CustomGetFile
		volatile	ModalFilterYDProcPtr,										// slightly different from CustomGetFile
					void *						volatile	yourDataPtr		);	// same as CustomGetFile

	pascal OSErr
MOASI_StandardGetFile
	(	volatile	FileFilterProcPtr,											// same as StandardGetFile
		volatile	short									numTypes,			// same as StandardGetFile
		volatile	ConstSFTypeListPtr,											// same as StandardGetFile
					StandardFileReply *		volatile						);	// same as StandardGetFile

	pascal OSErr
MOASI_StandardPutFile
	(	volatile	ConstStr255Param						prompt,				// same as StandardPutFile
		volatile	ConstStr255Param						defaultName,		// same as StandardPutFile
					StandardFileReply *		volatile						);	// same as StandardPutFile

	pascal OSErr
MOASI_StandardOpenDialog
	(				StandardFileReply *		volatile						);	// same as StandardOpenDialog

	pascal OSErr
MOASI_StandardGetFilePreview
	(	volatile	FileFilterProcPtr,											// same as MOASI_StandardGetFile
		volatile	short									numTypes,			// same as MOASI_StandardGetFile
		volatile	ConstSFTypeListPtr,											// same as MOASI_StandardGetFile
					StandardFileReply *		volatile						);	// same as MOASI_StandardGetFile

	pascal OSErr
MOASI_CustomGetFilePreview
	(	volatile	FileFilterYDProcPtr,										// same as MOASI_CustomGetFile
		volatile	short									numTypes,			// same as MOASI_CustomGetFile
		volatile	ConstSFTypeListPtr,											// same as MOASI_CustomGetFile
					StandardFileReply *			volatile,						// same as MOASI_CustomGetFile
		volatile	short									ditlResID,			// same as MOASI_CustomGetFile
		volatile	DlgHookYDProcPtr,											// same as MOASI_CustomGetFile
		volatile	ModalFilterYDProcPtr,										// same as MOASI_CustomGetFile
					void *						volatile	yourDataPtr		);	// same as MOASI_CustomGetFile

#ifdef __cplusplus
	}
#endif
