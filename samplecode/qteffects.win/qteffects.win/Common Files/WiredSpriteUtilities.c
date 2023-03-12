//////////
//
//	File:		WiredSpriteUtilities.c
//
//	Contains:	Utilities for creating wired sprite media.
//
//	Written by:	Sean Allen
//	Revised by:	Chris Flick and Tim Monroe
//
//	Copyright:	© 1998-2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//     <7>      02/21/03    era     kActionTrackSetClip takes a *RgnHandle as its parameter, not a RgnHandle (radar #3115855)
//	   <6>	 	02/28/01	rtm		added movie target utilities
//	   <5>	 	02/14/01	rtm		added QuickTime VR utilities; revised some other utilities
//	   <4>	 	02/02/01	rtm		general clean-up to bring this file into conformance with style of other
//									sample code; added some endian macros; removed some parameters from calls
//									(changes in the action parameters made them orphans)
//	   <3>	 	03/29/98	rtm		added Endian macros to theFlags parameter in WiredUtils_AddActionParameterOptions
//	   <2>	 	03/??/98	cf		added Endian macros
//	   <1>	 	12/??/97	sa		first file
//
//	NOTES:
//
//	***(1)***
//	You need to pay attention to the endianness of the data you pass to these routines. Wired sprite
//	media data is stored in QuickTime atoms and atom containers and must therefore be big-endian. We've
//	tried to conform to this rule: if the data to be written to a wired sprite media is 4 bytes or less,
//	then we will perform the endian swap for you. There are several exceptions to this rule; for instance,
//	we swap the data in any matrices you pass to the WiredUtils_AddSpriteSetMatrixAction function. But we
//	do not swap the data in any matrices passed to WiredUtils_AddActionParameterAtom (since we don't know
//	that they are matrices!). Let the caller beware!
//
//////////

#ifndef _WIREDSPRITEUTILITIES_
#include "WiredSpriteUtilities.h"
#endif


//////////
//
// WiredUtils_AddQTEventAtom
// Add an event atom of the specified type to the specified container. Return the new event atom to the caller.
//
// For an event of type kQTEventFrameLoaded, the theActionAtoms atom should be a sibling of a kSpriteAtomType.
//
//////////

