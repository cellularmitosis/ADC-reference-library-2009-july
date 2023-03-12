/*
	File:		HackTV.r
	
	Description: HackTV resources

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1992-2004 Apple Computer, Inc. All rights reserved.
	
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

	Revision 1.9	4/16/2001	updated for UI3.4 and X DTS
				  	2000/11/20	Add alternative strings for Async recording
	Revision 1.8 	2000/11/16	Add carb 0 so it launches as Carbon app
	Revision 1.7	2000/11/07	Widen default window as I'm showing errors in the titlebar
	Revision 1.6  	2000/04/12	Aqua-size the controls
	Revision 1.5  	2000/03/01 	Add more sizes & recording on Idle	
	Revision 1.4  	2000/02/25	Added quit
	Revision 1.3  	1999/12/1	one more carbon, Exit->Quit
	Revision 1.2  	Original	QTE
*/

#if (TARGET_API_MAC_CARBON)
	#if	TARGET_REZ_CARBON_MACHO
		#include <Carbon/Carbon.r>
	#elif TARGET_REZ_CARBON_CFM
		#include <Carbon.r>
	#else
		#error "I'm confused!"
	#endif	
#else
	#include "ConditionalMacros.r"
	#include "Types.r"
	#include "Menus.r"
#endif

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
		#if TARGET_REZ_CARBON_CFM
			/* [3] */
			"-", noIcon, noKey, noMark, plain,
			/* [4] */
			"Quit", noIcon, "Q", noMark, plain
		#endif
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
	0x7FFFEEDB,
	enabled,
	"Monitor", {
		"Video Settings…", noIcon, noKey, noMark, plain;
		"Sound Settings…", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Record Video", noIcon, noKey, noMark, plain;
		"Record Sound", noIcon, noKey, noMark, plain;
		"Split Track Files", noIcon, noKey, noMark, plain;
		"Low Latency Capture", noIcon, noKey, noMark, plain;
		"Always Use Time Base", noIcon, noKey, noMark, plain;
		"Use Sound Clock", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Quarter Size", noIcon, noKey, noMark, plain;
		"Half Size", noIcon, noKey, noMark, plain;
		"Full Size", noIcon, noKey, noMark, plain;
		"128x96", noIcon, noKey, noMark, plain;
		"160x120", noIcon, noKey, noMark, plain;
		"176x144", noIcon, noKey, noMark, plain;
		"320x240", noIcon, noKey, noMark, plain;
		"360x240", noIcon, noKey, noMark, plain;
		"352x288", noIcon, noKey, noMark, plain;
		"-", noIcon, noKey, noMark, plain;
		"Record", noIcon, "R", noMark, plain;
		"Record Until MouseClick", noIcon, noKey, noMark, plain
	}
};

