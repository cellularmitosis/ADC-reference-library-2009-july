/*
	File:		MoreControls.cp

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

$Log: MoreControls.cp,v $
Revision 1.12  2003/04/14 15:50:09         
In CopyTextControlTextCompat, use NewPtr rather than malloc to prevent "malloc(0) returns NULL" problem.

Revision 1.11  2002/11/25 18:26:46         
Copy/SetTextControlTextCompat have to take a password parameter because you have to do different actions based on whether the control is a password control on traditional Mac OS, and there's no way to determine that from the control.

Revision 1.10  2002/11/08 23:18:13         
When using framework includes, explicitly include the frameworks we need. Added GetControlByIDQ helper. Added routines to get/set text control text as a CFString. Added key and value filters for edit text controls; these allow you to get a Carbon event when an edit text control is modified. Added SetControlActive/Visible helpers. Include our prototype early to flush out any missing dependencies. Convert MoreAssert to MoreAssertPCG. Convert MoreAssertQ to assert. Eliminate implicit arithmetic conversion from CW. Convert nil to NULL.

Revision 1.9  2001/11/07 15:51:59         
Tidy up headers, add CVS logs, update copyright.


         <8>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <7>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <6>      1/3/00    Quinn   Completely rewrite GetControlDefProcResID.  This is the best
                                    approximation of this routine possible prior to new features
                                    that should be introduced in CarbonLib 1.1.  It won’t work
                                    properly on Mac OS X.  Also some other minor Carbon changes.
         <5>     2/23/99    PCG     fix copy and paste bug in MoveControlNear introduced during
                                    Carbonation
         <4>      2/9/99    PCG     more TARGET_CARBON
         <3>     1/25/99    PCG     TARGET_CARBON
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <5>    10/29/98    PCG     various
         <4>      9/1/98    PCG     Universal Headers 3.2
         <3>     7/24/98    PCG	    eliminate dependency on 'qd'
         <2>     7/21/98    PCG     remove annoying assert in Get1NewStaticTextControl
         <2>     6/23/98    PCG     add IsScrollBar
         <1>     6/16/98    PCG     initial checkin
*/

#include "MoreControls.h"

#include "MoreAppearance.h"
#include "MoreWindows.h"
#include "MoreQuickDraw.h"
#include "MoreMemory.h"

#if TARGET_API_MAC_CARBON
	#include "MoreCFQ.h"
#endif

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Script.h>
	#include <MacWindows.h>
	#include <Resources.h>
	#include <Gestalt.h>
#endif

#include <stdlib.h>

typedef struct
{
	//
	//	Used by AppendTabInfo and its callers.
	//	Caches tab contents data temporarily.
	//

	Boolean				enabled;
	ControlTabInfoRec	ctir;
}
FullControlTabInfoRec, *FullControlTabInfoRecP, **FullControlTabInfoRecH;

	//
	//	gControlScratchStr is used to relieve us from having to declare
	//	a Str255 on the stack when we call GetResInfo and we don't care
	//	about the name of the resource.
	//

static Str255 gControlScratchStr;

	//
	//	AssertControlHasDefProcResID is a macro which asserts that
	//	a given control's control definition procedure resource has
	//	a given resource ID. This is generally used to validate
	//	assumptions about the data tags supported by a given
	//	control definition procedure.
	//

#if MORE_DEBUG
	static pascal Boolean MoreAssertControlHasDefProcResID
		(ControlHandle c, short requiredDPR)
	{
		SInt16 actualDPR;

		if (!MoreAssertPCG (!GetControlDefProcResID (c,&actualDPR)))	return false;
		if (!MoreAssertPCG (actualDPR == requiredDPR))					return false;

		return true;
	}
#else
#	define MoreAssertControlHasDefProcResID(c,requiredDPR) (true)
#endif

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

struct HackControlRecord {
    ControlRef                      nextControl;
    WindowRef                       contrlOwner;
    Rect                            contrlRect;
    UInt8                           contrlVis;
    UInt8                           contrlHilite;
    SInt16                          contrlValue;
    SInt16                          contrlMin;
    SInt16                          contrlMax;
    Handle                          contrlDefProc;
};
typedef struct HackControlRecord HackControlRecord, *HackControlRecordPtr, **HackControlRecordHandle;

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

pascal OSStatus GetControlDefProcResID (ControlHandle control, short *resID)
{
	//
	//	Given a control, determines what the resource ID of the control's
	//	control definition procedure resource. If the resource has been
	//	detached, this routine will fail and return an appropriate error.
	//	Don't detach system definition procedure resources! Other people
	//	may be using them as resources (expecting LoadResource to work, etc.)
	//

	OSStatus err;
	ResType  junkResType;
	UInt32   gestaltResponse;
	Handle   controlDefH;
	
	err = noErr;
	
	// In Carbon 1.1 and higher (hopefully), there will be an API for doing this
	// properly.  The current plan is that this API will be called GetControlKind.
	// Unfortunately, this API is not in any current headers or libraries, so
	// I can’t conditionally use it.  Instead, I’ve just put in a hack implementation.
	// When that API is available, this routine will need to be modified to take
	// advantage of it.  Until then, software that absolutely relies on this
	// routine will not work properly on Mac OS X.  This shouldn’t be a problem
	// because Mac OS X will ship with Carbon 1.1.
	
	if ( (Gestalt(gestaltCarbonVersion, (SInt32 *) &gestaltResponse) == noErr)
			&& (gestaltResponse >= 0x011)) {
		#if MORE_DEBUG
			DebugStr("\pGetControlDefProcResID: Should be using GetControlKind");
		#endif
	}

	// The technique I’m currently using won’t work on Mac OS X, so we return
	// an error in that case.
	
	if ( (Gestalt(gestaltSystemVersion, (SInt32 *) &gestaltResponse) == noErr)
			&& (gestaltResponse >= 0x0a0)) {
		err = unimpErr;
	}

	// Now use the old, traditional Mac OS-only method of grabbing the control
	// definition handle out of the control handle.
	
	if (err == noErr) {
		controlDefH = (**((HackControlRecordHandle) control)).contrlDefProc;
		
		GetResInfo(controlDefH, resID, &junkResType, gControlScratchStr);
		err = ResError();
		
		assert(err || (junkResType == kControlDefProcType));
	}
	
	return err;
}

static pascal MoreControls_tAlignment FilterAlignment (MoreControls_tAlignment align)
{
	//
	//	If this function is passed MoreControls_kAlignSystem, it determines
	//	what the correct alignment constant is based on the direction of the
	//	system script. This way radio buttons on an Arabic system, whose
	//	bubbles are on the right side, will align down the right edge without
	//	the client program needing to know the details.
	//

	if (align == MoreControls_kAlignSystem)
		align = GetSysDirection ( ) == -1 ? MoreControls_kAlignRight : MoreControls_kAlignLeft;

	assert(align == MoreControls_kAlignLeft ||
		align == MoreControls_kAlignCenter || align == MoreControls_kAlignRight);

	return align;
}

#pragma mark -

#if 0

// Pete Gontier left this routine in this file, but it’s neither exported 
// not used by the other routines in the file.  Rather than delete it, 
// I’ve just left it here in case we ever need to resurrect it.
// -- Quinn, 8 Feb 2001

static pascal OSStatus AppendTabInfo
	(ControlHandle whichControl, UInt16 tabIndex,
		FullControlTabInfoRecP tabInfoP, FullControlTabInfoRecH tabInfoPackedArrayH)
{
	OSStatus err = noErr;

	//
	//	For a given tab, this function gets the tab icon and string as
	//	well whether the tab is enabled. Appends said data onto the end
	//	of an array stored in a handle.
	//
	//	This is a helper function. tabInfoP is supposed to have been allocated
	//	by the caller, but the caller is not expected to care what's in it;
	//	we do this so the caller can call AppendTabInfo in a loop without
	//	this helper function repeatedly creating and destroying the buffer,
	//	which would be slow, we presume. It's a classic case of optimizing without
	//	first measuring.
	//

	if (!(MoreAssertPCG (tabIndex && tabIndex <= GetControlMaximum (whichControl))))
		err = paramErr;
	else
	{
		Size actualSize;

		tabInfoP->ctir.version = 0;

		if (!(err = GetControlData (whichControl, tabIndex, kControlTabInfoTag,
			sizeof (tabInfoP->ctir), Ptr (&(tabInfoP->ctir)), &actualSize)))
		{
			if (!(err = GetControlData (whichControl, tabIndex, kControlTabEnabledFlagTag,
				sizeof (tabInfoP->enabled), Ptr (&(tabInfoP->enabled)), &actualSize)))
			{
				if (!MoreAssertPCG (actualSize == sizeof (tabInfoP->enabled)))
					err = paramErr;
				else
					err = PtrAndHand (tabInfoP, Handle (tabInfoPackedArrayH), 1 + *(tabInfoP->ctir.name) + (sizeof (*tabInfoP) - sizeof (Str255)));
			}
		}
	}

	return err;
}

