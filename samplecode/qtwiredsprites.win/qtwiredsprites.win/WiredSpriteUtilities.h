//////////
//
//	File:		WiredSpriteUtilities.h
//
//	Contains:	Utilities for creating wired sprite media.
//
//	Written by:	Sean Allen
//	Revised by:	Chris Flick and Tim Monroe
//
//	Copyright:	© 1998-2001 by Apple Computer, Inc., all rights reserved.
//
//	   <1>	 	03/02/01	rtm		cosmetic fixes to parallel changes in WiredSpriteUtilities.c
//	  
//
//////////

#ifndef _WIREDSPRITEUTILITIES_
#define _WIREDSPRITEUTILITIES_


//////////
//
// header files
//
//////////

#ifndef __ENDIANUTILITIES__
#include "EndianUtilities.h"
#endif

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif


//////////
//
// constants
//
//////////

enum {
	kFirstParam				= 1,
	kSecondParam			= 2,
	kThirdParam				= 3,
	kFourthParam			= 4,
	kFifthParam				= 5,
	kSixthParam				= 6,
	kSeventhParam			= 7,
	kEighthParam			= 8,
	kNinthParam				= 9,
	kTenthParam				= 10
};


//////////
//
// wired sprite utilities
//
//////////

OSErr		WiredUtils_AddQTEventAtom (QTAtomContainer theContainer, QTAtom theActionAtoms, QTAtomID theQTEventType, QTAtom *theNewQTEventAtom);
OSErr		WiredUtils_AddActionAtom (QTAtomContainer theContainer, QTAtom theEventAtom, long theActionConstant, QTAtom *theNewActionAtom);
OSErr		WiredUtils_AddActionParameterAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theParameterIndex, long theParamDataSize, void *theParamData, QTAtom *theNewParamAtom);
OSErr		WiredUtils_AddActionParameterOptions (QTAtomContainer theContainer, QTAtom theActionAtom, QTAtomID theParamID, long theFlags, long theMinValueSize, void *theMinValue, long theMaxValueSize, void *theMaxValue);

// target-setting utilities
OSErr		WiredUtils_AddTrackNameActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, Str255 theTrackName, QTAtom *theNewTargetAtom);
OSErr		WiredUtils_AddTrackIDActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackID, QTAtom *theNewTargetAtom);
OSErr		WiredUtils_AddTrackTypeActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, OSType theTrackType, QTAtom *theNewTargetAtom);
OSErr		WiredUtils_AddTrackIndexActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackIndex, QTAtom *theNewTargetAtom);
OSErr		WiredUtils_AddSpriteNameActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, Str255 theSpriteName, QTAtom *theNewTargetAtom);
OSErr		WiredUtils_AddSpriteIDActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, QTAtomID theSpriteID, QTAtom *theNewTargetAtom);
OSErr		WiredUtils_AddSpriteIndexActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, short theSpriteIndex, QTAtom *theNewTargetAtom);

// high-level utilities
OSErr		WiredUtils_AddQTEventAndActionAtoms (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theAction, QTAtom *theActionAtom);
OSErr		WiredUtils_AddTrackTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex);
OSErr		WiredUtils_AddSpriteTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theSpriteTargetType, void *theSpriteTarget);
OSErr		WiredUtils_AddTrackAndSpriteTargetAtoms (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget);

// movie action utilities
OSErr		WiredUtils_AddMovieSetVolumeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, short theVolume);
OSErr		WiredUtils_AddMovieSetRateAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Fixed theRate);
OSErr		WiredUtils_AddMovieSetLoopingFlagsAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theLoopingFlags);
OSErr		WiredUtils_AddMovieGoToTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, TimeValue theTime);
OSErr		WiredUtils_AddMovieGoToTimeByNameAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theTimeName);
OSErr		WiredUtils_AddMovieGoToBeginningAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent);
OSErr		WiredUtils_AddMovieGoToEndAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent);
OSErr		WiredUtils_AddMovieStepForwardAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent);
OSErr		WiredUtils_AddMovieStepBackwardAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent);
OSErr		WiredUtils_AddMovieSetSelectionAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, TimeValue theStartTime, TimeValue theEndTime);
OSErr		WiredUtils_AddMovieSetSelectionByNameAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theStartTimeName, Str255 theEndTimeName);
OSErr		WiredUtils_AddMoviePlaySelectionAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Boolean theSelectionOnly);
OSErr		WiredUtils_AddMovieSetLanguage (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theLanguage);