OSErr WiredUtils_AddQTEventAtom (QTAtomContainer theContainer, QTAtom theActionAtoms, QTAtomID theQTEventType, QTAtom *theNewQTEventAtom)
{
	OSErr	myErr = noErr;
	
	if ((theContainer == NULL) || (theQTEventType == 0) || (theNewQTEventAtom == NULL)) {
		myErr = paramErr;
		goto bail;
	}
	
	if (theQTEventType == kQTEventFrameLoaded) {
		*theNewQTEventAtom = QTFindChildByID(theContainer, theActionAtoms, kQTEventFrameLoaded, 1, NULL);
		if (*theNewQTEventAtom == 0)
			myErr = QTInsertChild(theContainer, theActionAtoms, kQTEventFrameLoaded, 1, 1, 0, NULL, theNewQTEventAtom);
	} else {
		*theNewQTEventAtom = QTFindChildByID(theContainer, theActionAtoms, kQTEventType, theQTEventType, NULL);
		if (*theNewQTEventAtom == 0)
			myErr = QTInsertChild(theContainer, theActionAtoms, kQTEventType, theQTEventType, 1, 0, NULL, theNewQTEventAtom);
	}
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddActionAtom
// Add an action atom of the specified type to the specified container. Return the new event atom to the caller.
//
//////////

OSErr WiredUtils_AddActionAtom (QTAtomContainer theContainer, QTAtom theEventAtom, long theActionConstant, QTAtom *theNewActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	if ((theContainer == NULL) || (theActionConstant == 0)) {
		myErr = paramErr;
		goto bail;
	}
	
	myErr = QTInsertChild(theContainer, theEventAtom, kAction, 0, 0, 0, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theActionConstant = EndianU32_NtoB(theActionConstant);
	myErr = QTInsertChild(theContainer, myActionAtom, kWhichAction, 1, 1, sizeof(theActionConstant), &theActionConstant, NULL);
	
bail:
	if (theNewActionAtom != NULL) {
		if (myErr != noErr)	
			*theNewActionAtom = 0;
		else
			*theNewActionAtom = myActionAtom;
	}
	
	return(myErr);
}


//////////
//
// WiredUtils_AddActionParameterAtom
// Add a parameter atom of the specified type to the specified action atom. Return the new parameter atom to the caller.
//
// N.B. The caller is responsible for ensuring that the data passed in the theParamData parameter is in big-endian format!
// You've been warned!
//
//////////

OSErr WiredUtils_AddActionParameterAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theParameterIndex, long theParamDataSize, void *theParamData, QTAtom *theNewParamAtom)
{
	return(QTInsertChild(theContainer, theActionAtom, kActionParameter, 0, (short)theParameterIndex, theParamDataSize, theParamData, theNewParamAtom));
}


//////////
//
// WiredUtils_AddActionParameterOptions
// Add a parameter options atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddActionParameterOptions (QTAtomContainer theContainer, QTAtom theActionAtom, QTAtomID theParamID, long theFlags, long theMinValueSize, void *theMinValue, long theMaxValueSize, void *theMaxValue)
{
	OSErr	myErr = noErr;

	theFlags = EndianU32_NtoB(theFlags);
	myErr = QTInsertChild(theContainer, theActionAtom, kActionFlags, theParamID, 0, sizeof(theFlags), &theFlags, NULL);
	if (myErr != noErr)
		goto bail;
		
	if (theMinValue != NULL)
		myErr = QTInsertChild(theContainer, theActionAtom, kActionParameterMinValue, theParamID, 0, theMinValueSize, theMinValue, NULL);

	if (theMaxValue != NULL)
		myErr = QTInsertChild(theContainer, theActionAtom, kActionParameterMaxValue, theParamID, 0, theMaxValueSize, theMaxValue, NULL);

bail:
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Target-setting utilities
//
// Use these functions to set track or sprite targets.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddMovieNameActionTargetAtom
// Add a target movie name atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddMovieNameActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, Str255 theMovieName, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}
		
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetMovieName, 1, 1, theMovieName[0] + 1, theMovieName, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieIDActionTargetAtom
// Add a target movie ID atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddMovieIDActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theMovieID, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	theMovieID = EndianU32_NtoB(theMovieID);
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetMovieID, 1, 1, sizeof(theMovieID), &theMovieID, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackNameActionTargetAtom
// Add a target track name atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddTrackNameActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, Str255 theTrackName, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}
		
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetTrackName, 1, 1, theTrackName[0] + 1, theTrackName, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackIDActionTargetAtom
// Add a target track ID atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddTrackIDActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackID, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	theTrackID = EndianU32_NtoB(theTrackID);
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetTrackID, 1, 1, sizeof(theTrackID), &theTrackID, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackTypeActionTargetAtom
// Add a target track type atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddTrackTypeActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, OSType theTrackType, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}
			
	theTrackType = EndianU32_NtoB(theTrackType);
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetTrackType, 1, 1, sizeof(theTrackType), &theTrackType, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackIndexActionTargetAtom
// Add a target track index atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddTrackIndexActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackIndex, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}
			
	theTrackIndex = EndianU32_NtoB(theTrackIndex);
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetTrackIndex, 1, 1, sizeof(theTrackIndex), &theTrackIndex, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteNameActionTargetAtom
// Add a sprite name target atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddSpriteNameActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, Str255 theSpriteName, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}

	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetSpriteName, 1, 1, theSpriteName[0] + 1, theSpriteName, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteIDActionTargetAtom
// Add a sprite ID target atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddSpriteIDActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, QTAtomID theSpriteID, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}

	theSpriteID = EndianU32_NtoB(theSpriteID);
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetSpriteID, 1, 1, sizeof(theSpriteID), &theSpriteID, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteIndexActionTargetAtom
// Add a sprite index target atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddSpriteIndexActionTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, short theSpriteIndex, QTAtom *theNewTargetAtom)
{
	QTAtom	myTargetAtom = 0;
	OSErr	myErr = noErr;
	
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = 0;
	
	myTargetAtom = QTFindChildByIndex(theContainer, theActionAtom, kActionTarget, 1, NULL);
	if (myTargetAtom == 0) {
		myErr = QTInsertChild(theContainer, theActionAtom, kActionTarget, 1, 1, 0, NULL, &myTargetAtom);
		if (myErr != noErr)
			goto bail;
	}

	theSpriteIndex = EndianU16_NtoB(theSpriteIndex);
	myErr = QTInsertChild(theContainer, myTargetAtom, kTargetSpriteIndex, 1, 1, sizeof(theSpriteIndex), &theSpriteIndex, NULL);

bail:
	if (theNewTargetAtom != NULL)
		*theNewTargetAtom = myTargetAtom;
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// High-level utilities
//
// Use these functions to add event and action atoms, and to set movie, track, and sprite targets.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddQTEventAndActionAtoms
// Add event and action atoms to the specified atom.
//
//////////

OSErr WiredUtils_AddQTEventAndActionAtoms (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theAction, QTAtom *theActionAtom)
{
	QTAtom	myEventAtom = 0;
	OSErr	myErr = noErr;
	
	myEventAtom = theAtom;
	
	if (theEvent != 0) {
		myErr = WiredUtils_AddQTEventAtom(theContainer, theAtom, theEvent, &myEventAtom);
		if (myErr != noErr)
			goto bail;
	}
	
	myErr = WiredUtils_AddActionAtom(theContainer, myEventAtom, theAction, theActionAtom);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieTargetAtom
// Add a movie target atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddMovieTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theMovieTargetType, void *theMovieTarget)
{
	OSErr	myErr = noErr;
	
	// allow zero for the default target (the movie that received the event)
	if (theMovieTargetType != 0) {
		switch (theMovieTargetType) {
			case kTargetMovieName: {
				StringPtr myMovieName = (StringPtr)theMovieTarget;
				
				myErr = WiredUtils_AddMovieNameActionTargetAtom(theContainer, theActionAtom, myMovieName, NULL);
				break;
			}
				
			case kTargetMovieID: {
				long myMovieID = (long)theMovieTarget;
				
				myErr = WiredUtils_AddMovieIDActionTargetAtom(theContainer, theActionAtom, myMovieID, NULL);
				break;
			}

			default:
				myErr = paramErr; 
		}
	}
	
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackTargetAtom
// Add a track target atom to the specified action atom.
//
// trackTypeIndex only used if trackTargetType is kTargetTrackType; it can be zero for a default index of 1
//
//////////

OSErr WiredUtils_AddTrackTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex)
{
	OSErr	myErr = noErr;
	
	// allow zero for the default target (the sprite track that received the event)
	if (theTrackTargetType != 0) {
		switch (theTrackTargetType) {
			case kTargetTrackName: {
				StringPtr myTrackName = (StringPtr)theTrackTarget;
				
				myErr = WiredUtils_AddTrackNameActionTargetAtom(theContainer, theActionAtom, myTrackName, NULL);
				break;
			}
				
			case kTargetTrackID: {
				long myTrackID = (long)theTrackTarget;
				
				myErr = WiredUtils_AddTrackIDActionTargetAtom(theContainer, theActionAtom, myTrackID, NULL);
				break;
			}

			case kTargetTrackType: {
				OSType myTrackType = (long)theTrackTarget;
				
				myErr = WiredUtils_AddTrackTypeActionTargetAtom(theContainer, theActionAtom, myTrackType, NULL);
				if (myErr != noErr)
					goto bail;
					
				if (theTrackTypeIndex != 0)
					myErr = WiredUtils_AddTrackIndexActionTargetAtom(theContainer, theActionAtom, theTrackTypeIndex, NULL);
				break;
			}
						
			case kTargetTrackIndex: {
				long myTrackIndex = (long)theTrackTarget;
				
				myErr = WiredUtils_AddTrackIndexActionTargetAtom(theContainer, theActionAtom, myTrackIndex, NULL);
				break;
			}
						
			default:
				myErr = paramErr; 
		}
	}
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteTargetAtom
// Add a sprite target atom to the specified action atom.
//
//////////

OSErr WiredUtils_AddSpriteTargetAtom (QTAtomContainer theContainer, QTAtom theActionAtom, long theSpriteTargetType, void *theSpriteTarget)
{
	OSErr	myErr = noErr;
	
	// allow zero for the default target (the sprite that received the event)
	if (theSpriteTargetType != 0) {
		switch (theSpriteTargetType) {
			case kTargetSpriteName: {
				StringPtr mySpriteName = (StringPtr)theSpriteTarget;
				
				myErr = WiredUtils_AddSpriteNameActionTargetAtom(theContainer, theActionAtom, mySpriteName, NULL);
				break;
			}
				
			case kTargetSpriteID: {
				QTAtomID mySpriteID = (QTAtomID)theSpriteTarget;
				
				myErr = WiredUtils_AddSpriteIDActionTargetAtom(theContainer, theActionAtom, mySpriteID, NULL);
				break;
			}

			case kTargetSpriteIndex: {
				short mySpriteIndex = (short)theSpriteTarget;
				
				myErr = WiredUtils_AddSpriteIndexActionTargetAtom(theContainer, theActionAtom, mySpriteIndex, NULL);
				break;
			}
						
			default:
				myErr = paramErr; 
		}
	}
	
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackAndSpriteTargetAtoms
// Add track and sprite target atoms to the specified action atom.
//
//////////

OSErr WiredUtils_AddTrackAndSpriteTargetAtoms (QTAtomContainer theContainer, QTAtom theActionAtom, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget)
{
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddTrackTargetAtom(theContainer, theActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddSpriteTargetAtom(theContainer, theActionAtom, theSpriteTargetType, theSpriteTarget);
	
bail:
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Movie action utilities
//
// Use these functions to add actions to movies.
//
// The movie is the default target for all movie actions, so we don't need to set an explicit target.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddMovieSetVolumeAction
// Add a kActionMovieSetVolume action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieSetVolumeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, short theVolume)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieSetVolume, &myActionAtom);
	if (myErr != noErr)
		goto bail;
		
	theVolume = EndianS16_NtoB(theVolume);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theVolume), &theVolume, NULL);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieSetRateAction
// Add a kActionMovieSetRate action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieSetRateAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Fixed theRate)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieSetRate, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theRate = EndianS32_NtoB(theRate);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theRate), &theRate, NULL);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieSetLoopingFlagsAction
// Add a kActionMovieSetLoopingFlags action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieSetLoopingFlagsAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theLoopingFlags)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieSetLoopingFlags, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theLoopingFlags = EndianU32_NtoB(theLoopingFlags);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theLoopingFlags), &theLoopingFlags, NULL);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieGoToTimeAction
// Add a kActionMovieGoToTime action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieGoToTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, TimeValue theTime)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieGoToTime, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theTime = EndianS32_NtoB(theTime);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theTime), &theTime, NULL);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieGoToTimeByNameAction
// Add a kActionMovieGoToTimeByName action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieGoToTimeByNameAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theTimeName)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieGoToTimeByName, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, theTimeName[0] + 1, theTimeName, NULL);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieGoToBeginningAction
// Add a kActionMovieGoToBeginning action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieGoToBeginningAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent)
{	
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieGoToBeginning, NULL));
}


//////////
//
// WiredUtils_AddMovieGoToEndAction
// Add a kActionMovieGoToEnd action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieGoToEndAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent)
{	
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieGoToEnd, NULL));
}


//////////
//
// WiredUtils_AddMovieStepForwardAction
// Add a kActionMovieStepForward action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieStepForwardAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent)
{	
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieStepForward, NULL));
}