#endif

pascal OSStatus MoreGetControlRegion (ControlHandle control, RgnHandle *rgn)
{
	//
	//	Given a control, allocates and initializes a region describing the
	//	control's bounds. Note this is different from (**control).contrlRect;
	//	a control whose control definition function calls DrawThemeEditTextFrame
	//	will (should) report its region as being 2 pixels larger than its contrlRect.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertPCG (control && rgn))
		err = nilHandleErr;
	else
	{
		*rgn = NewRgn ( );

		if (!(err = QDError ( )))
		{
//			if (GetControlRegion)
//				err = GetControlRegion (control,whichPartCodeIsEquivalent,*rgn);
//			else
				#if UNIVERSAL_INTERFACES_VERSION <= 0x0332
					(void) SendControlMessage (control, calcCntlRgn, (SInt32) *rgn);
				#else
					(void) SendControlMessage (control, calcCntlRgn, *rgn);
				#endif

			if (err)
			{
				DisposeRgn (*rgn);
				assert(noErr == QDError ( ));
				*rgn = NULL;
			}
		}

		if (err)
			*rgn = NULL;
	}

	return err;
}

pascal OSStatus InvalControl (ControlHandle control)
{
	OSStatus err = noErr;

	//
	//	Given a control, invalidates the area occupied by the control.
	//

	if (!(MoreAssertPCG (control)))
		err = nilHandleErr;
	else
	{
		RgnHandle rgn;

		err = MoreGetControlRegion (control, &rgn);

		if (!err)
		{
			GrafPtr preservePort;
			GetPort (&preservePort);
			WindowRef controlOwner = GetControlOwner (control);
			SetPortWindowPort (controlOwner);
			InvalWindowRgn (controlOwner,rgn);
			SetPort (preservePort);
			DisposeRgn (rgn);
		}
	}

	return err;
}

pascal OSStatus ValidControl (ControlHandle control)
{
	OSStatus err = noErr;

	//
	//	Given a control, validates the area occupied by the control.
	//

	if (!(MoreAssertPCG (control)))
		err = nilHandleErr;
	else
	{
		RgnHandle rgn;

		err = MoreGetControlRegion (control, &rgn);

		if (!err)
		{
			GrafPtr preservePort;
			GetPort (&preservePort);
			WindowRef controlOwner = GetControlOwner (control);
			SetPortWindowPort (controlOwner);
			ValidWindowRgn (controlOwner,rgn);
			SetPort (preservePort);
			DisposeRgn (rgn);
		}
	}

	return err;
}

pascal OSStatus SetTabIcon (ControlHandle control, ControlPartCode part, short iconSuiteID)
{
	OSStatus err = noErr;

	//
	//	This function demonstrates how to set the icon of a tab.
	//	Unfortunately, under 1.0.X, the control definition won't
	//	listen if the tab already has an icon which has been drawn.
	//	We don't assert for this because it works if the tab has
	//	no icon. Be careful.
	//

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertPCG (control))
		err = nilHandleErr;
	else if (!MoreAssertPCG (part >= GetControlMinimum (control)))
		err = paramErr;
	else if (!(MoreAssertPCG (part <= GetControlMaximum (control))))
		err = paramErr;
	else
	{
		ControlTabInfoRec *ctirp = (ControlTabInfoRec *) NewPtr (sizeof (*ctirp));

		if (!ctirp)
			err = MemError ( );
		else
		{
			Size actualSize;

			ctirp->version = 0;

			if (!(err = GetControlData (control,part,kControlTabInfoTag,sizeof(ControlTabInfoRec),
				Ptr(ctirp),&actualSize)))
			{
				ctirp->iconSuiteID = iconSuiteID;

				err = SetControlData (control,part,kControlTabInfoTag,actualSize,Ptr(ctirp));
			}

			DisposePtr (Ptr (ctirp));
			assert(noErr == MemError ( ));
		}
	}

	return err;
}

pascal OSStatus CopyTabFontStyle (ControlHandle oldTabs, ControlHandle newTabs)
{
	//
	//	Copies the control font style data from one tab control to another.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertPCG (oldTabs && newTabs))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (oldTabs,kControlTabsDefProcResID))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (newTabs,kControlTabsDefProcResID))
		err = paramErr;
	else
	{
		ControlFontStyleRec		cfsr;
		Size					actualSize;

		if (!(err = GetControlData (oldTabs,kControlNoPart,kControlTabFontStyleTag,sizeof(cfsr),Ptr(&cfsr),&actualSize)))
			err = SetControlData (newTabs,kControlNoPart,kControlTabFontStyleTag,actualSize,Ptr(&cfsr));
	}

	return err;
}

static pascal OSStatus DupTabsControl_Internal (ControlHandle old, ControlHandle *dup, SInt16 newControlMax)
{
	//
	//	This is a helper function containing common code to support
	//	other exported functions. It creates a control with an
	//	appropriate number of tabs, then embeds the control at
	//	the same place in the hierarchy as the old control. This
	//	function does not copy tab data from the old control to the
	//	new control, though it does copy the style info.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertPCG (GetAppearanceVersion ( ) >= 0x0101))
		err = paramErr;
	else if (!MoreAssertPCG (old && dup))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (old,kControlTabsDefProcResID))
		err = paramErr;
	else if (!MoreAssertPCG (GetControlMaximum (old) <= newControlMax))
		err = paramErr;

	//
	//	We assert that the old control is visible because there's a bug
	//	in the tabs CDEF such that it won't write tab data into a non-visible
	//	control. We copy tab data into the new control, so why do we
	//	assert the old one's state? Because we are duplicating the old one to
	//	the new one, and if the new one must be visible, it's not possible
	//	to duplicate an invisible control (the old one) to a visible one
	//	(the new one), because that wouldn't be duplication, now, would it?
	//

//	else if (!MoreAssertPCG (IsControlVisible (old)))
//		err = paramErr;
	else
	{
		ControlVariant		variant				= GetControlVariant (old);
		WindowPtr			oldControlOwner		= GetControlOwner (old);
		Rect				boundsRect;

		GetControlBounds (old,&boundsRect);

		*dup = NewControl (oldControlOwner, &boundsRect, "\p", IsControlVisible (old),
			0, 1, newControlMax, (short) ((kControlTabsDefProcResID << 4) | variant), 0);
		if (!*dup)
			err = nilHandleErr;
		else
		{
			if (!(err = CopyTabFontStyle (old,*dup)))
			{
				//
				//	Embed duplicate control in old control's super-control.
				//	If the old control is in a window without an embedding
				//	hierarchy, do nothing.
				//

				ControlHandle super;

				if (!(err = GetSuperControl (old,&super)))
					err = EmbedControl (*dup,super);
				else if (err == errNoRootControl)
					err = noErr;

				//
				//	Make sure the old and new controls have the same tab selected.
				//

				if (!err)
					SetControlValue (*dup, GetControlValue (old));
			}

			if (err)
			{
				DisposeControl (*dup);
				*dup = NULL;
			}
		}
	}

	return err;
}

static pascal OSStatus CopyTab
	(ControlHandle sourceControl, ControlHandle destControl, SInt16 sourceTab, SInt16 destTab)
{
	//
	//	Copies the tab data from a single tab in one control to a single tab in another control.
	//	This is a helper function which is designed to be called from inside a loop.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertPCG (GetAppearanceVersion ( ) >= 0x0101))
		err = paramErr;
	else if (!MoreAssertPCG (sourceControl))
		err = nilHandleErr;
	else if (!MoreAssertPCG (destControl))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (sourceControl,kControlTabsDefProcResID))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (destControl,kControlTabsDefProcResID))
		err = paramErr;
	else if (!MoreAssertPCG (sourceTab <= GetControlMaximum (sourceControl)))
		err = paramErr;
	else if (!MoreAssertPCG (destTab <= GetControlMaximum (destControl)))
		err = paramErr;
	else
	{
		Size				actualSize;
		ControlTabInfoRec	*ctirp			= (ControlTabInfoRec *) NewPtr (sizeof (*ctirp));

		if (!ctirp)
			err = MemError ( );
		else
		{
			Boolean tabIsEnabled;

			ctirp->version = 0;

			do
			{
				err = GetControlData (sourceControl,sourceTab,kControlTabInfoTag,sizeof(*ctirp),Ptr(ctirp),&actualSize);
				if (err) break;
				err = SetControlData (destControl,destTab,kControlTabInfoTag,actualSize,Ptr(ctirp));
				if (err) break;
				err = GetControlData (sourceControl,sourceTab,kControlTabEnabledFlagTag,sizeof(tabIsEnabled),Ptr(&tabIsEnabled),&actualSize);
				if (err) break;
				err = SetControlData (destControl,destTab,kControlTabEnabledFlagTag,actualSize,Ptr(&tabIsEnabled));
				if (err) break;
			}
			while (false);

			DisposePtr (Ptr (ctirp));
			assert(noErr == MemError ( ));
		}
	}

	return err;
}