resource 'STR#' (128) {
	{
	"Record Without Hogging Machine",
	"Stop Recording",
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
	{62, 81, 82, 301},
	movableDBoxProc,
	invisible,
	noGoAway,
	0x0,
	129,
	"Monitor",
	alertPositionMainScreen
};

resource 'DLOG' (128) {
	{80, 96, 222, 385},
	dBoxProc,
	visible,
	goAway,
	0x0,
	128,
	"",
	noAutoCenter
};

data 'dlgx' (128) {
	$"0000 0000 0001"                                     /* ...... */
};

resource 'DITL' (129) {
	{	/* array DITLarray: 0 elements */
	}
};

#if TARGET_API_MAC_CARBON
	#define STANDARD_CONTROL_WIDTH 69
	#define STANDARD_CONTROL_WIDTH_ADJUST 9
#else
	#define STANDARD_CONTROL_WIDTH 60
	#define STANDARD_CONTROL_WIDTH_ADJUST 0
#endif TARGET_API_MAC_CARBON

#define CONTROLWIDTH (STANDARD_CONTROL_WIDTH+STANDARD_CONTROL_WIDTH_ADJUST)

resource 'DITL' (128) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{107, 142-(CONTROLWIDTH/2), 127, 142-(CONTROLWIDTH/2)+CONTROLWIDTH},
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
			"Carbon HackTV with extra goodness\n"
			"The all purpose video capture sample"
		},
		/* [4] */
		{48, 8, 96, 254},
		StaticText {
			disabled,
			"Version 1.2\n"
			"Brought to a shambling mockery of life b"
			"y some malevolent peguins."
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
/* [1] */ {78, 341-STANDARD_CONTROL_WIDTH, 98, 341}, Button {enabled, "OK"},
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

data 'alrx' (130) {
	$"0001 0000 0001 0000 0000 0100 00"                   /* ............. */
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
	0x01,
	0x11,
	development,
	0x0,
	0,
	"1.2",
	"1.2, © Apple Computer, Inc., 1992-2004"
};

resource 'vers' (2) {
	0x01,
	0x11,
	development,
	0x0,
	0,
	"1.2",
	"by Videos 'R' Us & The Evil Penguin Club"
};

data 'aptv' (0, "Owner resource") {
	$"16A9 2041 7070 6C65 2043 6F6D 7075 7465"            /* .© Apple Compute */
	$"722C 2049 6E63 2E"                                  /* r, Inc. */
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0001 0000 1022 8000 1864 4000 0488 2000"            /* ....."Ä..d@..à . */
		$"0310 1003 7FF8 0807 FFFC 040B C00C 0213"            /* ............¿... */
		$"D86C 0007 D86C 004E DFEC 009C DFEC 0234"            /* ÿl..ÿl.N...ú...4 */
		$"D86C 0672 D86C 1CF1 C00C 19F0 FFFC 73E0"            /* ÿl.rÿl..¿.....s. */
		$"FFFD C000 7FF8 C800 1830 3FC0 0801 3FFF"            /* ..¿...»..0?¿..?. */
		$"0401 81FF 0202 002F 0103 C046 0080 0080"            /* ..Å..../..¿F.Ä.Ä */
		$"0040 0100 0020 0200 0010 0400 0008 0800"            /* .@... .......... */
		$"0004 1000 0002 2000 0001 4000 0000 8000",           /* ...... ...@...Ä. */
		/* [2] */
		$"0001 0000 1023 8000 1867 C000 048F E000"            /* .....#Ä..g¿..è.. */
		$"031F F003 7FFF F807 FFFF FC0F FFFF FE1F"            /* ................ */
		$"FFFF FFFF FFFF FFFE FFFF FFFC FFFF FFFC"            /* ................ */
		$"FFFF FFFE FFFF FFFF FFFF FFFF FFFF FFFF"            /* ................ */
		$"FFFF FFFF 7FFF FFFF 1FFF FFFF 0FFF FFFF"            /* ................ */
		$"07FF FFFF 03FF FFFF 01FF FFFF 00FF FFE0"            /* ................ */
		$"007F FFC0 003F FF80 001F FF00 000F FE00"            /* ...¿.?.Ä........ */
		$"0007 FC00 0003 F800 0001 F000 0000 E000"            /* ................ */
	}
};

