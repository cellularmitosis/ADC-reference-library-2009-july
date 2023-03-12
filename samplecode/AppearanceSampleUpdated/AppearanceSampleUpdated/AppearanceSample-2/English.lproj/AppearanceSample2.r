/*
	File:		AppearanceSample2.h

    Version:	Mac OS X

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

	Copyright © 1997-2001 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon.r>

resource 'ALRT' (128) {
	{211, 11, 297, 337},
	128,
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
	centerMainScreen
	/****** Extra bytes follow... ******/
	/* $"0003"                                               /* .. */
};

resource 'ALRT' (129) {
	{55, 39, 153, 407},
	129,
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
	alertPositionParentWindowScreen
};

resource 'ALRT' (200) {
	{40, 40, 125, 378},
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
	alertPositionMainScreen
};

resource 'DITL' (128) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 246, 72, 304},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 72, 35, 220},
		StaticText {
			disabled,
			"This is the about box"
		}
	}
};

resource 'DITL' (129) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{8, 74, 61, 356},
		StaticText {
			disabled,
			"^0"
		},
		/* [2] */
		{70, 299, 90, 357},
		Button {
			enabled,
			"OK"
		}
	}
};

resource 'DITL' (130) {
	{	/* array DITLarray: 1 elements */
		/* [1] */
		{25, 54, 57, 86},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (200) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{57, 267, 77, 325},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{18, 54, 34, 252},
		StaticText {
			disabled,
			"Here is a test alert."
		}
	}
};

resource 'DITL' (1001) {
	{	/* array DITLarray: 41 elements */
		/* [1] */
		{302, 4, 322, 83},
		Button {
			enabled,
			"Disabled"
		},
		/* [2] */
		{302, 91, 323, 171},
		Button {
			enabled,
			"Enabled"
		},
		/* [3] */
		{302, 179, 322, 239},
		Button {
			enabled,
			"On"
		},
		/* [4] */
		{302, 247, 322, 307},
		Button {
			enabled,
			"Off"
		},
		/* [5] */
		{302, 315, 322, 395},
		Button {
			enabled,
			"Mixed"
		},
		/* [6] */
		{24, 5, 60, 41},
		Control {
			enabled,
			138
		},
		/* [7] */
		{24, 47, 60, 83},
		Control {
			enabled,
			139
		},
		/* [8] */
		{24, 89, 60, 125},
		Control {
			enabled,
			140
		},
		/* [9] */
		{24, 143, 48, 167},
		Control {
			enabled,
			141
		},
		/* [10] */
		{24, 172, 48, 208},
		Control {
			enabled,
			142
		},
		/* [11] */
		{24, 213, 48, 273},
		Control {
			enabled,
			143
		},
		/* [12] */
		{23, 282, 47, 306},
		Control {
			enabled,
			147
		},
		/* [13] */
		{23, 311, 59, 347},
		Control {
			enabled,
			148
		},
		/* [14] */
		{23, 352, 71, 400},
		Control {
			enabled,
			149
		},
		/* [15] */
		{97, 7, 121, 31},
		Control {
			enabled,
			144
		},
		/* [16] */
		{97, 39, 133, 75},
		Control {
			enabled,
			145
		},
		/* [17] */
		{97, 82, 145, 130},
		Control {
			enabled,
			146
		},
		/* [18] */
		{91, 183, 111, 283},
		Control {
			enabled,
			183
		},
		/* [19] */
		{91, 283, 111, 383},
		Control {
			enabled,
			184
		},
		/* [20] */
		{111, 183, 131, 283},
		Control {
			enabled,
			188
		},
		/* [21] */
		{111, 283, 131, 383},
		Control {
			enabled,
			190
		},
		/* [22] */
		{174, 5, 194, 105},
		Control {
			enabled,
			185
		},
		/* [23] */
		{174, 105, 194, 205},
		Control {
			enabled,
			186
		},
		/* [24] */
		{174, 205, 194, 305},
		Control {
			enabled,
			187
		},
		/* [25] */
		{174, 305, 194, 405},
		Control {
			enabled,
			189
		},
		/* [26] */
		{215, 73, 239, 143},
		Control {
			enabled,
			191
		},
		/* [27] */
		{215, 143, 239, 213},
		Control {
			enabled,
			192
		},
		/* [28] */
		{204, 225, 252, 273},
		Control {
			enabled,
			193
		},
		/* [29] */
		{204, 273, 252, 321},
		Control {
			enabled,
			194
		},
		/* [30] */
		{262, 72, 286, 192},
		Control {
			enabled,
			195
		},
		/* [31] */
		{262, 192, 286, 292},
		Control {
			enabled,
			196
		},
		/* [32] */
		{262, 292, 286, 392},
		Control {
			enabled,
			197
		},
		/* [33] */
		{4, 4, 20, 131},
		StaticText {
			disabled,
			"Three Bevel Sizes:"
		},
		/* [34] */
		{4, 141, 20, 178},
		StaticText {
			disabled,
			"Text:"
		},
		/* [35] */
		{5, 280, 21, 347},
		StaticText {
			disabled,
			"Pictures:"
		},
		/* [36] */
		{74, 5, 90, 180},
		StaticText {
			disabled,
			"Icon (can be suite or cicn):"
		},
		/* [37] */
		{74, 182, 89, 358},
		StaticText {
			disabled,
			"Menus and text alignment:"
		},
		/* [38] */
		{157, 5, 171, 80},
		StaticText {
			disabled,
			"Behaviors"
		},
		/* [39] */
		{199, 5, 214, 62},
		StaticText {
			disabled,
			"Combos:"
		},
		/* [40] */
		{217, 5, 233, 72},
		StaticText {
			disabled,
			"Centered:"
		},
		/* [41] */
		{264, 5, 279, 67},
		StaticText {
			disabled,
			"Justified:"
		}
	}
};


resource 'DITL' (1000) {
	{	/* array DITLarray: 23 elements */
		/* [1] */
		{285, 311, 305, 369},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{285, 218, 305, 296},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{39, 9, 55, 79},
		StaticText {
			disabled,
			"Error Text:"
		},
		/* [4] */
		{40, 101, 56, 364},
		EditText {
			enabled,
			"Warp Core breach is imminent!"
		},
		/* [5] */
		{69, 9, 85, 94},
		StaticText {
			disabled,
			"Explanation:"
		},
		/* [6] */
		{69, 101, 123, 364},
		EditText {
			enabled,
			"Magnetic constrictors are offline. You"
			" might want to think about putting them"
			" back online. You have one minute."
		},
		/* [7] */
		{10, 238, 28, 344},
		CheckBox {
			enabled,
			"Movable"
		},
		/* [8] */
		{136, 9, 276, 368},
		Control {
			enabled,
			224
		},
		/* [9] */
		{155, 120, 187, 350},
		Control {
			enabled,
			221
		},
		/* [10] */
		{193, 120, 222, 349},
		Control {
			enabled,
			222
		},
		/* [11] */
		{228, 120, 261, 349},
		Control {
			enabled,
			223
		},
		/* [12] */
		{163, 24, 181, 100},
		StaticText {
			enabled,
			"Button 1:"
		},
		/* [13] */
		{198, 24, 216, 100},
		CheckBox {
			enabled,
			"Button 2"
		},
		/* [14] */
		{233, 24, 251, 100},
		CheckBox {
			enabled,
			"Button 3"
		},
		/* [15] */
		{162, 132, 180, 238},
		CheckBox {
			enabled,
			"Use Default"
		},
		/* [16] */
		{199, 132, 217, 238},
		CheckBox {
			enabled,
			"Use Default"
		},
		/* [17] */
		{233, 132, 251, 238},
		CheckBox {
			enabled,
			"Use Default"
		},
		/* [18] */
		{162, 246, 178, 321},
		EditText {
			enabled,
			"OK"
		},
		/* [19] */
		{199, 246, 215, 321},
		EditText {
			enabled,
			"Cancel"
		},
		/* [20] */
		{236, 246, 252, 321},
		EditText {
			enabled,
			"Don't Save"
		},
		/* [21] */
		{10, 10, 26, 85},
		StaticText {
			disabled,
			"Type:"
		},
		/* [22] */
		{9, 98, 25, 198},
		Control {
			enabled,
			225
		},
		/* [23] */
		{287, 7, 305, 139},
		CheckBox {
			enabled,
			"Show Help Button"
		}
	}
};



