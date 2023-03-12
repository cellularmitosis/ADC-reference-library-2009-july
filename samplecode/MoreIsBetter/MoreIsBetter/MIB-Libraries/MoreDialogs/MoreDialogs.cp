/*
	File:		MoreDialogs.cp

	Contains:	Dialog Manager utilities.

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

$Log: MoreDialogs.cp,v $
Revision 1.16  2003/04/17 13:41:11         
Flesh out the implementation of StandardAlertCFStringCompat.

Revision 1.15  2003/04/14 16:05:04         
Remove redundant include of "MoreToolbox.h".

Revision 1.14  2002/12/12 15:23:47         
Convert LF to CR in StandardAlertCFStringCompat to fix compatibility problems on Mac OS 9.

Revision 1.13  2002/11/08 23:26:48         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.12  2001/11/07 15:52:11         
Tidy up headers, add CVS logs, update copyright.


        <11>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <10>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <9>      8/2/01    Quinn   Undo previous checkin to fix 68K build.
         <8>    22/12/00    Quinn   Eliminate superfluous include of "MoreToolbox.h".
         <7>    22/12/00    Quinn   Added a whole bunch of new APIs.
         <6>     20/3/00    Quinn   Make SetDialogItemString work regardless of whether the dialog
                                    is using embedding or not.
         <5>      1/3/00    Quinn   SystemClick is no longer part of, or necessary in, Carbon.
         <4>     1/22/99    PCG     TARGET_CARBON
         <3>      1/7/99    PCG     add ToggleDialogCheckBox and MoreAppendDialogItemList
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <2>     7/24/98    PCG	    eliminate dependency on 'qd'
         <1>     6/16/98    PCG     initial checkin
*/

#include "MoreSetup.h"

#include "MoreDialogs.h"

#if ! MORE_FRAMEWORK_INCLUDES
	#include <ControlDefinitions.h>
	#include <Sound.h>
	#include <Script.h>
#endif

#include "MoreAppearance.h"
#include "MoreInterfaceLib.h"

pascal DialogRef MoreGetNewDialog (SInt16 dlogResID)
{
	return GetNewDialog (dlogResID, NULL, kFirstWindowOfClass);
}

pascal OSErr MoreAppendDialogItemList (DialogRef dialog, SInt16 ditlID, DITLMethod method)
{
	//
	//	Under Mac OS 8.5 and 8.5.1, AppendDialogItemList returns garbage when called
	//	from 68K code (through the address in the trap table). We attempt to detect
	//	the context in which this is a problem and work around it by doing our own
	//	feeble but not grossly incorrect error checking. One thing to be aware of
	//	is that this function does not allow you to append 0-item dialog item lists
	//	on systems which have the bug. If you have a better idea for how to do a
	//	modicum of error checking without a significant increase in complexity,
	//	let us know.
	//

	#if !TARGET_CPU_68K
		return AppendDialogItemList (dialog,ditlID,method);
	#else
		Boolean appendDialogItemListReturnsGarbage = (GetSystemVersion ( ) & 0xFFFFFFF0) == 0x00000850;

		if (!appendDialogItemListReturnsGarbage)
			return AppendDialogItemList (dialog,ditlID,method);

		DialogItemIndex		itemCountBefore		= CountDITL (dialog);
		OSErr				err					= AppendDialogItemList (dialog,ditlID,method);

		if (err)
		{
			DialogItemIndex itemCountAfter = CountDITL (dialog);
			err = (itemCountBefore < itemCountAfter) ? noErr : paramErr;
		}

		return err;
	#endif
}

pascal void SaferShortenDITL (DialogRef dialog, DialogItemIndex index)
{
	while (index--) ShortenDITL (dialog, 1);
}

pascal void SelectAllDialogItemText (DialogRef dialog, DialogItemIndex index)
{
	SelectDialogItemText (dialog,index,0,32767);
}