resource 'icl4' (128) {
	$"0000 0000 0000 000F 0000 0000 0000 0000"            /* ................ */
	$"000F 0000 00F0 00F0 F000 0000 0000 0000"            /* ................ */
	$"000F F000 0FF0 0F0C 0F00 0000 0000 0000"            /* ................ */
	$"0000 0F00 F000 F0C0 00F0 0000 0000 0000"            /* .......¿........ */
	$"0000 00FF 000F 000C 0C0F 0000 0000 0033"            /* ...............3 */
	$"0FFF FFFF FFFF FC00 C000 F000 0000 0A33"            /* ........¿......3 */
	$"FEEE EEEE EEEE EF00 0C0C 0F00 0000 EC3A"            /* ...............: */
	$"FEC0 C0C0 C0C0 EF0C 00C0 00F0 000D 1CEA"            /* .¿¿¿¿¿...¿...¬.. */
	$"FE0F FC0C 0FFC 3300 C000 CC0C DCB1 CAAE"            /* ......3.¿.Ã..± Æ */
	$"FECF F0C0 CFF0 330C 000C 0CC1 CB1D AAE0"            /* .œ.¿œ.3....¡À.™. */
	$"FE0F FFFF FFFC EF00 C00C DCCC 31CA AE00"            /* ........¿..Ã1 Æ. */
	$"FECF FFFF FFF0 EFC0 00DD CB2B 1CA3 DE00"            /* .œ.....¿..À+.£.. */
	$"FE0F FC0C 0FFC EF0C 0CC1 4BE1 CAAE 28D0"            /* .........¡K. Æ(– */
	$"FECF F0C0 CFF0 EF00 C02B EB1C EAE2 CCDE"            /* .œ.¿œ...¿+....Ã. */
	$"FE0C 0C0C 0C0C EF0C C2DB BDD3 FE3E 2CCD"            /* ........¬.Ω”.>,Õ */
	$"FEEE EEEE EEEE EF08 2BBE C2BF F3BD CC2C"            /* ........+æ¬ø.ΩÃ, */
	$"FEEE EEEE EEEE EF0B DA4C 2CCD CDC1 C48D"            /* .........L,ÕÕ¡ƒç */
	$"0FFF FFFF FFFF FC80 AA0D DB4C 1CCC CCDC"            /* .......Ä™¬.L.ÃÃ. */
	$"000F F000 00FF 000C BD3B AAAB B2BC 2C24"            /* ........Ω;™´≤º,$ */
	$"0000 FCC0 0C00 080D 1BAD BBAA EEEB BBBB"            /* ...¿...¬.≠ª™..ªª */
	$"0000 0FCC 00C0 C0CE DDCC 08CB 3F3F AAEB"            /* ...Ã.¿¿Œ.Ã.À??™. */
	$"0000 00FC C00C 00DD C0C0 80C0 00FD DFEA"            /* ....¿...¿¿Ä¿.... */
	$"0000 000F C0C0 0CAE ED08 0800 0FCC CFFC"            /* ....¿¿.Æ.....Ãœ. */
	$"0000 0000 FD00 C008 00C0 0080 FCD0 0000"            /* ......¿..¿.Ä.–.. */
	$"0000 0000 0FC0 00C0 0C0C 0C0F CD00 0000"            /* .....¿.¿....Õ... */
	$"0000 0000 00FD 0C00 C000 00FC D000 0000"            /* ........¿...–... */
	$"0000 0000 000F C000 0C0C 0FCD 0000 0000"            /* ......¿....Õ.... */
	$"0000 0000 0000 FCC0 C000 FCD0 0000 0000"            /* .......¿¿..–.... */
	$"0000 0000 0000 0FC0 0C0F CD00 0000 0000"            /* .......¿..Õ..... */
	$"0000 0000 0000 00FD 00FC D000 0000 0000"            /* ..........–..... */
	$"0000 0000 0000 000F DFCD 0000 0000 0000"            /* .........Õ...... */
	$"0000 0000 0000 0000 FDC0 0000 0000 0000"            /* .........¿...... */
};