resource 'DITL' (4000) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{8, 53, 42, 358},
		StaticText {
			disabled,
			"Here's an example of auto-sizing of a di"
			"alog. Press the auto-size button to see "
			"the rest of this text. I purposely put m"
			"ore text than could be viewed in the spa"
			"ce allotted. AutoSizeDialog grows the di"
			"alog so that you can read all the text."
		},
		/* [2] */
		{68, 300, 88, 358},
		Button {
			enabled,
			"OK"
		},
		/* [3] */
		{69, 54, 89, 140},
		Button {
			enabled,
			"Auto-Size"
		},
		/* [4] */
		{9, 13, 41, 45},
		Icon {
			disabled,
			1
		}
	}
};

resource 'DITL' (7000) {
	{	/* array DITLarray: 0 elements */
	}
};


resource 'DITL' (2020) {
	{	/* array DITLarray: 0 elements */
	}
};

resource 'DITL' (3000) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{85, 394, 105, 452},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{29, 15, 45, 181},
		StaticText {
			disabled,
			"This dialog will go away in"
		},
		/* [3] */
		{29, 183, 45, 200},
		StaticText {
			disabled,
			"10"
		},
		/* [4] */
		{29, 202, 45, 442},
		StaticText {
			disabled,
			"seconds with no user action. Mouse"
		},
		/* [5] */
		{52, 15, 67, 450},
		StaticText {
			disabled,
			"movement or keypresses will reset the ti"
			"mer to the starting value."
		}
	}
};