pascal void MoveableModalDialog (ModalFilterUPP mfp, DialogItemIndex *itemHit)
{
	//
	//	I just made this logic up.
	//	In a perfect world, I would steal the implementation
	//	of the real ModalDialog and tweak it a little bit.
	//	Consider it a to-do item.
	//

	EventRecord		event;
	DialogRef		pop, dummy;
	WindowRef		whichWindow;
	short			partCode;
	Boolean			handledIt = false;

	pop = GetDialogFromWindow (FrontWindow( ));
	*itemHit = kDialogItemIndexNone;

	do
	{
		WaitNextEvent (everyEvent & ~highLevelEventMask, &event, GetCaretTime( ), NULL);

		switch (event.what)
		{
			case mouseDown:

				partCode = FindWindow (event.where, &whichWindow);

				if (whichWindow != GetDialogWindow (pop))
				{
					#if TARGET_API_MAC_CARBON
						SysBeep(10);
					#else
						if (partCode == inSysWindow)
							SystemClick (&event,whichWindow);
						else
							SysBeep(10);
					#endif
					
					break;
				}

				if (inDrag == partCode)
				{
					Rect dragBounds;


					dragBounds = (**GetMainDevice ( )).gdRect;
					InsetRect (&dragBounds, 4, 4);
					DragWindow (GetDialogWindow (pop), event.where, &dragBounds);
					break;
				}

				// fall thru

			default:

				if (mfp)
					handledIt = InvokeModalFilterUPP (pop,&event,itemHit,mfp);

				if (!handledIt && IsDialogEvent(&event))
					DialogSelect(&event,&dummy,itemHit);

				break;
		}
	}
	while (*itemHit == kDialogItemIndexNone);
}

#if MORE_DEBUG
	#define DialogItemAssert(dlg, item) assert(((dlg) != NULL) && ((item) >= 1) && (item) <= CountDITL(dlg));
#else
	#define DialogItemAssert(dlg, item)
#endif

extern pascal ControlRef GetDialogControlRef(DialogRef dlg, DialogItemIndex item)
{
	ControlRef control;
	
	DialogItemAssert(dlg, item);
	
	if ( HaveAppearance() && (GetDialogItemAsControl(dlg, item, &control) == noErr) && (control != NULL) ) {
		// do nothing, control is set up by call to GetDialogItemAsControl
	} else {
		control = (ControlRef) GetDialogItemHandle(dlg, item);
	}
	return control;
}

extern pascal Handle GetDialogItemHandle(DialogRef dlg, DialogItemIndex item)
{
	DialogItemType 	junkKind;
	Handle 			itemH;
	Rect 			junkRect;

	DialogItemAssert(dlg, item);
	GetDialogItem(dlg, item, &junkKind, &itemH, &junkRect);
	return itemH;
}

extern pascal void SetDialogItemHandle(DialogRef dlg, DialogItemIndex item, Handle itemH)
{
	DialogItemType 	itemKind;
	Handle 			junkItemH;
	Rect 			itemRect;

	DialogItemAssert(dlg, item);
	GetDialogItem(dlg, item, &itemKind, &junkItemH, &itemRect);
	SetDialogItem(dlg, item, itemKind,       itemH, &itemRect);
}

extern pascal void SetDialogItemUserItemProc (DialogRef dlg, DialogItemIndex item, UserItemUPP userItemUPP)
{
	SetDialogItemHandle(dlg, item, (Handle) userItemUPP);
}

extern pascal void GetDialogItemKind(DialogRef dlg, DialogItemIndex item, DialogItemType *kind)
{
	Rect 			junkRect;
	Handle 			junkItemH;

	DialogItemAssert(dlg, item);
	assert(kind != NULL);
	GetDialogItem(dlg, item, kind, &junkItemH, &junkRect);
}

extern pascal void SetDialogItemKind(DialogRef dlg, DialogItemIndex item, DialogItemType kind)
{
	DialogItemType 	junkKind;
	Handle 			itemH;
	Rect			itemRect;

	DialogItemAssert(dlg, item);
	GetDialogItem(dlg, item, &junkKind, &itemH, &itemRect);
	SetDialogItem(dlg, item, kind,       itemH, &itemRect);
}

extern pascal void GetDialogItemRect(DialogRef dlg, DialogItemIndex item, Rect *itemRect)
{
	DialogItemType 	junkKind;
	Handle 			junkItemH;

	DialogItemAssert(dlg, item);
	assert(itemRect != NULL);
	GetDialogItem(dlg, item, &junkKind, &junkItemH, itemRect);
}