resource 'icl8' (128) {
	$"0000 0000 0000 0000 0000 0000 0000 00FF"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF 0000 0000 0000 FF00 0000 FFF5"            /* ................ */
	$"FF00 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF FF00 0000 00FF FF00 00FF F5F5"            /* ................ */
	$"F5FF 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 00FF 0000 FF00 0000 FFF5 F5F5"            /* ................ */
	$"F5F5 FF00 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 FFFF 0000 00FF F5F5 F5F5"            /* ................ */
	$"F5F5 F5FF 0000 0000 0000 0000 0000 DB22"            /* ..............." */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFF5 F5F5"            /* ................ */
	$"F5F5 F5F5 FF00 0000 0000 0000 00DB 22DA"            /* ..............". */
	$"FFFC FCFC FCFC FCFC FCFC FCFC FCFF F5F5"            /* ................ */
	$"F5F5 F5F5 F5FF 0000 0000 0000 81F7 DADB"            /* ............Å... */
	$"FFFC F6F6 F6F6 F6F6 F6F6 F6F6 FCFF F5F5"            /* ................ */
	$"F5F5 F5F5 F5F5 FF00 0000 0081 0356 81DE"            /* ...........Å.VÅ. */
	$"FFFC F6FF FFF6 F6F6 F6FF FFF6 D8D8 F5F5"            /* ............ÿÿ.. */
	$"F5F5 F5F5 F5F6 0708 3232 5803 3488 DEFC"            /* ........22X.4à.. */
	$"FFFC F6FF FFF6 F6F6 F6FF FFF6 D8D8 F5F5"            /* ............ÿÿ.. */
	$"F5F5 F5F5 F508 0808 325E 0334 88DE FC00"            /* ........2^.4à... */
	$"FFFC F6FF FFFF FFFF FFFF FFF6 FCFF F5F5"            /* ................ */
	$"F5F5 F52C 0E08 0832 5E03 3488 DEFC 0000"            /* ...,...2^.4à.... */
	$"FFFC F6FF FFFF FFFF FFFF FFF6 FCFF F5F5"            /* ................ */
	$"F5F5 320E 3239 5E58 0333 88DE 3381 0000"            /* ..2.29^X.3à.3Å.. */
	$"FFFC F6FF FFF6 F6F6 F6FF FFF6 FCFF F5F5"            /* ................ */
	$"F52C 0833 3988 5E03 3388 DE5E 3332 8100"            /* .,.39à^.3à.^32Å. */
	$"FFFC F6FF FFF6 F6F6 F6FF FFF6 FCFF F5F5"            /* ................ */
	$"2C08 335E 885E 0933 88DE 885E 3332 3281"            /* ,.3^à^∆3à.à^322Å */
	$"FFFC F6F6 F6F6 F6F6 F6F6 F6F6 FCFF F5F5"            /* ................ */
	$"2C33 3988 5E09 3388 E088 645E 3932 3232"            /* ,39à^∆3à.àd^9222 */
	$"FFFC FCFC FCFC FCFC FCFC FCFC FCFF F52B"            /* ...............+ */
	$"3964 8882 2D33 88FF DF64 5E33 3332 3232"            /* 9dàÇ-3à..d^33222 */
	$"FFFC FCFC FCFC FCFC FCFC FCFC FCFF F55D"            /* ...............] */
	$"6488 3933 3332 3233 5732 0808 0E32 3232"            /* dà933223W2...222 */
	$"00FF FFFF FFFF FFFF FFFF FFFF FF2B F5F5"            /* .............+.. */
	$"8888 0832 5D39 3932 0808 0808 0832 3232"            /* àà.2]992.....222 */
	$"0000 00FF FF00 F5F5 F5F5 FFFF F5F5 F52B"            /* ...............+ */
	$"5833 6488 DE89 8888 5E5E 3332 3233 3339"            /* X3dà.âàà^^322339 */
	$"0000 0000 FFF8 F5F5 F5F5 F5F5 F5F5 F55D"            /* ...............] */
	$"3358 88FA 5D88 89AD 8888 8888 5E5E 5E5E"            /* 3Xà.]àâ≠àààà^^^^ */
	$"0000 0000 00FF F8F5 F5F5 F5F5 F5F5 2C81"            /* ..............,Å */
	$"82F9 56F7 F5F5 325D 89DE DEB3 AD88 8888"            /* Ç.V...2]â..≥≠ààà */
	$"0000 0000 0000 FFF8 F5F5 F5F5 F5F5 8156"            /* ..............ÅV */
	$"2BF6 F5F5 F5F5 F5F5 F5F5 FFF8 FBFF 8888"            /* +.............àà */
	$"0000 0000 0000 00FF F8F5 F5F5 F5F5 FDFC"            /* ................ */
	$"FB81 F5F5 F5F5 F500 00FF F8F8 08FF FF56"            /* .Å.............V */
	$"0000 0000 0000 0000 FFF8 F5F5 F5F5 F5F6"            /* ................ */
	$"F5F5 F5F5 F5F5 F5F5 FFF8 F800 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 00FF F8F5 F5F5 F5F5"            /* ................ */
	$"F5F5 F5F5 F5F5 F5FF F8F8 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 FFF8 F5F5 F5F5"            /* ................ */
	$"F5F5 F5F5 F5F5 FFF8 F800 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 00FF F8F5 F5F5"            /* ................ */
	$"F5F5 F5F5 F5FF F8F8 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 FFF8 F5F5"            /* ................ */
	$"F5F5 F5F5 FFF8 F800 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 00FF F8F5"            /* ................ */
	$"F5F5 F5FF F8F8 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 FFF8"            /* ................ */
	$"F5F5 FFF8 F800 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 00FF"            /* ................ */
	$"F8FF F8F8 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"FFF8 F800 0000 0000 0000 0000 0000 0000"            /* ................ */
};
resource 'ics#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0180 02C0 0460 0830 1018 200C 4006 80FB"            /* .Ä.¿.`.0.. .@.Ä. */
		$"C1CD 63FF 3143 18C3 0C7F 0623 0340 0180",           /* ¡Õc.1C.√...#.@.Ä */
		/* [2] */
		$"0180 03C0 07E0 0FF0 1FF8 3FFC 7FFE FFFF"            /* .Ä.¿......?..... */
		$"FFFF 7FFF 3FFF 1FFF 0FFF 07E3 03C0 0180"            /* ....?........¿.Ä */
	}
};