pascal OSStatus InsertTab (ControlHandle oldTabs, ControlHandle *dup, SInt16 after)
{
	//
	//	Adds a single tab to a tabs control. Since there are no APIs for adding
	//	a tab (except at the end of the list [and that feature is buggy anyway]),
	//	we duplicate the entire control with an additional tab, then copy the tab
	//	data from the old control to the new control, leaving a gap for the new tab.
	//	The after parameter, of course, specifies the tab after which the new tab
	//	should appear. 0 is a valid value, even though it does not correspond to
	//	any existing tab; it means the new tab should appear first. This function
	//	also handles unreasonably high values by assuming you want a tab added after
	//	the last existing tab.
	//

	OSStatus err = noErr;

	if (!(MoreAssertPCG (oldTabs && dup)))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (oldTabs,kControlTabsDefProcResID))
		err = paramErr;
	else
	{
		SInt16 oldMax = GetControlMaximum (oldTabs);

		ControlHandle newTabs;

		if (!(err = DupTabsControl_Internal (oldTabs, &newTabs, (short) (oldMax + 1))))
		{
			if (SInt16 index = oldMax)
			{
				if (after > index) after = index;

				if (!(MoreAssertPCG (index >= after)))
					err = paramErr;
				else
				{
					// copy the tab data for the tabs after the new one

					while (index > after)
					{
						err = CopyTab (oldTabs,newTabs,index,(short)(index+1));
						if (err) break;				
						--index;
					}

					// copy the tab data for the tabs before the new one

					if (!err) while (index)
					{
						err = CopyTab (oldTabs,newTabs,index,index);
						if (err) break;
						--index;
					}
				}
			}

			if (err)
				DisposeControl (newTabs);
			else
			{
				short oldValue = GetControlValue (oldTabs);

				if (oldValue > after)
					SetControlValue (newTabs, (short) (oldValue + 1));

				*dup = newTabs;
			}
		}
	}

	return err;
}

pascal OSStatus RemoveTab (ControlHandle oldTabs, ControlHandle *dup, SInt16 victim)
{
	//
	//	Removes a single tab from a tabs control. Since there are no APIs for
	//	removing an arbitrary tab except at the end of the list, we duplicate
	//	the entire control with one fewer tab, then copy the tab data from the
	//	old control to the new control, skipping the doomed tab.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (victim && oldTabs && dup))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (oldTabs,kControlTabsDefProcResID))
		err = paramErr;
	else
	{
		SInt16 max = GetControlMaximum (oldTabs);

		if (!MoreAssertPCG (max && max >= victim))
			err = paramErr;
		else
		{
			ControlHandle newTabs;

			if (!(err = DupTabsControl_Internal (oldTabs, &newTabs, max)))
			{
				SInt16 index = 1;

				// copy the tab data for the tabs before the victim

				while (index < victim)
				{
					err = CopyTab (oldTabs,newTabs,index,index);
					if (err) break;
					++index;
				}

				// copy the tab data for the tabs after the victim

				if (!err) while (index <= max - 1)
				{
					err = CopyTab (oldTabs, newTabs, (short) (index + 1), index);
					if (err) break;				
					++index;
				}

				if (err)
					DisposeControl (newTabs);
				else
				{
					SetControlMaximum (newTabs, (short) (max - 1));

					short oldValue = GetControlValue (oldTabs);

					if (oldValue > victim)
					{
						short newValue = (short) (oldValue - 1);

						if (newValue != GetControlValue (newTabs))
							SetControlValue (newTabs, newValue);
					}

					*dup = newTabs;
				}
			}
		}
	}

	return err;
}

pascal OSStatus AppendTabs (ControlHandle control, UInt16 count)
{
	//
	//	The tabs CDEF in Appearance 1.0.X has a fencepost
	//	error in which it trashes its private tab data when
	//	growing the tabs, so we detect this and refuse to
	//	allow it to occur. If you need to add tabs to a tabs
	//	control under Appearance 1.0.X, you need to use
	//	InsertTab. The API is more cumbersome, but it does
	//	work.
	//

	OSStatus err = noErr;

	if (!(MoreAssertPCG (control)))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlTabsDefProcResID))
		err = paramErr;
	else if (!MoreAssertPCG (GetAppearanceVersion ( ) > 0x0101))
		err = paramErr;
	else
	{
		SInt16 max = GetControlMaximum (control);
		SetControlMaximum (control, (SInt16) (max + count));
	}

	return err;
}

pascal OSStatus TruncateTabs (ControlHandle control, UInt16 count)
{
	//
	//	The tabs CDEF in Appearance 1.0.X has a fencepost
	//	error in which it trashes its private tab data when
	//	shrinking the tabs, so we detect this and refuse to
	//	allow it to occur. If you need to truncate tabs from
	//	a tabs control under Appearance 1.0.X, you need to use
	//	RemoveTab. The API is more cumbersome, but it does
	//	work.
	//

	OSStatus err = noErr;

	if (!(MoreAssertPCG (control)))
		err = nilHandleErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlTabsDefProcResID))
		err = paramErr;
	else if (!MoreAssertPCG (GetAppearanceVersion ( ) > 0x0101))
		err = paramErr;
	else
	{
		SInt16 max = GetControlMaximum (control);

		if (!MoreAssertPCG (max && max >= count))
			err = paramErr;
		else
			SetControlMaximum (control, (SInt16) (max - count));
	}

	return err;
}

#if ! TARGET_API_MAC_CARBON

static pascal OSStatus SpoofFindControlUnderMouseForTabs
	(Point where, WindowRef window, ControlPartCode *cpc, ControlHandle whichControl)
{
	//
	//	The 1.0.X Tabs control has a bug which causes
	//	FindControlUnderMouse to produce kControlNoPart when the
	//	mouse is over the current tab. The work-around is a quick
	//	and dirty hack we can get away with because it only kicks
	//	in under certain (old) versions of the API which we know
	//	will never change. When this code was written, the bug had
	//	been fixed in a pre-release version of Mac OS.
	//	There's no iron-clad guarantee that version will ship with
	//	the version number I expect it to, so there is some small but
	//	non-zero chance this code may fail to work around the bug
	//	if it is present in a system with a version of Appearance
	//	greater than 1.0.1.
	//

	OSStatus err = noErr;

	if (*cpc == kControlNoPart && GetAppearanceVersion ( ) <= 0x0101)
	{
		SInt16 contrlValue = GetControlValue (whichControl);
		(**whichControl).contrlValue = 0;
		ControlHandle newWhichControl =  FindControlUnderMouse (where,window,cpc);
		assert(newWhichControl == whichControl);
		(**whichControl).contrlValue = contrlValue;
	}

	return err;
}

#endif

static pascal OSStatus SpoofFindControlUnderMouseForPopUpButton (ControlPartCode *cpc)
{
	//
	//	The popup button control has bugs which cause FindControlUnderMouse
	//	to produce kControlNoPart when the mouse is over the title and
	//	kControlLabelPart when the mouse is over the menu.
	//

	OSStatus err = noErr;

	if (*cpc == kControlLabelPart)
		*cpc = kControlMenuPart;
	else if (*cpc == kControlNoPart)
		*cpc = kControlLabelPart;

	return err;
}

pascal OSStatus MoreFindControlUnderMouse
	(Point where, WindowRef window, ControlPartCode *cpc, ControlHandle *whichControl)
{
	//
	//	Second-guesses the results of FindControlUnderMouse for certain controls.
	//	Decides which helper function to call based on the control definition
	//	procedure resource ID. The truly interesting stuff happens in the helper
	//	functions.
	//

	OSStatus err = noErr;

	if (HaveAppearance ( ))
		*whichControl = FindControlUnderMouse (where,window,cpc);
	else
		*cpc = FindControl (where,window,whichControl);

	if (*whichControl)
	{
		if (!(MoreAssertPCG (window == GetControlOwner (*whichControl))))
			err = paramErr;
		else
		{
			short resID;

			if (!(err = GetControlDefProcResID (*whichControl,&resID)))
			{
				if (HaveAppearance ( ))
				{
					switch (resID)
					{

#if ! TARGET_API_MAC_CARBON

						case kControlTabsDefProcResID :

							err = SpoofFindControlUnderMouseForTabs (where,window,cpc,*whichControl);
							break;

#endif

						case kControlPopupButtonDefProcResID	:

							err = SpoofFindControlUnderMouseForPopUpButton (cpc);
							break;
					}
				}
				else
				{
					// here we would work around bugs in pre-Appearance control definitions
				}
			}
		}
	}

	return err;
}