extern pascal void SetDialogItemRect(DialogRef dlg, DialogItemIndex item, const Rect *itemRect)
{
	DialogItemType 	itemKind;
	Handle 			itemH;
	Rect 			tmpItemRect;

	DialogItemAssert(dlg, item);
	assert(itemRect != NULL);
	GetDialogItem(dlg, item, &itemKind, &itemH, &tmpItemRect);
	tmpItemRect = *itemRect;
	SetDialogItem(dlg, item,  itemKind,  itemH, &tmpItemRect);
}

pascal void GetDialogItemString (DialogRef dialog, DialogItemIndex index, Str255 str)
{
	if (MoreAssertPCG (dialog && index && str))
	{
		GetDialogItemText(GetDialogItemHandle(dialog, index), str);
	}
}

pascal void SetDialogItemString (DialogRef dialog, DialogItemIndex index, ConstStringPtr str)
{
	Handle     iHandle;
	ControlRef control;

	if (MoreAssertPCG (dialog && index)) {
		if (str == NULL) {
			str = "\p";
		}
		
		// If the dialog item is using a control rather than the pre-Appearance
		// sharing of a TEHandle, we must set the text using the control handle 
		// as the item handle.  If you don’t do this, it mostly works, but weird 
		// things happen with the text highlighting in edit text items.

		control = GetDialogControlRef(dialog, index);
		if (control != NULL) {
			SetDialogItemText( (Handle) control, str);
		} else {
			iHandle = GetDialogItemHandle(dialog, index);
			SetDialogItemText(iHandle, str);
		}
	}
}

// Most of the following routines don’t need to call DialogItemAssert because 
// they immediately call GetDialogControlRef, which does the assert.

extern pascal SInt16 GetDialogControlValue(DialogRef dlg, DialogItemIndex item)
{
	// DialogItemAssert(dlg, item);
	return GetControlValue(GetDialogControlRef(dlg, item));
}

extern pascal void SetDialogControlValue(DialogRef dlg, DialogItemIndex item, SInt16 value)
{
	// DialogItemAssert(dlg, item);
	SetControlValue(GetDialogControlRef(dlg, item), value);
}

extern pascal SInt16 GetDialogControlMinimum(DialogRef dlg, DialogItemIndex item)
{
	// DialogItemAssert(dlg, item);
	return GetControlMinimum(GetDialogControlRef(dlg, item));
}

extern pascal void SetDialogControlMinimum(DialogRef dlg, DialogItemIndex item, SInt16 min)
{
	// DialogItemAssert(dlg, item);
	SetControlMinimum(GetDialogControlRef(dlg, item), min);
}

extern pascal SInt16 GetDialogControlMaximum(DialogRef dlg, DialogItemIndex item)
{
	// DialogItemAssert(dlg, item);
	return GetControlMaximum(GetDialogControlRef(dlg, item));
}

extern pascal void SetDialogControlMaximum(DialogRef dlg, DialogItemIndex item, SInt16 max)
{
	// DialogItemAssert(dlg, item);
	SetControlMaximum(GetDialogControlRef(dlg, item), max);
}

extern pascal Boolean GetDialogControlBoolean(DialogRef dlg, DialogItemIndex item)
{
	// DialogItemAssert(dlg, item);
	return GetControlValue(GetDialogControlRef(dlg, item)) != 0;
}

extern pascal void SetDialogControlBoolean(DialogRef dlg, DialogItemIndex item, Boolean value)
{
	// DialogItemAssert(dlg, item);
	SetControlValue(GetDialogControlRef(dlg, item), value);
}

extern pascal void ToggleDialogControlBoolean(DialogRef dlg, DialogItemIndex item)
{
	// DialogItemAssert(dlg, item);
	SetDialogControlBoolean(dlg, item, ! GetDialogControlBoolean(dlg, item));
}

pascal Boolean ToggleDialogCheckBox (DialogRef dialog, DialogItemIndex itemIndex)
{
	if (!MoreAssertPCG (itemIndex > 0 && itemIndex <= CountDITL (dialog)))
		return false;

	short iType; Handle iHandle; Rect iRect;
	::GetDialogItem (dialog, itemIndex, &iType, &iHandle, &iRect);

	if (!MoreAssertPCG ((iType & ~(kItemDisableBit | kControlDialogItem)) == chkCtrl))
		return false;

	Boolean result = !::GetControlValue (ControlRef (iHandle));
	::SetControlValue (ControlRef (iHandle), result);

	return result;
}

