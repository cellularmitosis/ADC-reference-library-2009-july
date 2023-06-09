/*

File: PrefsWindow.r

Abstract: Resources for the Preferences window

Version: 4.0

Change History:
	
	<4.0>	conditionalized the resources to be included only where 
			needed
	<3.0>	set the standard handler attribute for the 'wind' 
			resource for Mac OS X (for Carbon running in Mac OS 8/9 
			you have to set that attribute programmaticaly)
	<2.0>	added alternate 'wind' and 'CNTL' resources to support 
			the Mac OS X Aqua appearance HIG
	<1.0>	first release version
			we decided to be Mac OS 8.5 savvy and use a 'wind' 
			resource

� Copyright 2005 Apple Computer, Inc. All rights reserved.

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
#include "MissingConstants.r"	// for the 'wind' resource
#include "PrefsWindow.h"


#if !TARGET_API_MAC_OSX
resource 'wind' (rPrefsWindowPlatinum, "Preferences window (Platinum)", purgeable)
{
	noAttributes,
	{		// remember to sort the collection items by both tag and ID
		kStoredWindowSystemTag, kStoredWindowPascalTitleID, itemUnlocked, itemPersistent, 0,
		kStoredWindowSystemTag, kStoredBasicWindowDescriptionID, itemUnlocked, itemPersistent, 0
	},
	{		// and remember to list the collection data in the same order as the items
		"\0d018Preferences Window",		// length byte, then a string
		
		$"0000002C"							// sizeof(BasicWindowDescription)
		$"0064"$"0064"$"0226"$"0177"		// {100, 100, 550, 375} - ContentRect
		$"0000"$"0000"$"0000"$"0000"		// {0, 0, 0, 0} - ZoomRect
		$"00000000"							// 0 - RefCon
		$"00000000"							// StateFlags
		$"00000000"							// last used windowPositionMethod
		kWindowDefinitionVersionTwoHexStr	// windowDefinitionVersion
		kDocumentWindowClassHexStr			// windowDefinition.versionTwo.windowClass
		$"00000019"						// windowDefinition.versionTwo.windowAttributes
							// (kWindowCloseBoxAttribute) | (kWindowCollapseBoxAttribute) | 
							// (kWindowResizableAttribute)
	}		// you cannot set kWindowStandardHandlerAttribute in a 'wind' resource if you want 
};			// your application to run successfully in Mac OS 8/9 (Classic or CarbonLib)
#else
resource 'wind' (rPrefsWindowAqua, "Preferences window (Aqua)", purgeable)
{
	noAttributes,
	{
		kStoredWindowSystemTag, kStoredWindowPascalTitleID, itemUnlocked, itemPersistent, 0,
		kStoredWindowSystemTag, kStoredBasicWindowDescriptionID, itemUnlocked, itemPersistent, 0
	},
	{
		"\0d018Preferences Window",
		
		$"0000002C"						// see the above 'wind' for a field description
		$"0064"$"0064"$"0230"$"0181"	// {100, 100, 560, 385}
		$"0000"$"0000"$"0000"$"0000"	// {0, 0, 0, 0}
		$"00000000"
		$"00000000"
		$"00000000"
		kWindowDefinitionVersionTwoHexStr
		kDocumentWindowClassHexStr
		$"02000019"		// (kWindowCloseBoxAttribute) | (kWindowCollapseBoxAttribute) | 
	}					// (kWindowResizableAttribute) | (kWindowStandardHandlerAttribute)
};
#endif

#if !TARGET_API_MAC_OSX
resource 'CNTL' (cPlatinumWindowUserPaneVisible, "Current prefs window pane (Platinum)", 
					purgeable)
{
	{kPlatinumWindowEdgeSpacing, 
		kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing, 
		kPrefsWindowPlatinumHeight - (kSizeBoxWidth + kPlatinumMinimumSpacing), 
		kPrefsWindowPlatinumWidth - (kSizeBoxWidth + kPlatinumMinimumSpacing)},
	kControlSupportsEmbedding,
	visible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#else
resource 'CNTL' (cAquaWindowUserPaneVisible, "Current prefs window pane (Aqua)", 
					purgeable)
{
	{kAquaWindowEdgeSpacing, 
		kAquaWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing, 
		kPrefsWindowAquaHeight - (kSizeBoxWidth + kAquaMinimumSpacing), 
		kPrefsWindowAquaWidth - (kSizeBoxWidth + kAquaMinimumSpacing)},
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
resource 'CNTL' (cPlatinumWindowUserPaneInvisible, "Noncurrent prefs window panes (Platinum)", 
					purgeable)
{
	{kPlatinumWindowEdgeSpacing, 
		kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing, 
		kPrefsWindowPlatinumHeight - (kSizeBoxWidth + kPlatinumMinimumSpacing), 
		kPrefsWindowPlatinumWidth - (kSizeBoxWidth + kPlatinumMinimumSpacing)},
	kControlSupportsEmbedding,
	invisible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#else
resource 'CNTL' (cAquaWindowUserPaneInvisible, "Noncurrent prefs window panes (Aqua)", 
					purgeable)
{
	{kAquaWindowEdgeSpacing, 
		kAquaWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing, 
		kPrefsWindowAquaHeight - (kSizeBoxWidth + kAquaMinimumSpacing), 
		kPrefsWindowAquaWidth - (kSizeBoxWidth + kAquaMinimumSpacing)},
	kControlSupportsEmbedding,
	invisible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};
#endif
resource 'CNTL' (cStaticText, "Preferences controls", purgeable)
{
	{213, 149, 229, 213},	// these coordinates are arbitrary
	0,
	visible,
	0,
	0,
	kControlStaticTextProc,
	0,
	""
};