pascal OSStatus Get1NewControl (short resID, WindowRef window, ControlHandle *control)
{
	//
	//	Simple glue for Get1Resource. It only does two extra things. It verifies
	//	the control resource with the specified exists in the topmost resource
	//	file in the search chain and it releases the contol resource after
	//	GetNewControl is done with it. I have an aversion to purgeable handles,
	//	because they obfuscate leak detection, so I tend to write code like this.
	//

	OSStatus err = noErr;

	Handle controlResource = Get1Resource (kControlTemplateResourceType,resID);

	if (!controlResource)
	{
		err = ResError ( );
		if (!err) err = resNotFound;
	}
	else
	{
		*control = GetNewControl (resID,window);

		if (!*control)
			err = nilHandleErr;

		ReleaseResource (controlResource);
		assert(ResError ( ) == noErr);
	}

	return err;
}

pascal OSStatus SetControlTextFromResource (ControlHandle control, short textResID)
{
	OSStatus err = noErr;

	short controlDefProcResID;

	if (MoreAssertPCG (!(err = GetControlDefProcResID (control,&controlDefProcResID))))
	{
		if (!MoreAssertPCG (controlDefProcResID == kControlStaticTextDefProcResID || controlDefProcResID == kControlEditTextDefProcResID))
			err = paramErr;
		else
		{	
			Handle text = Get1Resource ('TEXT',textResID);

			if (!text)
			{
				err = ResError ( );
				if (!err) err = resNotFound;
			}
			else
			{
				Size size = GetHandleSize (text);

				if (MoreAssertPCG (MemError ( ) == noErr))
				{
					HLock (text);

					if (MoreAssertPCG (MemError ( ) == noErr))
						err = SetControlData (control,kControlNoPart,kControlStaticTextTextTag,size,*text);
				}

				ReleaseResource (text);
				assert(ResError ( ) == noErr);
			}
		}
	}

	return err;
}

pascal OSStatus SetControlTextStyleFromResource (ControlHandle control, short resID)
{
	OSStatus err = noErr;

	Handle controlStyle = Get1Resource (kMoreControls_ControlStyleType, resID);

	if (!controlStyle)
	{
		err = ResError ( );
		if (!err) err = resNotFound;
	}
	else
	{
		Size size = GetHandleSize (controlStyle);
		if (MoreAssertPCG (MemError ( ) == noErr))
		{
			if (!MoreAssertPCG (size == sizeof (ControlFontStyleRec)))
				err = paramErr;
			else
			{
				HLock (controlStyle);

				if (MoreAssertPCG (MemError ( ) == noErr))
					err = SetControlFontStyle (control, (const ControlFontStyleRec *) *controlStyle);
			}
		}

		ReleaseResource (controlStyle);
		assert(ResError ( ) == noErr);
	}

	return err;
}

pascal OSStatus Get1NewStaticTextControl (short resID, WindowRef window, ControlHandle *control)
{
	//
	//	Since static text controls do not contain data describing their contents
	//	or style, this function assumes the calling programmer has created (or chosen
	//	not to) text and style resources with the same ID as the control resource.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!(err = Get1NewControl (resID,window,control)))
	{
		err = SetControlTextFromResource (*control,resID);

		if (err == resNotFound)
			err = noErr;

		if (!err)
		{
			err = SetControlTextStyleFromResource (*control,resID);

			if (err == resNotFound)
				err = noErr;
		}

		if (err)
		{
			DisposeControl (*control);
			*control = NULL;
		}
	}

	return err;
}

pascal OSStatus Get1NewPopupButtonControl (short resID, WindowRef window, ControlHandle *control)
{
	//
	//	Some versions of Appearance (as of this writing, 1.0.2 and earlier)
	//	have a bug in the popup menu button CDEF which causes a crash on
	//	GetBestControlRect if the menu ID specified at creation of the control
	//	does not exist (and is not -12345). Luckily, we can test for this
	//	condition before it's too late. If the CDEF gets fixed, it's unlikely
	//	that the fix will involve setting contrlData to NIL.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!(err = Get1NewControl (resID,window,control)))
	{
		if (!MoreAssertControlHasDefProcResID (*control,kControlPopupButtonDefProcResID))
			err = paramErr;
		else if (!MoreAssertPCG (GetControlDataHandle (*control))) // will fire if non-existent menu ID specified
			err = paramErr;

		if (err)
			DisposeControl (*control);
	}

	return err;
}

pascal OSStatus SetUserPaneDrawProc (ControlHandle control, ControlUserPaneDrawUPP upp)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlUserPaneDefProcResID))
		err = paramErr;
	else
		err = SetControlData (control,kControlNoPart,kControlUserPaneDrawProcTag,sizeof(upp),(Ptr)&upp);

	return err;
}

pascal OSStatus SetUserPaneSetUpSpecialBackgroundProc (ControlHandle control, ControlUserPaneBackgroundUPP upp)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlUserPaneDefProcResID))
		err = paramErr;
	else
		err = SetControlData (control,kControlNoPart,kControlUserPaneBackgroundProcTag,sizeof(upp),(Ptr)&upp);

	return err;
}

pascal OSStatus SetUserPaneTrackingProc (ControlHandle control, ControlUserPaneTrackingUPP upp)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlUserPaneDefProcResID))
		err = paramErr;
	else
		err = SetControlData (control,kControlNoPart,kControlUserPaneTrackingProcTag,sizeof(upp),(Ptr)&upp);

	return err;
}

static pascal OSStatus GetEnclosingBounds (ControlHandle control, Rect *bounds)
{
	//
	//	Gets the bounds of the specified control's super-control.
	//	the the specified control has no super-control or the specified
	//	control's super control is the root control (whose bounds
	//	encompass all of QuickDraw, which is usually not useful), this
	//	function gets the bounds of the owning window.
	//

	OSStatus err = noErr;

	WindowRef controlOwner = GetControlOwner (control);

	if (!MoreAssertPCG (control && controlOwner))
		err = nilHandleErr;
	else if (!HaveAppearance ( ))
		GetWindowPortBounds (controlOwner,bounds);
	else
	{
		ControlHandle superControl;

		err = GetSuperControl (control,&superControl);

		if (err == errNoRootControl)
		{
			GetControlBounds (control,bounds);
			err = noErr;
		}
		else if (!err)
		{
			ControlHandle rootControl;

			if (!(err = GetRootControl (controlOwner, &rootControl)))
			{
				if (rootControl == superControl)
					GetWindowPortBounds (controlOwner,bounds);
				else
					GetControlBounds (superControl,bounds);
			}
		}
	}

	return err;
}

pascal OSStatus ExpandControl (ControlHandle control)
{
	//
	//	Expands the bounds of the specified control to fill its enclosing rectangle.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (control))
		err = nilHandleErr;
	else
	{
		Rect bounds;

		if (!(err = GetEnclosingBounds (control,&bounds)))
		{
			MoveControl (control, bounds.left, bounds.top);
			SizeControl (control, (short) (bounds.right - bounds.left), (short) (bounds.bottom - bounds.top));
		}
	}

	return err;
}

pascal OSStatus InsetControl (ControlHandle control, SInt16 byHowMuchH, SInt16 byHowMuchV)
{
	//
	//	Insets a control just like InsetRect insets a Rect.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (control))
		err = nilHandleErr;
	else
	{
		Rect contrlRect;
		GetControlBounds (control,&contrlRect);
		InsetRect (&contrlRect, byHowMuchH, byHowMuchV);
		MoveControl (control, contrlRect.left, contrlRect.top);
		SizeControl (control, (short) (contrlRect.right - contrlRect.left), (short) (contrlRect.bottom - contrlRect.top));
	}

	return err;
}

pascal OSStatus OffsetControl (ControlHandle control, SInt16 h, SInt16 v)
{
	//
	//	Offsets a control just like OffsetRect offsets a Rect.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (control))
		err = nilHandleErr;
	else
	{
		Rect contrlRect;
		GetControlBounds (control,&contrlRect);
		OffsetRect (&contrlRect,h,v);
		MoveControl (control,contrlRect.left,contrlRect.top);
	}

	return err;
}