resource 'MENU' (131) {
	131,
	63,
	0x7FFFFF77,
	enabled,
	"Popup Button",
	{	/* array: 10 elements */
		/* [1] */
		"Wake up...", noIcon, noKey, noMark, plain,
		/* [2] */
		"I need an exit, fast!", noIcon, noKey, noMark, plain,
		/* [3] */
		"I am not a battery", noIcon, noKey, noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Whoa!", noIcon, noKey, noMark, plain,
		/* [6] */
		"I know Kung Fu", noIcon, noKey, noMark, plain,
		/* [7] */
		"He is the One", noIcon, noKey, noMark, plain,
		/* [8] */
		"-", noIcon, noKey, noMark, plain,
		/* [9] */
		"There is no spoon", noIcon, noKey, noMark, plain,
		/* [10] */
		"I can dodge bullets!", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (132) {
	131,
	63,
	0x7FFFFFF7,
	enabled,
	"Popup Button",
	{	/* array: 5 elements */
		/* [1] */
		"This, pray tell, is item one", noIcon, noKey, noMark, plain,
		/* [2] */
		"And this gem must be item two", noIcon, noKey, noMark, plain,
		/* [3] */
		"Let me guess: item three", noIcon, noKey, noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Fin", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134) {
	134,
	63,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 3 elements */
		/* [1] */
		"Text Title", noIcon, noKey, noMark, plain,
		/* [2] */
		"Check Box Title", noIcon, noKey, noMark, plain,
		/* [3] */
		"Popup Button Title", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (135) {
	135,
	63,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 4 elements */
		/* [1] */
		"East", noIcon, noKey, noMark, plain,
		/* [2] */
		"West", noIcon, noKey, noMark, plain,
		/* [3] */
		"North", noIcon, noKey, noMark, plain,
		/* [4] */
		"South", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (136) {
	136,
	63,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 3 elements */
		/* [1] */
		"Icon Suite", noIcon, noKey, noMark, plain,
		/* [2] */
		"Color Icon", noIcon, noKey, noMark, plain,
		/* [3] */
		"Picture", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (137, "Sizes") {
	137,
	63,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 3 elements */
		/* [1] */
		"Small", noIcon, noKey, noMark, plain,
		/* [2] */
		"Normal", noIcon, noKey, noMark, plain,
		/* [3] */
		"Large", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (138) {
	138,
	63,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 3 elements */
		/* [1] */
		"Momentary", noIcon, noKey, noMark, plain,
		/* [2] */
		"Toggles", noIcon, noKey, noMark, plain,
		/* [3] */
		"Sticky", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (140, "Orientations") {
	140,
	63,
	allEnabled,
	enabled,
	"Untitled",
	{	/* array: 10 elements */
		/* [1] */
		"Sys Direction", noIcon, noKey, noMark, plain,
		/* [2] */
		"Center", noIcon, noKey, noMark, plain,
		/* [3] */
		"Left", noIcon, noKey, noMark, plain,
		/* [4] */
		"Right", noIcon, noKey, noMark, plain,
		/* [5] */
		"Top", noIcon, noKey, noMark, plain,
		/* [6] */
		"Bottom", noIcon, noKey, noMark, plain,
		/* [7] */
		"Top Left", noIcon, noKey, noMark, plain,
		/* [8] */
		"Bottom Left", noIcon, noKey, noMark, plain,
		/* [9] */
		"Top Right", noIcon, noKey, noMark, plain,
		/* [10] */
		"Bottom Right", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (141) {
	141,
	63,
	allEnabled,
	enabled,
	"Untitled",
	{	/* array: 4 elements */
		/* [1] */
		"Sys Direction", noIcon, noKey, noMark, plain,
		/* [2] */
		"Flush Left", noIcon, noKey, noMark, plain,
		/* [3] */
		"Flush Right", noIcon, noKey, noMark, plain,
		/* [4] */
		"Centered", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (142) {
	142,
	63,
	allEnabled,
	enabled,
	"Untitled",
	{	/* array: 6 elements */
		/* [1] */
		"Normally", noIcon, noKey, noMark, plain,
		/* [2] */
		"Left of Graphic", noIcon, noKey, noMark, plain,
		/* [3] */
		"Right of Graphic", noIcon, noKey, noMark, plain,
		/* [4] */
		"Above Graphic", noIcon, noKey, noMark, plain,
		/* [5] */
		"Below Graphic", noIcon, noKey, noMark, plain,
		/* [6] */
		"Sys Direction", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (143) {
	143,
	63,
	allEnabled,
	enabled,
	"WYSIWYG",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (144, "Alert Icons") {
	144,
	63,
	allEnabled,
	enabled,
	"StandardAlert",
	{	/* array: 3 elements */
		/* [1] */
		"Stop", noIcon, noKey, noMark, plain,
		/* [2] */
		"Note", noIcon, noKey, noMark, plain,
		/* [3] */
		"Caution", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (139) {
	139,
	63,
	allEnabled,
	enabled,
	"Untitled",
	{	/* array: 4 elements */
		/* [1] */
		"Text Only", noIcon, noKey, noMark, plain,
		/* [2] */
		"Icon Suite", noIcon, noKey, noMark, plain,
		/* [3] */
		"Color Icon", noIcon, noKey, noMark, plain,
		/* [4] */
		"Picture", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (147) {
	147,
	textMenuProc,
	allEnabled,
	enabled,
	"Hier",
	{	/* array: 3 elements */
		/* [1] */
		"Item 1", noIcon, noKey, noMark, plain,
		/* [2] */
		"Item 2", noIcon, noKey, noMark, plain,
		/* [3] */
		"Item 3", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (130) {
	130,
	63,
	0x7FFFFFF7,
	enabled,
	"Examples",
	{	/* array: 15 elements */
		/* [1] */
		"Finder-like Window", noIcon, "1", noMark, plain,
		/* [2] */
		"Dialog-like Window", noIcon, "2", noMark, plain,
		/* [3] */
		"Bevel Button Dialog", noIcon, "3", noMark, plain,
		/* [4] */
		"NewThemeDialog", noIcon, "4", noMark, plain,
		/* [5] */
		"Standard Alert", noIcon, "5", noMark, plain,
		/* [6] */
		"Bevel Button Content", noIcon, "6", noMark, plain,
		/* [7] */
		"CDEF Tester", noIcon, "7", noMark, plain,
		/* [8] */
		"Live Feeback Dialog", noIcon, "8", noMark, plain,
		/* [9] */
		"Mega Dialog", noIcon, "M", noMark, plain,
		/* [10] */
		"Utility Window", noIcon, "U", noMark, plain,
		/* [11] */
		"Side Utility Window", noIcon, "S", noMark, plain,
		/* [12] */
		"AutoSizeDialog", noIcon, "A", noMark, plain,
		/* [13] */
		"Vertical Zoom", noIcon, noKey, noMark, plain,
		/* [14] */
		"Horizontal Zoom", noIcon, noKey, noMark, plain,
		/* [15] */
		"Proxy/WindowPath Dialog", noIcon, noKey, noMark, plain,
		/* [16] */
		"Help Tags", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (148) {
	148,
	textMenuProc,
	allEnabled,
	enabled,
	"Test API",
	{	/* array: 5 elements */
		/* [1] */
		"Menu Drawing", noIcon, noKey, noMark, plain,
		/* [2] */
		"Dump Control Hierarchy", noIcon, noKey, noMark, plain,
		/* [3] */
		"Hide Menu", noIcon, "H", noMark, plain,
		/* [4] */
		"Dialog Timeouts", noIcon, noKey, noMark, plain,
		/* [5] */
		"SetThemeCursor", noIcon, noKey, noMark, plain
	}
};


resource 'MENU' (145) {
	145,
	63,
	allEnabled,
	enabled,
	"Modifiers",
	{	/* array: 60 elements */
		/* [1] */
		"Normal", noIcon, "J", noMark, plain,
		/* [2] */
		"Cmd-Shift", noIcon, "G", noMark, plain,
		/* [3] */
		"Cmd-Shift-Option", noIcon, "Y", noMark, plain,
		/* [4] */
		"Cmd-Shift-Option-Ctrl", noIcon, "E", noMark, plain,
		/* [5] */
		"Command-Delete", noIcon, noKey, noMark, plain,
		/* [6] */
		"Icon Suites", noIcon, noKey, noMark, plain,
		/* [7] */
		"left-to-right tab", noIcon, "`", noMark, plain,
		/* [8] */
		"right-to-left tab", noIcon, "`", noMark, plain,
		/* [9] */
		"enter", noIcon, "`", noMark, plain,
		/* [10] */
		"shift", noIcon, "`", noMark, plain,
		/* [11] */
		"control", noIcon, "`", noMark, plain,
		/* [12] */
		"option", noIcon, "`", noMark, plain,
		/* [13] */
		"null", noIcon, "`", noMark, plain,
		/* [14] */
		"space", noIcon, "`", noMark, plain,
		/* [15] */
		"delete left", noIcon, "`", noMark, plain,
		/* [16] */
		"left-to-right return", noIcon, "`", noMark, plain,
		/* [17] */
		"right-to-left return", noIcon, "`", noMark, plain,
		/* [18] */
		"nonmarking return", noIcon, "`", noMark, plain,
		/* [19] */
		"pencil", noIcon, "`", noMark, plain,
		/* [20] */
		"downward dashed arrow", noIcon, "`", noMark, plain,
		/* [21] */
		"command", noIcon, "`", noMark, plain,
		/* [22] */
		"checkmark", noIcon, "`", noMark, plain,
		/* [23] */
		"diamond", noIcon, "`", noMark, plain,
		/* [24] */
		"apple logo filled", noIcon, "`", noMark, plain,
		/* [25] */
		"delete right", noIcon, "`", noMark, plain,
		/* [26] */
		"leftward dashed arrow", noIcon, "`", noMark, plain,
		/* [27] */
		"upward dashed arrow", noIcon, "`", noMark, plain,
		/* [28] */
		"rightward dashed arrow", noIcon, "`", noMark, plain,
		/* [29] */
		"escape", noIcon, "`", noMark, plain,
		/* [30] */
		"clear", noIcon, "`", noMark, plain,
		/* [31] */
		"blank", noIcon, "`", noMark, plain,
		/* [32] */
		"page up", noIcon, "`", noMark, plain,
		/* [33] */
		"caps lock", noIcon, "`", noMark, plain,
		/* [34] */
		"left arrow", noIcon, "`", noMark, plain,
		/* [35] */
		"right arrow", noIcon, "`", noMark, plain,
		/* [36] */
		"northwest arrow", noIcon, "`", noMark, plain,
		/* [37] */
		"help", noIcon, "`", noMark, plain,
		/* [38] */
		"up arrow", noIcon, "`", noMark, plain,
		/* [39] */
		"southeast arrow", noIcon, "`", noMark, plain,
		/* [40] */
		"down arrow", noIcon, "`", noMark, plain,
		/* [41] */
		"page down", noIcon, "`", noMark, plain,
		/* [42] */
		"apple logo unfilled", noIcon, "`", noMark, plain,
		/* [43] */
		"contextual menu", noIcon, "`", noMark, plain,
		/* [44] */
		"power", noIcon, "`", noMark, plain,
		/* [45] */
		"f1", noIcon, "`", noMark, plain,
		/* [46] */
		"f2", noIcon, "`", noMark, plain,
		/* [47] */
		"f3", noIcon, "`", noMark, plain,
		/* [48] */
		"f4", noIcon, "`", noMark, plain,
		/* [49] */
		"f5", noIcon, "`", noMark, plain,
		/* [50] */
		"f6", noIcon, "`", noMark, plain,
		/* [51] */
		"f7", noIcon, "`", noMark, plain,
		/* [52] */
		"f8", noIcon, "`", noMark, plain,
		/* [53] */
		"f9", noIcon, "`", noMark, plain,
		/* [54] */
		"f10", noIcon, "`", noMark, plain,
		/* [55] */
		"f11", noIcon, "`", noMark, plain,
		/* [56] */
		"f12", noIcon, "`", noMark, plain,
		/* [57] */
		"f13", noIcon, "`", noMark, plain,
		/* [58] */
		"f14", noIcon, "`", noMark, plain,
		/* [59] */
		"f15", noIcon, "`", noMark, plain,
		/* [60] */
		"control key (ISO)", noIcon, "`", noMark, plain
	}
};

resource 'MENU' (149) {
	149,
	textMenuProc,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 17 elements */
		/* [1] */
		"Arrow", noIcon, noKey, noMark, plain,
		/* [2] */
		"CopyArrow", noIcon, noKey, noMark, plain,
		/* [3] */
		"AliasArrow", noIcon, noKey, noMark, plain,
		/* [4] */
		"ContextualMenuArrow", noIcon, noKey, noMark, plain,
		/* [5] */
		"IBeam", noIcon, noKey, noMark, plain,
		/* [6] */
		"Cross", noIcon, noKey, noMark, plain,
		/* [7] */
		"Plus", noIcon, noKey, noMark, plain,
		/* [8] */
		"Watch", noIcon, noKey, noMark, plain,
		/* [9] */
		"ClosedHand", noIcon, noKey, noMark, plain,
		/* [10] */
		"OpenHand", noIcon, noKey, noMark, plain,
		/* [11] */
		"PointingHand", noIcon, noKey, noMark, plain,
		/* [12] */
		"CountingUpHand", noIcon, noKey, noMark, plain,
		/* [13] */
		"CountingDownHand", noIcon, noKey, noMark, plain,
		/* [14] */
		"CountingUpAndDownHand", noIcon, noKey, noMark, plain,
		/* [15] */
		"Spinning", noIcon, noKey, noMark, plain,
		/* [16] */
		"ResizeLeft", noIcon, noKey, noMark, plain,
		/* [17] */
		"ResizeRight", noIcon, noKey, noMark, plain,
		/* [18] */
		"ResizeLeftRight", noIcon, noKey, noMark, plain
	}
};

resource 'WIND' (129) {
        {137, 122, 283, 458},
        1024,
        visible,
        goAway,
        0x0,
        "Dialog Simulator",
        staggerParentWindow
};

resource 'WIND' (130) {
        {54, 39, 335, 470},
        1024,
        visible,
        goAway,
        0x0,
        "CDEF Tester",
        staggerParentWindow
};

resource 'WIND' (131) {
        {95, 83, 169, 380},
        1059,
        visible,
        goAway,
        0x0,
        "Utility Window",
        staggerParentWindow
};

resource 'WIND' (132) {
        {133, 104, 207, 401},
        1075,
        visible,
        goAway,
        0x0,
        "Utility Window",
        staggerParentWindow
};


resource 'clut' (-10237) {
	{	/* array ColorSpec: 12 elements */
		/* [1] */
		0, 0, 0,
		/* [2] */
		65535, 65535, 65535,
		/* [3] */
		0, 0, 0,
		/* [4] */
		43690, 43690, 43690,
		/* [5] */
		65535, 65535, 65535,
		/* [6] */
		65535, 65535, 65535,
		/* [7] */
		43690, 43690, 43690,
		/* [8] */
		43690, 43690, 43690,
		/* [9] */
		21845, 21845, 21845,
		/* [10] */
		21845, 21845, 21845,
		/* [11] */
		56797, 56797, 56797,
		/* [12] */
		21845, 21845, 21845
	}
};

resource 'clut' (-10236) {
	{	/* array ColorSpec: 12 elements */
		/* [1] */
		0, 0, 0,
		/* [2] */
		43690, 43690, 43690,
		/* [3] */
		0, 0, 0,
		/* [4] */
		43690, 43690, 43690,
		/* [5] */
		65535, 65535, 65535,
		/* [6] */
		21845, 21845, 21845,
		/* [7] */
		43690, 43690, 43690,
		/* [8] */
		43690, 43690, 43690,
		/* [9] */
		21845, 21845, 21845,
		/* [10] */
		8738, 8738, 8738,
		/* [11] */
		21845, 21845, 21845,
		/* [12] */
		43690, 43690, 43690
	}
};

resource 'clut' (-10235) {
	{	/* array ColorSpec: 12 elements */
		/* [1] */
		21845, 21845, 21845,
		/* [2] */
		65535, 65535, 65535,
		/* [3] */
		21845, 21845, 21845,
		/* [4] */
		61166, 61166, 61166,
		/* [5] */
		61166, 61166, 61166,
		/* [6] */
		65535, 65535, 65535,
		/* [7] */
		65535, 65535, 65535,
		/* [8] */
		61166, 61166, 61166,
		/* [9] */
		61166, 61166, 61166,
		/* [10] */
		21845, 21845, 21845,
		/* [11] */
		65535, 65535, 65535,
		/* [12] */
		65535, 65535, 65535
	}
};

resource 'CNTL' (129) {
	{44, 16, 76, 216},
	0,
	visible,
	255,
	0,
	2050,
	0,
	"Green"
};

resource 'CNTL' (128) {
	{76, 16, 108, 216},
	0,
	visible,
	255,
	0,
	2049,
	0,
	"Red"
};

resource 'CNTL' (130) {
	{108, 16, 140, 216},
	0,
	visible,
	255,
	0,
	2048,
	0,
	"Blue"
};

resource 'CNTL' (131, purgeable) {
	{116, 200, 300, 216},
	0,
	visible,
	1,
	0,
	kControlSliderProc,
	0,
	""
};

resource 'CNTL' (132) {
	{268, 288, 288, 346},
	0,
	visible,
	100,
	0,
	4,
	0,
	"OK"
};

resource 'CNTL' (133) {
	{240, 16, 260, 219},
	0,
	visible,
	67,
	133,
	1009,
	0,
	"Peaches:"
};

resource 'CNTL' (134) {
	{116, 16, 140, 40},
	131,
	visible,
	128,
	-1,
	65,
	0,
	""
};

resource 'CNTL' (136, purgeable) {
	{116, 104, 168, 160},
	0,
	visible,
	128,
	3,
	65,
	0,
	""
};

resource 'CNTL' (135, purgeable) {
	{116, 48, 156, 88},
	0,
	visible,
	0,
	2,
	64,
	0,
	""
};

resource 'CNTL' (137) {
	{0, 0, 100, 100},
	0,
	visible,
	128,
	2,
	66,
	0,
	""
};

resource 'CNTL' (160) {
	{0, 0, 219, 349},
	0,
	visible,
	0,
	0,
	1104,
	0,
	"CNTL"
};

resource 'CNTL' (161) {
	{0, 0, 20, 100},
	0,
	visible,
	0,
	0,
	160,
	0,
	"CNTL"
};

resource 'CNTL' (162) {
	{0, 0, 20, 100},
	0,
	visible,
	0,
	0,
	160,
	0,
	"CNTL"
};

resource 'CNTL' (163) {
	{0, 0, 16, 150},
	50,
	visible,
	100,
	0,
	scrollBarProc,
	0,
	"CNTL"
};

resource 'CNTL' (164) {
	{0, 0, 16, 150},
	50,
	visible,
	100,
	0,
	scrollBarProc,
	0,
	"CNTL"
};

resource 'CNTL' (165) {
	{0, 0, 100, 16},
	50,
	visible,
	100,
	0,
	scrollBarProc,
	0,
	"CNTL"
};

resource 'CNTL' (166) {
	{0, 0, 16, 110},
	0,
	visible,
	2,
	0,
	radioButProc,
	0,
	"Radio Button"
};

resource 'CNTL' (167) {
	{0, 0, 16, 110},
	0,
	visible,
	2,
	0,
	checkBoxProc,
	0,
	"Check Box"
};

resource 'CNTL' (168) {
	{0, 0, 16, 300},
	50,
	visible,
	100,
	0,
	scrollBarProc,
	0,
	"CNTL"
};


resource 'CNTL' (150) {
	{0, 0, 24, 24},
	131,
	visible,
	128,
	3,
	36,
	0,
	""
};

resource 'CNTL' (151) {
	{0, 0, 36, 36},
	131,
	visible,
	128,
	259,
	37,
	0,
	""
};

resource 'CNTL' (152) {
	{0, 0, 48, 48},
	131,
	visible,
	128,
	514,
	42,
	0,
	""
};

resource 'CNTL' (153) {
	{0, 0, 150, 400},
	128,
	visible,
	0,
	0,
	136,
	0,
	""
};

resource 'CNTL' (155) {
	{0, 0, 16, 16},
	0,
	visible,
	0,
	0,
	112,
	0,
	"CNTL"
};

resource 'CNTL' (156) {
	{0, 0, 12, 150},
	0,
	visible,
	200,
	0,
	80,
	0,
	"CNTL"
};

resource 'CNTL' (157) {
	{0, 0, 16, 150},
	200,
	visible,
	200,
	0,
	80,
	0,
	"CNTL"
};

resource 'CNTL' (158) {
	{0, 0, 16, 150},
	150,
	visible,
	200,
	0,
	81,
	0,
	"CNTL"
};

resource 'CNTL' (159) {
	{0, 0, 16, 150},
	25,
	visible,
	200,
	0,
	81,
	0,
	"CNTL"
};

resource 'CNTL' (170) {
	{74, 17, 97, 30},
	0,
	visible,
	0,
	0,
	96,
	0,
	"CNTL"
};

resource 'CNTL' (169) {
	{13, 340, 163, 356},
	50,
	visible,
	100,
	0,
	scrollBarProc,
	0,
	"CNTL"
};

resource 'CNTL' (171) {
	{0, 0, 5, 150},
	0,
	visible,
	0,
	0,
	144,
	0,
	"CNTL"
};

resource 'CNTL' (172) {
	{0, 0, 150, 6},
	0,
	visible,
	0,
	0,
	144,
	0,
	"CNTL"
};

resource 'CNTL' (173) {
	{0, 0, 12, 12},
	0,
	visible,
	1,
	0,
	64,
	0,
	""
};

resource 'CNTL' (174) {
	{0, 0, 37, 37},
	129,
	visible,
	0,
	1,
	176,
	0,
	""
};

resource 'CNTL' (175, purgeable) {
	{120, 164, 200, 332},
	0,
	visible,
	0,
	0,
	160,
	0,
	""
};

resource 'CNTL' (176, purgeable) {
	{132, 168, 172, 240},
	0,
	visible,
	0,
	0,
	161,
	0,
	""
};

resource 'CNTL' (178) {
	{0, 0, 100, 200},
	133,
	visible,
	0,
	0,
	162,
	0,
	"Testing"
};

resource 'CNTL' (179) {
	{0, 0, 22, 100},
	0,
	visible,
	0,
	0,
	32,
	0,
	"Test"
	/****** Extra bytes follow... ******/
	/* $"0000 0000"                                          /* .... */
};

resource 'CNTL' (180) {
	{0, 0, 40, 40},
	0,
	visible,
	0,
	132,
	32,
	0,
	""
};

resource 'CNTL' (154) {
	{48, 224, 71, 237},
	0,
	visible,
	0,
	0,
	96,
	0,
	""
};

resource 'CNTL' (177, purgeable) {
	{8, 8, 200, 408},
	128,
	visible,
	100,
	0,
	128,
	0,
	""
};

resource 'CNTL' (181) {
	{0, 0, 20, 20},
	0,
	visible,
	0,
	0,
	64,
	0,
	"CNTL"
};

resource 'CNTL' (182) {
	{0, 0, 20, 20},
	0,
	visible,
	0,
	0,
	64,
	0,
	"CNTL"
};


resource 'CNTL' (201) {
	{110, 111, 194, 239},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Bevel"
};

resource 'CNTL' (203) {
	{214, 330, 298, 592},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Behavior"
};

resource 'CNTL' (205) {
	{-16, 0, 0, 100},
	0,
	visible,
	0,
	0,
	144,
	0,
	"CNTL"
};

resource 'CNTL' (210) {
	{209, 118, 228, 317},
	0,
	visible,
	60,
	134,
	401,
	0,
	"Variant:"
};

resource 'CNTL' (211) {
	{145, 120, 165, 274},
	0,
	visible,
	-1,
	135,
	401,
	0,
	"Direction:"
};

resource 'CNTL' (212) {
	{0, 0, 16, 100},
	0,
	visible,
	60,
	136,
	1009,
	0,
	"Content:"
};

resource 'CNTL' (213) {
	{0, 0, 19, 200},
	0,
	visible,
	-1,
	136,
	401,
	0,
	"Content:"
};

resource 'CNTL' (202) {
	{120, 8, 208, 172},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Content"
};

resource 'CNTL' (204) {
	{12, 232, 112, 444},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Menu"
};

resource 'CNTL' (209) {
	{120, 184, 208, 444},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Align Graphic"
};

resource 'CNTL' (214) {
	{12, 8, 31, 168},
	0,
	visible,
	50,
	137,
	401,
	0,
	"Bevel:"
};

resource 'CNTL' (215) {
	{60, 20, 80, 203},
	0,
	visible,
	45,
	138,
	401,
	0,
	"Type:"
};

resource 'CNTL' (216, purgeable) {
	{36, 8, 112, 220},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Behavior"
};

resource 'CNTL' (217, purgeable) {
	{144, 20, 168, 148},
	0,
	visible,
	0,
	139,
	401,
	0,
	""
};

resource 'CNTL' (218, purgeable) {
	{144, 196, 168, 344},
	0,
	visible,
	0,
	140,
	401,
	0,
	""
};

resource 'CNTL' (206) {
	{372, 340, 503, 444},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Place"
};

resource 'CNTL' (207) {
	{380, 244, 510, 370},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Align"
};

resource 'CNTL' (208) {
	{212, 8, 300, 264},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Text"
};

resource 'CNTL' (219, purgeable) {
	{236, 16, 256, 160},
	0,
	visible,
	50,
	141,
	401,
	0,
	"Align:"
};

resource 'CNTL' (220, purgeable) {
	{272, 16, 292, 192},
	0,
	visible,
	50,
	142,
	401,
	0,
	"Place:"
};

resource 'CNTL' (221, "Std Alert - Secondary group 1") {
	{243, 202, 275, 432},
	0,
	visible,
	0,
	0,
	164,
	0,
	""
};

resource 'CNTL' (222, "Std Alert - Secondary group 2") {
	{277, 202, 306, 431},
	0,
	visible,
	0,
	0,
	164,
	0,
	""
};

resource 'CNTL' (223, "Std Alert - Secondary Group 3") {
	{306, 202, 339, 431},
	0,
	visible,
	0,
	0,
	164,
	0,
	""
};

resource 'CNTL' (224, "Std Alert - Buttons group") {
	{238, 114, 378, 473},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Buttons"
};

resource 'CNTL' (225, "Std Alert - Type popup") {
	{134, 204, 153, 323},
	0,
	visible,
	0,
	144,
	401,
	0,
	""
};

resource 'CNTL' (226, "Live Feedback - Scroll Bar") {
	{249, 180, 265, 400},
	0,
	visible,
	100,
	0,
	386,
	0,
	""
};

resource 'CNTL' (227, "Live Feedback - Slider") {
	{196, 117, 213, 336},
	0,
	visible,
	100,
	0,
	57,
	0,
	"CNTL"
};

resource 'CNTL' (228, "Mega Dialog - Tabs") {
	{10, 0, 262, 576},
	6000,
	visible,
	0,
	0,
	kControlTabSmallNorthProc,
	0,
	"CNTL"
};



resource 'CNTL' (233) {
	{175, 90, 198, 232},
	0,
	visible,
	100,
	0,
	52,
	0,
	""
};

resource 'CNTL' (237) {
	{0, 0, 100, 16},
	0,
	visible,
	100,
	0,
	384,
	0,
	""
};

resource 'CNTL' (39) {
	{0, 0, 22, 60},
	0,
	visible,
	1,
	0,
	368,
	0,
	"CNTL"
};

resource 'CNTL' (246) {
	{0, 0, 120, 160},
	0,
	visible,
	0,
	0,
	1584,
	0,
	""
};

resource 'CNTL' (250) {
	{158, 324, 212, 472},
	0,
	visible,
	0,
	0,
	162,
	0,
	"Secondary"
};

resource 'CNTL' (264) {
	{154, 179, 174, 269},
	0,
	visible,
	-6046,
	0,
	374,
	0,
	""
};

resource 'CNTL' (280) {
	{0, 0, 3, 100},
	0,
	visible,
	0,
	0,
	144,
	0,
	"CNTL"
};

resource 'CNTL' (281, "Radio Group - Divider Dialog") {
	{128, 109, 179, 225},
	0,
	visible,
	0,
	0,
	416,
	0,
	""
};

resource 'CNTL' (282, "Radio Group - Group Box (CDEF Tester)") {
	{129, 113, 176, 249},
	0,
	visible,
	0,
	0,
	416,
	0,
	""
};

resource 'CNTL' (283, "CDEF Tester - RadioGroup (Scroll Bars)") {
	{132, 111, 177, 225},
	0,
	visible,
	0,
	0,
	416,
	0,
	""
};

resource 'CNTL' (287, "Tab - RadioGroup Size") {
	{279, 26, 300, 175},
	0,
	visible,
	0,
	0,
	416,
	0,
	""
};

resource 'CNTL' (288) {
	{192, 351, 237, 451},
	0,
	visible,
	0,
	0,
	176,
	0,
	"CurrentIcon"
};

resource 'CNTL' (289) {
	{73, 7, 91, 144},
	0,
	visible,
	1,
	0,
	371,
	0,
	"Window Modified"
};

resource 'CNTL' (290) {
	{0, 0, 20, 140},
	0,
	visible,
	1,
	0,
	371,
	0,
	"Drag Destination?"
};

resource 'STR#' (129) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Show Disabled Look",
		/* [2] */
		"Show Enabled Look"
	}
};

resource 'STR#' (128, "Startup Errors") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Unable to start the application because "
		"your system is older than God (or approp"
		"riate deity).",
		/* [2] */
		"Unable to start the application because "
		"the Appearance extension is not installe"
		"d.",
		/* [3] */
		"Unable to start the application because "
		"a necessary resource was missing."
	}
};

	resource 'STR#' (130, "List Box examples") {
		{
			"List box item 1",
			"List box item 2",
			"Yes, the 3rd list item",
			"4 times a list box item",
			"5, 5, 5 items in the list box",
			"This is item number 6. Oh yes.",
			"The 7th item is a charm"
		}
	};

resource 'STR#' (6000, "Mega Dialog Tab Help Text") {
	{
		"The usual suspects",
		"The slickers",
		"The forward thinkers",
		"The intellectuals",
		"The collective",
		"The misfits",
		"Hey, a disabled tab!"
	}
};

resource 'STR#' (6001, "Mega Dialog Tab Help Text") {
	{
		"The usual suspects, expanded",
		"The slickers, expanded",
		"The forward thinkers, expanded",
		"The intellectuals, expanded",
		"The collective, expanded",
		"The misfits, expanded",
		"Hey, a disabled tab!, expanded"
	}
};

resource 'STR ' (130, "Examples Menu Title Help Text") {
	"Show different Appearance stuff"
};

resource 'STR ' (6000, "Mega Dialog Window Help Text") {
	"Pick a tab, any tab"
};

resource 'STR#' (6003, "Mega Dialog Classic Pane Help Text") {
	{
		"Aye, Aye, Sir!",
		"Roger!",
		"We’re all gonna die!",
		"Shields Up"
	}
};

resource 'ics#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"7000 FC00 BFC0 8E20 8230 81F8 8038 8038"
		$"8038 8038 C038 7038 1C3C 073F 01FF 007C",
		/* [2] */
		$"7000 FC00 FFC0 FFE0 FFF0 FFF8 FFF8 FFF8"
		$"FFF8 FFF8 FFF8 7FF8 1FFC 07FF 01FF 007C"
	}
};

resource 'ics#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 0FE0 0770 0770 0770 07E0"
		$"0770 0770 0770 0FE0",
		/* [2] */
		$"0000 0000 0000 0FE0 0770 0770 0770 07E0"
		$"0770 0770 0770 0FE0"
	}
};

resource 'ics#' (130) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 00FC 0030 0060 00C0 0180"
		$"0300 0600 0C00 3F",
		/* [2] */
		$"0000 0000 0000 00FC 0030 0060 00C0 0180"
		$"0300 0600 0C00 3F"
	}
};

resource 'ics#' (131) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"0000 0000 0000 1CE0 0C60 0C60 0C60 0C60"
		$"0C60 07E0 0000 1FF0"
	}
};

resource 'ics#' (132) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"0000 0000 0000 0000 0000 3F80 0000 3FFC"
		$"0000 3FE0 0000 3FFC"
	}
};

resource 'ics#' (133) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"0000 0000 0000 0000 07C0 0000 1FF0 0000"
		$"07C0 0000 3FF8"
	}
};

resource 'ics#' (134) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"0000 0000 0000 0000 0000 01FC 0000 3FFC"
		$"0000 07FC 0000 3FFC"
	}
};

resource 'ics#' (135) {
	{	/* array: 2 elements */
		/* [1] */
		$"",
		/* [2] */
		$"0000 0000 0000 0000 3FFC 0000 3FFC 0000"
		$"3FFC 0000 3FFC"
	}
};

resource 'ics8' (128) {
	$"00FF FFFF 0000 0000 0000 0000 0000 0000"
	$"FFFC FCFC FFFF 0000 0000 0000 0000 0000"
	$"FF2A FCFC FCFC FFFF FFFF 0000 0000 0000"
	$"FF2A 2A2A FCFC FC54 5454 FF00 0000 0000"
	$"FF2A 2A2A 2A2A FC54 5454 80FF 0000 0000"
	$"FF2A 2A2A 2A2A 54FC FCFC FCFF FF00 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A FCFF FF00 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 7FFF FF00 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 7FFF FF00 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 7FFF FF00 0000"
	$"FFFC 542A 2A2A 2A2A 2A2A 7FFF FF00 0000"
	$"00FF FFFC 542A 2A2A 2A2A 7FFF FF00 0000"
	$"0000 00FF FFFC 542A 2A2A 7FFF FFFC 0000"
	$"0000 0000 00FF FFFC 542A 7FFF FFFC FCFC"
	$"0000 0000 0000 00FF FFFC 7FFF FFFC FCFC"
	$"0000 0000 0000 0000 00FF FFFF FFFC"
};

resource 'ics8' (129) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 FFFF FFFF FFFF FF00 0000 0000"
	$"0000 0000 00FF FFFF 00FF FFFF 0000 0000"
	$"0000 0000 00FF FFFF 00FF FFFF 0000 0000"
	$"0000 0000 00FF FFFF 00FF FFFF 0000 0000"
	$"0000 0000 00FF FFFF FFFF FF00 0000 0000"
	$"0000 0000 00FF FFFF 00FF FFFF 0000 0000"
	$"0000 0000 00FF FFFF 00FF FFFF 0000 0000"
	$"0000 0000 00FF FFFF 00FF FFFF 0000 0000"
	$"0000 0000 FFFF FFFF FFFF FF"
};

resource 'ics8' (130) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 FFFF 0000 0000"
	$"0000 0000 0000 0000 00FF FF00 0000 0000"
	$"0000 0000 0000 0000 FFFF 0000 0000 0000"
	$"0000 0000 0000 00FF FF00 0000 0000 0000"
	$"0000 0000 0000 FFFF 0000 0000 0000 0000"
	$"0000 0000 00FF FF00 0000 0000 0000 0000"
	$"0000 0000 FFFF 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF"
};

resource 'ics8' (131) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 00FF FFFF 0000 FFFF FF00 0000 0000"
	$"0000 0000 FFFF 0000 00FF FF00 0000 0000"
	$"0000 0000 FFFF 0000 00FF FF00 0000 0000"
	$"0000 0000 FFFF 0000 00FF FF00 0000 0000"
	$"0000 0000 FFFF 0000 00FF FF00 0000 0000"
	$"0000 0000 FFFF 0000 00FF FF00 0000 0000"
	$"0000 0000 00FF FFFF FFFF FF00 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 00FF FFFF FFFF FFFF FFFF"
};

resource 'ics8' (132) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FF00 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FF00 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'ics8' (133) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 00FF FFFF FFFF 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 00FF FFFF FFFF FFFF FFFF 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 00FF FFFF FFFF 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FF"
};

