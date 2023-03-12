/*
	File:		MegaDialog.r

	Contains:	Resources for our MegaDialog.

    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

	Copyright © 1997-2001 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon.r>

#define	teCenter 1 						/*center justify (word alignment) */

//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//	MegaDialog stuff
//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹

resource 'dlgx' (6000, "Mega Dialog")
{
	versionZero
	{
		11
	}
};

resource 'dftb' (6000, purgeable)
{
	versionZero
	{
		{
			skipItem {}
		}
	};
};

resource 'dftb' (6200, purgeable)
{
	versionZero
	{
		{
			skipItem {},
			skipItem {},
			skipItem {},
			skipItem {},
			skipItem {},
			skipItem {},
			skipItem {},
			dataItem { kDialogFontUseFontMask + kDialogFontUseJustMask, kControlFontSmallBoldSystemFont, 0, 0, 0, teCenter, 0, 0, 0, 0, 0, 0, "" }
		}
	}
};

resource 'dftb' (6500, purgeable)
{
	versionZero
	{
		{
			skipItem {},
			skipItem {},
			skipItem {},
			dataItem { kDialogFontUseFontMask, kControlFontSmallBoldSystemFont, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" }
		}
	}
};

resource 'dftb' (6600, purgeable)
{
	versionZero
	{
		{
			skipItem {},
			skipItem {},
			dataItem { kDialogFontUseFontMask, kControlFontSmallSystemFont, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" },
			dataItem { kDialogFontUseFontMask, kControlFontSmallSystemFont, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" }
		}
	}
};

resource 'DITL' (6000, "Mega Dialog")
{
	{	/* array DITLarray: 1 elements */
		/* [1] */
		{9, 0, 262, 576},
		Control {
			enabled,
			228
		}
	}
};

resource 'tab#' (6000)
{
	versionZero {
		{	/* array TabInfo: 7 elements */
			/* [1] */
			0,
			"Classic",
			/* [2] */
			0,
			"Sliders",
			/* [3] */
			0,
			"Progress",
			/* [4] */
			0,
			"Text",
			/* [5] */
			0,
			"Groups",
			/* [6] */
			0,
			"Grab Bag",
			/* [7] */
			0,
			"Disabled Tab"
			
		}
	}
};


//=============================================================================
//	¥ C L A S S I C   P A N E
//=============================================================================

resource 'DITL' (6100, "Classic")
{
	{	/* array DITLarray: 15 elements */
		/* [1] */
		{50+15, 26, 50+20+15, 155},
		Button {
			enabled,
			"Glissade"
		},
		/* [2] */
		{50+15, 165, 50+20+15, 250},
		Button {
			enabled,
			"Retreat!"
		},
		/* [3] */
		{50+15, 260, 50+20+15, 375},
		Button {
			enabled,
			"Fix Rope"
		},
		/* [4] */
		{50+15, 385, 50+20+15, 535},
		Button {
			enabled,
			"Dig Ice Cave"
		},
		/* [5] */
		{80+15, 26, 80+20+15, 184},
		Control {
			enabled,
			6101
		},
		/* [6] */
		{87+15, 296, 214+15, 464},
		UserItem {
			disabled
		},
		/* [7] */
		{87+15, 464, 214+15, 480},
		Control {
			enabled,
			6102
		},
		/* [8] */
		{214+15, 296, 230+15, 464},
		Control {
			enabled,
			6103
		},
		/* [9] */
		{112+15, 26, 112+15+18, 132},
		CheckBox {
			enabled,
			"Geared Up"
		},
		/* [10] */
		{135+15, 43, 135+15+18, 219},
		CheckBox {
			enabled,
			"Crampons"
		},
		/* [11] */
		{160+15, 43, 160+15+18, 180},
		CheckBox {
			enabled,
			"Ice Axe"
		},
		/* [12] */
		{190+15, 25, 236+15, 179},
		Control {
			enabled,
			6104
		},
		/* [13] */
		{190+15, 25, 190+15+18, 155},
		RadioButton {
			enabled,
			"Hypoxic"
		},
		/* [14] */
		{214+15, 25, 214+15+18, 179},
		RadioButton {
			enabled,
			"Exhausted"
		},
		/* [15] */
		{173+15, 184, 173+15+20, 260},
		Button {
			enabled,
			"Both!"
		}
	}
};

resource 'CNTL' (6101, "Mega Dialog - Popup button")
{
	{0, 0, 20, 158},
	0,
	visible,
	0,
	6101,
	401,
	0,
	""
};

resource 'CNTL' (6102, "Mega Dialog - Vert live scroll bar")
{
	{203, 456, 330, 472},
	0,
	visible,
	100,
	0,
	386,
	0,
	""
};

resource 'CNTL' (6103, "Mega Dialog - Horiz live scroll bar")
{
	{314, 255, 330, 423},
	0,
	visible,
	100,
	0,
	386,
	0,
	""
};

resource 'CNTL' (6104, "Mega Dialog - RadioGroup (Classic)")
{
	{279, 26, 324, 155},
	0,
	visible,
	0,
	-1,
	416,
	0,
	""
};


resource 'MENU' (6101)
{
	6101,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Title",
	{
		"Everest", noIcon, noKey, noMark, plain,
		"K2", noIcon, noKey, noMark, plain,
		"Kanchenjunga", noIcon, noKey, noMark, plain,
		"Lhotse", noIcon, noKey, noMark, plain,
		"Makalu", noIcon, noKey, noMark, plain,
		"Cho Oyu", noIcon, noKey, noMark, plain,
		"Dhaulagiri", noIcon, noKey, noMark, plain,
		"Manaslu", noIcon, noKey, noMark, plain,
		"Nanga Parbat", noIcon, noKey, noMark, plain,
		"Annapurna I", noIcon, noKey, noMark, plain,
		"Gasherbrum I", noIcon, noKey, noMark, plain,
		"Broad Peak", noIcon, noKey, noMark, plain,
		"Xixabangma", noIcon, noKey, noMark, plain,
		"Gasherbrum II", noIcon, noKey, noMark, plain
	}
};

//=============================================================================
//	¥ S L I D E R   P A N E
//=============================================================================

resource 'DITL' (6200, "Sliders")
{
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{30+15, 22, 30+26+15, 22+143},
		Control {
			enabled,
			6201
		},
		/* [2] */
		{60+15, 22, 60+26+15, 22+143},
		Control {
			enabled,
			6202
		},
		/* [3] */
		{90+15, 22, 90+26+15, 22+143},
		Control {
			enabled,
			6203
		},
		/* [4] */
		{124+15, 23, 124+100+15, 23+26},
		Control {
			enabled,
			6204
		},
		/* [5] */
		{124+15, 87, 124+100+15, 87+41},
		Control {
			enabled,
			6205
		},
		/* [6] */
		{124+15, 143, 124+100+15, 143+26},
		Control {
			enabled,
			6206
		},
		/* [7] */
		{171+15, 224, 193+15, 345},
		CheckBox {
			enabled,
			"Live feedback"
		},
		/* [8] */
		{139+15, 265, 156+15, 309},
		StaticText {
			disabled,
			"0"
		},
		/* [9] */
		{38+15, 224, 133+15, 351},
		Control {
			enabled,
			6207
		}
	}
};

resource 'CNTL' (6201, "Mega Dialog - Down pointing slider")
{
	{0, 0, 26, 143},
	0,
	visible,
	255,
	0,
	49,
	0,
	""
};

resource 'CNTL' (6202, "Mega Dialog - Up pointing slider")
{
	{0, 0, 26, 143},
	0,
	visible,
	255,
	0,
	53,
	0,
	""
};

resource 'CNTL' (6203, "Mega Dialog - Non-directional slider (horiz)")
{
	{0, 0, 26, 143},
	0,
	visible,
	255,
	0,
	57,
	0,
	""
};

resource 'CNTL' (6204, "Mega Dialog - Left pointing slider")
{
	{0, 0, 100, 26},
	0,
	visible,
	255,
	0,
	53,
	0,
	""
};

resource 'CNTL' (6205, "Mega Dialog - Right pointing slider w/Tick marks")
{
	{0, 0, 100, 41},
	5,
	visible,
	255,
	0,
	51,
	0,
	""
};

resource 'CNTL' (6206, "Mega Dialog - non-directional slider (vert)")
{
	{0, 0, 100, 26},
	0,
	visible,
	255,
	0,
	57,
	0,
	""
};

resource 'CNTL' (6207, "Mega Dialog - Fading Picture User Pane")
{
	{0, 0, 95, 127},
	0,
	visible,
	0,
	0,
	256,
	0,
	""
};

//=============================================================================
//	¥ P R O G R E S S   P A N E
//=============================================================================

resource 'DITL' (6300, "Progress Pane")
{
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{51, 18, 51+20, 175},
		Control {
			enabled,
			6301
		},
		/* [2] */
		{77, 19, 77+20, 176},
		Control {
			enabled,
			6302
		},
		/* [3] */
		{120, 18, 120+130, 18+20},
		Control {
			enabled,
			6304
		},
		/* [4] */
		{120, 44, 120+130, 44+20},
		Control {
			enabled,
			6305
		},
		/* [5] */
		{103, 19, 103+10, 176},
		Control {
			enabled,
			6306
		},
		/* [6] */
		{61, 204, 61+16, 220},
		Control {
			enabled,
			6303
		},
		/* [7] */
		{61, 240, 83, 340},
		Button {
			enabled,
			"Click Me"
		},
		/* [8] */
		{51, 330+18, 51+24, 330+175},
		Control {
			enabled,
			6311
		},
		/* [9] */
		{77, 330+19, 77+24, 330+176},
		Control {
			enabled,
			6312
		},
		/* [10] */
		{130, 330+18, 130+120, 330+18+24},
		Control {
			enabled,
			6314
		},
		/* [11] */
		{130, 330+44, 130+120, 330+44+24},
		Control {
			enabled,
			6315
		},
		/* [12] */
		{103, 330+19, 103+16, 330+176},
		Control {
			enabled,
			6316
		},
		/* [13] */
		{61, 330+204, 61+32, 330+204+32},
		Control {
			enabled,
			6313
		},
	}
};

resource 'CNTL' (6301, "Mega Dialog - Determinate Progress")
{
	{345, 248, 361, 405},
	0,
	visible,
	100,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6302, "Mega Dialog - Indeterminate progress")
{
	{379, 247, 395, 404},
	0,
	visible,
	0,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6303, "Mega Dialog - Chasing Arrows")
{
	{0, 0, 16, 16},
	0,
	visible,
	0,
	0,
	112,
	0,
	""
};

resource 'CNTL' (6304, "Mega Dialog - Vertical Determinate Progress")
{
	{0, 0, 130, 16},
	0,
	visible,
	100,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6305, "Mega Dialog - Vertical Indeterminate progress")
{
	{0, 0, 130, 16},
	0,
	visible,
	0,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6306, "Mega Dialog - Relevance")
{
	{0, 0, 10, 157},
	0,
	visible,
	100,
	0,
	81,
	0,
	""
};

resource 'CNTL' (6311, "Mega Dialog - Determinate Progress - Large")
{
	{345, 248, 361, 405},
	0,
	visible,
	100,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6312, "Mega Dialog - Indeterminate progress - Large")
{
	{379, 247, 395, 404},
	0,
	visible,
	0,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6313, "Mega Dialog - Chasing Arrows - Large")
{
	{0, 0, 32, 32},
	0,
	visible,
	0,
	0,
	112,
	0,
	""
};

resource 'CNTL' (6314, "Mega Dialog - Vertical Determinate Progress - Large")
{
	{0, 0, 130, 16},
	0,
	visible,
	100,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6315, "Mega Dialog - Vertical Indeterminate progress - Large")
{
	{0, 0, 130, 16},
	0,
	visible,
	0,
	0,
	80,
	0,
	""
};

resource 'CNTL' (6316, "Mega Dialog - Relevance - Large")
{
	{0, 0, 10, 157},
	0,
	visible,
	100,
	0,
	81,
	0,
	""
};

//=============================================================================
//	¥ T E X T   P A N E
//=============================================================================

resource 'DITL' (6400, "Text")
{
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{58, 188, 74, 263},
		StaticText {
			disabled,
			"Static Text"
		},
		/* [2] */
		{58, 281, 74, 356},
		StaticText {
			disabled,
			"Static Text"
		},
		/* [3] */
		{60, 31, 76, 143},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [4] */
		{92, 31, 108, 143},
		Control {
			enabled,
			6401
		},
		/* [5] */
		{123, 31, 139, 143},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{153, 28, 175, 145},
		Control {
			enabled,
			6402
		},
		/* [7] */
		{89, 189, 192, 398},
		Control {
			enabled,
			6403
		},
		/* [8] */
		{195, 28, 215, 146},
		Button {
			enabled,
			"Show Password"
		},
		/* [9] */
		{194, 188, 210, 298},
		StaticText {
			disabled,
			"The password is:"
		},
		/* [10] */
		{212, 188, 228, 397},
		StaticText {
			disabled,
			""
		}
	}
};