pascal OSStatus MoveControlNear (ControlHandle moveMe, ControlHandle near, MoreControls_tDirection dir)
{
	//
	//	Moves one control near another. If the controls both have radio behavior,
	//	the distance between them is 0. If at least one of the controls does not
	//	have radio behavior, the distance between the two controls is non-zero
	//	(but not large; see definition of MoreControls_kControlGap).
	//
	//	To understand the relative positioning, see the comments in MoreControls_tDirection.
	//
	//	Right now this function only supports north, south, east, and west.
	//	I can't decide what it means to move a control to the southwest of another.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (moveMe && near))
		err = nilHandleErr;
	else if (!MoreAssertPCG (GetControlOwner (moveMe) == GetControlOwner (near)))
		err = paramErr;
	else if (!MoreAssertPCG (dir == MoreControls_kDirectionNorth || dir == MoreControls_kDirectionSouth ||
		dir == MoreControls_kDirectionEast || dir == MoreControls_kDirectionWest))
	{
		err = paramErr;
	}
	else
	{
		RgnHandle	moveMeRgn	= NULL,
					nearRgn		= NULL;

		if (!(err = MoreGetControlRegion (moveMe,&moveMeRgn)))
		if (!(err = MoreGetControlRegion (near,&nearRgn)))
		{
			Rect	moveMeRect, nearRect;

			GetRegionBounds (moveMeRgn,&moveMeRect);
			GetRegionBounds (nearRgn,&nearRect);

			SInt16	moveMeWidth		= (short) (moveMeRect.right - moveMeRect.left),
					moveMeHeight	= (short) (moveMeRect.bottom - moveMeRect.top),
					nearWidth		= (short) (nearRect.right - nearRect.left),
					nearHeight		= (short) (nearRect.bottom - nearRect.top);

			switch (dir)
			{
				case MoreControls_kDirectionNorth	:
				case MoreControls_kDirectionSouth	:

					moveMeRect.left		= (short) (nearRect.left + (nearWidth / 2) - (moveMeWidth / 2));
					moveMeRect.right	= (short) (moveMeRect.left + moveMeWidth);

					break;

				case MoreControls_kDirectionEast	:
				case MoreControls_kDirectionWest	:

					moveMeRect.top		= (short) (nearRect.top + (nearHeight / 2) - (moveMeHeight / 2));
					moveMeRect.bottom	= (short) (moveMeRect.top + moveMeHeight);

					break;
			}

			switch (dir)
			{
				case MoreControls_kDirectionWest	:

					moveMeRect.right	= (short) (nearRect.left - MoreControls_kControlGap);
					moveMeRect.left		= (short) (moveMeRect.right - moveMeWidth);
					break;

				case MoreControls_kDirectionEast	:

					moveMeRect.left		= (short) (nearRect.right + MoreControls_kControlGap);
					moveMeRect.right	= (short) (moveMeRect.left + moveMeWidth);
					break;

				case MoreControls_kDirectionNorth	:

					moveMeRect.bottom	= (short) (nearRect.top - MoreControls_kControlGap);
					moveMeRect.top		= (short) (moveMeRect.bottom - moveMeHeight);

					break;

				case MoreControls_kDirectionSouth	:

					moveMeRect.top		= (short) (nearRect.bottom + MoreControls_kControlGap);
					moveMeRect.bottom	= (short) (moveMeRect.top + moveMeHeight);

					break;
			}

			MoveControl (moveMe, moveMeRect.left, moveMeRect.top); // finally, the pay-off!
		}

		if (moveMeRgn)
		{
			DisposeRgn (moveMeRgn);
			assert(noErr == QDError ( ));
		}

		if (nearRgn)
		{
			DisposeRgn (nearRgn);
			assert(noErr == QDError ( ));
		}
	}

	return err;
}

pascal OSStatus SetControlQuadrant (ControlHandle control, MoreControls_tDirection quadrant)
{
	//
	//	Moves a control to one of nine quadrants within its enclosing rectangle.
	//	To understand the positioning, see the comments in MoreControls_tDirection.
	//

	OSStatus err = noErr;

    // Previously this used to be:
    //
    // if (!(MoreAssertPCG (quadrant >= MoreControls_kDirectionNorthWest && quadrant <= MoreControls_kDirectionSouthEast)))
    //
    // about which gcc complains that "the comparison is always true due to 
    // the limited range of the data type".  This is usually a good warning,
    // but in this case the code was correct.  However, because I don’t want 
    // to turn the warning off I just changed the code to the less neat but 
    // still helpful asserts shown below.
    // -- Quinn, 8 Feb 2001

    assert(MoreControls_kDirectionNorthWest == 0);
	if (!(MoreAssertPCG (quadrant <= MoreControls_kDirectionSouthEast)))
		err = paramErr;
	else
	{
		Rect enclosingBounds;

		if (!(err = GetEnclosingBounds (control,&enclosingBounds)))
		{
			Rect	controlRect;

			GetControlBounds (control,&controlRect);

			SInt16	controlWidth		= (short) (controlRect.right - controlRect.left),
					controlHeight		= (short) (controlRect.bottom - controlRect.top),
					enclosingWidth		= (short) (enclosingBounds.right - enclosingBounds.left),
					enclosingHeight		= (short) (enclosingBounds.bottom - enclosingBounds.top);

			switch (quadrant)
			{
				case MoreControls_kDirectionNorthWest	:
				case MoreControls_kDirectionNorth		:
				case MoreControls_kDirectionNorthEast	:

					controlRect.top		= enclosingBounds.top;
					controlRect.bottom	= (short) (enclosingBounds.top + controlHeight);
					break;

				case MoreControls_kDirectionSouthWest	:
				case MoreControls_kDirectionSouth		:
				case MoreControls_kDirectionSouthEast	:

					controlRect.bottom	= enclosingBounds.bottom;
					controlRect.top		= (short) (enclosingBounds.bottom - controlHeight);
					break;

				case MoreControls_kDirectionWest		:
				case MoreControls_kDirectionCenter		:
				case MoreControls_kDirectionEast		:

					controlRect.top		= (short) (enclosingBounds.top + (enclosingHeight / 2) - (controlHeight / 2));
					controlRect.bottom	= (short) (controlRect.top + controlHeight);
					break;
			}

			switch (quadrant)
			{
				case MoreControls_kDirectionNorthWest	:
				case MoreControls_kDirectionWest		:
				case MoreControls_kDirectionSouthWest	:

					controlRect.left	= enclosingBounds.left;
					controlRect.right	= (short) (enclosingBounds.left + controlWidth);
					break;

				case MoreControls_kDirectionNorthEast	:
				case MoreControls_kDirectionEast		:
				case MoreControls_kDirectionSouthEast	:

					controlRect.right	= enclosingBounds.right;
					controlRect.left	= (short) (enclosingBounds.right - controlWidth);
					break;

				case MoreControls_kDirectionNorth		:
				case MoreControls_kDirectionCenter		:
				case MoreControls_kDirectionSouth		:

					controlRect.left	= (short) (enclosingBounds.left + (enclosingWidth / 2) - (controlWidth / 2));
					controlRect.right	= (short) (controlRect.left + controlWidth);
					break;
			}

			MoveControl (control, controlRect.left, controlRect.top);
		}
	}

	return err;
}

pascal OSStatus SetBestControlRect (ControlHandle control, SInt16 *baseLineOffsetP)
{
	//
	//	Takes GetBestControlRect to its logical conclusion and moves and resizes
	//	the specified control to fit its best control bounds rectangle, if any.
	//	You can pass NIL for baseLineOffsetP if you don't care to know the text
	//	baseline for the control. This function should probably produce a value
	//	denoting whether the control moved or changed size.
	//

	OSStatus err = noErr;

	// it's OK for baseLineOffsetP to be NIL

	if (!MoreAssertPCG (control))
		err = nilHandleErr;
	else if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else
	{
		UInt32 features;
		err = GetControlFeatures (control, &features);

		if (!err && (kControlSupportsCalcBestRect & features))
		{
			Rect	best = { 0,0,0,0 };
			SInt16	dontCare;

			if (!baseLineOffsetP)
				baseLineOffsetP = &dontCare;

			if (!(err = GetBestControlRect (control,&best,baseLineOffsetP)))
			{
				SizeControl (control, (short) (best.right - best.left), (short) (best.bottom - best.top) );
				MoveControl (control, best.left, best.top);
			}
		}
	}

	return err;
}

pascal OSStatus GetListBoxControlListHandle (ControlHandle control, ListHandle *result)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlListBoxDefProcResID))
		err = paramErr;
	else
	{
		Size actualListHandleSize;
		err = GetControlData (control, kControlNoPart, kControlListBoxListHandleTag,
			sizeof (*result), (Ptr) result, &actualListHandleSize);
		if (!err) assert(actualListHandleSize == sizeof (*result));
	}

	return err;
}

pascal OSStatus GetStaticTextControlTextHeight (ControlHandle control, SInt16 *textHeight)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlStaticTextDefProcResID))
		err = paramErr;
	else
	{
		Size actualTextHeightSize;
		err = GetControlData (control, kControlNoPart, kControlStaticTextTextHeightTag,
			sizeof (*textHeight), (Ptr) textHeight, &actualTextHeightSize);
		if (!err) assert(actualTextHeightSize == sizeof (*textHeight));
	}

	return err;
}