resource 'ics4' (128) {
	$"0000 000F F000 0000 0000 00F0 FF00 0000"            /* ................ */
	$"0000 0F00 0FF0 0000 0000 F000 00FF 0000"            /* ................ */
	$"000F 0000 000F F000 00F0 0000 0000 FF00"            /* ................ */
	$"0F00 0000 0000 0FF0 F000 0000 FFFF F0FF"            /* ................ */
	$"FF00 000F FFCC FF0F 0FF0 00FF FFFF FFFF"            /* .....Ã.......... */
	$"00FF 000F CFCC CCFF 000F F000 FFCC CCFF"            /* ....œÃÃ......ÃÃ. */
	$"0000 FF00 0FFF FFFF 0000 0FF0 00F0 00FF"            /* ................ */
	$"0000 00FF 0F00 0000 0000 000F F000 0000"            /* ................ */
};

resource 'ics8' (128) {
	$"0000 0000 0000 00FF FF00 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 FFF5 FFFF 0000 0000 0000"            /* ................ */
	$"0000 0000 00FF F5F5 F5FF FF00 0000 0000"            /* ................ */
	$"0000 0000 FFF5 F5F5 F5F5 FFFF 0000 0000"            /* ................ */
	$"0000 00FF F5F5 F5F5 F5F5 F5FF FF00 0000"            /* ................ */
	$"0000 FFF5 F5F5 F5F5 F5F5 F5F5 FFFF 0000"            /* ................ */
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF FF00"            /* ................ */
	$"FFF5 F5F5 F5F5 F5F5 FFFF FFFF FFF5 FFFF"            /* ................ */
	$"FFFF F5F5 F5F5 F5FF FFFF 0808 FFFF F5FF"            /* ................ */
	$"00FF FFF5 F5F5 FFFF FFFF FFFF FFFF FFFF"            /* ................ */
	$"0000 FFFF F5F5 F5FF 08FF 0808 0808 FFFF"            /* ................ */
	$"0000 00FF FFF5 F5F5 FFFF 0808 0808 FFFF"            /* ................ */
	$"0000 0000 FFFF F5F5 F5FF FFFF FFFF FFFF"            /* ................ */
	$"0000 0000 00FF FFF5 F5F5 FF00 0000 FFFF"            /* ................ */
	$"0000 0000 0000 FFFF F5FF 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FF00 0000 0000 0000"            /* ................ */
};
