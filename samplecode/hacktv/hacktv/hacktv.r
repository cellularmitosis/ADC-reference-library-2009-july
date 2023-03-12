#define SystemSevenOrLater 1
#include "ConditionalMacros.r"
#include "Types.r"
#include "Menus.r"
#include "Dialogs.r"
#include "Finder.r"

resource 'MENU' (128) {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About Hack TV…", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129) {
	129,
	textMenuProc,
	0x7FFFFFFB,
	enabled,
	"File",
	{	/* array: 4 elements */
		/* [1] */
		"Page Setup…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Print", noIcon, "P", noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130) {
	130,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"Edit",
	{	/* array: 6 elements */
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
		"Clear", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (131) {
	131,
	textMenuProc,
	0x7FFFFDDB,
	enabled,
	"Monitor",
	{	/* array: 11 elements */
		/* [1] */
		"Video Settings…", noIcon, noKey, noMark, plain,
		/* [2] */
		"Sound Settings…", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Record Video", noIcon, noKey, noMark, plain,
		/* [5] */
		"Record Sound", noIcon, noKey, noMark, plain,
		/* [6] */
		"Split Track Files", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Quarter Size", noIcon, noKey, noMark, plain,
		/* [9] */
		"Half Size", noIcon, noKey, noMark, plain,
		/* [10] */
		"Full Size", noIcon, noKey, noMark, plain,
		/* [11] */
		"-", noIcon, noKey, noMark, plain,
		/* [12] */
		"Record", noIcon, noKey, noMark, plain
	}
};

resource 'MBAR' (128) {
	{	/* array MenuArray: 4 elements */
		/* [1] */
		128,
		/* [2] */
		129,
		/* [3] */
		130,
		/* [4] */
		131
	}
};

resource 'DLOG' (129) {
	{62, 81, 82, 101},
	movableDBoxProc,
	invisible,
	noGoAway,
	0x0,
	129,
	"Monitor",
	alertPositionMainScreen
};

resource 'DLOG' (128) {
	{80, 96, 242, 385},
	dBoxProc,
	visible,
	goAway,
	0x0,
	128,
	"",
	noAutoCenter
};

resource 'DITL' (129) {
	{	/* array DITLarray: 0 elements */
	}
};

resource 'DITL' (128) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{130, 112, 150, 176},
		Button {
			enabled,
			"So What"
		},
		/* [2] */
		{107, 112, 127, 176},
		UserItem {
			disabled
		},
		/* [3] */
		{8, 8, 40, 282},
		StaticText {
			disabled,
			"Hack TV with softVdig, \nthe all purpose "
			"software video digitizer"
		},
		/* [4] */
		{48, 8, 120, 254},
		StaticText {
			disabled,
			"Brought to a shambling mockery of life b"
			"y Peter Hoddie, Casey King, and Gary Woo"
			"dcock.  Brought back to life by Brian S. Friedkin."
		}
	}
};

resource 'dctb' (129) {
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

resource 'DITL' (130) {
	{
/* [1] */ {78, 273, 98, 341}, Button {enabled, "OK"},
/* [2] */ {13, 65, 61, 348}, StaticText {disabled, "Your movie has been recorded."},
	}
};

resource 'ALRT' (130) {
	{0, 0, 110, 356},
	130,
	{
/* [1] */ OK, visible, 0,
/* [2] */ OK, visible, sound1,
/* [3] */ OK, visible, sound1,
/* [4] */ OK, visible, sound1
	},
	alertPositionParentWindow
};

resource 'BNDL' (128) {
	'aptv',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128
		}
	}
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'vers' (1) {
	0x1,
	0x0,
	development,
	0x1,
	0,
	"1.0d2",
	"1.0d2, © Apple Computer, Inc., 1992-1998"
};

resource 'vers' (2) {
	0x1,
	0x0,
	development,
	0x1,
	0,
	"1.0d2",
	"by Videos 'R' Us"
};

data 'aptv' (0, "Owner resource") {
	$"20A9 2031 3939 322D 3139 3938 2041 7070"            /*  © 1992-1998 App */
	$"6C65 2043 6F6D 7075 7465 722C 2049 6E63"            /* le Computer, Inc */
	$"2E"                                                 /* . */
};

resource 'ICN#' (200) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 0000 0000 0000 0001 0000"
		$"0001 0000 0001 0000 000F E000 0007 C000"
		$"0003 8000 0001 0000 01FF FF00 0100 0180"
		$"0100 0180 0100 0180 013F F980 113F F988"
		$"1923 898C FD03 81FE FF03 81FF FD03 81FE"
		$"1903 818C 1103 8188 0103 8180 0103 8180"
		$"0100 0180 0100 0180 01FF FF80 00FF FF80",
		/* [2] */
		$"0000 0000 0000 0000 0000 0000 0001 0000"
		$"0001 0000 0001 0000 000F E000 0007 C000"
		$"0003 8000 0001 0000 01FF FF00 01FF FF80"
		$"01FF FF80 01FF FF80 01FF FF80 11FF FF88"
		$"19FF FF8C FDFF FFFE FFFF FFFF FDFF FFFE"
		$"19FF FF8C 11FF FF88 01FF FF80 01FF FF80"
		$"01FF FF80 01FF FF80 01FF FF80 00FF FF80"
	}
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0001 0000 0002 8000 0004 4000 0008 2000"
		$"0010 1000 0020 0800 0040 0400 0080 0200"
		$"0100 0100 0200 0080 0400 0040 0800 0020"
		$"1000 0010 2000 0008 4000 3F04 8000 4082"
		$"4000 8041 2001 3022 1001 C814 080E 7F8F"
		$"0402 3007 0201 0007 0100 8007 0080 6007"
		$"0040 1FE7 0020 021F 0010 0407 0008 0800"
		$"0004 1000 0002 2000 0001 4000 0000 80",
		/* [2] */
		$"0001 0000 0003 8000 0007 C000 000F E000"
		$"001F F000 003F F800 007F FC00 00FF FE00"
		$"01FF FF00 03FF FF80 07FF FFC0 0FFF FFE0"
		$"1FFF FFF0 3FFF FFF8 7FFF FFFC FFFF FFFE"
		$"7FFF FFFF 3FFF FFFE 1FFF FFFC 0FFF FFFF"
		$"07FF FFFF 03FF FFFF 01FF FFFF 00FF FFFF"
		$"007F FFFF 003F FE1F 001F FC07 000F F800"
		$"0007 F000 0003 E000 0001 C000 0000 80"
	}
};