extern pascal Boolean GetDialogControlEnable(DialogRef dlg, DialogItemIndex item)
{
	Boolean 	result;
	ControlRef 	controlH;

	// DialogItemAssert(dlg, item);

	controlH = GetDialogControlRef(dlg, item);
	
	// Beware the wacky syntax!
	
	#if !TARGET_API_MAC_CARBON
		if (!HaveAppearance()) {
			result = ( (**controlH).contrlHilite != 255 );
		} else 
	#endif
		{
			result = IsControlActive(controlH);
		}
	return result;
}

extern pascal void SetDialogControlEnable(DialogRef dlg, DialogItemIndex item, Boolean enable)
{
	OSStatus 	junk;
	ControlRef 	controlH;
		
	// DialogItemAssert(dlg, item);

	controlH = GetDialogControlRef(dlg, item);

	// Beware the wacky syntax!

	#if !TARGET_API_MAC_CARBON
		if (!HaveAppearance()) {
			UInt8 newHighlight;
			
			// Only call HiliteControl if we’re changing the state
			// to avoid unsightly flashing.
			
			if (GetDialogControlEnable(dlg, item) != enable) {
				newHighlight = (255 * !enable);
				if ((**controlH).contrlHilite != newHighlight) {
					HiliteControl(controlH, newHighlight);
				}
			}
		} else 
	#endif
		{
			// Appearance controls don’t flash, so we don't have to call
			// GetDialogControlEnable.  Also, we shouldn’t do it because 
			// Appearance controls can be semi-active, ie active but
			// IsControlActive returns false because the parent control
			// is active, and we don’t want this to prevent us setting the
			// true state.
			
			if (enable) {
				junk = ActivateControl(controlH);
				assert(junk == noErr);
			} else {
				junk = DeactivateControl(controlH);
				assert(junk == noErr);
			}
		}
}

extern pascal void		InvalDialogItem			  (DialogRef dlg, DialogItemIndex item)
{
	OSStatus junk;
	Rect itemRect;

	GetDialogItemRect(dlg, item, &itemRect);
	junk = MoreInvalWindowRect(GetDialogWindow(dlg), &itemRect);
	assert(junk == noErr);
}

extern pascal void		SetupStandardDialogItems  (DialogRef dlg, DialogItemIndex defaultItem, DialogItemIndex cancelItem)
{
	OSStatus junk;
	
	if (defaultItem != 0) {
		junk = SetDialogDefaultItem(dlg, defaultItem);
		assert(junk == noErr);
	}
	if (cancelItem != 0) {
		junk = SetDialogCancelItem(dlg, cancelItem);
		assert(junk == noErr);
	}
	junk = SetDialogTracksCursor(dlg, true);
	assert(junk == noErr);
}

extern pascal Boolean	OKModalFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item)
	/* 	This is pretty much the standard modal dialog filter function.  It handles
		mapping returns and enters to the kStdOkItemIndex button, and also deals with
		shift tab.
	*/
{
	Boolean result;
	UInt8 typedChar;

	result = StdFilterProc(dlg, event, item);
	if ( !result && ( (event->what == keyDown) || (event->what == autoKey) )) {
		typedChar = (UInt8) (event->message & charCodeMask);
		if ((typedChar == kTabCharCode) and ((event->modifiers & shiftKey) != 0)) {
			if (GetDialogKeyboardFocusItem(dlg) != 0) {
				DialogShiftTab(dlg);
				result = true;
			}
		}
	}
	return result;
}

extern pascal Boolean	OKCancelModalFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item)
	/* This is the standard modal filter for dialogs with an OK and a Cancel button.
		It handles all that OKModalFilter does, and then deals with mapping Escape
		and command-dot to kStdCancelItemIndex.
	*/
{
	return OKModalFilter(dlg, event, item);
}