//////////
//
// WiredUtils_AddMovieStepBackwardAction
// Add a kActionMovieStepBackward action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieStepBackwardAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent)
{	
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieStepBackward, NULL));
}


//////////
//
// WiredUtils_AddMovieSetSelectionAction
// Add a kActionMovieSetSelection action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieSetSelectionAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, TimeValue theStartTime, TimeValue theEndTime)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieSetSelection, &myActionAtom);
	if (myErr != noErr)
		goto bail;
		
	theStartTime = EndianS32_NtoB(theStartTime);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theStartTime), &theStartTime, NULL);
	if (myErr != noErr)
		goto bail;

	theEndTime = EndianS32_NtoB(theEndTime);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(theEndTime), &theEndTime, NULL);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieSetSelectionByNameAction
// Add a kActionMovieSetSelectionByName action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieSetSelectionByNameAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theStartTimeName, Str255 theEndTimeName)
{	
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieSetSelectionByName, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, theStartTimeName[0] + 1, theStartTimeName, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, theEndTimeName[0] + 1, theEndTimeName, NULL);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMoviePlaySelectionAction
// Add a kActionMoviePlaySelection action to the specified atom.
//
//////////

OSErr WiredUtils_AddMoviePlaySelectionAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Boolean theSelectionOnly)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMoviePlaySelection, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	// Booleans don't need endian flipping
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theSelectionOnly), &theSelectionOnly, NULL);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddMovieSetLanguage
// Add a kActionMovieSetLanguage action to the specified atom.
//
//////////