pascal OSStatus SetBevelButtonMenu (ControlHandle control, MenuRef menu)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlBevelButtonDefProcResID))
		err = paramErr;
	else
	{
		//
		//	There appears to be a bug in the bevel button CDEF which prevents you
		//	from setting a menu if the control was not created with a menu, so to
		//	avoid this problem, we assert so you can tell when you are likely to
		//	run afoul of this problem. I can't remember whether I filed a bug
		//	against this misbehavior. I should check.
		//

#if MORE_DEBUG

		MenuHandle	existingMenuH;
		Size		actualSize;

		err = GetControlData (control, kControlNoPart, kControlBevelButtonMenuHandleTag,
			sizeof(existingMenuH), Ptr (&existingMenuH), &actualSize);
		if (err) return err;
		if (!MoreAssertPCG (actualSize == sizeof(existingMenuH)))
			err = paramErr;
		else if (!MoreAssertPCG (existingMenuH))
			err = nilHandleErr;
		else

#endif

		err = SetControlData (control, kControlNoPart, kControlBevelButtonMenuHandleTag,
			sizeof(MenuRef), Ptr (&menu));
	}

	return err;
}

pascal OSStatus SetPopUpButtonMenu (ControlHandle control, MenuRef menu)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	assert(HaveAppearance ( ));
	assert(GetAppearanceVersion ( ) >= 0x0101);
	MoreAssertControlHasDefProcResID (control,kControlPopupButtonDefProcResID);

	OSStatus err = SetControlData (control, kControlNoPart, kControlPopupButtonMenuHandleTag,
		sizeof(MenuRef), Ptr (&menu));
	return err;
}

pascal OSStatus SetBevelButtonMenuDelay (ControlHandle control, SInt32 delay)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertPCG (GetAppearanceVersion ( ) >= 0x0101))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlBevelButtonDefProcResID))
		err = paramErr;
	else
		err = SetControlData (control, kControlNoPart, kControlBevelButtonMenuDelayTag, sizeof (delay), Ptr (&delay));

	return err;
}

pascal OSStatus SetProgressBarIndeterminate (ControlHandle control, Boolean i)
{
	//
	//	This is just convenience glue to avoid stringing a bunch of long
	//	constant names together all the time.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (!MoreAssertControlHasDefProcResID (control,kControlProgressBarDefProcResID))
		err = paramErr;
	else if (!MoreAssertPCG (i == true || i == false)) // talk about paranoid!
		err = paramErr;
	else
		err = SetControlData (control, kControlNoPart, kControlProgressBarIndeterminateTag, sizeof (i), Ptr (&i));

	return err;
}

pascal OSStatus ToggleControl (ControlHandle control)
{
	//
	//	Given a control whose limits suggest it toggles between 0 and 1
	//	(like a check box or a radio button), this function sets the control
	//	value to the opposite of its present value.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (control))
		err = nilHandleErr;
	else if (!MoreAssertPCG (GetControlMaximum (control) == 1))
		err = paramErr;
	else if (!MoreAssertPCG (GetControlMinimum (control) == 0))
		err = paramErr;
	else
		SetControlValue (control, GetControlValue (control) ? (short) 0 : (short) 1);

	return err;
}

static pascal OSStatus EmbedSubControls (ControlHandle source, ControlHandle destination, SInt16 index)
{
	//
	//	This is a helper function for EncloseSubControls; if you find a use
	//	for it elsewhere, keep in mind that the controls are embedded into
	//	the destination control in the opposite order from that in which
	//	they appear in the source control. This is fine as long as you
	//	copy the controls back with EmbedSubControls. This is just what
	//	EncloseSubControls needed. Why do we do things this way? We want
	//	to be recursive so we can easily back out of a partial embedding
	//	transfer, and this is the easiest way to do it.
	//

	OSStatus err = noErr;

	if (index)
	{
		ControlHandle subControl;

		if (!(err = GetIndexedSubControl (source,(UInt16) index,&subControl)))
		if (!(err = EmbedControl (subControl,destination)))
		{
			err = EmbedSubControls (source,destination, (short) (index-1) );
			if (err) (void) EmbedControl (subControl,source);
		}
	}

	return err;
}

pascal OSStatus EncloseSubControls (ControlHandle parent)
{
	//
	//	Moves and resizes the parent control to enclose all its sub-controls.
	//	However, we can't just move the parent control, because the
	//	sub-controls will move along with it, which is not what we want.
	//	So we create a temporary embeddable control and move the sub-controls
	//	to this temporary control while we move the parent control.
	//	Note that EmbedSubControls reverses the order of the sub-controls,
	//	which is fine for our purposes, since we call it twice.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (parent))
		err = nilHandleErr;
	else
	{

		SInt16 subControlCount;

		if (!(err = CountSubControls (parent,(UInt16 *)&subControlCount)))
		{
			if (!MoreAssertPCG (subControlCount > 0))
				err = paramErr;
			else
			{
				static Rect tempUserPaneBounds;
				WindowPtr controlOwner = GetControlOwner (parent);

				ControlHandle tempUserPane = NewControl (controlOwner, &tempUserPaneBounds, "\p", false,
					kControlSupportsEmbedding, 0, 0, kControlUserPaneProc, 0);

				if (!tempUserPane)
					err = nilHandleErr;
				else
				{
					SInt16 subControlIndex = subControlCount;
					ControlHandle subControl;

					if (!(err = GetIndexedSubControl (parent,(UInt16)subControlIndex,&subControl)))
					{
						if (!MoreAssertPCG (subControl))
							err = nilHandleErr;
						else
						{
							Rect parentRect;

							GetControlBounds (subControl,&parentRect);

							while (--subControlIndex)
							{
								err = GetIndexedSubControl (parent,(UInt16)subControlIndex,&subControl);
								if (err) break;

								if (!MoreAssertPCG (subControl))
								{
									err = nilHandleErr;
									break;
								}

								Rect contrlRect;

								GetControlBounds (subControl,&contrlRect);

								if (contrlRect.bottom > parentRect.bottom)
									parentRect.bottom = contrlRect.bottom;
								if (contrlRect.right > parentRect.right)
									parentRect.right = contrlRect.right;
								if (contrlRect.left < parentRect.left)
									parentRect.left = contrlRect.left;
								if (contrlRect.top < parentRect.top)
									parentRect.top = contrlRect.top;
							}

							if (!err && !(err = EmbedSubControls (parent,tempUserPane,subControlCount)))
							{
								InsetRect (&parentRect, -(MoreControls_kControlGap), -(MoreControls_kControlGap));
								MoveControl (parent, parentRect.left, parentRect.top);
								SizeControl (parent, (short) (parentRect.right - parentRect.left), (short) (parentRect.bottom - parentRect.top));
								OSStatus err2 = EmbedSubControls (tempUserPane,parent,subControlCount);
								if (!err) err2 = err;
							}
						}
					}
					DisposeControl (tempUserPane);
				}
			}
		}
	}
	return err;
}

pascal OSStatus AlignControl (ControlHandle anchor, ControlHandle moveMe, MoreControls_tAlignment align)
{
	//
	//	Aligns one control (moveMe) to an edge of another (anchor).
	//	For an explanation of MoreControls_tAlignment, see the header.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (moveMe && anchor))
		err = nilHandleErr;
	else if (!MoreAssertPCG (align == MoreControls_kAlignSystem || align == MoreControls_kAlignLeft ||
		align == MoreControls_kAlignCenter || align == MoreControls_kAlignRight))
	{
		err = paramErr;
	}
	else
	{
		align = FilterAlignment (align);

		if (!MoreAssertPCG (align == MoreControls_kAlignLeft ||
			align == MoreControls_kAlignCenter || align == MoreControls_kAlignRight))
		{
			err = paramErr;
		}
		else
		{
			Rect moveRect, anchorRect;

			GetControlBounds (moveMe,&moveRect);
			GetControlBounds (anchor,&anchorRect);

			short horiz, vert = moveRect.top;

			switch (align)
			{
				case MoreControls_kAlignLeft	:

					horiz = anchorRect.left;
					break;

				case MoreControls_kAlignCenter	:

					assert(false && MoreControls_kAlignCenter); // untraced logic
					break;

				case MoreControls_kAlignRight	:

					assert(false && MoreControls_kAlignRight); // untraced logic
					horiz = (short) (anchorRect.right - (moveRect.right - moveRect.left));
					break;
			}

			MoveControl (moveMe, horiz, vert);
		}
	}

	return err;
}

