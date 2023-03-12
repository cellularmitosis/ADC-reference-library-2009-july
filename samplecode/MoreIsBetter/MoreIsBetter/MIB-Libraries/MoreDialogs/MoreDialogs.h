/*
	File:		MoreDialogs.h

	Contains:	Dialog Manager utilities.

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

$Log: MoreDialogs.h,v $
Revision 1.14  2003/04/17 13:41:21         
Flesh out the implementation of StandardAlertCFStringCompat.

Revision 1.13  2003/03/28 16:15:06         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.12  2002/11/08 23:26:09         
Added StandardAlertCFStringCompat. Convert nil to NULL. Include our prototype early to flush out any missing dependencies.

Revision 1.11  2001/11/07 15:52:14         
Tidy up headers, add CVS logs, update copyright.


        <10>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <9>     15/2/01    Quinn   Eliminate accessor macros because UI 3.4 defines them for us.
         <8>    22/12/00    Quinn   Fixed bug I introduced in the prototype of
                                    MoreAppendDialogItemList.
         <7>    22/12/00    Quinn   Added many new APIs.
         <6>     20/3/00    Quinn   Fix prototype for GetDialogItemString.
         <5>     1/22/99    PCG     TARGET_CARBON
         <4>      1/7/99    PCG     add ToggleDialogItemCheckBox
         <3>      1/7/99    PCG     add MoreAppendDialogItemList
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <7>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <6>      9/9/98    PCG     re-work import and export pragmas
         <5>      9/1/98    PCG     Universal Headers 3.2
         <4>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <3>     7/24/98    PCG	    add MoveableModalDialog
         <2>     7/24/98    PCG	    eliminate dependency on 'qd'
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <Dialogs.h>
#endif

enum { kDialogItemIndexNone = -1 };

#ifdef __cplusplus
	extern "C" {
#endif

#if UNIVERSAL_INTERFACES_VERSION < 0x0320

	extern pascal OSErr AppendDialogItemList (DialogRef dialog, SInt16 ditlID, DITLMethod method)
		THREEWORDINLINE (0x303C, 0x0412, 0xAA68);

#endif // UNIVERSAL_INTERFACES_VERSION < 0x0320

pascal DialogRef	MoreGetNewDialog			(SInt16 dlogResID);
pascal OSErr		MoreAppendDialogItemList	(DialogRef, SInt16, DITLMethod);
pascal void			SaferShortenDITL			(DialogRef, DialogItemIndex);
pascal void			SelectAllDialogItemText		(DialogRef, DialogItemIndex);
pascal void			MoveableModalDialog			(ModalFilterUPP, DialogItemIndex *itemHit);

extern pascal ControlRef GetDialogControlRef(DialogRef dlg, DialogItemIndex item);
	// Returns a control for the specified dialog item. 
	// Uses GetDialogItemAsControl if appropriate, otherwise just 
	// calls GetDialogItemHandle.
	
extern pascal Handle 	 GetDialogItemHandle   (DialogRef dlg, DialogItemIndex item);
	// Returns the handle for the dialog item, as returned by 
	// GetDialogItem.  Different from GetDialogControlRef in the 
	// case of text based dialog items (static text and edit text) 
	// when embedding is turned on.  In that case, GetDialogControlRef 
	// will return a control handle for the text control, but this 
	// routine will return a handle containing the item�s text.

// Setters and getters for the various dialog item attributes.

extern pascal void 		SetDialogItemHandle		  (DialogRef dlg, DialogItemIndex item, Handle itemH);
extern pascal void 		SetDialogItemUserItemProc (DialogRef dlg, DialogItemIndex item, UserItemUPP userItemUPP);
	// Wrapper for SetDialogItemHandle that does the user item cast for you.
	// Long term this should also include smarter asserts.
extern pascal void 		GetDialogItemKind		  (DialogRef dlg, DialogItemIndex item, DialogItemType *kind);
extern pascal void 		SetDialogItemKind		  (DialogRef dlg, DialogItemIndex item, DialogItemType  kind);
extern pascal void 		GetDialogItemRect		  (DialogRef dlg, DialogItemIndex item,       Rect *itemRect);
extern pascal void 		SetDialogItemRect		  (DialogRef dlg, DialogItemIndex item, const Rect *itemRect);
extern pascal void		GetDialogItemString		  (DialogRef dlg, DialogItemIndex item,         Str255 str);
extern pascal void		SetDialogItemString		  (DialogRef dlg, DialogItemIndex item, ConstStringPtr str);
	// NULL is legal for str; equivalent to the empty string.

// Setters and getters for control attributes where those controls are 
// contained in a dialog.

extern pascal SInt16 	GetDialogControlValue     (DialogRef dlg, DialogItemIndex item);
extern pascal void 		SetDialogControlValue     (DialogRef dlg, DialogItemIndex item, SInt16 value);
extern pascal SInt16 	GetDialogControlMinimum   (DialogRef dlg, DialogItemIndex item);
extern pascal void 		SetDialogControlMinimum   (DialogRef dlg, DialogItemIndex item, SInt16 min);
extern pascal SInt16 	GetDialogControlMaximum   (DialogRef dlg, DialogItemIndex item);
extern pascal void 		SetDialogControlMaximum   (DialogRef dlg, DialogItemIndex item, SInt16 max);
extern pascal Boolean 	GetDialogControlBoolean	  (DialogRef dlg, DialogItemIndex item);
extern pascal void 		SetDialogControlBoolean   (DialogRef dlg, DialogItemIndex item, Boolean value);
extern pascal void 		ToggleDialogControlBoolean(DialogRef dlg, DialogItemIndex item);
extern pascal Boolean 	GetDialogControlEnable    (DialogRef dlg, DialogItemIndex item);
extern pascal void 		SetDialogControlEnable    (DialogRef dlg, DialogItemIndex item, Boolean enable);

pascal Boolean		    ToggleDialogCheckBox	  (DialogRef, DialogItemIndex);
	// Another implementation of ToggleDialogControlBoolean.  I haven�t 
	// replaced one with the other because a) the implementations are 
	// subtley different, and b) I can�t be bothered fixing the existing 
	// clients of this routine.

extern pascal void		InvalDialogItem			  (DialogRef dlg, DialogItemIndex item);

extern pascal void		SetupStandardDialogItems  (DialogRef dlg, DialogItemIndex defaultItem, DialogItemIndex cancelItem);
	// Pass 0 in the item parameters if your dialog doesn�t have 
	// that type of item.

// The following filter routines are exported as both UPP and routines because
// in some places (such as filter functions that are layered
// on top of these default filter functions) you need to call them directly.

// These are the filter routines themselves.  We have two subtley different 
// variants, one for modal dialogs and the other for alerts.  All filters 
// assume that the default button is item 1 (kStdOkItemIndex), the Cancel 
// button (if present) is item 2 (kStdCancelItemIndex), and the Discard button 
// (if present) is item 3 (kStdDiscardItemIndex).

enum {
	kStdDiscardItemIndex = 3
};

extern pascal Boolean	OKModalFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item);
extern pascal Boolean	OKCancelModalFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item);
extern pascal Boolean	OKCancelDiscardModalFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item);

extern pascal Boolean	OKAlertFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item);
extern pascal Boolean	OKCancelAlertFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item);
extern pascal Boolean	OKCancelDiscardAlertFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item);

// These are the UPP getters for the filter routines.  They are implemented 
// as getters rather than global variables because that way we can avoid have 
// an initialisation routine for this module.

extern pascal ModalFilterUPP GetOKModalFilterUPP(void);
extern pascal ModalFilterUPP GetOKCancelModalFilterUPP(void);
extern pascal ModalFilterUPP GetOKCancelDiscardModalFilterUPP(void);

extern pascal ModalFilterUPP GetOKAlertFilterUPP(void);
extern pascal ModalFilterUPP GetOKCancelAlertFilterUPP(void);
extern pascal ModalFilterUPP GetOKCancelDiscardAlertFilterUPP(void);

// The following helper routines are required by the various filters, 
// and are exported for your convenience.

extern pascal Boolean	DialogItemHidden(DialogRef dlg, DialogItemIndex item);
	// Returns true if the item is hidden.

extern pascal void 		FlashDialogControl(DialogRef dlg, DialogItemIndex item);
	// Flashes the dialog item control (typically a button).

extern pascal void		DialogShiftTab(DialogRef dlg);
	// Performs shift-Tab functionality.

extern pascal OSStatus StandardAlertCFStringCompat(AlertType                     	inAlertType,
										  		   CFStringRef	                 	inError,
										  		   CFStringRef                   	inExplanation,       /* can be NULL */
										     const AlertStdCFStringAlertParamRec *  inAlertParam,        /* can be NULL */
												   SInt16 *                      	outItemHit);
	// Displays a standard alert based on some CFString parameters. 
	// Uses native CFString support, if it's available, or converts 
	// the CFStrings to Pascal strings and calls StandardAlert if 
	// it's not.

#ifdef __cplusplus
	}
#endif