OSErr WiredUtils_AddMovieSetLanguage (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theLanguage)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMovieSetLanguage, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theLanguage = EndianS32_NtoB(theLanguage);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theLanguage), &theLanguage, NULL);

bail:
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Track action utilities
//
// Use these functions to add actions to tracks.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddTrackSetVolumeAction
// Add a kActionTrackSetVolume action to the specified atom.
//
//////////

OSErr WiredUtils_AddTrackSetVolumeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, short theVolume)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionTrackSetVolume, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theVolume = EndianS16_NtoB(theVolume);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theVolume), &theVolume, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackSetBalanceAction
// Add a kActionTrackSetBalance action to the specified atom.
//
//////////

OSErr WiredUtils_AddTrackSetBalanceAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, short theBalance)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionTrackSetBalance, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theBalance = EndianS16_NtoB(theBalance);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theBalance), &theBalance, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackSetEnabledAction
// Add a kActionTrackSetEnabled action to the specified atom.
//
//////////

OSErr WiredUtils_AddTrackSetEnabledAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, Boolean theEnabled)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionTrackSetEnabled, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theEnabled), &theEnabled, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackSetMatrixAction
// Add a kActionTrackSetMatrix action to the specified atom.
//
//////////

OSErr WiredUtils_AddTrackSetMatrixAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, MatrixRecordPtr theMatrix, QTAtom *theActionAtom)
{
	QTAtom			myActionAtom = 0;
	MatrixRecord	myMatrix;
	OSErr			myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionTrackSetMatrix, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myMatrix = *theMatrix;
	EndianUtils_MatrixRecord_NtoB(&myMatrix);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(MatrixRecord), &myMatrix, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
	
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackSetLayerAction
// Add a kActionTrackSetLayer action to the specified atom.
//
//////////

OSErr WiredUtils_AddTrackSetLayerAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, short theLayer)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionTrackSetLayer, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theLayer = EndianS16_NtoB(theLayer);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theLayer), &theLayer, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddTrackSetClipAction
// Add a kActionTrackSetClip action to the specified atom.
//
//////////

