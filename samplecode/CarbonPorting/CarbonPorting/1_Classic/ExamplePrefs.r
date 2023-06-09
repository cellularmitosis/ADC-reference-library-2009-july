/*

File: ExamplePrefs.r

Abstract: Resources for the ExamplePrefs application

Version: 1.0

Change History:
	
	<1.0>	first release version
			it's kind of cheating but we decided to use Carbon Event 
			HI Command IDs even though we're not using Carbon Events 
			(or Carbon for that matter)

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

#include "CarbonEvents.r"
#include "ControlDefinitions.r"
#include "Controls.r"
#include "MacTypes.r"
#include "Menus.r"
#include "Script.r"

#define rApple 128
#define rFile 129
#define rEdit 130
#define rDemonstration 131


resource 'vers' (1, purgeable)
{
	1,
	0,
	final,
	0,
	verUS,
	"1.0",			// Short version number
	"ExamplePrefs 1.0"
};

resource 'MBAR' (rMenuBar, "Main menu bar", preload, nonpurgeable)
{
	{rApple, rFile, rEdit, rDemonstration}
};

resource 'MENU' (rApple, preload, nonpurgeable)
{
	mApple,
	kMenuStdMenuProc,
	allEnabled,
	enabled,
	apple,
	{
		"About ExamplePrefs...", noIcon, noKey, noMark, plain
	}
};
resource 'xmnu' (rApple)
{
	versionOne
	{
		{
			dataItem {kHICommandAbout, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
		}
	}
};

resource 'MENU' (rFile, preload, nonpurgeable)
{
	mFile,
	kMenuStdMenuProc,
	0x7FFFF800,
	enabled,
	"File",
	{
		"New", noIcon, "N", noMark, plain,
		"Open...", noIcon, "O", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Close", noIcon, "W", noMark, plain,
		"Save", noIcon, "S", noMark, plain,
		"Save As...", noIcon, "S", noMark, plain,
		"Revert", noIcon, "R", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Page Setup...", noIcon, "P", noMark, plain,
		"Print...", noIcon, "P", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
	}
};
resource 'xmnu' (rFile)
{
	versionOne
	{
		{
			dataItem {kHICommandNew, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandOpen, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			skipItem {},
			dataItem {kHICommandClose, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandSave, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandSaveAs, kMenuShiftModifier, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandRevert, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			skipItem {},
			dataItem {kHICommandPageSetup, kMenuShiftModifier, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandPrint, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			skipItem {},
			dataItem {kHICommandQuit, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph}
		}
	}
};

resource 'MENU' (rEdit, preload, nonpurgeable)
{
	mEdit,
	kMenuStdMenuProc,
	0x7FFFFF00,
	disabled,
	"Edit",
	{
		"Undo", noIcon, "Z", noMark, plain,
		"Redo", noIcon, "Z", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Cut", noIcon, "X", noMark, plain,
		"Copy", noIcon, "C", noMark, plain,
		"Paste", noIcon, "V", noMark, plain,
		"Delete", noIcon, noKey, noMark, plain,
		"Select All", noIcon, "A", noMark, plain
	}
};
resource 'xmnu' (rEdit)
{
	versionOne
	{
		{
			dataItem {kHICommandUndo, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandRedo, kMenuShiftModifier, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			skipItem {},
			dataItem {kHICommandCut, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandCopy, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandPaste, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandClear, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kHICommandSelectAll, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph}
		}
	}
};

resource 'MENU' (rDemonstration, preload, nonpurgeable)
{
	mDemonstration,
	kMenuStdMenuProc,
	allEnabled,
	enabled,
	"Demonstration",
	{
		"Preferences Window", noIcon, "1", noMark, plain,
		"Preferences Dialog", noIcon, "2", noMark, plain
	}
};
resource 'xmnu' (rDemonstration)
{
	versionOne
	{
		{
			dataItem {kCommandPrWin, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph},
			dataItem {kCommandPrDlg, kMenuNoModifiers, noVKey, noAttributes, currScript, 0, noIndent, noHierID, sysFont, naturalGlyph}
		}
	}
};

resource 'STR#' (rIconListStrings, "Category list strings", purgeable)
{
	{
		"Mail",
		"Stickies",
		"iSync",
		"Safari",
		"iChat",
		"Dashboard",
		"Console",
		"iCal",
		"Grab",
		"Dictionary"
	}
};