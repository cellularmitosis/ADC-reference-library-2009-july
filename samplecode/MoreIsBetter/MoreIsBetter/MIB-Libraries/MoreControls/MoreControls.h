/*
	File:		MoreControls.h

	Contains:	Control Manager utilities.

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

$Log: MoreControls.h,v $
Revision 1.11  2003/03/28 16:14:58         
Removed a bunch of #pragma import/export directives.  IIRC PCG added them for reasons I don't fully understand.  He may have meant #pragma internal.  Regardless, there presence was causing lots of MIB symbols to be CFM exported, which is wrong.  MIB is not meant to ship as a shared library.

Revision 1.10  2002/11/25 18:26:21         
Copy/SetTextControlTextCompat have to take a password parameter because you have to do different actions based on whether the control is a password control on traditional Mac OS, and there's no way to determine that from the control.

Revision 1.9  2002/11/08 23:15:34         
When using framework includes, explicitly include the frameworks we need. Insert/RemoveTab now use an SInt16 to denote the tab (trust me, it makes sense). Added GetControlByIDQ helper. Added routines to get/set text control text as a CFString. Added key and value filters for edit text controls; these allow you to get a Carbon event when an edit text control is modified. Added SetControlActive/Visible helpers.

Revision 1.8  2001/11/07 15:52:02         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>     17/1/00    Quinn   Universal Interfaces 3.3
         <5>     2/11/99    PCG     Carbon d7
         <4>      2/9/99    PCG     more TARGET_CARBON
         <3>     1/25/99    PCG     TARGET_CARBON
         <2>    11/11/98    PCG     fix headers
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <7>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <6>    10/29/98    PCG     various
         <5>      9/9/98    PCG     re-work import and export pragmas
         <4>      9/1/98    PCG     Universal Headers 3.2
         <3>     7/24/98    PCG	    coddle linker (C++, CFM-68K)
         <2>     7/24/98    PCG	    eliminate dependency on 'qd'
         <2>     6/23/98    PCG     add IsScrollBar
         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <Controls.h>
	#include <ControlDefinitions.h>
	#include <Lists.h>
#endif

enum
{
	//////////////////////////////////////////////////////////////////
	//
	//	resource IDs of CDEFs (control definition proc resources)
	//
	//////////////////////////////////////////////////////////////////

	kControlOldButtonDefProcResID,
	kControlOldScrollBarDefProcResID,
	kControlBevelButtonDefProcResID,
	kControlSliderDefProcResID,
	kControlTriangleDefProcResID,
	kControlProgressBarDefProcResID,
	kControlLittleArrowsDefProcResID,
	kControlChasingArrowsDefProcResID,
	kControlTabsDefProcResID,
	kControlSeparatorLineDefProcResID,
	kControlGroupBoxDefProcResID,
	kControlImageWellDefProcResID,
	kControlPopupArrowDefProcResID,
	kControlUnluckyDefProcResID,
	kControlPlacardDefProcResID,
	kControlClockDefProcResID,
	kControlUserPaneDefProcResID,
	kControlEditTextDefProcResID,
	kControlStaticTextDefProcResID,
	kControlPictureDefProcResID,
	kControlIconDefProcResID,
	kControlWindowHeaderDefProcResID,
	kControlListBoxDefProcResID,
	kControlPushButtonDefProcResID,
	kControlScrollBarDefProcResID,
	kControlPopupButtonDefProcResID,
	kControlRadioGroupDefProcResID,
	kControlOldPopupMenuDefProcResID = 63
};

enum
{
	//////////////////////////////////////////////////////////////////
	//
	//	resource type for individual control font style record
	//
	//////////////////////////////////////////////////////////////////

	kMoreControls_ControlStyleType = FOUR_CHAR_CODE('cfsr')
};

enum
{
	//////////////////////////////////////////////////////////////////
	//
	//	gap between controls and from edges of the parent window
	//
	//////////////////////////////////////////////////////////////////

	MoreControls_kControlGap = (3 + 1) // 3 for focus, 1 for white space
};

enum
{
	//////////////////////////////////////////////////////////////////
	//
	//	alignments for AlignControl and AlignSubControls
	//
	//////////////////////////////////////////////////////////////////


	MoreControls_kAlignSystem,
	MoreControls_kAlignLeft,
	MoreControls_kAlignCenter,
	MoreControls_kAlignRight
};

typedef UInt8 MoreControls_tAlignment;

enum
{
	//////////////////////////////////////////////////////////////////
	//
	//	directions for MoveControlNear and SetControlQuadrant
	//
	//////////////////////////////////////////////////////////////////

	MoreControls_kDirectionNorthWest,
	MoreControls_kDirectionNorth,
	MoreControls_kDirectionNorthEast,
	MoreControls_kDirectionWest,
	MoreControls_kDirectionCenter,
	MoreControls_kDirectionEast,
	MoreControls_kDirectionSouthWest,
	MoreControls_kDirectionSouth,
	MoreControls_kDirectionSouthEast
};

typedef UInt8 MoreControls_tDirection;

//////////////////////////////////////////////////////////////////
//
//	tab control stuff for Appearance 1.0.1 or later
//
//////////////////////////////////////////////////////////////////

#if UNIVERSAL_INTERFACES_VERSION < 0x0310

enum
{
	kControlTabInfoTag			= 'tabi'						/* ControlTabInfoRec*/
};