OSErr WiredUtils_AddTrackSetClipAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, RgnHandle theClip)
{
#if TARGET_RT_LITTLE_ENDIAN
	RgnHandle		myClip = NULL;
#else
	SignedByte		mySavedState;
#endif
	QTAtom			myActionAtom = 0;
	OSErr			myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionTrackSetClip, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	// theClip is native-endian but it needs to be big-endian to be stored in the data atom container;
	// we may need to flip it

#if TARGET_RT_LITTLE_ENDIAN
	// for little-endian platforms, we copy the clip into a temporary region handle and then flip that region
	myClip = (RgnHandle)NewHandle(0);
	myErr = MemError();
	if (myErr != noErr)
		goto bail;
	
	myErr = HandAndHand((Handle)theClip, (Handle)myClip);
	if (myErr != noErr)
		goto bail;
	
	EndianUtils_RgnHandle_NtoB(myClip);

	// lock the handle down so that it doesn't move during addition to theContainer;
	// we don't unlock since we dispose of it at bottom of this function.
	HLockHi((Handle)myClip);
	
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, GetHandleSize((Handle)myClip), *myClip, NULL);
	if (myErr != noErr)
		goto bail;
#else
	// for big endian platforms, just add the clip
	
	// lock the handle down so that it doesn't move during addition to container QTAtomContainer;
	// save state so that we can restore its "lockedness" when we're done
	mySavedState = HGetState((Handle)theClip);
	HLockHi((Handle)theClip);
	
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, GetHandleSize((Handle)theClip), *(Handle)theClip, NULL);
	
	// restore handle's "lockedness"
	HSetState((Handle)theClip, mySavedState);
	if (myErr != noErr)
		goto bail;
#endif

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
	
bail:
#if TARGET_RT_LITTLE_ENDIAN
	if (myClip != NULL)
		DisposeRgn(myClip);
#endif
	
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite action utilities
//
// Use these functions to add actions to sprites.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddSpriteSetMatrixAction
// Add a kActionSpriteSetMatrix action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteSetMatrixAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, MatrixRecordPtr theMatrix, QTAtom *theActionAtom)
{
	QTAtom			myActionAtom = 0;
	MatrixRecord	myMatrix;
	OSErr			myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteSetMatrix, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myMatrix = *theMatrix;
	EndianUtils_MatrixRecord_NtoB(&myMatrix);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(MatrixRecord), &myMatrix, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;
			   								 
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteSetImageIndexAction
// Add a kActionSpriteSetImageIndex action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteSetImageIndexAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, short theImageIndex, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteSetImageIndex, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theImageIndex = EndianS16_NtoB(theImageIndex);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theImageIndex), &theImageIndex, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;
			   								 
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteSetVisibleAction
// Add a kActionSpriteSetVisible action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteSetVisibleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, short theVisible, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteSetVisible, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theVisible = EndianS16_NtoB(theVisible);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theVisible), &theVisible, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;
											 
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteSetLayerAction
// Add a kActionSpriteSetLayer action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteSetLayerAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, short theLayer)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteSetLayer, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theLayer = EndianS16_NtoB(theLayer);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theLayer), &theLayer, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteSetGraphicsModeAction
// Add a kActionSpriteSetGraphicsMode action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteSetGraphicsModeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, ModifierTrackGraphicsModeRecord *theGraphicsMode, QTAtom *theActionAtom)
{
	ModifierTrackGraphicsModeRecord		myGraphicsMode;
	QTAtom								myActionAtom = 0;
	OSErr								myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteSetGraphicsMode, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myGraphicsMode 					= *theGraphicsMode;
	myGraphicsMode.graphicsMode		= EndianU32_NtoB(myGraphicsMode.graphicsMode);
	myGraphicsMode.opColor.red 		= EndianU16_NtoB(myGraphicsMode.opColor.red);
	myGraphicsMode.opColor.green	= EndianU16_NtoB(myGraphicsMode.opColor.green);
	myGraphicsMode.opColor.blue		= EndianU16_NtoB(myGraphicsMode.opColor.blue);
	
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(ModifierTrackGraphicsModeRecord), &myGraphicsMode, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;
		
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteTranslateAction
// Add a kActionSpriteTranslate action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteTranslateAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget,Fixed theX, Fixed theY, Boolean theRelative, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteTranslate, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theX = EndianS32_NtoB(theX);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theX), &theX, NULL);
	if (myErr != noErr)
		goto bail;

	theY = EndianS32_NtoB(theY);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(theY), &theY, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kThirdParam, sizeof(theRelative), &theRelative, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteScaleAction
// Add a kActionSpriteScale action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteScaleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, Fixed theXScale, Fixed theYScale, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteScale, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theXScale = EndianS32_NtoB(theXScale);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theXScale), &theXScale, NULL);
	if (myErr != noErr)
		goto bail;

	theYScale = EndianS32_NtoB(theYScale);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(theYScale), &theYScale, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteRotateAction
// Add a kActionSpriteRotate action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteRotateAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, Fixed theDegrees, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteRotate, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theDegrees = EndianS32_NtoB(theDegrees);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theDegrees), &theDegrees, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSpriteStretchAction
// Add a kActionSpriteStretch action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteStretchAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget, Fixed p1x, Fixed p1y, Fixed p2x, Fixed p2y, Fixed p3x, Fixed p3y, Fixed p4x, Fixed p4y, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteStretch, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	p1x = EndianS32_NtoB(p1x);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(p1x), &p1x, NULL);
	if (myErr != noErr)
		goto bail;

	p1y = EndianS32_NtoB(p1y);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(p1y), &p1y, NULL);
	if (myErr != noErr)
		goto bail;

	p2x = EndianS32_NtoB(p2x);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kThirdParam, sizeof(p2x), &p2x, NULL);
	if (myErr != noErr)
		goto bail;

	p2y = EndianS32_NtoB(p2y);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFourthParam, sizeof(p2y), &p2y, NULL);
	if (myErr != noErr)
		goto bail;

	p3x = EndianS32_NtoB(p3x);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFifthParam, sizeof(p3x), &p3x, NULL);
	if (myErr != noErr)
		goto bail;

	p3y = EndianS32_NtoB(p3y);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSixthParam, sizeof(p3y), &p3y, NULL);
	if (myErr != noErr)
		goto bail;

	p4x = EndianS32_NtoB(p4x);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSeventhParam, sizeof(p4x), &p4x, NULL);
	if (myErr != noErr)
		goto bail;

	p4y = EndianS32_NtoB(p4y);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kEighthParam, sizeof(p4y), &p4y, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// QuickTime VR utilities