resource 'CNTL' (6401, "Mega Dialog - Password field") {
	{0, 0, 16, 112},
	0,
	visible,
	0,
	0,
	914,   // kControlEditUnicodeTextPasswordProc
	0,
	"Secret"
};

resource 'CNTL' (6402, "Mega Dialog - Clock")
{
	{269, 95, 296, 212},
	0,
	visible,
	0,
	0,
	242,
	0,
	""
};

resource 'CNTL' (6403, "Mega Dialog - List Box")
{
	{193, 255, 296, 464},
	6400,
	visible,
	0,
	0,
	353,
	0,
	""
};


resource 'ldes' (6400, purgeable)
{
	versionZero {
		0,
		1,
		0,
		0,
		hasVertScroll,
		noHorizScroll,
		0,
		noGrowSpace
	}
};

//=============================================================================
//	¥ G R O U P  P A N E
//=============================================================================

resource 'DITL' (6500, "Groups")
{
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{40+15, 15, 221+15, 229},
		Control {
			enabled,
			6501
		},
		/* [2] */
		{64+15, 21, 216+15, 220},
		Control {
			enabled,
			6502
		},
		/* [3] */
		{122+15, 237, 127+15, 417},
		Control {
			enabled,
			6503
		},
		/* [4] */
		{139+15, 27, 214+15, 216},
		Control {
			enabled,
			6504
		},
		/* [5] */
		{67+15, 24, 132+15, 142},
		Control {
			enabled,
			6505
		},
		/* [6] */
		{70+15, 26, 88+15, 132},
		RadioButton {
			enabled,
			"Level 1"
		},
		/* [7] */
		{89+15, 26, 107+15, 132},
		RadioButton {
			enabled,
			"Level 2"
		},
		/* [8] */
		{108+15, 26, 126+15, 132},
		RadioButton {
			enabled,
			"Level 3"
		},
		/* [9] */
		{160+15, 35, 178+15, 213},
		CheckBox {
			enabled,
			"Use Millicochranes"
		},
		/* [10] */
		{184+15, 35, 202+15, 189},
		CheckBox {
			enabled,
			"Full Isolinear Check"
		},
		/* [11] */
		{48+15, 321, 118+15, 324},
		Control {
			enabled,
			6506
		},
		/* [12] */
		{74+15, 258, 106+15, 290},
		Control {
			enabled,
			6507
		},
		/* [13] */
		{74+15, 361, 106+15, 393},
		Control {
			enabled,
			6508
		},
		/* [14] */
		{134+15, 259, 222+15, 393},
		Picture {
			disabled,
			6002
		}
	}
};