resource 'ics8' (134) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 00FF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 00FF FFFF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'ics8' (135) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF"
};



resource 'icl8' (128) {
	$"0000 FFFF FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 FF2A 54FC FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"FFFF FC54 542A 54FC FF00 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"FF00 54FC FC54 542A 54FC FF00 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"FF2A 2A00 54FC FC54 542A 54FC FF00 00FF"
	$"FFFF 0000 0000 0000 0000 0000 0000 0000"
	$"FF2A 2A2A 2A00 54FC FC54 542A 54FC FF54"
	$"2A54 FCFF 0000 0000 0000 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A00 54FC FC54 542A 5454"
	$"5454 2A54 FCFC 0000 0000 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A00 54FC FC54 5454"
	$"5454 5454 2AFF 0000 0000 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A00 FC54 5454"
	$"5454 5454 5480 FC00 0000 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A FCFC 5454"
	$"5454 5454 5454 FF00 0000 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 0054 FCFC"
	$"54FC FC54 5454 54FF 0000 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 0054"
	$"FCFC 54FC FC54 542A FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A00 54FC 5454 FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A00 FF54 FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"FF2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"FF7F 542A 2A2A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"00FF FC7F 542A 2A2A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"0000 00FF FC7F 542A 2A2A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"0000 0000 00FF FC7F 542A 2A2A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FF00 0000 0000 0000"
	$"0000 0000 0000 00FF FC7F 542A 2A2A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FFFC 0000 0000 0000"
	$"0000 0000 0000 0000 00FF FC7F 542A 2A2A"
	$"2A2A 2A2A 2A7F FF7F FFFC FCFC 0000 0000"
	$"0000 0000 0000 0000 0000 00FF FC7F 542A"
	$"2A2A 2A2A 2A7F FF7F FFFC FCFC FCFC 0000"
	$"0000 0000 0000 0000 0000 0000 00FF FC7F"
	$"542A 2A2A 2A7F FF7F FFFC FCFC FCFC FC00"
	$"0000 0000 0000 0000 0000 0000 0000 00FF"
	$"FC7F 542A 2A7F FF7F FFFC FCFC FCFC FC00"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"00FF FC7F 7F7F FF7F FFFC FCFC FCFC 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 00FF FC7F FF7F FFFC FCFC 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 00FF FFFF FCFC"
};