//
// Use these functions to add QTVR-related actions.
//
// The movie is the default target for all QTVR actions, so we don't need to set an explicit target.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddQTVRSetAngleAction
// Add an angle action of the specified type to the specified atom.
//
//////////

static OSErr WiredUtils_AddQTVRSetAngleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, float theAngle, QTAtom *theActionAtom, long theActionConstant)
{
	QTAtom	myActionAtom = 0;
	float	myAngle = theAngle;
	OSErr	myErr = noErr;
	
	// make sure the caller passed us a valid angle action constant
	if ((theActionConstant < kActionQTVRSetPanAngle) || (theActionConstant > kActionQTVRSetFieldOfView))
		return(paramErr);
		
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, theActionConstant, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	EndianUtils_Float_NtoB(&myAngle);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(myAngle), &myAngle, NULL);
	if (myErr != noErr)
		goto bail;

	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddQTVRSetPanAngleAction
// Add a kActionQTVRSetPanAngle action to the specified atom.
//
//////////

OSErr WiredUtils_AddQTVRSetPanAngleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, float theAngle, QTAtom *theActionAtom)
{
	return(WiredUtils_AddQTVRSetAngleAction(theContainer, theAtom, theEvent, theAngle, theActionAtom, kActionQTVRSetPanAngle));
}


//////////
//
// WiredUtils_AddQTVRSetTiltAngleAction
// Add a kActionQTVRSetTiltAngle action to the specified atom.
//
//////////

OSErr WiredUtils_AddQTVRSetTiltAngleAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, float theAngle, QTAtom *theActionAtom)
{
	return(WiredUtils_AddQTVRSetAngleAction(theContainer, theAtom, theEvent, theAngle, theActionAtom, kActionQTVRSetTiltAngle));
}


//////////
//
// WiredUtils_AddQTVRSetFieldOfViewAction
// Add a kActionQTVRSetFieldOfView action to the specified atom.
//
//////////

OSErr WiredUtils_AddQTVRSetFieldOfViewAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, float theAngle, QTAtom *theActionAtom)
{
	return(WiredUtils_AddQTVRSetAngleAction(theContainer, theAtom, theEvent, theAngle, theActionAtom, kActionQTVRSetFieldOfView));
}


//////////
//
// WiredUtils_AddQTVRShowDefaultViewAction
// Add a kActionQTVRShowDefaultView action to the specified atom.
//
//////////

OSErr WiredUtils_AddQTVRShowDefaultViewAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, QTAtom *theActionAtom)
{
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionQTVRShowDefaultView, theActionAtom));
}


