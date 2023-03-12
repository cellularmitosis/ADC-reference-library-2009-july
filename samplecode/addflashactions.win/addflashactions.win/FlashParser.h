#ifndef __FlashParser__
#define __FlashParser__

#include <MacTypes.h>
#include <Lists.h>

#include <StdIO.h>

#ifndef __MOVIES__
#include "Movies.h"
#endif


// Global Types
#if TARGET_OS_MAC
	typedef unsigned long BOOL;
	#define inline
	// RTM
#endif
#if TARGET_OS_WIN32
	#include <windows.h>
	#define inline
#endif

typedef unsigned long U32, *P_U32, **PP_U32;
typedef signed long S32, *P_S32, **PP_S32;
typedef unsigned short U16, *P_U16, **PP_U16;
typedef signed short S16, *P_S16, **PP_S16;
typedef unsigned char U8, *P_U8, **PP_U8;
typedef signed char S8, *P_S8, **PP_S8;
typedef signed long SFIXED, *P_SFIXED;
typedef signed long SCOORD, *P_SCOORD;

typedef struct SPOINT
{
    SCOORD x;
    SCOORD y;
} SPOINT, *P_SPOINT;

typedef struct SRECT 
{
    SCOORD xmin;
    SCOORD xmax;
    SCOORD ymin;
    SCOORD ymax;
} SRECT, *P_SRECT;

// Start Sound Flags
enum {
	soundHasInPoint		= 0x01,
	soundHasOutPoint	= 0x02,
	soundHasLoops		= 0x04,
	soundHasEnvelope	= 0x08

	// the upper 4 bits are reserved for synchronization flags
};

enum {
	fillGradient		=	0x10,
	fillLinearGradient 	=	0x10,
	fillRadialGradient 	=	0x12,
	fillMaxGradientColors 	=	8,
	// Texture/bitmap fills
	fillBits			=	0x40	// if this bit is set, must be a bitmap pattern
};

// Flags for defining a shape character
enum {
		// These flag codes are used for state changes - and as return values from ShapeParser::GetEdge()
		eflagsMoveTo	   = 0x01,
		eflagsFill0	   	   = 0x02,
		eflagsFill1		   = 0x04,
		eflagsLine		   = 0x08,
		eflagsNewStyles	   = 0x10,

		eflagsEnd 	   	   = 0x80  // a state change with no change marks the end
};

/*
typedef struct MATRIX
{
    SFIXED a;
    SFIXED b;
    SFIXED c;
    SFIXED d;
    SCOORD tx;
    SCOORD ty;
} MATRIX, *P_MATRIX;
*/
/*
typedef struct CXFORM
{
    S32 flags;
    enum
    { 
        needA=0x1,	// Set if we need the multiply terms.
        needB=0x2	// Set if we need the constant terms.
    };
    S16 aa, ab;	// a is multiply factor, b is addition factor
    S16 ra, rb;
    S16 ga, gb;
    S16 ba, bb;
} CXFORM, *P_CXFORM;
*/
#ifndef NULL
#define NULL 0
#endif

// Tag values that represent actions or data in a Flash script.
enum
{ 
    stagEnd 				= 0,
    stagShowFrame 			= 1,
    stagDefineShape		 	= 2,
    stagFreeCharacter 		= 3,
    stagPlaceObject 		= 4,
    stagRemoveObject 		= 5,
    stagDefineBits 			= 6,
    stagDefineButton 		= 7,
    stagJPEGTables 			= 8,
    stagSetBackgroundColor	= 9,
    stagDefineFont			= 10,
    stagDefineText			= 11,
    stagDoAction			= 12,
    stagDefineFontInfo		= 13,
    stagDefineSound			= 14,	// Event sound tags.
    stagStartSound			= 15,
    stagDefineButtonSound	= 17,
    stagSoundStreamHead		= 18,
    stagSoundStreamBlock	= 19,
    stagDefineBitsLossless	= 20,	// A bitmap using lossless zlib compression.
    stagDefineBitsJPEG2		= 21,	// A bitmap using an internal JPEG compression table.
    stagDefineShape2		= 22,
    stagDefineButtonCxform	= 23,
    stagProtect				= 24,	// This file should not be importable for editing.

    // These are the new tags for Flash 3.
    stagPlaceObject2		= 26,	// The new style place w/ alpha color transform and name.
    stagRemoveObject2		= 28,	// A more compact remove object that omits the character tag (just depth).
    stagDefineShape3		= 32,	// A shape V3 includes alpha values.
    stagDefineText2			= 33,	// A text V2 includes alpha values.
    stagDefineButton2		= 34,	// A button V2 includes color transform, alpha and multiple actions
    stagDefineBitsJPEG3		= 35,	// A JPEG bitmap with alpha info.
    stagDefineBitsLossless2 = 36,	// A lossless bitmap with alpha info.
    stagDefineSprite		= 39,	// Define a sequence of tags that describe the behavior of a sprite.
    stagNameCharacter		= 40,	// Name a character definition, character id and a string, (used for buttons, bitmaps, sprites and sounds).
    stagFrameLabel			= 43,	// A string label for the current frame.
    stagSoundStreamHead2	= 45,	// For lossless streaming sound, should not have needed this...
    stagDefineMorphShape	= 46,	// A morph shape definition
    stagDefineFont2			= 48	// 
};