pascal OSStatus AlignSubControls (ControlHandle parent, MoreControls_tAlignment align)
{
	//
	//	Given a parent control, aligns all its sub-controls along the most
	//	extreme edge. So if left alignment is specified, this function finds
	//	the sub-control which is leftmost and aligns the rest of the sub-controls
	//	to its left edge. For an explanation of MoreControls_tAlignment, see the header.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (parent))
		err = nilHandleErr;
	else if (!MoreAssertPCG (align == MoreControls_kAlignSystem || align == MoreControls_kAlignLeft ||
		align == MoreControls_kAlignCenter || align == MoreControls_kAlignRight))
	{
		err = paramErr;
	}
	else
	{
		align = FilterAlignment (align);

		if (!MoreAssertPCG (align == MoreControls_kAlignLeft ||
			align == MoreControls_kAlignCenter || align == MoreControls_kAlignRight))
		{
			err = paramErr;
		}
		else
		{

			UInt16 subControlCount;

			if (!(err = CountSubControls (parent,&subControlCount)))
			{
				if (!MoreAssertPCG (subControlCount > 0))
					err = paramErr;
				else if (subControlCount == 1)
				{
					assert(false && subControlCount == 1); // untraced logic
				}
				else
				{
					UInt16 subControlIndex = subControlCount;
					ControlHandle subControl;

					if (!(err = GetIndexedSubControl (parent,subControlIndex,&subControl)))
					{
						if (!MoreAssertPCG (subControl))
							err = nilHandleErr;
						else
						{
							ControlHandle anchor = subControl;

							while (--subControlIndex)
							{
								err = GetIndexedSubControl (parent,subControlIndex,&subControl);
								if (err) break;

								Rect subRect, anchorRect;

								GetControlBounds (subControl,&subRect);
								GetControlBounds (anchor,&anchorRect);

								switch (align)
								{
									case MoreControls_kAlignLeft	:

										if (subRect.left < anchorRect.left)
											anchor = subControl;
										break;

									case MoreControls_kAlignCenter	:

										assert(false && MoreControls_kAlignCenter); // untraced logic
										break;

									case MoreControls_kAlignRight	:

										if (subRect.right > anchorRect.right)
											anchor = subControl;
										break;
								}
							}

							if (!err)
							{
								subControlIndex = subControlCount;

								do
								{
									err = GetIndexedSubControl (parent,subControlIndex,&subControl);
									if (err) break;

									Rect subRect, anchorRect;

									GetControlBounds (subControl,&subRect);
									GetControlBounds (anchor,&anchorRect);

									if (subControl != anchor)
									{
										short horiz, vert = subRect.top;

										switch (align)
										{
											case MoreControls_kAlignLeft	:

												horiz = anchorRect.left;
												break;

											case MoreControls_kAlignCenter	:

												assert(false && MoreControls_kAlignCenter); // untraced logic
												break;

											case MoreControls_kAlignRight	:

												horiz = (short) (anchorRect.right - (subRect.right - subRect.left));
												break;
										}

										MoveControl (subControl,horiz,vert);
									}
								}
								while (--subControlIndex);
							}
						}
					}
				}
			}
		}
	}

	return err;
}

pascal OSStatus IdleControlsInAllWindows (void)
{
	//
	//	For each visible window in the window list, this function gives
	//	the controls a chance to idle.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else if (WindowPtr window = FrontWindow ( ))
	{
		IdleControls (window);

		window = GetNextWindow (window);

		while (window)
		{
			if (IsWindowVisible (window))
				IdleControls (window);

			window = GetNextWindow (window);
		}
	}

	return err;
}

pascal OSStatus SetCursorAccordingToControl (void)
{
	//
	//	Sets the cursor according to which control the cursor
	//	is floating over. In a perfect world, we'd be able to
	//	ask the control to set the cursor for us, but in the
	//	real world we just guess.
	//

	OSStatus err = noErr;

	WindowPtr window = FrontWindow ( );

	if (!window)
		SetArrowCursor ( );
	else
	{
		GrafPtr preservedPort;

		GetPort (&preservedPort);
		SetPortWindowPort (window);

		Point mouse;
		GetMouse (&mouse);

		ControlPartCode		cpc;
		ControlHandle		whichControl;

		err = MoreFindControlUnderMouse (mouse, window, &cpc, &whichControl);

		if (!err)
		{
			if (!whichControl || cpc == kControlNoPart || !HaveAppearance ( ))
				SetArrowCursor ( );
			else
			{
				short resID;

				if (!(err = GetControlDefProcResID (whichControl, &resID)))
				{
					if (resID == kControlEditTextDefProcResID)
						SetCursor (*GetCursor (iBeamCursor));
					else
						SetArrowCursor ( );
				}
			}
		}

		SetPort (preservedPort);
	}

	return err;
}

pascal OSStatus IsScrollBar (ControlHandle ch, Boolean *isScrollBar)
{
	//
	//	Tests whether a given control is a scroll bar.
	//	Replaces bogus code from AppsToGo. TO DO: find
	//	a better abstraction for this. Waiting for
	//	developer feedback. This function may go away.
	//

	OSStatus err = noErr;

	if (!MoreAssertPCG (ch && isScrollBar))
		err = nilHandleErr;
	else
	{
		short resID;

		if (!(err = GetControlDefProcResID (ch, &resID)))
		{
			*isScrollBar = (resID == kControlOldScrollBarDefProcResID || resID == kControlScrollBarDefProcResID);
		}
	}

	return err;
}

pascal OSStatus ActivateControls (WindowPtr w)
{
	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else
	{
		ControlHandle root;

		if (!(err = GetRootControl (w, &root)))
		{
			err = ActivateControl (root);
		}
	}

	return err;
}

pascal OSStatus DeactivateControls (WindowPtr w)
{
	OSStatus err = noErr;

	if (!MoreAssertPCG (HaveAppearance ( )))
		err = paramErr;
	else
	{
		ControlHandle root;

		if (!(err = GetRootControl (w, &root)))
		{
			err = DeactivateControl (root);
		}
	}

	return err;
}

extern OSStatus GetControlByIDQ(WindowRef inWindow, OSType signature, SInt32 id, ControlRef *outControl)
{
	ControlID theID;

	theID.signature = signature;
	theID.id = id;	
	return GetControlByID(inWindow, &theID, outControl);
}

#if TARGET_API_MAC_CARBON

static OSType kMainTags[2]   = { kControlEditTextCFStringTag, kControlEditTextPasswordCFStringTag };
static OSType kCompatTags[2] = { kControlEditTextTextTag,     kControlEditTextPasswordTag         };

extern pascal OSStatus CopyTextControlTextCompat(ControlRef control, Boolean isPassword, CFStringRef *textStr)
{
	OSStatus err;
	
	assert(control  != NULL);
	assert( textStr != NULL);
	assert(*textStr == NULL);
	
	err = GetControlData(control, kControlEntireControl, kMainTags[isPassword], sizeof(*textStr), textStr, NULL);
	if (err != noErr) {
		Size dataSize;
		char *dataBuffer;
		
		dataBuffer = NULL;
		
		err = GetControlData(control, kControlEntireControl, kCompatTags[isPassword], 0, NULL, &dataSize);
		if (err == noErr) {
			dataBuffer = NewPtr(dataSize);
			err = MoreMemError(dataBuffer);
		}
		if (err == noErr) {
			err = GetControlData(control, kControlEntireControl, kCompatTags[isPassword], dataSize, dataBuffer, &dataSize);
		}
		if (err == noErr) {
			*textStr = CFStringCreateWithBytes(NULL, (UInt8 *) dataBuffer, dataSize, GetApplicationTextEncoding(), false);
			err = CFQError(*textStr);
		}
		
		if (dataBuffer != NULL) {
			DisposePtr(dataBuffer);
			assert(MemError() == noErr);
		}
	}
	
	assert( (err == noErr) == (*textStr != NULL) );
	return err;
}

static Boolean GetControlEncoding(ControlRef inControl, CFStringEncoding* outEncoding)
	// Get the text encoding used by the control.
{
	OSStatus				junk;
	Boolean					gotFont;
	ControlFontStyleRec		fontStyle;
	short					fID;
	long					oldFontForce;
	
	gotFont = false;
	
	// First see if we have a font ID.
	
	if ( GetControlData( inControl, kControlEntireControl, kControlFontStyleTag, 
						 sizeof( fontStyle ), &fontStyle, NULL ) == noErr ) {
		if ( (fontStyle.flags & kControlUseFontMask) != 0 ) {
			fID = fontStyle.font;
			gotFont = true;
		}
	}
	
	// If not, see whether we're using the window's font.
	
	if ( !gotFont && ( GetControlVariant(inControl) & kControlUsesOwningWindowsFontVariant ) != 0 ) {
		fID = GetPortTextFont(GetWindowPort(GetControlOwner(inControl)));
		gotFont = true;
	}
	
	// If we managed to get a font ID, map that to its script system 
	// using FontToScript.  Note that we have to disable smFontForce 
	// to get accurate results (otherwise FontToScript might base its 
	// response on the current port!).
	
	if (gotFont) {
		oldFontForce = GetScriptManagerVariable(smFontForce);
		junk = SetScriptManagerVariable(smFontForce, 0);
		assert(junk == noErr);

		*outEncoding = FontToScript(fID);

		junk = SetScriptManagerVariable(smFontForce, oldFontForce);
		assert(junk == noErr);
	}
	
	return gotFont;
}