enum {
	kControlTabInfoVersionZero	= 0
};

struct ControlTabInfoRec {
	SInt16 							version;					/* version of this structure.*/
	SInt16 							iconSuiteID;				/* icon suite to use. Zero indicates no icon*/
	Str255 							name;						/* name to be displayed on the tab*/
};
typedef struct ControlTabInfoRec ControlTabInfoRec;

#endif // UNIVERSAL_INTERFACES_VERSION < 0x0310

//////////////////////////////////////////////////////////////////
//
//	finally, the function prototypes
//
//////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	extern "C" {
#endif

pascal OSStatus OffsetControl							(ControlHandle, SInt16 h, SInt16 v);
pascal OSStatus InsetControl							(ControlHandle, SInt16 h, SInt16 v);
pascal OSStatus ExpandControl							(ControlHandle);
pascal OSStatus EncloseSubControls						(ControlHandle);
pascal OSStatus AlignControl							(ControlHandle anchor, ControlHandle, MoreControls_tAlignment);
pascal OSStatus AlignSubControls						(ControlHandle, MoreControls_tAlignment);
pascal OSStatus SetBestControlRect						(ControlHandle, SInt16 *baseLineOffset);
pascal OSStatus MoveControlNear							(ControlHandle moveMe, ControlHandle near, MoreControls_tDirection);
pascal OSStatus SetControlQuadrant						(ControlHandle, MoreControls_tDirection);
pascal OSStatus MoreFindControlUnderMouse				(Point, WindowRef, ControlPartCode *, ControlHandle *);

pascal OSStatus GetListBoxControlListHandle				(ControlHandle, ListHandle *);
pascal OSStatus GetStaticTextControlTextHeight			(ControlHandle, SInt16 *);

pascal OSStatus SetUserPaneTrackingProc					(ControlHandle, ControlUserPaneTrackingUPP);
pascal OSStatus SetUserPaneDrawProc						(ControlHandle, ControlUserPaneDrawUPP);
pascal OSStatus SetUserPaneSetUpSpecialBackgroundProc	(ControlHandle, ControlUserPaneBackgroundUPP);

pascal OSStatus SetBevelButtonMenu						(ControlHandle, MenuRef);
pascal OSStatus SetBevelButtonMenuDelay					(ControlHandle, SInt32);

pascal OSStatus SetProgressBarIndeterminate				(ControlHandle, Boolean);

pascal OSStatus SetPopUpButtonMenu						(ControlHandle, MenuRef);

pascal OSStatus Get1NewControl							(short resID, WindowRef, ControlHandle *);
pascal OSStatus Get1NewStaticTextControl				(short resID, WindowRef, ControlHandle *);
pascal OSStatus Get1NewPopupButtonControl				(short resID, WindowRef, ControlHandle *);

pascal OSStatus InvalControl							(ControlHandle);
pascal OSStatus ValidControl							(ControlHandle);

pascal OSStatus CopyTabFontStyle						(ControlHandle oldTabs, ControlHandle newTabs);
pascal OSStatus ToggleControl							(ControlHandle);
pascal OSStatus SetCursorAccordingToControl				(void);
pascal OSStatus GetControlDefProcResID					(ControlHandle, short *);
pascal OSStatus MoreGetControlRegion					(ControlHandle, RgnHandle *);
pascal OSStatus SetControlTextFromResource				(ControlHandle, short textResID);
pascal OSStatus SetControlTextStyleFromResource			(ControlHandle, short styleResID);