// PlaceObject2 Flags
enum
{
    splaceMove			= 0x01,	// this place moves an exisiting object
    splaceCharacter		= 0x02,	// there is a character tag	(if no tag, must be a move)
    splaceMatrix		= 0x04,	// there is a matrix (matrix)
    splaceColorTransform= 0x08,	// there is a color transform (cxform with alpha)
    splaceRatio			= 0x10,	// there is a blend ratio (word)
    splaceName			= 0x20,	// there is an object name (string)
	splaceDefineClip	= 0x40  // this shape should open or close a clipping bracket (character != 0 to open, character == 0 to close)
    // one bit left for expansion
};

// Action codes
enum {
	sactionHasLength	= 0x80,
	sactionNone			= 0x00,
	sactionGotoFrame	= 0x81,	// frame num (WORD)
	sactionGetURL		= 0x83,	// url (STR), window (STR)
	sactionNextFrame	= 0x04,
	sactionPrevFrame	= 0x05,
	sactionPlay			= 0x06,
	sactionStop			= 0x07,
	sactionToggleQuality= 0x08,
	sactionStopSounds	= 0x09,
	sactionWaitForFrame	= 0x8A,	// frame needed (WORD), actions to skip (BYTE)
	sactionSetTarget	= 0x8B,	// name (STR)
	sactionGotoLabel	= 0x8C,	// name (STR)
	
	sactionWiredActions	= 0xAA
};

// Flash Enum			
enum { 
	bsIdleToOverUp = 0,		// mouse enter up
	bsOverUpToIdle,			// mouse exit up
	bsOverUpToOverDown,		// mouse down		click
	bsOverDownToOverUp,		// mouse up in   clickEndTrigger

	// These transitions only apply when tracking "push" buttons
	bsOverDownToOutDown,	// mouse exit down
	bsOutDownToOverDown,	// mouse enter down
	bsOutDownToIdle,		// mouse up out

	// These transitions only apply when tracking "menu" buttons
	bsIdleToOverDown,		// mouse enter down
	bsOverDownToIdle		// mouse exit down
};


#define kIdleToOverUp			(1L << bsIdleToOverUp)
#define kOverUpToIdle			(1L << bsOverUpToIdle)
#define kOverUpToOverDown		(1L << bsOverUpToOverDown)
#define kOverDownToOverUp		(1L << bsOverDownToOverUp)

#define kOverDownToOutDown		(1L << bsOverDownToOutDown)
#define kOutDownToOverDown		(1L << bsOutDownToOverDown)
#define kOutDownToIdle			(1L << bsOutDownToIdle)

#define kIdleToOverDown			(1L << bsIdleToOverDown)
#define kOverDownToIdle			(1L << bsOverDownToIdle)



struct FlashParserStruct
{

	Handle	m_theData;

    // Pointer to file contents buffer.
    U8 *m_fileBuf;

    // File state information.
    U32 m_filePos;
    U32 m_fileSize;
    U32 m_fileStart;
    U16 m_fileVersion;

    // Bit Handling
    S32 m_bitPos;
    U32 m_bitBuf;

    // Tag parsing information.
    U32 m_tagStart;
    U32 m_tagEnd;
    U32 m_tagLen;
    
 	// Parsing information.
	S32 m_nFillBits;
	S32 m_nLineBits;

   // Set to true if we wish to dump all contents long form
    U32 m_dumpAll;

    // if set to true will dump image guts (i.e. jpeg, zlib, etc. data)
    U32 m_dumpGuts;
    

};

typedef struct FlashParserStruct FlashParserStruct, *FlashParserPtr, **FlashParserHandle;

inline U8			GetByte (void) ;
inline U16			GetWord (void);
inline U32			GetDWord (void);
char *				GetAString(void);
void				InitBits(void);
U32					GetBits (S32 n);
S32					GetSBits (S32 n);
void				GetRect (SRECT * r);
U16					GetTag (void);
void				SetNewHeaderLengthTagLength (U32 fileDifference, U32 tagDifference);
void				ParseTags (Boolean sprite, long *buttonID);
U32					GetOffsetForButton (long buttonID);
void				GetOffsetForFrame (long frameID, U32 *frameStart, U32 *frameEnd);
OSErr				LocateFirstButton (Handle theSample, long *buttonID);


#endif // __FlashParser__