// track action utilities
OSErr		WiredUtils_AddTrackSetVolumeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, short theVolume);
OSErr		WiredUtils_AddTrackSetBalanceAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, short theBalance);
OSErr		WiredUtils_AddTrackSetEnabledAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, Boolean theEnabled);
OSErr		WiredUtils_AddTrackSetMatrixAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, MatrixRecordPtr theMatrix, QTAtom *theActionAtom);
OSErr		WiredUtils_AddTrackSetLayerAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, short theLayer);
OSErr		WiredUtils_AddTrackSetClipAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, RgnHandle theClip);

// sprite action utilities
OSErr		WiredUtils_AddSpriteSetMatrixAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, MatrixRecordPtr theMatrix, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteSetImageIndexAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, short theImageIndex, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteSetVisibleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, short theVisible, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteSetLayerAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, short theLayer);
OSErr		WiredUtils_AddSpriteSetGraphicsModeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, ModifierTrackGraphicsModeRecord *theGraphicsMode, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteTranslateAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget,Fixed theX, Fixed theY, Boolean theRelative, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteScaleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, Fixed theXScale, Fixed theYScale, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteRotateAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, Fixed theDegrees, QTAtom *theActionAtom);
OSErr		WiredUtils_AddSpriteStretchAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, Fixed p1x, Fixed p1y, Fixed p2x, Fixed p2y, Fixed p3x, Fixed p3y, Fixed p4x, Fixed p4y, QTAtom *theActionAtom);

// music action utilities
OSErr		WiredUtils_AddMusicPlayNoteAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSampleDescIndex, long thePartIndex, long thePitch, long theVelocity, long theDuration);

// sprite track action utilities
OSErr		WiredUtils_AddSpriteTrackSetVariableAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, QTAtomID theVariableID, float theValue, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex);

// system action utilities
OSErr		WiredUtils_AddGoToURLAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Handle theURL);
OSErr		WiredUtils_AddSendQTEventAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, QTEventRecordPtr theEventRec, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget);
OSErr		WiredUtils_AddDebugStrAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theString);
OSErr		WiredUtils_AddPushCurrentTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent);
OSErr		WiredUtils_AddPushCurrentTimeWithLabelAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theLabel);
OSErr		WiredUtils_AddPopAndGotoTopTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent);
OSErr		WiredUtils_AddPopAndGotoLabeledTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theLabel);
OSErr		WiredUtils_AddApplicationNumberAndStringAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theNumber, Str255 theString);

// expression and flow-control utilities
OSErr		WiredUtils_AddOperandAtom (QTAtomContainer theContainer, QTAtom theOperatorAtom, QTAtomType theOperandType, short theOperandIndex, QTAtomContainer theOperandAtoms, float theConstantValue);
OSErr		WiredUtils_AddVariableOperandAtom (QTAtomContainer theContainer, QTAtom theOperatorAtom, short theOperandIndex, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, QTAtomID theVariableID);
OSErr		WiredUtils_AddOperatorAtom (QTAtomContainer theContainer, QTAtom theExpressionAtom, QTAtomID theOperatorType, QTAtom *theOperatorAtom);
OSErr		WiredUtils_AddExpressionContainerAtomType (QTAtomContainer theContainer, QTAtom theAtom, QTAtom *theExpressionAtom);
OSErr		WiredUtils_AddConditionalAtom (QTAtomContainer theContainer, QTAtom theAtom, short theConditionIndex, QTAtom *theConditionalAtom);
OSErr		WiredUtils_AddActionListAtom (QTAtomContainer theContainer, QTAtom theAtom, QTAtom *theActionListAtom);

#endif	// _WIREDSPRITEUTILITIES_