resource 'icl8' (128) {
	$"0000 0000 0000 0000 0000 0000 0000 00FF"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 FFF5"
	$"FF00 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 00FF F5F5"
	$"F5FF 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 FFF5 F5F5"
	$"F5F5 FF00 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 00FF F5F5 F5F5"
	$"F5F5 F5FF 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 FFF5 F5F5 F5F5"
	$"F5F5 F5F5 FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 00FF F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFF5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 FF00 0000 0000 0000 0000"
	$"0000 0000 0000 00FF F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5FF 0000 0000 0000 0000"
	$"0000 0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 FF00 0000 0000 0000"
	$"0000 0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 FFFF FFFF FFFF F5F5 F5F5 F5FF 0000"
	$"FFF5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5FF 0808 0808 0808 FFF5 F5F5 F5F5 FF00"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"FF08 0808 0808 0808 08FF F5F5 F5F5 F5FF"
	$"0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5 F5FF"
	$"0808 FFFF 0808 0808 0808 FFF5 F5F5 FF00"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF"
	$"FFFF F5F5 FF08 0808 0808 08FF F5FF 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 FFFF FF08"
	$"08FF FFFF FFFF FFFF FF08 0808 FFFF FFFF"
	$"0000 0000 00FF F5F5 F5F5 F5F5 F5F5 FF08"
	$"0808 FFFF 0808 0808 0808 0808 08FF FFFF"
	$"0000 0000 0000 FFF5 F5F5 F5F5 F5F5 F5FF"
	$"0808 0808 0808 0808 0808 0808 08FF FFFF"
	$"0000 0000 0000 00FF F5F5 F5F5 F5F5 F5F5"
	$"FF08 0808 0808 0808 0808 0808 08FF FFFF"
	$"0000 0000 0000 0000 FFF5 F5F5 F5F5 F5F5"
	$"F5FF FF08 0808 0808 0808 0808 08FF FFFF"
	$"0000 0000 0000 0000 00FF F5F5 F5F5 F5F5"
	$"F5F5 F5FF FFFF FFFF FFFF FF08 08FF FFFF"
	$"0000 0000 0000 0000 0000 FFF5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 FF00 0000 00FF FFFF FFFF"
	$"0000 0000 0000 0000 0000 00FF F5F5 F5F5"
	$"F5F5 F5F5 F5FF 0000 0000 0000 00FF FFFF"
	$"0000 0000 0000 0000 0000 0000 FFF5 F5F5"
	$"F5F5 F5F5 FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 00FF F5F5"
	$"F5F5 F5FF 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 FFF5"
	$"F5F5 FF00 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 00FF"
	$"F5FF 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"FF"
};

