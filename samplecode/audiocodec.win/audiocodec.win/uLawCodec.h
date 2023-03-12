// common header file shared by both the code and resource compilers


/*
	How this works:

	In uLaw, 14-bits of linear sampling is reduced to 8 bits of logarithmic data.
	It is used on North American and Japanese phone systems, and is coming into use
	for voice data interchange, for PBXs, voicemail, MIME (Internet standard for multimedia mail),
	and Internet Talk Radio. µLaw sounds are almost always encoded at 8000, 8012, or 8012.8210513
	samples/sec. 8000 because it is the specified standard for phones, and 8012.8210513
	(and rounded to 8012) because it is apparently the actual rate used in domestic digital phone switches.

	---------------------------
	U-LAW and A-LAW definitions
	---------------------------

	[Adapted from information provided by duggan@cc.gatech.edu (Rick
	Duggan) and davep@zenobia.phys.unsw.EDU.AU (David Perry)]

	u-LAW (really mu-LAW) is

	          sgn(m)   (     |m |)       |m |
	   y=    ------- ln( 1+ u|--|)       |--| =< 1
	         ln(1+u)   (     |mp|)       |mp|

	A-LAW is

	     |     A    (m )                 |m |    1
	     |  ------- (--)                 |--| =< -
	     |  1+ln A  (mp)                 |mp|    A
	   y=|
	     | sgn(m) (        |m |)    1    |m |
	     | ------ ( 1+ ln A|--|)    - =< |--| =< 1
	     | 1+ln A (        |mp|)    A    |mp|

	Values of u=100 and 255, A=87.6, mp is the Peak message value, m is the current quantised message value.
	(The formulae get simpler if you substitute x for m/mp and sgn(x) for sgn(m); then -1 <= x <= 1.)

	Converting from u-LAW to A-LAW is in a sense "lossy" since there are quantizing errors introduced in
	the conversion.

	"..the u-LAW used in North America and Japan, and the A-LAW used in Europe and the rest of the world
	and international routes.."

	References:

	Modern Digital and Analog Communication Systems, B.P.Lathi., 2nd ed. ISBN 0-03-027933-X

	Transmission Systems for Communications
	Fifth Edition
	by Members of the Technical Staff at Bell Telephone Laboratories
	Bell Telephone Laboratories, Incorporated
	Copyright 1959, 1964, 1970, 1982
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// resource IDs

#define kSoundDecompressorResID			1000		// ID of decompressor 'thng', code resources, and table
#define kSoundCompressorResID			2000		// ID of compressor 'thng', code resources, and table
#define kSoundCodecNameStringResID		1000		// ID of codec name
#define kSoundCodecInfoStringResID		1001		// ID of codec info string
#define kSoundComponentIconResID		1000		// ID of codec icon

#define kSoundComponentManufacturer		'????'		// your company's OSType
#define kCodecFormat					kULawCompression


#define kSoundDecompressorVersion		0x00050000	// Use a higher version to overide the default component
#define kSoundCompressorVersion			0x00050000	// Use a higher version to overide the default component

#define k16BitTableResType				'Wtab'		// resource type for a 16 bit array
#define k8BitTableResType				'Btab'		// resource type for a 8 bit array


// resource names will be created for debug versions
#ifdef _DEBUG
#define ULawDecompressorName			"µLaw Decompressor"
#define ULawCompressorName				"µLaw Compressor"
#else
#define ULawDecompressorName			""
#define ULawCompressorName				""
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// code constants

#define kMaxOutputSamples		1024		/* approximate no. samples in output buffer */

#define kULawBlockSamples		1			/* no. samples in a block */
#define kULawBlockBytes			1			/* no. bytes in a block */
#define kULawBytesPerSample		2			/* no. bytes in decompressed sample */


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// failure handling macros
/*
	Some macros used to check for errors and also to allow for
	handling them by using a goto statement.  This makes the source
	code easier to read.  It will break into the debugger with a
	message showing the condition that caused the failure.  In some
	of the macros the debug message is removed but goto remains.  In
	other macros all of it is removed when doing a non-debug build.

    Note that these macros use the "\p" construct for creating
	Pascal strings at compile time.  Most non-Mac compilers do
	not recognize this, give a warning, and put 'p' as the first
	character of the string.  You can ignore the warning because
	the non-Mac version of DebugStr deals with this just fine.
	For Microsoft's Visual C++, we suppress the warning below.
*/
#if defined(_MSC_VER) && !defined(__MWERKS__) 
	// Visual C++ from Microsoft
	#pragma warning(disable:4129) // unrecognized character escape sequence
#endif

// This checks for the exception, and if true then goto handler

#ifdef _DEBUG
#define FailIf(cond, handler)								\
	if (cond) {												\
		DebugStr((ConstStr255Param)"\p"#cond " goto " #handler);	\
		goto handler;										\
	} else 0
#else
#define FailIf(cond, handler)								\
	if (cond) {												\
		goto handler;										\
	} else 0
#endif

// This checks for the exception, and if true do the action and goto handler

#ifdef _DEBUG
#define FailWithAction(cond, action, handler)				\
	if (cond) {												\
		DebugStr((ConstStr255Param)"\p"#cond " goto " #handler);	\
		{ action; }											\
		goto handler;										\
	} else 0
#else
#define FailWithAction(cond, action, handler)				\
	if (cond) {												\
		{ action; }											\
		goto handler;										\
	} else 0
#endif

// This will insert debugging code in the application to check conditions
// and displays the condition in the debugger if true.  This code is
// completely removed in non-debug builds.

#ifdef _DEBUG
#define FailMessage(cond)									\
	if (cond)												\
		DebugStr((ConstStr255Param)"\p"#cond);				\
	else 0
#else
#define FailMessage(cond)
#endif

// This allows you to test for the result of a condition (i.e. CloseComponent)
// and break if it returns a non zero result, otherwise it ignores the result.
// When a non-debug build is done, the result is ignored.

#ifdef _DEBUG
#define ErrorMessage(cond)									\
	if (cond)												\
		DebugStr((ConstStr255Param)"\p"#cond);				\
	else 0
#else
#define ErrorMessage(cond)		cond
#endif

// This will display a given message in the debugger, this code is completely
// removed in non-debug builds.

#ifdef _DEBUG
#define DebugMessage(s)			DebugString((ConstStr255Param)s)
#else
#define DebugMessage(s)
#endif

