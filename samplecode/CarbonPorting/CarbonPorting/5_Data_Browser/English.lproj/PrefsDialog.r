/*

File: PrefsDialog.r

Abstract: Resources for the Preferences dialog

Version: 5.0

Change History:
	
	<5.0>	removed the list box 'CNTL'/'ldes' resources and dialog 
			items
			adjusted the dialog items and controls to account for 
			the 3 pixel padding around the data browser contents
			removed the control dialog item for the list box control
	<4.0>	conditionalized the resources to be included only where 
			needed
	<3.0>	no changes necessary
	<2.0>	added alternate 'DLOG'/'dlgx', 'DITL', and 'CNTL' 
			resources to support the Mac OS X Aqua appearance HIG
	<1.0>	first release version

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ExamplePrefs.h"
#include "Help.h"
#include "PrefsDialog.h"

#if !TARGET_API_MAC_OSX
#define rPrefsDITLPlatinum 128
#else
#define rPrefsDITLAqua 129
#endif


#if !TARGET_API_MAC_OSX
resource 'DLOG' (rPrefsDialogPlatinum, "Preferences dialog (Platinum)", purgeable)
{
	{100, 100, 600, 375},
	kWindowDocumentProc,
	invisible,
	noGoAway,
	0,
	rPrefsDITLPlatinum,
	"Preferences Dialog",
	staggerParentWindowScreen
};
resource 'dlgx' (rPrefsDialogPlatinum, "Preferences dialog extension (Platinum)", purgeable)
{
	VersionZero
	{
		kDialogFlagsUseThemeBackground | kDialogFlagsUseControlHierarchy | 
			kDialogFlagsHandleMovableModal | kDialogFlagsUseThemeControls
	}
};

#else
resource 'DLOG' (rPrefsDialogAqua, "Preferences dialog (Aqua)", purgeable)
{
	{100, 100, 616, 391},
	kWindowDocumentProc,
	invisible,
	noGoAway,
	0,
	rPrefsDITLAqua,
	"Preferences Dialog",
	staggerParentWindowScreen
};
resource 'dlgx' (rPrefsDialogAqua, "Preferences dialog extension (Aqua)", purgeable)
{
	VersionZero
	{
		kDialogFlagsUseThemeBackground | kDialogFlagsUseControlHierarchy | 
			kDialogFlagsHandleMovableModal | kDialogFlagsUseThemeControls
	}
};
#endif

#if !TARGET_API_MAC_OSX
resource 'DITL' (rPrefsDITLPlatinum, "Preferences dialog item list (Platinum)", purgeable)
{
	{
		{kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing - kPushButtonHeight, 
			kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing - kPlatinumPushButtonWidth, 
			kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing, 
			kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing},
		Button
		{
			enabled,
			"OK"
		},
		
		{kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing - kPushButtonHeight, 
			kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing - kPlatinumPushButtonWidth - 
			kPushButtonSpacing - kPlatinumPushButtonWidth, 
			kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing, 
			kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing - kPlatinumPushButtonWidth - 
			kPushButtonSpacing},
		Button
		{
			enabled,
			"Cancel"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneVisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 1"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 2"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 3"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 4"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 5"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 6"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 7"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 8"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 9"
		},
		
		{kUserPanePlatinumBounds},
		Control
		{
			disabled,
			cPlatinumDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 10"
		}
	}
};

#else
resource 'DITL' (rPrefsDITLAqua, "Preferences dialog item list (Aqua)", purgeable)
{
	{
		{kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing - kPushButtonHeight, 
			kPrefsDialogAquaWidth - kAquaWindowEdgeSpacing - kAquaPushButtonWidth, 
			kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing, 
			kPrefsDialogAquaWidth - kAquaWindowEdgeSpacing},
		Button
		{
			enabled,
			"OK"
		},
		
		{kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing - kPushButtonHeight, 
			kPrefsDialogAquaWidth - kAquaWindowEdgeSpacing - kAquaPushButtonWidth - 
			kPushButtonSpacing - kAquaPushButtonWidth, 
			kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing, 
			kPrefsDialogAquaWidth - kAquaWindowEdgeSpacing - kAquaPushButtonWidth - 
			kPushButtonSpacing},
		Button
		{
			enabled,
			"Cancel"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneVisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 1"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 2"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 3"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 4"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 5"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 6"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 7"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 8"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 9"
		},
		
		{kUserPaneAquaBounds},
		Control
		{
			disabled,
			cAquaDialogUserPaneInvisible
		},
		
		{kStaticTextBounds},
		StaticText
		{
			disabled,
			"Panel 10"
		}
	}
};
#endif

#if !TARGET_API_MAC_OSX
resource 'CNTL' (cPlatinumDialogUserPaneVisible, "Current prefs dialog pane (Platinum)", 
					purgeable)
{
	{kUserPanePlatinumBounds},
	kControlSupportsEmbedding,
	visible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#else
resource 'CNTL' (cAquaDialogUserPaneVisible, "Current prefs dialog pane (Aqua)", 
					purgeable)
{
	{kUserPaneAquaBounds},
	kControlSupportsEmbedding,
	visible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#endif
#if !TARGET_API_MAC_OSX
resource 'CNTL' (cPlatinumDialogUserPaneInvisible, "Noncurrent prefs dialog panes (Platinum)", 
					purgeable)
{
	{kUserPanePlatinumBounds},
	kControlSupportsEmbedding,
	invisible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#else
resource 'CNTL' (cAquaDialogUserPaneInvisible, "Noncurrent prefs dialog panes (Aqua)", 
					purgeable)
{
	{kUserPaneAquaBounds},
	kControlSupportsEmbedding,
	invisible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#endif