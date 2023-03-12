//////////
//
//	File:		QTActionTargets.h
//
//	Contains:	QuickTime wired sprites target support for QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	02/28/01	rtm		first file; based on existing QTWiredSpritesJr sample code
//	   
//////////

//////////
//
// header files
//
//////////

#ifndef _SPRITEUTILITIES_
#include "SpriteUtilities.h"
#endif

#ifndef _WIREDSPRITEUTILITIES_
#include "WiredSpriteUtilities.h"
#endif

#ifndef __ENDIANUTILITIES__
#include "EndianUtilities.h"
#endif

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif


//////////
//
// compiler directives
//
//////////

#define USE_WIRED_UTILITIES					1
#define ALLOW_SELF_TARGETING				0


//////////
//
// constants for controller buttons sprites
//
//////////

// controller image PICT resource IDs
#define kTiltPosUpID						131
#define kTiltPosDownID						132
#define kPanRightUpID						133
#define kPanRightDownID						134
#define kPanLeftUpID						135
#define kPanLeftDownID						136
#define kZoomOutUpID						137
#define kZoomOutDownID						138
#define kTiltNegUpID						139
#define kTiltNegDownID						140
#define kZoomInUpID							141
#define kZoomInDownID						142

// image indices
#define kTiltPosUpIndex						1
#define kTiltPosDownIndex					2
#define kPanRightUpIndex					3
#define kPanRightDownIndex					4
#define kPanLeftUpIndex						5
#define kPanLeftDownIndex					6
#define kZoomOutUpIndex						7
#define kZoomOutDownIndex					8
#define kTiltNegUpIndex						9
#define kTiltNegDownIndex					10
#define kZoomInUpIndex						11
#define kZoomInDownIndex					12

#define kNumControllerButtons				6
#define kNumControllerImages				kZoomInDownIndex
#define kFirstControllerImageID				kTiltPosUpID

// sprite atom IDs	
#define kTiltPosSpriteAtomID				1
#define kPanRightSpriteAtomID				2
#define kPanLeftSpriteAtomID				3
#define kZoomOutSpriteAtomID				4
#define kTiltNegSpriteAtomID				5
#define kZoomInSpriteAtomID					6

// position indices	
#define kPanLeftSpritePosition				0
#define kTiltNegSpritePosition				1
#define kZoomOutSpritePosition				2
#define kZoomInSpritePosition				3
#define kTiltPosSpritePosition				4
#define kPanRightSpritePosition				5

#define kButtonHeight						17
#define kButtonWidth						17

#define kSpriteMediaTimeScale				600

#define kMouseOverPanLeftVariableID			2
#define kMouseOverPanRightVariableID		3
#define kMouseOverTiltUpVariableID			4
#define kMouseOverTiltDownVariableID		5
#define kMouseOverZoomInVariableID			6
#define kMouseOverZoomOutVariableID			7

//////////
//
// constants for text button sprite
//
//////////

// button image PICT resource IDs
#define kTextUpID							143
#define kTextDownID							144

// image indices
#define kTextUpIndex						1
#define kTextDownIndex						2

#define kNumTextButtons						1
#define kNumTextImages						2
#define kFirstTextImageID					kTextUpID

#define kTextSpriteAtomID					1

//////////
//
// constants for VR controller movie
//
//////////

#define kVRControlMovieHeight				kButtonHeight
#define kVRControlMovieWidth				((kButtonWidth*6)+(kButtonWidth*5))

#define kVRControlMovieDuration				600

#define kTarget1							"Summer"
#define kTarget2							"Winter"


//////////
//
// miscellaneous constants
//
//////////

#define kMaxLayerNumber						0x7fff			// maximum layer number

#define kSpriteSavePrompt					"Save New Sprite Movie As:"
#define kSpriteSaveMovieFileName			"WiredSprite.mov"

#define kMovieNamePrefix					"moviename="
#define kMovieIDPrefix						"movieid="

#define kIconDimension						32

// sizes of the sprite track for the "twin sprites" movie
#define kIconSpriteTrackWidth				150
#define kIconSpriteTrackHeight				50

// PICT resource IDs
#define kOldQTIconID						200
#define kNewQTIconID						201

// image indices
#define kOldQTIconImageIndex				1
#define kNewQTIconImageIndex				2

// sprite atom IDs	
#define kOldQTIconSpriteAtomID				1
#define kNewQTIconSpriteAtomID				2

#define kSpriteMediaFrameDurationIcon		1000

#define kGetStr_DLOGID						1001
#define kGetStr_OKButton					1
#define kGetStr_CancelButton				2
#define kGetStr_StrTextItem					3
#define kGetStr_StrLabelItem				4

#define kTextKindsResourceID				1001
#define kMovieNameResIndex					1
#define kMovieIDResIndex					2

#define kQTTargAlertID						201

// until we get a new Movies.h...
#define mcActionGetMovieName        		90
#define mcActionGetMovieID            		91


//////////
//
// function prototypes
//
//////////

char *							QTTarg_GetStringFromUser (short thePromptStringIndex);
void							QTTarg_ShowStringToUser (StringPtr theString);

OSErr							QTTarg_CreateTwinSpritesMovie (void);
OSErr							QTTarg_AddIconMovieSamplesToMedia (Media theMedia);

void							QTTarg_AddVRControllerButtonSamplesToMedia (Media theMedia, long theTrackWidth, long theTrackHeight, TimeValue theDuration);
void							QTTarg_AddTextButtonSamplesToMedia (Media theMedia, long theTrackWidth, long theTrackHeight, TimeValue theDuration);
OSErr							QTTarg_MakeDualVRControllerMovie (void);
OSErr							QTTarg_AddTextToggleButtonTrack (Movie theMovie);
void							QTTarg_SetTrackProperties (Media theMedia, UInt32 theIdleFrequency);
short							QTTarg_GetLowestLayerInMovie (Movie theMovie);
OSErr							QTTarg_AddIdleEventVarTestAction (QTAtomContainer theContainer, QTAtom theAtom, QTAtomID theVariableID, UInt32 theTrueValue, long theActionConstant, QTAtom *theNewActionAtom);


char *							QTUtils_GetMovieTargetName (Movie theMovie);
OSErr							QTUtils_SetMovieTargetName (Movie theMovie, char *theTargetName);
long							QTUtils_GetMovieTargetID (Movie theMovie, Boolean *theMovieHasID);
OSErr							QTUtils_SetMovieTargetID (Movie theMovie, long theTargetID);

static long						QTUtils_FindUserDataItemWithPrefix (UserData theUserData, OSType theType, char *thePrefix);
static char *					QTUtils_GetUserDataPrefixedValue (UserData theUserData, OSType theType, char *thePrefix);

void							QTFrame_FindExternalMovieTarget (MovieController theMC, QTGetExternalMoviePtr theEMRecPtr);

