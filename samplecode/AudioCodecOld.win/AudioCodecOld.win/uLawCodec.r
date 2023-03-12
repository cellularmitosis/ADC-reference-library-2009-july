//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// defines needed before including headers

// use the platform info array in the 'thng' resource for 68k and PowerPC, or Windows
#define thng_RezTemplateVersion 1

#define DLOG_RezTemplateVersion 1


#include "MacTypes.r"
#include "Components.r"
#include "Sound.r"
#include "Icons.r"
#include "Dialogs.r"
#include "Appearance.r"

#include "uLawCodec.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 킠aw decompressor

resource 'thng' (kSoundDecompressorResID, ULawDecompressorName, purgeable) {
	kSoundDecompressor, kCodecFormat, kSoundComponentManufacturer, 
	k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut, 0,
	kSoundComponentType, kSoundDecompressorResID,
	'STR ', kSoundCodecNameStringResID,
	'STR ', kSoundCodecInfoStringResID,
	'ICON', kSoundComponentIconResID,
	kSoundDecompressorVersion,
	componentDoAutoVersion|componentHasMultiplePlatforms, 0,
	{
#if TARGET_OS_MAC
		k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut,
			kSoundComponentType, kSoundDecompressorResID, platform68k;
		k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut,
			kSoundComponentPPCType, kSoundDecompressorResID, platformPowerPC
#else
		k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut,
			'dlle', kSoundDecompressorResID, platformWin32
#endif
	}
};


// name and info string are shared by the compressor and decompressor
resource 'STR ' (kSoundCodecNameStringResID, ULawDecompressorName, purgeable) {
	"Example 킠aw 2:1"
};

resource 'STR ' (kSoundCodecInfoStringResID, ULawDecompressorName, purgeable) {
	"킠aw (CCITT Recommendation G.711) compression with a 2 to 1 ratio."
};

resource 'ICON' (kSoundComponentIconResID, ULawDecompressorName, purgeable) {
	$"00000000000000000000000000000000"
	$"00000000003FC00001C0380002040400"
	$"022A840003915F80027FE44002000720"
	$"020004A0020004A0020004A0020004A0"
	$"020004A0020007200200044002000780"
	$"0200040001C03800003FC00000001000"
	$"00888000002020000085400000100000"
	$"0042"
};

#if !TARGET_OS_MAC
resource 'dlle' (kSoundDecompressorResID, ULawDecompressorName) {
	"__uLawDecompComponentDispatch"
};
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 킠aw compressor

resource 'thng' (kSoundCompressorResID, ULawCompressorName, purgeable) {
	kSoundCompressor, kCodecFormat, kSoundComponentManufacturer,
	k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut, 0,
	kSoundComponentType, kSoundCompressorResID,
	'STR ', kSoundCodecNameStringResID,
	'STR ', kSoundCodecInfoStringResID,
	'ICON', kSoundComponentIconResID,
	kSoundCompressorVersion,
	componentDoAutoVersion|componentHasMultiplePlatforms, 0,
	{
#if TARGET_OS_MAC
		k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut,
			kSoundComponentType, kSoundCompressorResID, platform68k;
		k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut,
			kSoundComponentPPCType, kSoundCompressorResID, platformPowerPC
#else
		k16BitIn|kStereoIn | k8BitRawOut|k16BitOut|kStereoOut,
			'dlle', kSoundCompressorResID, platformWin32
#endif
	}
};


#if !TARGET_OS_MAC
resource 'dlle' (kSoundCompressorResID, ULawCompressorName) {
	"__uLawCompComponentDispatch"
};
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 킠aw decompressor table

type k16BitTableResType {
	array {
		integer;			// array of 16 bit integers
	};
};

// make the resource locked since it's always locked while in use,
// marking it locked helps memory management: Resource Mgr will load it lower in heap