static pascal Boolean DoButtonKey(DialogRef dlg, DialogItemIndex item,
												EventRecord *event, DialogItemIndex *resultItem)
	/* This function is used to respond to a keyboard event hitting a dialog
		button.  If the button denoted by dlg and item is enabled, the function
		flashes the button, sets resultItem to item, and returns true.
		Otherwise it beeps and sets event to a null event.
	*/
{
	Boolean result;
	
	if ( GetDialogControlEnable(dlg, item) ) {
		*resultItem = item;
		FlashDialogControl(dlg, item);
		result = true;
	} else {
		SysBeep(10);
		event->what = nullEvent;
		result = false;
	}
	return result;
}

static pascal Boolean   DiscardButtonFilter(DialogPtr dlg, EventRecord *event, DialogItemIndex *item)
{
	Boolean result;
	UInt8 typedChar;

	result = false;
	if ((event->what == keyDown) || (event->what == autoKey)) {
		typedChar = (UInt8) (event->message & charCodeMask);
		if ((typedChar == 'd') and ((event->modifiers & cmdKey) != 0)) {
			result = DoButtonKey(dlg, kStdDiscardItemIndex, event, item);
		}
	}
	return result;
}

extern pascal Boolean	OKCancelDiscardModalFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item)
	/* This is the standard modal filter for dialogs with an OK, Cancel and Dont Save button.
		It handles all that OKCancelModalFilter does, and then deals with mapping 
		command-D to ditDontSave.
	*/
{
	Boolean result;
	
	result = OKCancelModalFilter(dlg, event, item);
	if (! result) {
		result = DiscardButtonFilter(dlg, event, item);
	}
	return result;
}

extern pascal Boolean	OKAlertFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item)
{
	Boolean result;
	UInt8 typedChar;

	result = OKModalFilter(dlg, event, item);
	if (! result && ( (event->what == keyDown) || (event->what == autoKey) )) {
		typedChar = (UInt8) (event->message & charCodeMask);
		if ((typedChar == kReturnCharCode) || (typedChar == kEnterCharCode)) {
			result = DoButtonKey(dlg, kStdOkItemIndex, event, item);
		}
	}
	return result;
}

extern pascal Boolean	OKCancelAlertFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item)
{
	Boolean result;
	UInt8 typedChar;

	result = OKAlertFilter(dlg, event, item);
	if (! result && ( (event->what == keyDown) || (event->what == autoKey) )) {
		typedChar = (UInt8) (event->message & charCodeMask);
		if ((typedChar == kEscapeCharCode) || IsCmdChar(event, '.')) {
			result = DoButtonKey(dlg, kStdCancelItemIndex, event, item);
		}
	}
	return result;
}

extern pascal Boolean	OKCancelDiscardAlertFilter(DialogRef dlg, EventRecord *event, DialogItemIndex *item)
{
	Boolean result;

	result = OKCancelAlertFilter(dlg, event, item);
	if (! result) {
		result = DiscardButtonFilter(dlg, event, item);
	}
	return result;
}

// These are the UPP getters for the filter routines.  They are implemented 
// as getters rather than global variables because that way we can avoid have 
// an initialisation routine for this module.

static ModalFilterUPP gOKModalFilterUPP = NULL;

extern pascal ModalFilterUPP GetOKModalFilterUPP(void)
{
	if (gOKModalFilterUPP == NULL) {
		gOKModalFilterUPP = NewModalFilterUPP(OKModalFilter);
	}
	return gOKModalFilterUPP;
}

static ModalFilterUPP gOKCancelModalFilterUPP = NULL;

extern pascal ModalFilterUPP GetOKCancelModalFilterUPP(void)
{
	if (gOKCancelModalFilterUPP == NULL) {
		gOKCancelModalFilterUPP = NewModalFilterUPP(OKCancelModalFilter);
	}
	return gOKCancelModalFilterUPP;
}

static ModalFilterUPP gOKCancelDiscardModalFilterUPP = NULL;

extern pascal ModalFilterUPP GetOKCancelDiscardModalFilterUPP(void)
{
	if (gOKCancelDiscardModalFilterUPP == NULL) {
		gOKCancelDiscardModalFilterUPP = NewModalFilterUPP(OKCancelDiscardModalFilter);
	}
	return gOKCancelDiscardModalFilterUPP;
}

static ModalFilterUPP gOKAlertFilterUPP = NULL;