pascal OSStatus InsertTab								(ControlHandle, ControlHandle *, SInt16 before);
pascal OSStatus RemoveTab								(ControlHandle, ControlHandle *, SInt16 which);
pascal OSStatus SetTabIcon								(ControlHandle, ControlPartCode, short iconSuiteID);
pascal OSStatus AppendTabs								(ControlHandle, UInt16);
pascal OSStatus TruncateTabs							(ControlHandle, UInt16);

pascal OSStatus IsScrollBar								(ControlHandle, Boolean *);

pascal OSStatus IdleControlsInAllWindows				(void);

pascal OSStatus ActivateControls						(WindowPtr);
pascal OSStatus DeactivateControls						(WindowPtr);

extern OSStatus GetControlByIDQ(WindowRef inWindow, OSType signature, SInt32 id, ControlRef *outControl);

#if TARGET_API_MAC_CARBON

extern pascal OSStatus CopyTextControlTextCompat(ControlRef control, Boolean isPassword, CFStringRef *textStr);
	// Gets the text of an edit or static text control as a CFString. 
	// You are responsible for releasing the string.

extern pascal OSStatus SetTextControlTextCompat(ControlRef control, Boolean isPassword, CFStringRef textStr);
	// Sets the text of an edit or static text control as a CFString.

#endif

extern pascal OSStatus 					InstallControlKeyFilter(ControlRef control, OSType myTag, ControlKeyFilterUPP newFilter);
extern pascal ControlKeyFilterResult 	CallNextControlKeyFilter(ControlRef control, OSType myTag, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers);
	// Allows you to install a chain of kControlKeyFilterTag callbacks on a 
	// control.  When you install, the old value is stored as a property 
	// of the control (specified by tag).


extern pascal ControlKeyFilterResult ControlNoReturnsKeyFilter(ControlRef theControl, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers);
	// A control key filter callback that excludes CR.

extern pascal ControlKeyFilterUPP GetControlNoReturnsKeyFilterUPP(void);
	// Returns a UPP for the above.

enum {
	kMIBControlNoReturnsKeyFilterKeyFilterTag = 'NoCR'
		// Use this if you want to call InstallControlKeyFilter with GetControlNoReturnsKeyFilterUPP.
};

#if TARGET_API_MAC_CARBON

enum {
	kMoreIsBetterEventClass = 'MIB!',
	kMIBControlEditTextModifiedKind = 'EDTX'
};

extern pascal ControlKeyFilterResult ControlEditTextModifiedKeyFilter(ControlRef theControl, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers);
	// A control key filter callback that sends a
	// kMoreIsBetterEventClass/kMIBControlEditTextModifiedKind Carbon event
	// to the control if the key could potentially modify the control.

extern pascal ControlKeyFilterUPP GetControlEditTextModifiedKeyFilterUPP(void);
	// Returns a UPP for the above.

enum {
	kMIBControlEditTextModifiedKeyFilterTag = 'ModK'
		// Use this if you want to call InstallControlKeyFilter with GetControlEditTextModifiedKeyFilterUPP.
};

extern pascal void ControlEditTextModifiedValidationProc(ControlRef control);
	// A control validation proc that sends a
	// kMoreIsBetterEventClass/kMIBControlEditTextModifiedKind Carbon event
	// to the control if the control could potentially have been modified.

extern pascal ControlEditTextValidationUPP GetControlEditTextModifiedValidationProcUPP(void);
	// Returns a UPP for the above.

#endif

extern pascal void SetControlActive(ControlRef control, Boolean active);
extern pascal void SetControlVisible(ControlRef control, Boolean visible);

#ifdef __cplusplus
	}
#endif

#if ! TARGET_API_MAC_CARBON
	#define GetControlDefinition(x)		((**(x)).contrlDefProc)
	#define GetControlOwner(x)			((**(x)).contrlOwner)
	#define GetControlDataHandle(x)		((**(x)).contrlData)
	#define GetControlBounds(x,r)		do { *(r) = (**(x)).contrlRect; } while (false)
#endif
