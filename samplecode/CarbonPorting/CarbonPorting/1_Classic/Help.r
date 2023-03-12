/*

File: Help.r

Abstract: Resources for Balloon Help in the ExamplePrefs application

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
#include "MissingConstants.r"	// for the Balloon Help resources
#include "PrefsDialog.h"

#include "Balloons.r"
#include "MacTypes.r"

#define rPrefsDialogHelp 128


/*
	Since we have dynamic content (a resizable window), we need to use Help Manager 
	calls to provide Balloon Help for the preferences window as opposed to 'hrct' 
	resources.
*/
/*
	As mentioned above, we will be setting and displaying help content for the 
	preferences window programmatically.  For the preferences dialog we can use Balloon 
	Help resources.  HelpItem 'DITL' items don't always work, especially for modeless 
	dialogs, so we'll use an 'hwin' resource to associate our 'hdlg' with the 
	preferences dialog instead.
*/
resource 'hwin' (128, "Balloon Help windows", purgeable)
{
	hmBalloonHelpVersion,
	hmDefaultOptions,
	{
		rPrefsDialogHelp, 'hdlg', 18, "Preferences Dialog"
	}
};

resource 'hdlg' (rPrefsDialogHelp, "Preferences Dialog Balloon Help", purgeable)
{
	hmBalloonHelpVersion,
	2,						// skip the OK and Cancel buttons
	hmDefaultOptions,
	kBalloonWDEFID,
	kTopLeftTipPointsLeftVariant,
	HMSkipItem
	{
	},
	{
		HMStringResItem
		{
			{10, kListWidth},
			{0, 0, 0, 0},
			rHelpStrings, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{10, 
				(kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing) - 
				(kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
				10},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 3,
			0, 0,
			0, 0
		},
		HMStringResItem
		{
			{0, 0},
			{0, 0, 0, 0},
			0, 0,
			rHelpStrings, 2,
			0, 0,
			0, 0
		}
	}
};

resource 'STR#' (rHelpStrings, "Help strings", purgeable)
{
	{
		"This list was created by LNew and uses the Icon List Definition.  "
			"The cell selection algorithm has been customized "
			"so as to allow the user to select one cell only.  "
			"It should mimic a list box control.",
		"This is a static text control to label the category number you have selected.  "
			"In a real preferences dialog this would be a control "
			"that represents a setting for the selected preference category.",
		"This is a user pane control.  "
			"In a real preferences window you would "
			"put the actual controls for the settings here.",
		"This list is a list box control that uses the Icon List Definition.  "
			"The cell selection algorithm has been customized "
			"so as to allow the user to select one cell only."
	}
};