resource 'CNTL' (6501, "Mega Dialog - Run Diagnostic group box")
{
	{146, 83, 327, 297},
	0,
	visible,
	1,
	0,
	161,
	0,
	"Run diagnostic"
};

resource 'CNTL' (6502, "Mega Dialog - Layout: grouping user pane")
{
	{478, 154, 630, 353},
	2,
	visible,
	0,
	0,
	256,
	0,
	""
};

resource 'CNTL' (6503, "Mega Dialog - Horiz Separator") {
	{233, 303, 238, 483},
	0,
	visible,
	0,
	0,
	144,
	0,
	""
};

resource 'CNTL' (6504, "Mega Dialog - Secondary group box") {
	{242, 92, 317, 281},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Secondary"
};

resource 'CNTL' (6505, "Mega Dialog - Radio Group (Layout)") {
	{172, 90, 237, 208},
	0,
	visible,
	0,
	0,
	416,
	0,
	""
};

resource 'CNTL' (6506, "Mega Dialog - Vertical Separator") {
	{0, 0, 70, 3},
	0,
	visible,
	0,
	0,
	144,
	0,
	""
};

resource 'CNTL' (6507, "Mega Dialog - Layout pane icon 1") {
	{0, 0, 32, 32},
	-3982,
	visible,
	0,
	0,
	323,
	0,
	""
};

