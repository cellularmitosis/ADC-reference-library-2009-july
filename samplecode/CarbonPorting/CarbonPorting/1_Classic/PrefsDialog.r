/*

File: PrefsDialog.r

Abstract: Resources for the Preferences dialog
          The 'ldes' list box description resource defines all of the necessary 
          parameters for the icon list.

Version: 1.0

Change History:
	
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

#include "ControlDefinitions.r"
#include "Controls.r"
#include "Dialogs.r"

#define rPrefsDITLPlatinum 128
#define rIconListDesc 128


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
		
		{kListBoxPlatinumBounds},
		Control
		{
			enabled,
			cPlatinumIconList
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

resource 'CNTL' (cPlatinumIconList, "Category list box control (Platinum)", purgeable)
{
	{kListBoxPlatinumBounds},
	rIconListDesc,
	visible,
	0,
	0,
	kControlListBoxProc,
	0,
	""
};
resource 'ldes' (rIconListDesc, "Category list description")
{
	VersionZero
	{
		0,
		1,
		kCellHeight,
		kListWidth,
		hasVertScroll,
		noHorizScroll,
		kIconListLDEF,
		noGrowSpace,
	}
};

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