extern pascal ModalFilterUPP GetOKAlertFilterUPP(void)
{
	if (gOKAlertFilterUPP == NULL) {
		gOKAlertFilterUPP = NewModalFilterUPP(OKAlertFilter);
	}
	return gOKAlertFilterUPP;
}

static ModalFilterUPP gOKCancelAlertFilterUPP = NULL;

extern pascal ModalFilterUPP GetOKCancelAlertFilterUPP(void)
{
	if (gOKCancelAlertFilterUPP == NULL) {
		gOKCancelAlertFilterUPP = NewModalFilterUPP(OKCancelAlertFilter);
	}
	return gOKCancelAlertFilterUPP;
}

static ModalFilterUPP gOKCancelDiscardAlertFilterUPP = NULL;

extern pascal ModalFilterUPP GetOKCancelDiscardAlertFilterUPP(void)
{
	if (gOKCancelDiscardAlertFilterUPP == NULL) {
		gOKCancelDiscardAlertFilterUPP = NewModalFilterUPP(OKCancelDiscardAlertFilter);
	}
	return gOKCancelDiscardAlertFilterUPP;
}

// The following helper routines are required by the various filters, 
// and are exported for your convenience.

extern pascal Boolean	DialogItemHidden(DialogRef dlg, DialogItemIndex item)
	/* Returns true if the item has been hidden with HideDialogItem. */
{
	Rect itemRect;

	GetDialogItemRect(dlg, item, &itemRect);
	return (itemRect.top < 16384) and (itemRect.left < 16384);
}

extern pascal void FlashDialogControl(DialogRef dlg, DialogItemIndex item)
{
	UInt32 junkLong;

	// DialogItemAssert(dlg, item);

	HiliteControl(GetDialogControlRef(dlg, item), kControlButtonPart);
	Delay(2, &junkLong);
	HiliteControl(GetDialogControlRef(dlg, item), 0);
}

static Boolean IsVisibleEditText(DialogPtr dlg, DialogItemIndex item)
{
	DialogItemType kind;
	
	GetDialogItemKind(dlg, item, &kind);
	return ((kind == editText) && DialogItemHidden(dlg, item));
}

extern pascal void		DialogShiftTab(DialogRef dlg)
	/*	Performs a shift tab operation in the dialog, ie moves the text selection
		to the edit text item immediately before the current one.
		
		This code doesn't deal well with there being no currently selected
		text.  Fortunately this shouldn’t comes up, because you should always 
		select the first text item before bringing up a dialog.  Something to
		work on later I guess.
	*/
{
	DialogItemIndex originalItem;
	DialogItemIndex itemIndex;
	DialogItemIndex itemCount;

	originalItem = GetDialogKeyboardFocusItem(dlg);
	itemCount = CountDITL(dlg);
	
	/* We only have to do work if there's more than one item in the dialog.
		Also bail out of there's no text selected because we don't deal with that case
	*/
	if ((originalItem > 0) and (itemCount > 1)) {

		/* Start at the originalItem and walk backwards looking for
			an unhidden text item.  If itemIndex hits 0, wrap around
			to the last item. Stop if we get back to the original
			item or we find an item.
		*/
		itemIndex = originalItem;
		do {
			itemIndex = (DialogItemIndex) (itemIndex - 1);
			if (itemIndex == 0) {
				itemIndex = itemCount;
			}
		} while ( (itemIndex != originalItem) && !IsVisibleEditText(dlg, itemIndex) );
		if ((itemIndex != originalItem) && IsVisibleEditText(dlg, itemIndex)) {
			SelectDialogItemText(dlg, itemIndex, 0, 32767);
		}
	}
}

