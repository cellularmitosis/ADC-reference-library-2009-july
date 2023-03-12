/*
	File:		BasicDataBrowser.r

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

	Copyright © 2000-2003 Apple Computer, Inc., All Rights Reserved
*/

#ifdef __APPLE_CC__
#include <Carbon/Carbon.r>
#else
#ifndef __CARBON__
#include <Carbon.r>
#endif
#endif

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	notDisplayManagerAware,
	reserved,
	reserved,
	5120000,
	460800
};

resource 'MENU' (128, preload) {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About DataBrowser...", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129, preload) {
	129,
	textMenuProc,
	allEnabled,
	enabled,
	"File",
	{	/* array: 2 elements */
		/* [1] */
		"Close", noIcon, "W", noMark, plain,
		/* [2] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130) {
	130,
	textMenuProc,
	0x7FFFFFBD,
	enabled,
	"Edit",
	{	/* array: 8 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Select All", noIcon, "A", noMark, plain
	}
};

resource 'MENU' (140) {
	140,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"Columns",
	{	/* array: 10 elements */
		/* [1] */
		"None", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Checkbox", noIcon, noKey, noMark, plain,
		/* [4] */
		"Flavor", noIcon, noKey, noMark, plain,
		/* [5] */
		"Color", noIcon, noKey, noMark, plain,
		/* [6] */
		"Index", noIcon, noKey, noMark, plain,
		/* [7] */
		"Item", noIcon, noKey, noMark, plain,
		/* [8] */
		"column 6", noIcon, noKey, noMark, plain,
		/* [9] */
		"column 7", noIcon, noKey, noMark, plain,
		/* [10] */
		"column 8", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (200) {
	200,
	textMenuProc,
	allEnabled,
	enabled,
	"Contextual Menu",
	{	/* array: 5 elements */
		/* [1] */
		"Lime is luscious.", noIcon, noKey, noMark, plain,
		/* [2] */
		"Tangerine in tangy.", noIcon, noKey, noMark, plain,
		/* [3] */
		"Strawberry is super.", noIcon, noKey, noMark, plain,
		/* [4] */
		"Grape is grand.", noIcon, noKey, noMark, plain,
		/* [5] */
		"Blueberry is nice.", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (131) {
	131,
	textMenuProc,
	0x7FFFFFF5,
	enabled,
	"View",
	{	/* array: 7 elements */
		/* [1] */
		"as List", noIcon, noKey, noMark, plain,
		/* [2] */
		"as Icons", noIcon, noKey, noMark, plain,
		/* [3] */
		"as Columns", noIcon, noKey, noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Settings…", noIcon, noKey, noMark, plain,
		/* [6] */
		"Customize…", noIcon, noKey, noMark, plain,
		/* [7] */
		"Show Placard", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (132) {
	132,
	textMenuProc,
	0x7FFFFFFC,
	enabled,
	"API",
	{	/* array: 2 elements */
		/* [1] */
		"Reveal Item…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Selection Flags…", noIcon, noKey, noMark, plain
	}
};

data 'carb' (0) {
};

resource 'STR#' (128, purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Lime",
		/* [2] */
		"Tangerine",
		/* [3] */
		"Strawberry",
		/* [4] */
		"Grape",
		/* [5] */
		"Blueberry"
	}
};

resource 'STR#' (129, purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Green",
		/* [2] */
		"Orange",
		/* [3] */
		"Red",
		/* [4] */
		"Purple",
		/* [5] */
		"Blue"
	}
};

resource 'STR#' (130, purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Lime is luscious.",
		/* [2] */
		"Tangerine is Tangy.",
		/* [3] */
		"Strawberry is Super.",
		/* [4] */
		"Grape is grand.",
		/* [5] */
		"Blueberry is nice."
	}
};

resource 'MBAR' (128) {
	{	/* array MenuArray: 5 elements */
		/* [1] */
		128,
		/* [2] */
		129,
		/* [3] */
		130,
		/* [4] */
		131,
		/* [5] */
		132
	}
};

resource 'mctb' (130) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'mctb' (131) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'mctb' (132) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'xmnu' (128) {
	versionZero {
		{	/* array ItemExtensions: 1 elements */
			/* [1] */
			dataItem {
				1633841013,
				0x0,
				currScript,
				1,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			}
		}
	}
};

resource 'xmnu' (129) {
	versionZero {
		{	/* array ItemExtensions: 2 elements */
			/* [1] */
			dataItem {
				1668050803,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [2] */
			dataItem {
				1903520116,
				0x0,
				currScript,
				1,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			}
		}
	}
};

resource 'xmnu' (130) {
	versionZero {
		{	/* array ItemExtensions: 8 elements */
			/* [1] */
			dataItem {
				1970168943,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [2] */
			skipItem {

			},
			/* [3] */
			dataItem {
				1668641824,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [4] */
			dataItem {
				1668247673,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [5] */
			dataItem {
				1885434740,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [6] */
			dataItem {
				1668048225,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [7] */
			skipItem {

			},
			/* [8] */
			dataItem {
				1935764588,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			}
		}
	}
};

resource 'xmnu' (131) {
	versionZero {
		{	/* array ItemExtensions: 7 elements */
			/* [1] */
			dataItem {
				1819505782,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [2] */
			dataItem {
				1768124022,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [3] */
			dataItem {
				1668050294,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [4] */
			skipItem {

			},
			/* [5] */
			dataItem {
				1937010279,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [6] */
			dataItem {
				1668641652,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [7] */
			dataItem {
				1886151011,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			}
		}
	}
};

resource 'xmnu' (132) {
	versionZero {
		{	/* array ItemExtensions: 10 elements */
			/* [1] */
			dataItem {
				1633907830,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [2] */
			dataItem {
				1986158962,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [3] */
			dataItem {
				1751277938,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [4] */
			skipItem {

			},
			/* [5] */
			dataItem {
				1718772077,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [6] */
			skipItem {

			},
			/* [7] */
			dataItem {
				1936224119,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [8] */
			dataItem {
				1718378855,
				0x0,
				currScript,
				2,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [9] */
			skipItem {

			},
			/* [10] */
			skipItem {

			}
		}
	}
};

resource 'ALRT' (200, purgeable) {
	{186, 230, 290, 602},
	200,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	noAutoCenter
};

resource 'CNTL' (1291, purgeable) {
	{13, 19, 29, 333},
	0,
	visible,
	-1,
	140,
	1016,
	0,
	"Disclosure Column:"
};

resource 'CNTL' (1292, purgeable) {
	{45, 19, 95, 333},
	0,
	visible,
	100,
	0,
	161,
	0,
	"Variable Height Rows"
};

resource 'CNTL' (1301, purgeable) {
	{45, 19, 135, 333},
	0,
	visible,
	100,
	0,
	160,
	0,
	"Selection Flags"
};

resource 'CNTL' (1308, purgeable) {
	{156, 28, 172, 168},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Active Items"
};

resource 'CNTL' (1309, purgeable) {
	{176, 28, 192, 168},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Vertical Scrollbar"
};

resource 'CNTL' (13010, purgeable) {
	{156, 172, 172, 322},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Show Frame & Focus"
};

resource 'CNTL' (13011, purgeable) {
	{176, 172, 192, 322},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Horizontal Scrollbar"
};

resource 'CNTL' (1302, purgeable) {
	{36, 32, 52, 172},
	0,
	visible,
	100,
	0,
	371,
	0,
	"No Disjoint"
};

resource 'CNTL' (1303, purgeable) {
	{56, 32, 72, 172},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Drag Select"
};

resource 'CNTL' (1304, purgeable) {
	{76, 32, 92, 172},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Always Extend"
};

resource 'CNTL' (1305, purgeable) {
	{36, 180, 52, 320},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Select Only One"
};

resource 'CNTL' (1306, purgeable) {
	{56, 180, 72, 320},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Reset Selection"
};

resource 'CNTL' (1307, purgeable) {
	{76, 180, 92, 320},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Command Toggles"
};

resource 'CNTL' (1293, purgeable) {
	{68, 39, 86, 195},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Expandable Rows"
};

resource 'CNTL' (1294, purgeable) {
	{106, 29, 122, 160},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Plain Background"
};

resource 'CNTL' (1296, purgeable) {
	{128, 29, 144, 188},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Use Relative Dates"
};

resource 'CNTL' (1295, purgeable) {
	{106, 223, 122, 308},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Fill Hilite"
};

resource 'CNTL' (1297, purgeable) {
	{128, 223, 144, 343},
	0,
	visible,
	100,
	0,
	371,
	0,
	"Show Headers"
};

resource 'dctb' (200) {
	{	/* array ColorSpec: 5 elements */
		/* [1] */
		wContentColor, 65535, 65535, 65535,
		/* [2] */
		wFrameColor, 0, 0, 0,
		/* [3] */
		wTextColor, 0, 0, 0,
		/* [4] */
		wHiliteColor, 0, 0, 0,
		/* [5] */
		wTitleBarColor, 65535, 65535, 65535
	}
};

resource 'dctb' (130) {
	{	/* array ColorSpec: 5 elements */
		/* [1] */
		wContentColor, 65535, 65535, 65535,
		/* [2] */
		wFrameColor, 0, 0, 0,
		/* [3] */
		wTextColor, 0, 0, 0,
		/* [4] */
		wHiliteColor, 0, 0, 0,
		/* [5] */
		wTitleBarColor, 65535, 65535, 65535
	}
};

resource 'DITL' (200, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{25, 123, 84, 260},
		StaticText {
			disabled,
			"BasicDataBrowser\n\n        by J. Rodden"
		},
		/* [2] */
		{77, 277, 97, 357},
		Button {
			enabled,
			"OK"
		}
	}
};

resource 'DITL' (130, purgeable) {
	{	/* array DITLarray: 11 elements */
		/* [1] */
		{61, 19, 151, 333},
		Control {
			enabled,
			1301
		},
		/* [2] */
		{84, 32, 100, 172},
		Control {
			enabled,
			1302
		},
		/* [3] */
		{104, 32, 120, 172},
		Control {
			enabled,
			1303
		},
		/* [4] */
		{124, 32, 140, 172},
		Control {
			enabled,
			1304
		},
		/* [5] */
		{84, 180, 100, 320},
		Control {
			enabled,
			1305
		},
		/* [6] */
		{104, 180, 120, 320},
		Control {
			enabled,
			1306
		},
		/* [7] */
		{124, 180, 140, 320},
		Control {
			enabled,
			1307
		},
		/* [8] */
		{12, 32, 28, 172},
		Control {
			enabled,
			1308
		},
		/* [9] */
		{12, 180, 28, 330},
		Control {
			enabled,
			13010
		},
		/* [10] */
		{32, 32, 48, 172},
		Control {
			enabled,
			1309
		},
		/* [11] */
		{32, 180, 48, 330},
		Control {
			enabled,
			13011
		}
	}
};

resource 'DITL' (129, purgeable) {
	{	/* array DITLarray: 7 elements */
		/* [1] */
		{13, 19, 29, 333},
		Control {
			enabled,
			1291
		},
		/* [2] */
		{45, 19, 95, 333},
		Control {
			enabled,
			1292
		},
		/* [3] */
		{68, 39, 86, 195},
		Control {
			enabled,
			1293
		},
		/* [4] */
		{106, 29, 122, 160},
		Control {
			enabled,
			1294
		},
		/* [5] */
		{106, 199, 122, 284},
		Control {
			enabled,
			1295
		},
		/* [6] */
		{128, 29, 144, 188},
		Control {
			enabled,
			1296
		},
		/* [7] */
		{128, 199, 144, 319},
		Control {
			enabled,
			1297
		}
	}
};

resource 'dlgx' (129) {
	versionZero {
		15
	}
};

resource 'dlgx' (130) {
	versionZero {
		15
	}
};

resource 'DLOG' (130, "DataBrowser settings", purgeable) {
	{142, 659, 307, 1009},
	1057,
	visible,
	goAway,
	0x0,
	130,
	"DataBrowser Settings",
	noAutoCenter
};

resource 'DLOG' (129, "ListView customization", purgeable) {
	{337, 660, 508, 1010},
	1057,
	visible,
	goAway,
	0x0,
	129,
	"Customize ListView",
	noAutoCenter
};