resource k16BitTableResType (kSoundDecompressorResID, ULawDecompressorName, locked) {
	{
		-32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
		-23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
		-15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
		-11900, -11388, -10876, -10364,  -9852,  -9340,  -8828,  -8316,
		-7932,  -7676,  -7420,  -7164,  -6908,  -6652,  -6396,  -6140,
		-5884,  -5628,  -5372,  -5116,  -4860,  -4604,  -4348,  -4092,
		-3900,  -3772,  -3644,  -3516,  -3388,  -3260,  -3132,  -3004,
		-2876,  -2748,  -2620,  -2492,  -2364,  -2236,  -2108,  -1980,
		-1884,  -1820,  -1756,  -1692,  -1628,  -1564,  -1500,  -1436,
		-1372,  -1308,  -1244,  -1180,  -1116,  -1052,   -988,   -924,
		-876,   -844,   -812,   -780,   -748,   -716,   -684,   -652,
		-620,   -588,   -556,   -524,   -492,   -460,   -428,   -396,
		-372,   -356,   -340,   -324,   -308,   -292,   -276,   -260,
		-244,   -228,   -212,   -196,   -180,   -164,   -148,   -132,
		-120,   -112,   -104,    -96,    -88,    -80,    -72,    -64,
		-56,    -48,    -40,    -32,    -24,    -16,     -8,      0,
		32124,  31100,  30076,  29052,  28028,  27004,  25980,  24956,
		23932,  22908,  21884,  20860,  19836,  18812,  17788,  16764,
		15996,  15484,  14972,  14460,  13948,  13436,  12924,  12412,
		11900,  11388,  10876,  10364,   9852,   9340,   8828,   8316,
		7932,   7676,   7420,   7164,   6908,   6652,   6396,   6140,
		5884,   5628,   5372,   5116,   4860,   4604,   4348,   4092,
		3900,   3772,   3644,   3516,   3388,   3260,   3132,   3004,
		2876,   2748,   2620,   2492,   2364,   2236,   2108,   1980,
		1884,   1820,   1756,   1692,   1628,   1564,   1500,   1436,
		1372,   1308,   1244,   1180,   1116,   1052,    988,    924,
		876,    844,    812,    780,    748,    716,    684,    652,
		620,    588,    556,    524,    492,    460,    428,    396,
		372,    356,    340,    324,    308,    292,    276,    260,
		244,    228,    212,    196,    180,    164,    148,    132,
		120,    112,    104,     96,     88,     80,     72,     64,
		 56,     48,     40,     32,     24,     16,      8,      0,
	}
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 킠aw compressor exponent table

type k8BitTableResType {
	array {
		byte;				// array of bytes
	};
};

// make the resource locked since it's always locked while in use,
// marking it locked helps memory management: Resource Mgr will load it lower in heap

resource k8BitTableResType (kSoundCompressorResID, ULawCompressorName, locked) {
	{
		0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
	}
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// options dialog resources

resource 'DLOG' (kSoundCompressorResID, ULawCompressorName, purgeable) {
	{0, 0, 150+13, 214+13},
#if TARGET_OS_MAC
	dBoxProc,
#else
	movableDBoxProc,
#endif
	invisible, noGoAway, 0, kSoundCompressorResID, ULawCompressorName, alertPositionParentWindow
};

resource 'DITL' (kSoundCompressorResID, ULawCompressorName, purgeable) {
	{	
/* [1] */	{130, 156, 150, 214}, Button {enabled, "OK"},
/* [2] */	{130, 78, 150, 136}, Button {enabled, "Cancel"},
/* [3] */	{13, 13, 45, 45}, Icon {disabled, kSoundComponentIconResID},
/* [4] */	{26, 65, 44, 216}, CheckBox {enabled, "I like this Icon"},
/* [5] */	{52, 13, 114, 217}, StaticText {disabled, "Example Sound Codec\n� 1998 Apple Computer Inc."
		}
	}
};

resource 'dlgx' (kSoundCompressorResID, ULawCompressorName, purgeable) {
	versionZero		/* sometimes the headers have this as latestVersion */
	{kDialogFlagsUseThemeBackground | kDialogFlagsUseControlHierarchy | kDialogFlagsUseThemeControls}
};