resource 'CNTL' (6508, "Mega Dialog - Layout pane icon 2") {
	{0, 0, 32, 32},
	-3987,
	visible,
	0,
	0,
	323,
	0,
	""
};


//=============================================================================
//	¥ G R O U P  P A N E
//=============================================================================

resource 'DITL' (6600, "Grab Bag")
{
	{	/* array DITLarray: 23 elements */
		/* [1] */
		{36+15, 17, 76+15, 57},
		Control {
			enabled,
			6601
		},
		/* [2] */
		{81+15, 148, 105+15, 248},
		Control {
			enabled,
			6602
		},
		/* [3] */
		{36+15, 148, 76+15, 188},
		Control {
			enabled,
			6603
		},
		/* [4] */
		{35+15, 208, 75+15, 248},
		Control {
			enabled,
			6604
		},
		/* [5] */
		{111+15, 148, 135+15, 248},
		Control {
			enabled,
			6605
		},
		/* [6] */
		{36+15, 77, 76+15, 117},
		Control {
			enabled,
			6606
		},
		/* [7] */
		{81+15, 17, 135+15, 117},
		Control {
			enabled,
			6607
		},
		/* [8] */
		{35+15, 261, 59+15, 361},
		Control {
			enabled,
			6608
		},
		/* [9] */
		{64+15, 261, 88+15, 361},
		Control {
			enabled,
			6609
		},
		/* [10] */
		{198+15, 260, 230+15, 292},
		Control {
			enabled,
			6610
		},
		/* [11] */
		{198+15, 322, 230+15, 354},
		Control {
			enabled,
			6611
		},
		/* [12] */
		{95+15, 261, 193+15, 398},
		Control {
			enabled,
			6612
		},
		/* [13] */
		{143+15, 149, 183+15, 189},
		Control {
			enabled,
			6613
		},
		/* [14] */
		{152+15, 209, 175+15, 222},
		Control {
			enabled,
			6614
		},
		/* [15] */
		{148+15, 18, 164+15, 34},
		Control {
			enabled,
			6615
		},
		/* [16] */
		{148+15, 34, 164+15, 50},
		Control {
			enabled,
			6616
		},
		/* [17] */
		{148+15, 50, 164+15, 66},
		Control {
			enabled,
			6617
		},
		/* [18] */
		{166+15, 13, 188+15, 87},
		Control {
			enabled,
			6618
		},
		/* [19] */
		{169+15, 18, 185+15, 34},
		Control {
			enabled,
			6619
		},
		/* [20] */
		{169+15, 34, 185+15, 50},
		Control {
			enabled,
			6620
		},
		/* [21] */
		{169+15, 50, 185+15, 66},
		Control {
			enabled,
			6621
		},
		/* [22] */
		{169+15, 66, 185+15, 82},
		Control {
			enabled,
			6622
		},
		/* [23] */
		{196+15, 15, 212+15, 31},
		Control {
			enabled,
			6623
		},
		/* [24] */
		{196+15, 65, 228+15, 215},
		Control {
			enabled,
			6624
		},
		/* [25] */
		{35+15, 410, 35+15+20, 410+20},
		Control {
			enabled,
			6625
		},
		/* [26] */
		{35+15, 440, 35+15+20, 440+20},
		Control {
			enabled,
			6626
		},
		/* [27] */
		{35+15, 470, 35+15+25, 470+25},
		Control {
			enabled,
			6627
		},
		/* [28] */
		{35+15, 510, 35+15+25, 510+25},
		Control {
			enabled,
			6628
		}
	}
};