resource 'icl4' (128) {
	$"00FF F000 0000 0000 0000 0000 0000 0000"
	$"00FC CEF0 0000 0000 0000 0000 0000 0000"
	$"FFEC CCCE F000 0000 0000 0000 0000 0000"
	$"F0CE ECCC CEF0 0000 0000 0000 0000 0000"
	$"FCC0 CEEC CCCE F00F FF00 0000 0000 0000"
	$"FCCC C0CE ECCC CEFC CCEF 0000 0000 0000"
	$"FCCC CCC0 CEEC CCCC CCCC EE00 0000 0000"
	$"FCCC CCCC C0CE ECCC CCCC CF00 0000 0000"
	$"FCCC CCCC CCC0 ECCC CCCC CDE0 0000 0000"
	$"FCCC CCCC CCCC EECC CCCC CCF0 0000 0000"
	$"FCCC CCCC CCCC 0CEE CEEC CCCF 0000 0000"
	$"FCCC CCCC CCCC CC0C EECE ECCC F000 0000"
	$"FCCC CCCC CCCC CCCC CCC0 CECC F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC C0FC F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"FCCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"FDCC CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"0FED CCCC CCCC CCCC CCCC CDFD F000 0000"
	$"000F EDCC CCCC CCCC CCCC CDFD F000 0000"
	$"0000 0FED CCCC CCCC CCCC CDFD F000 0000"
	$"0000 000F EDCC CCCC CCCC CDFD FE00 0000"
	$"0000 0000 0FED CCCC CCCC CDFD FEEE 0000"
	$"0000 0000 000F EDCC CCCC CDFD FEEE EE00"
	$"0000 0000 0000 0FED CCCC CDFD FEEE EEE0"
	$"0000 0000 0000 000F EDCC CDFD FEEE EEE0"
	$"0000 0000 0000 0000 0FED DDFD FEEE EE00"
	$"0000 0000 0000 0000 000F EDFD FEEE 0000"
	$"0000 0000 0000 0000 0000 0FFF EE"
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"3800 0000 2600 0000 E180 0000 9860 0000"
		$"8619 C000 8186 3000 8060 0C00 8018 0400"
		$"8008 0600 800C 0200 8003 6100 8000 D880"
		$"8000 0480 8000 0280 8000 0780 8000 0780"
		$"8000 0780 8000 0780 8000 0780 8000 0780"
		$"C000 0780 7000 0780 1C00 0780 0700 0780"
		$"01C0 07C0 0070 07F0 001C 07FC 0007 07FE"
		$"0001 C7FE 0000 7FFC 0000 1FF0 0000 07C0",
		/* [2] */
		$"3800 0000 3E00 0000 FF80 0000 FFE0 0000"
		$"FFF9 C000 FFFF F000 FFFF FC00 FFFF FC00"
		$"FFFF FE00 FFFF FE00 FFFF FF00 FFFF FF80"
		$"FFFF FF80 FFFF FF80 FFFF FF80 FFFF FF80"
		$"FFFF FF80 FFFF FF80 FFFF FF80 FFFF FF80"
		$"FFFF FF80 7FFF FF80 1FFF FF80 07FF FF80"
		$"01FF FFC0 007F FFF0 001F FFFC 0007 FFFE"
		$"0001 FFFE 0000 7FFC 0000 1FF0 0000 07C0"
	}
};