resource 'icl4' (128) {
	$"0000 0000 0000 000F 0000 0000 0000 0000"
	$"0000 0000 0000 00FC F000 0000 0000 0000"
	$"0000 0000 0000 0FCC CF00 0000 0000 0000"
	$"0000 0000 0000 FCCC CCF0 0000 0000 0000"
	$"0000 0000 000F CCCC CCCF 0000 0000 0000"
	$"0000 0000 00FC CCCC CCCC F000 0000 0000"
	$"0000 0000 0FCC CCCC CCCC CF00 0000 0000"
	$"0000 0000 FCCC CCCC CCCC CCF0 0000 0000"
	$"0000 000F CCCC CCCC CCCC CCCF 0000 0000"
	$"0000 00FC CCCC CCCC CCCC CCCC F000 0000"
	$"0000 0FCC CCCC CCCC CCCC CCCC CF00 0000"
	$"0000 FCCC CCCC CCCC CCCC CCCC CCF0 0000"
	$"000F CCCC CCCC CCCC CCCC CCCC CCCF 0000"
	$"00FC CCCC CCCC CCCC CCCC CCCC CCCC F000"
	$"0FCC CCCC CCCC CCCC CCFF FFFF CCCC CF00"
	$"FCCC CCCC CCCC CCCC CFBB BBBB FCCC CCF0"
	$"0FCC CCCC CCCC CCCC FBBB BBBB BFCC CCCF"
	$"00FC CCCC CCCC CCCF BBFF BBBB BBFC CCF0"
	$"000F CCCC CCCC CCCF FF00 FBBB BBBF CF00"
	$"0000 FCCC CCCC FFFB BFFF FFFF FBBB FFFF"
	$"0000 0FCC CCCC CCFB BBFF BBBB BBBB BFFF"
	$"0000 00FC CCCC CCCF BBBB BBBB BBBB BFFF"
	$"0000 000F CCCC CCCC FBBB BBBB BBBB BFFF"
	$"0000 0000 FCCC CCCC CFFB BBBB BBBB BFFF"
	$"0000 0000 0FCC CCCC CCCF FFFF FFFB BFFF"
	$"0000 0000 00FC CCCC CCCC CCF0 000F FFFF"
	$"0000 0000 000F CCCC CCCC CF00 0000 0FFF"
	$"0000 0000 0000 FCCC CCCC F000 0000 0000"
	$"0000 0000 0000 0FCC CCCF 0000 0000 0000"
	$"0000 0000 0000 00FC CCF0 0000 0000 0000"
	$"0000 0000 0000 000F CF00 0000 0000 0000"
	$"0000 0000 0000 0000 F0"
};

resource 'ics#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0180 02C0 0460 0830 1018 200C 4006 80FB"
		$"C1CD 63FF 3143 18C3 0C7F 0623 0340 0180",
		/* [2] */
		$"0180 03C0 07E0 0FF0 1FF8 3FFC 7FFE FFFF"
		$"FFFF 7FFF 3FFF 1FFF 0FFF 07E3 03C0 0180"
	}
};

resource 'ics8' (128) {
	$"0000 0000 0000 00FF FF00 0000 0000 0000"
	$"0000 0000 0000 FFF5 FFFF 0000 0000 0000"
	$"0000 0000 00FF F5F5 F5FF FF00 0000 0000"
	$"0000 0000 FFF5 F5F5 F5F5 FFFF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5FF FF00 0000"
	$"0000 FFF5 F5F5 F5F5 F5F5 F5F5 FFFF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF FF00"
	$"FFF5 F5F5 F5F5 F5F5 FFFF FFFF FFF5 FFFF"
	$"FFFF F5F5 F5F5 F5FF FFFF 0808 FFFF F5FF"
	$"00FF FFF5 F5F5 FFFF FFFF FFFF FFFF FFFF"
	$"0000 FFFF F5F5 F5FF 08FF 0808 0808 FFFF"
	$"0000 00FF FFF5 F5F5 FFFF 0808 0808 FFFF"
	$"0000 0000 FFFF F5F5 F5FF FFFF FFFF FFFF"
	$"0000 0000 00FF FFF5 F5F5 FF00 0000 FFFF"
	$"0000 0000 0000 FFFF F5FF 0000 0000 0000"
	$"0000 0000 0000 00FF FF"
};

resource 'ics4' (128) {
	$"0000 000F F000 0000 0000 00F0 FF00 0000"
	$"0000 0F00 0FF0 0000 0000 F000 00FF 0000"
	$"000F 0000 000F F000 00F0 0000 0000 FF00"
	$"0F00 0000 0000 0FF0 F000 0000 FFFF F0FF"
	$"FF00 000F FFCC FF0F 0FF0 00FF FFFF FFFF"
	$"00FF 000F CFCC CCFF 000F F000 FFCC CCFF"
	$"0000 FF00 0FFF FFFF 0000 0FF0 00F0 00FF"
	$"0000 00FF 0F00 0000 0000 000F F0"
};