resource 'CNTL' (6601, "Mega Dialog - Small Bevel w/icon suite")
{
	{0, 0, 40, 40},
	0,
	visible,
	-3994,
	1,
	32,
	0,
	""
};

resource 'CNTL' (6602, "Mega Dialog - Bevel button w/text to right")
{
	{0, 0, 24, 100},
	0,
	visible,
	-16386,
	1,
	33,
	0,
	"On Right"
};

resource 'CNTL' (6603, "Mega Dialog - Bevel with text below graphic")
{
	{0, 0, 40, 40},
	0,
	visible,
	-3970,
	1,
	34,
	0,
	"Below"
};

resource 'CNTL' (6604, "Mega Dialog - Bevel Button w/text above graphic")
{
	{0, 0, 40, 40},
	0,
	visible,
	-3995,
	1,
	33,
	0,
	"Above"
};

resource 'CNTL' (6605, "Mega Dialog - Bevel Button w/text to left")
{
	{0, 0, 24, 100},
	0,
	visible,
	-16386,
	1,
	33,
	0,
	"On Left"
};

resource 'CNTL' (6606, "Mega Dialog - Medium bevel with cicon")
{
	{0, 0, 40, 40},
	0,
	visible,
	2,
	2,
	33,
	0,
	""
};

resource 'CNTL' (6607, "Mega Dialog - Bevel button w/picture")
{
	{244, 199, 298, 299},
	0,
	visible,
	32236,
	3,
	33,
	0,
	""
};