resource 'ics4' (128) {
	$"0FFF 0000 0000 0000 FEEE FF00 0000 0000"
	$"FCEE EEFF FF00 0000 FCCC EEEC CCF0 0000"
	$"FCCC CCEC CCDF 0000 FCCC CCCE EEEF F000"
	$"FCCC CCCC CCEF F000 FCCC CCCC CCDF F000"
	$"FCCC CCCC CCDF F000 FCCC CCCC CCDF F000"
	$"FECC CCCC CCDF F000 0FFE CCCC CCDF F000"
	$"000F FECC CCDF FE00 0000 0FFE CCDF FEEE"
	$"0000 000F FEDF FEEE 0000 0000 0FFF FE"
};

resource 'ics4' (130) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 FFFF FF00"
	$"0000 0000 00FF 0000 0000 0000 0FF0 0000"
	$"0000 0000 FF00 0000 0000 000F F000 0000"
	$"0000 00FF 0000 0000 0000 0FF0 0000 0000"
	$"0000 FF00 0000 0000 00FF FFFF"
};

resource 'cicn' (128) {
	4,
	{0, 0, 12, 16},
	2,
	$"FFFC FFFC FFFC FFFF FFFF FFFF FFFF FFFF"
	$"1FFF 1FFF 1FFF 0000",
	$"FFFC FFFC FFFC FFFF F001 F001 F001 F001"
	$"1001 1001 1FFF 0000",
	{	/* array ColorSpec: 4 elements */
		/* [1] */
		65535, 65535, 65535,
		/* [2] */
		43690, 43690, 43690,
		/* [3] */
		21845, 21845, 21845,
		/* [4] */
		0, 0, 0
	},
	$"FFFF FFF0 EAAA AAB0 EAAA AAB0 EBFF FFFF"
	$"EB55 5557 EB55 5557 EB55 5557 FF55 5557"
	$"0355 5557 0355 5557 03FF FFFF 0000 0000"
};