//////////
//
// WiredUtils_AddQTVRGoToNodeIDAction
// Add a kActionQTVRGoToNodeID action to the specified atom.
//
//////////

OSErr WiredUtils_AddQTVRGoToNodeIDAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, UInt32 theNodeID, QTAtom *theActionAtom)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionQTVRGoToNodeID, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theNodeID = EndianU32_NtoB(theNodeID);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theNodeID), &theNodeID, NULL);
	if (myErr != noErr)
		goto bail;

	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

bail:
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Music utilities
//
// Use these functions to add music actions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddMusicPlayNoteAction
// Add a kActionMusicPlayNote action to the specified atom.
//
//////////

OSErr WiredUtils_AddMusicPlayNoteAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSampleDescIndex, long thePartIndex, long thePitch, long theVelocity, long theDuration)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionMusicPlayNote, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theSampleDescIndex = EndianS32_NtoB(theSampleDescIndex);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theSampleDescIndex), &theSampleDescIndex, NULL);
	if (myErr != noErr)
		goto bail;
	
	thePartIndex = EndianS32_NtoB(thePartIndex);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(thePartIndex), &thePartIndex, NULL);
	if (myErr != noErr)
		goto bail;
	
	thePitch = EndianS32_NtoB(thePitch);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kThirdParam, sizeof(thePitch), &thePitch, NULL);
	if (myErr != noErr)
		goto bail;
	
	theVelocity = EndianS32_NtoB(theVelocity);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFourthParam, sizeof(theVelocity), &theVelocity, NULL);
	if (myErr != noErr)
		goto bail;
	
	theDuration = EndianS32_NtoB(theDuration);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFifthParam, sizeof(theDuration), &theDuration, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);

bail:
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite track utilities
//
// Use these functions to add sprite track actions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddSpriteTrackSetVariableAction
// Add a kActionSpriteTrackSetVariable action to the specified atom.
//
//////////

OSErr WiredUtils_AddSpriteTrackSetVariableAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, QTAtomID theVariableID, float theValue, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSpriteTrackSetVariable, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theVariableID = EndianU32_NtoB(theVariableID);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theVariableID), &theVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	EndianUtils_Float_NtoB(&theValue);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(theValue), &theValue, NULL);

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myActionAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);

bail:
	return(myErr);
}									   
						

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// System action utilities
//
// Use these functions to add system actions. Most of these have no target.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddGoToURLAction
// Add a kActionGoToURL action to the specified atom.
//
//////////

OSErr WiredUtils_AddGoToURLAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Handle theURL)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionGoToURL, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, GetHandleSize(theURL), *theURL, NULL);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddSendQTEventAction
// Add a kActionSendQTEventToSprite action to the specified atom.
//
//////////

OSErr WiredUtils_AddSendQTEventAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, QTEventRecordPtr theEventRec, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, long theSpriteTargetType, void *theSpriteTarget)
{
	QTAtom	myActionAtom = 0;
	QTAtom	myParamAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionSendQTEventToSprite, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;

	// note that these target atoms are the first parameter to the kActionSendQTEvent action 
	myErr = WiredUtils_AddTrackAndSpriteTargetAtoms(theContainer, myParamAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex, theSpriteTargetType, theSpriteTarget);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, sizeof(*theEventRec), theEventRec, &myParamAtom);

bail:
	return(myErr);
}									   
						

//////////
//
// WiredUtils_AddDebugStrAction
// Add a kActionDebugStr action to the specified atom.
//
//////////

OSErr WiredUtils_AddDebugStrAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theString)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionDebugStr, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, theString[0] + 1, theString, NULL);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddPushCurrentTimeAction
// Add a kActionPushCurrentTime action to the specified atom.
//
//////////

OSErr WiredUtils_AddPushCurrentTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent)
{
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionPushCurrentTime, NULL));
}
			

//////////
//
// WiredUtils_AddPushCurrentTimeWithLabelAction
// Add a kActionPushCurrentTimeWithLabel action to the specified atom.
//
//////////

OSErr WiredUtils_AddPushCurrentTimeWithLabelAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theLabel)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr =WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionPushCurrentTimeWithLabel, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, theLabel[0] + 1, theLabel, NULL);

bail:
	return(myErr);
}
		

//////////
//
// WiredUtils_AddPopAndGotoTopTimeAction
// Add a kActionPopAndGotoTopTime action to the specified atom.
//
//////////

OSErr WiredUtils_AddPopAndGotoTopTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent)
{
	return(WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionPopAndGotoTopTime, NULL));
}


//////////
//
// WiredUtils_AddPopAndGotoLabeledTimeAction
// Add a kActionPopAndGotoLabeledTime action to the specified atom.
//
//////////