extern pascal OSStatus StandardAlertCFStringCompat(AlertType                     	inAlertType,
										  		   CFStringRef	                 	inError,
										  		   CFStringRef                   	inExplanation,
										     const AlertStdCFStringAlertParamRec *  inAlertParam,        /* can be NULL */
												   SInt16 *                      	outItemHit)
	// See comment in header file.
{
	OSStatus err;
	
	assert(inError != NULL);
	assert(outItemHit != NULL);
	
	if (CreateStandardAlert != NULL && RunStandardAlert != NULL) {
		DialogRef 	alertRef;
		
		alertRef = NULL;
		
		err = CreateStandardAlert(inAlertType, inError, inExplanation, inAlertParam, &alertRef);
		if (err == noErr) {
			err = RunStandardAlert(alertRef, NULL, outItemHit);
		}
	} else {
		Str255		errPStr;
		Str255		expPStr;
		AlertStdAlertParamRec 	alertParams;
		AlertStdAlertParamRec *	alertParamsPtr;
		Str255		defaultPStr;
		Str255		cancelPStr;
		Str255		otherPStr;
		int			i;

		alertParamsPtr = NULL;
		
		err = noErr;
		if ( ! CFStringGetPascalString(inError, errPStr, sizeof(errPStr), GetApplicationTextEncoding()) ) {
			err = coreFoundationUnknownErr;
		}
		if (err == noErr) {
			if (inExplanation == NULL) {
				expPStr[0] = 0;
			} else {
				if ( ! CFStringGetPascalString(inExplanation, expPStr, sizeof(expPStr), GetApplicationTextEncoding()) ) {
					err = coreFoundationUnknownErr;
				}
			}
		}
		
		if ( (err == noErr) && (inAlertParam != NULL) ) {
			alertParamsPtr = &alertParams;

			// Convert all of the fields that can't cause an error.
			
			assert(inAlertParam->version == kStdCFStringAlertVersionOne);
			alertParams.movable       = inAlertParam->movable;
			alertParams.helpButton    = inAlertParam->helpButton;
			alertParams.filterProc    = NULL;
			alertParams.defaultButton = inAlertParam->defaultButton;
			alertParams.cancelButton  = inAlertParam->cancelButton;
			alertParams.position      = inAlertParam->position;
			assert(inAlertParam->flags == 0);

			// Convert the strings.  This is complicated by the fact that the 
			// incoming value (for example, inAlertParam->defaultText) can be either NULL 
			// or -1, and we have to pass those values along unmodified, but convert 
			// any other values.
			
			if ( (inAlertParam->defaultText == NULL) || (inAlertParam->defaultText == (CFStringRef) kAlertDefaultOKText) ) {
				alertParams.defaultText = (StringPtr) inAlertParam->defaultText;
			} else {
				alertParams.defaultText = defaultPStr;
				if ( ! CFStringGetPascalString(inAlertParam->defaultText, defaultPStr, sizeof(defaultPStr), GetApplicationTextEncoding()) ) {
					err = coreFoundationUnknownErr;
				}
			}
			if (err == noErr) {
				if ( (inAlertParam->cancelText == NULL) || (inAlertParam->cancelText == (CFStringRef) kAlertDefaultCancelText) ) {
					alertParams.cancelText = (StringPtr) inAlertParam->cancelText;
				} else {
					alertParams.cancelText = cancelPStr;
					if ( ! CFStringGetPascalString(inAlertParam->cancelText, cancelPStr, sizeof(cancelPStr), GetApplicationTextEncoding()) ) {
						err = coreFoundationUnknownErr;
					}
				}
			}			
			if (err == noErr) {
				if ( (inAlertParam->otherText == NULL) || (inAlertParam->otherText == (CFStringRef) kAlertDefaultOtherText) ) {
					alertParams.otherText = (StringPtr) inAlertParam->otherText;
				} else {
					alertParams.otherText = otherPStr;
					if ( ! CFStringGetPascalString(inAlertParam->otherText, otherPStr, sizeof(otherPStr), GetApplicationTextEncoding()) ) {
						err = coreFoundationUnknownErr;
					}
				}
			}			
		}
		
		if (err == noErr) {
			// To work correctly with the CFBundle's localization support, 
			// alert strings should have line breaks delimited by LF.  
			// However, StandardAlert doesn't handle LF, so if we're going 
			// to call it we convert the LFs to CRs.
			
			for (i = 1; i < errPStr[0]; i++) {
				if (errPStr[i] == 10) {
					errPStr[i] = 13;
				}
			}
			for (i = 1; i < expPStr[0]; i++) {
				if (expPStr[i] == 10) {
					expPStr[i] = 13;
				}
			}

			err = StandardAlert(inAlertType, errPStr, expPStr, alertParamsPtr, outItemHit);
		}
	}
	return err;
}