data 'ppat' (128) {
	$"0001 0000 001C 0000 004E 0000 0000 FFFF"            /* .........N....ùù */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"8001 0000 0000 0008 0008 0000 0000 0000"            /* ƒ............... */
	$"0000 0048 0000 0048 0000 0000 0001 0001"            /* ...H...H........ */
	$"0001 0000 0000 0000 0056 0000 0000 0000"            /* .........V...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 8888 0000"                                     /* ..‡‡.. */
};

resource 'DLOG' (1000) {
	{40, 40, 356, 414},
	dBoxProc,
	visible,
	goAway,
	0x0,
	1000,
	"Standard Alert",
	alertPositionParentWindowScreen
};

resource 'DLOG' (6000, "Edit Text") {
	{66, 42, 325, 42+576},
	kWindowDocumentProc,
	visible,
	goAway,
	0x0,
	6000,
	"Mega Dialog",
	noAutoCenter
};

resource 'DLOG' (4000, "AutoSize Tester") {
  {40, 40, 136, 408},
  1042,
  visible,
  goAway,
  0x0,
  4000,   
  "",
  noAutoCenter
};
  

resource 'DLOG' (7000) {
  {40, 40, 240, 280}, 
  1046,
  visible,
  goAway,
  0x0,
  7000,
  "",
  noAutoCenter
};