static OSStatus ConvertControlCFStringToPascalString(ControlRef inControl, CFStringRef inText, StringPtr outString)
	// Converts a CFString to a Pascal string in the context of the control.
{
	OSStatus			err;
	CFStringEncoding	encoding;
	ByteCount			textLength;
	
	assert(inControl != NULL);
	assert(inText    != NULL);
	assert(outString != NULL);
	
	err = noErr;
	
	// First, check if the control has a font, or if it's useWFont, and try to extract 
	// the title using that encoding.
	
	outString[0] = 0;
	if ( GetControlEncoding(inControl, &encoding) ) {
		CFStringGetPascalString(inText, outString, sizeof(Str255), encoding);
	}
	
	// If the control is just drawn with the system font, or we failed to extract the title
	// using the control's font encoding, then fall back to CFStringGetBestEncodingAndString.

	if ( outString[0] == 0 ) {
		err = GetTextAndEncodingFromCFString(inText, &outString[1], 255, &textLength, &encoding);
		if (err == noErr || err == kTECOutputBufferFullStatus) {
			outString[0] = (UInt8) textLength;
		}
	}
	
	return err;
}

extern pascal OSStatus SetTextControlTextCompat(ControlRef control, Boolean isPassword, CFStringRef textStr)
{
	OSStatus err;
	assert(control  != NULL);
	assert( textStr != NULL);
	
	err = SetControlData(control, kControlEntireControl, kMainTags[isPassword], sizeof(textStr), &textStr);
	if ( err != noErr ) {
		Str255 tmpStr;
		
		// Do it the hard way.  This compatibility code was largely stolen from the 
		// code in CarbonLib.
		
		err = ConvertControlCFStringToPascalString(control, textStr, tmpStr);
		if (err == noErr) {
			err = SetControlData(control, kControlEntireControl, kCompatTags[isPassword], tmpStr[0], &tmpStr[1]);
		}
	}
	
	return err;
}

#endif

extern pascal OSStatus InstallControlKeyFilter(ControlRef control, OSType myTag, ControlKeyFilterUPP newFilter)
{
	OSStatus 			err;
	ControlKeyFilterUPP	oldFilter;
	
	err = GetControlData(control, kControlEntireControl, kControlKeyFilterTag, sizeof(oldFilter), &oldFilter, NULL);
	if (err == noErr) {
		err = SetControlProperty(control, 'MIB!', myTag, sizeof(oldFilter), &oldFilter);
	}
	if (err == noErr) {
		err = SetControlData(control, kControlEntireControl, kControlKeyFilterTag, sizeof(newFilter), &newFilter);
	}
	return err;
}

extern pascal ControlKeyFilterResult CallNextControlKeyFilter(ControlRef control, OSType myTag, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers)
{
	ControlKeyFilterResult	result;
	ControlKeyFilterUPP		oldFilter;
	
	result = kControlKeyFilterPassKey;
	
	if ( GetControlProperty(control, 'MIB!', myTag, sizeof(oldFilter), NULL, &oldFilter) != noErr ) {
		oldFilter = NULL;
	}
	if (oldFilter != NULL) {
		result = InvokeControlKeyFilterUPP(control, keyCode, charCode, modifiers, oldFilter);
	}
	return result;
}

static ControlKeyFilterUPP gControlNoReturnsKeyFilterUPP;		// -> NoReturnsControlKeyFilter

extern pascal ControlKeyFilterUPP GetControlNoReturnsKeyFilterUPP(void)
{
	if (gControlNoReturnsKeyFilterUPP == NULL) {
		gControlNoReturnsKeyFilterUPP = NewControlKeyFilterUPP(ControlNoReturnsKeyFilter);
	}
	return gControlNoReturnsKeyFilterUPP;
}

extern pascal ControlKeyFilterResult ControlNoReturnsKeyFilter(ControlRef theControl, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers)
{
	ControlKeyFilterResult result;
	
	result = kControlKeyFilterPassKey;
	if (*charCode == kReturnCharCode) {
		result = kControlKeyFilterBlockKey;
	}
	if (result == kControlKeyFilterPassKey) {
		result = CallNextControlKeyFilter(theControl, kMIBControlNoReturnsKeyFilterKeyFilterTag, keyCode, charCode, modifiers);
	}
	return result;
}

#if TARGET_API_MAC_CARBON

#if UNIVERSAL_INTERFACES_VERSION < 0x0400

	enum {

	  /*
	   * This event parameter may be added to any event that is posted to
	   * the main event queue. When the event is removed from the queue and
	   * sent to the event dispatcher, the dispatcher will retrieve the
	   * EventTargetRef contained in this parameter and send the event
	   * directly to that event target. If this parameter is not available
	   * in the event, the dispatcher will send the event to a suitable
	   * target, or to the application target if no more specific target is
	   * appropriate. Available in CarbonLib 1.3.1 and later, and Mac OS X.
	   */
	  kEventParamPostTarget         = 'ptrg', /* typeEventTargetRef*/

	  /*
	   * Indicates an event parameter of type EventTargetRef.
	   */
	  typeEventTargetRef            = 'etrg' /* EventTargetRef*/
	};

#endif

static OSStatus PostControlEditTextModifiedEvent(ControlRef control)
{
	OSStatus		err;
	EventTargetRef 	target;
	EventRef		event;

	assert(control != NULL);

	event = NULL;
	
	target = GetControlEventTarget(control);
	assert(target != NULL);
	
	err = MacCreateEvent(NULL, kMoreIsBetterEventClass, kMIBControlEditTextModifiedKind, GetCurrentEventTime(), kEventAttributeNone, &event);
	if (err == noErr) {
		err = SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(control), &control);
	}
	if (err == noErr) {
		err = SetEventParameter(event, kEventParamPostTarget, typeEventTargetRef, sizeof(target), &target);
	}
	if (err == noErr) {
		err = PostEventToQueue(GetCurrentEventQueue(), event, kEventPriorityStandard);
	}
	
	if (event != NULL) {
		ReleaseEvent(event);
	}

	return err;
}

static ControlKeyFilterUPP gControlEditTextModifiedKeyFilterUPP;	// -> ControlEditTextModifiedKeyFilter

extern pascal ControlKeyFilterUPP GetControlEditTextModifiedKeyFilterUPP(void)
{
	if (gControlEditTextModifiedKeyFilterUPP == NULL) {
		gControlEditTextModifiedKeyFilterUPP = NewControlKeyFilterUPP(ControlEditTextModifiedKeyFilter);
	}
	return gControlEditTextModifiedKeyFilterUPP;
}

extern pascal ControlKeyFilterResult ControlEditTextModifiedKeyFilter(ControlRef theControl, SInt16 *keyCode, SInt16 *charCode, EventModifiers *modifiers)
{
	OSStatus 				junk;
	ControlKeyFilterResult 	result;

	result = CallNextControlKeyFilter(theControl, kMIBControlEditTextModifiedKeyFilterTag, keyCode, charCode, modifiers);
	if (result == kControlKeyFilterPassKey) {
		junk = PostControlEditTextModifiedEvent(theControl);
		assert(junk == noErr);
	}
	return result;
}

static ControlEditTextValidationUPP gControlEditTextModifiedValidationProcUPP;	// -> ControlEditTextModifiedValidationProc

extern pascal ControlEditTextValidationUPP GetControlEditTextModifiedValidationProcUPP(void)
{
	if (gControlEditTextModifiedValidationProcUPP == NULL) {
		gControlEditTextModifiedValidationProcUPP = NewControlEditTextValidationUPP(ControlEditTextModifiedValidationProc);
	}
	return gControlEditTextModifiedValidationProcUPP;
}

extern pascal void ControlEditTextModifiedValidationProc(ControlRef control)
{
	OSStatus junk;
	
	junk = PostControlEditTextModifiedEvent(control);
	assert(junk == noErr);
}

#endif

extern pascal void SetControlActive(ControlRef control, Boolean active)
{
	assert(control != NULL);
	
	if (active) {
		ActivateControl(control);
	} else {
		DeactivateControl(control);
	}
}

extern pascal void SetControlVisible(ControlRef control, Boolean visible)
{
	assert(control != NULL);
	
	if (visible) {
		ShowControl(control);
	} else {
		HideControl(control);
	}
}