resource 'CNTL' (6608, "Mega Dialog - Bevel button with menu to right")
{
	{0, 0, 24, 100},
	6101,
	visible,
	0,
	0,
	37,
	0,
	"Menus, too!"
};

resource 'CNTL' (6609, "Mega Dialog - bevel button w/menu downward")
{
	{0, 0, 24, 100},
	6101,
	visible,
	0,
	8192,
	33,
	0,
	"Downward"
};

resource 'CNTL' (6610, "Mega Dialog - Clickable icon")
{
	{0, 0, 32, 32},
	-3985,
	visible,
	0,
	0,
	322,
	0,
	""
};

resource 'CNTL' (6611, "Mega Dialog - Non-tracking icon")
{
	{0, 0, 32, 32},
	-3982,
	visible,
	0,
	0,
	323,
	0,
	""
};

resource 'CNTL' (6612, "Mega Dialog - Clickable Picture")
{
	{0, 0, 98, 137},
	131,
	visible,
	0,
	0,
	304,
	0,
	""
};

resource 'CNTL' (6613, "Mega Dialog - Image Well")
{
	{0, 0, 40, 40},
	-4000,
	visible,
	0,
	1,
	176,
	0,
	""
};

resource 'CNTL' (6614, "Mega Dialog - Little Arrows")
{
	{0, 0, 23, 13},
	0,
	visible,
	100,
	0,
	96,
	0,
	""
};

resource 'CNTL' (6615, "Mega Dialog - Toggling bevel button (Bold)")
{
	{0, 0, 16, 16},
	0,
	visible,
	129,
	257,
	32,
	0,
	""
};

resource 'CNTL' (6616, "Mega Dialog - Toggling Bevel Button (italic)")
{
	{0, 0, 16, 16},
	0,
	visible,
	130,
	257,
	32,
	0,
	""
};

resource 'CNTL' (6617, "Mega Dialog - Toggling Bevel Button (underline)")
{
	{0, 0, 16, 16},
	0,
	visible,
	131,
	257,
	32,
	0,
	""
};


resource 'CNTL' (6618, "Mega Dialog - Bevel Radio Group")
{
	{271, 79, 293, 153},
	0,
	visible,
	0,
	0,
	416,
	0,
	""
};

resource 'CNTL' (6619, "Mega Dialog - Sticky Bevel Button 1")
{
	{0, 0, 16, 16},
	0,
	visible,
	132,
	513,
	32,
	0,
	""
};

resource 'CNTL' (6620, "Mega Dialog - Sticky Bevel Button 2")
{
	{0, 0, 16, 16},
	0,
	visible,
	133,
	513,
	32,
	0,
	""
};

resource 'CNTL' (6621, "Mega Dialog - Sticky Bevel Button 3")
{
	{0, 0, 16, 16},
	0,
	visible,
	134,
	513,
	32,
	0,
	""
};

resource 'CNTL' (6622, "Mega Dialog - Sticky Bevel Button 4")
{
	{0, 0, 16, 16},
	0,
	visible,
	135,
	513,
	32,
	0,
	""
};

resource 'CNTL' (6623, "Mega Dialog - auto toggling discl. triangle")
{
	{0, 0, 16, 16},
	0,
	visible,
	1,
	0,
	66,
	0,
	""
};

resource 'CNTL' (6624, "Mega Dialog - Custom control def")
{
	{0, 0, 32, 150},
	0,
	visible,
	0,
	0,
	500 * 16,
	0,
	"Custom Control Definition"
};

resource 'CNTL' (6625, "Mega Dialog - Round Button, Normal")
{
	{0, 0, 20, 20},
	0,
	visible,
	0,
	0,
	31 * 16,
	0,
	""
};

resource 'CNTL' (6626, "Mega Dialog - Round Button, Normal")
{
	{0, 0, 20, 20},
	0,
	visible,
	0,
	0,
	31 * 16,
	0,
	""
};

resource 'CNTL' (6627, "Mega Dialog - Round Button, Large")
{
	{0, 0, 25, 25},
	0,
	visible,
	0,
	0,
	(31 * 16) + 1,
	0,
	""
};

resource 'CNTL' (6628, "Mega Dialog - Round Button, Large")
{
	{0, 0, 25, 25},
	0,
	visible,
	0,
	0,
	(31 * 16) + 1,
	0,
	""
};