resource 'DLOG' (2020, "ThemeColorDialog") {
  {106, 171, 306, 411},
  documentProc,
  visible,
  goAway,
  0x0,
  2020,
  "ThemeColorDialog",
  noAutoCenter
};

resource 'DLOG' (3000) {
	{107, 74, 224, 535},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	3000,
	"",
	alertPositionMainScreen
};

resource 'DLOG' (1001, "Bevel Button dialog") {
	{69, 135, 401, 548},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	1001,
	"Bevel Buttons",
	staggerParentWindow
	/****** Extra bytes follow... ******/
	/* $"0001"                                               /* .. */
};

resource 'dlgx' (4000) {
	versionZero {
		11
	}
};

resource 'dlgx' (7000) {
	versionZero {
		11
	}
};

resource 'dlgx' (281) {
	versionZero {
		15
	}
};

resource 'dlgx' (282) {
	versionZero {
		15
	}
};

resource 'dlgx' (283) {
	versionZero {
		15
	}
};


resource 'dlgx' (128) {
	versionZero {
		0
	}
};

resource 'dlgx' (1000) {
	versionZero {
		11
	}
};

resource 'dlgx' (1001) {
	versionZero {
		11
	}
};

resource 'xmnu' (131, purgeable) {
	versionZero {
		{	/* array ItemExtensions: 10 elements */
			/* [1] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [2] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [3] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [4] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [5] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [6] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [7] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [8] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [9] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			},
			/* [10] */
			dataItem {
				0,
				0x0,
				0,
				0,
				0,
				noHierID,
				sysFont,
				naturalGlyph
			}
		}
	}
};

resource 'STR#' (4500) {
	{	/* array StringArray: 24 elements */
		/* [1] */
		"kHMDefaultSide",
		/* [2] */
		"kHMOutsideTopScriptAligned",
		/* [3] */
		"kHMOutsideLeftCenterAligned",
		/* [4] */
		"kHMOutsideBottomScriptAligned",
		/* [5] */
		"kHMOutsideRightCenterAligned",
		/* [6] */
		"kHMOutsideTopLeftAligned",
		/* [7] */
		"kHMOutsideTopRightAligned",
		/* [8] */
		"kHMOutsideLeftTopAligned",
		/* [9] */
		"kHMOutsideLeftBottomAligned",
		/* [10] */
		"kHMOutsideBottomLeftAligned",
		/* [11] */
		"kHMOutsideBottomRightAligned",
		/* [12] */
		"kHMOutsideRightTopAligned",
		/* [13] */
		"kHMOutsideRightBottomAligned",
		/* [14] */
		"kHMOutsideTopCenterAligned",
		/* [15] */
		"kHMOutsideBottomCenterAligned",
		/* [16] */
		"kHMInsideRightCenterAligned",
		/* [17] */
		"kHMInsideLeftCenterAligned",
		/* [18] */
		"kHMInsideBottomCenterAligned",
		/* [19] */
		"kHMInsideTopCenterAligned",
		/* [20] */
		"kHMInsideTopLeftCorner",
		/* [21] */
		"kHMInsideTopRightCorner",
		/* [22] */
		"kHMInsideBottomLeftCorner",
		/* [23] */
		"kHMInsideBottomRightCorner",
		/* [24] */
		"kHMAbsoluteCenterAligned"		
	}
};
