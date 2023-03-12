/*
	File:		AppearanceSample.r

	Contains:	Resources for our sample app using new Appearance types.

	Version:	Appearance 1.0 SDK

	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.

	File Ownership:

		DRI:				Edward Voas

		Other Contact:		7 of 9, Borg Collective

		Technology:			OS Technologies Group

	Writers:

		(MAA)	Matt Ackeret
		(edv)	Ed Voas

	Change History (most recent first):

		<10>	 6/22/98	MAA		Put CNTL(247) in here to update the procID to
									kControlEditTextPasswordProc.. The ResEdit templates aren't
									playing nice under Allegro yet.
		 <9>	  6/8/98	MAA		Add new Cursor xmnu
		 <8>	  5/4/98	MAA		remove null item (item 7) from xmnu ID 145
		 <7>	  3/9/98	MAA		add dlgx
		 <6>	  3/2/98	MAA		add xmnu for 145
		 <5>	 2/18/98	MAA		add ldes
		 <4>	 1/22/98	MAA		Add xmnu items for proxy
		 <3>	 1/12/98	MAA		Add menu hide/show and dialog timeouts xmnu items
		 <2>	12/18/97	edv		Remove UNIVERSAL_HEADERS_THREE conditional.
		 <1>	 9/11/97	edv		First checked in.
*/

#include <Carbon.r>

#define teFlushRight -1 					/*flush right for all scripts */


resource 'ldes' (128, purgeable)
{
	versionZero
	{
		30,		// rows
		30,		// columns
		16, 	// width
		50, 	// height
		hasVertScroll,
		hasHorizScroll, 
		0,
		hasGrowSpace
	};
};

		
resource 'xmnu' (145, purgeable)
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
			dataItem { '    ', kMenuNoModifiers, sysScript, 0, 0, noHierID, sysFont, 2 },
			dataItem { '    ', kMenuNoModifiers, sysScript, 0, 0, noHierID, sysFont, 3 },
			dataItem { '    ', kMenuNoModifiers, sysScript, 0, 0, noHierID, sysFont, 4 },
			dataItem { '    ', kMenuNoModifiers, sysScript, 0, 0, noHierID, sysFont, 5 },
			dataItem { '    ', kMenuNoModifiers, sysScript, 0, 0, noHierID, sysFont, 6 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 7 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 8 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 9 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x0a },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x0b },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x0c },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x0d },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x0f },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x10 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x11 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x12 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x13 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x14 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x17 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x18 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x19 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x1a },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x1b },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x1c },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x61 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x62 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x63 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x64 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x65 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x66 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x67 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x68 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x69 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x6a },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x6b },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x6c },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x6d },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x6e },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x6f },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x70 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x71 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x72 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x73 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x74 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x75 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x76 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x77 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x78 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x79 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x7a },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x87 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x88 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x89 },
			dataItem { '    ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, 0x8a }
		}
	};
};


resource 'xmnu' (149, purgeable)
{
	versionZero
	{
		{
			dataItem { 'arrc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'carc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'aarc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'cmac', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'ibec', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'croc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'pluc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'watc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'clhc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'ophc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'pthc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'cuhc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'cdhc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'cbhc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'spnc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'rslc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'rsrc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'rsbc', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph }
		}
	};
};