OSErr WiredUtils_AddPopAndGotoLabeledTimeAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, Str255 theLabel)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionPopAndGotoLabeledTime, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, theLabel[0] + 1, theLabel, NULL);

bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddApplicationNumberAndStringAction
// Add a kActionApplicationNumberAndString action to the specified atom.
//
//////////

OSErr WiredUtils_AddApplicationNumberAndStringAction (QTAtomContainer theContainer, QTAtom theAtom, long theEvent, long theNumber, Str255 theString)
{
	QTAtom	myActionAtom = 0;
	OSErr	myErr = noErr;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(theContainer, theAtom, theEvent, kActionApplicationNumberAndString, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	theNumber = EndianS32_NtoB(theNumber);
	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kFirstParam, sizeof(theNumber), &theNumber, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theContainer, myActionAtom, kSecondParam, theString[0] + 1, theString, NULL);

bail:
	return(myErr);
}
						
							   
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Expression and flow-control utilities
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// WiredUtils_AddOperandAtom
// Add a kOperandAtomType atom to the specified atom.
//
//////////

OSErr WiredUtils_AddOperandAtom (QTAtomContainer theContainer, QTAtom theOperatorAtom, QTAtomType theOperandType, short theOperandIndex, QTAtomContainer theOperandAtoms, float theConstantValue)
{
	QTAtom	myOperandAtom, myOperandTypeAtom;
	OSErr 	myErr = noErr;
	
	myErr = QTInsertChild(theContainer, theOperatorAtom, kOperandAtomType, 0, theOperandIndex, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theContainer, myOperandAtom, theOperandType, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	if (theOperandType == kOperandConstant) {
		EndianUtils_Float_NtoB(&theConstantValue);
		myErr = QTSetAtomData(theContainer, myOperandTypeAtom, sizeof(theConstantValue), &theConstantValue);
	} else {
		if (theOperandAtoms != NULL)
			myErr = QTInsertChildren(theContainer, myOperandTypeAtom, theOperandAtoms);
	}
	
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddVariableOperandAtom
// Add a kOperandAtomType atom for a variable to the specified atom.
//
//////////

OSErr WiredUtils_AddVariableOperandAtom (QTAtomContainer theContainer, QTAtom theOperatorAtom, short theOperandIndex, long theTrackTargetType, void *theTrackTarget, long theTrackTypeIndex, QTAtomID theVariableID)
{
	QTAtom	myOperandAtom, myOperandTypeAtom;
	OSErr 	myErr = noErr;
	
	myErr = QTInsertChild(theContainer, theOperatorAtom, kOperandAtomType, 0, theOperandIndex, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theContainer, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	theVariableID = EndianU32_NtoB(theVariableID);
	myErr = QTInsertChild(theContainer, myOperandTypeAtom, kActionParameter, 1, 1, sizeof(theVariableID), &theVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddTrackTargetAtom(theContainer, myOperandTypeAtom, theTrackTargetType, theTrackTarget, theTrackTypeIndex);
		
bail:
	return(myErr);
}


//////////
//
// WiredUtils_AddOperatorAtom
// Add a kOperatorAtomType atom to the specified atom.
//
//////////

OSErr WiredUtils_AddOperatorAtom (QTAtomContainer theContainer, QTAtom theExpressionAtom, QTAtomID theOperatorType, QTAtom *theOperatorAtom)
{
	return(QTInsertChild(theContainer, theExpressionAtom, kOperatorAtomType, theOperatorType, 1, 0, NULL, theOperatorAtom));
}


//////////
//
// WiredUtils_AddExpressionContainerAtomType
// Add a kExpressionContainerAtomType atom to the specified atom.
//
//////////

OSErr WiredUtils_AddExpressionContainerAtomType (QTAtomContainer theContainer, QTAtom theAtom, QTAtom *theExpressionAtom)
{
	return(QTInsertChild(theContainer, theAtom, kExpressionContainerAtomType, 1, 1, 0, NULL, theExpressionAtom));
}


//////////
//
// WiredUtils_AddConditionalAtom
// Add a kConditionalAtomType atom to the specified atom.
//
//////////

OSErr WiredUtils_AddConditionalAtom (QTAtomContainer theContainer, QTAtom theAtom, short theConditionIndex, QTAtom *theConditionalAtom)
{
	return(QTInsertChild(theContainer, theAtom, kConditionalAtomType, 0, theConditionIndex, 0, NULL, theConditionalAtom));
}


//////////
//
// WiredUtils_AddActionListAtom
// Add a kActionListAtomType atom to the specified atom.
//
//////////

OSErr WiredUtils_AddActionListAtom (QTAtomContainer theContainer, QTAtom theAtom, QTAtom *theActionListAtom)
{
	return(QTInsertChild(theContainer, theAtom, kActionListAtomType, 1, 1, 0, NULL, theActionListAtom));
}
