//////////
//
//	File:		VRHash.c
//
//	Contains:	Functions for hash table management.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	12/02/98		rtm		renamed VRHash_CreateHashTable and _DestroyHashTable to _Init and _Stop
//	   <3>	 	12/02/98		rtm		simplified VRHash_DestroyHashTable
//	   <2>	 	11/29/98		rtm		minor clean-up to some routines
//	   <1>	 	11/28/98		rtm		first file (Happy Thanksgiving!)
//	
//	Our hash table is an array of "bucket pointers"; each bucket (defined by the VRScriptHash
//	data structure) contains the command word, its corresponding command code, and a pointer to
//	the next bucket in the linked list (or NULL if there is no next bucket).
//
//	There are many ways to implement a hash table management scheme; the one used here is eerily
//	similar (as I now discover) to the one described by Kernighan & Ritchie in The C Programming
//	Language.
//
//////////

//////////
//
// header files
//	   
//////////

#include "VRHash.h"


//////////
//
// global variables
//	   
//////////

static VRScriptHashPtr			gHashTable[kNumEntriesInTable];			// the hash table


//////////
//
// VRHash_Init
// Create the hash table.
//
//////////

void VRHash_Init (void)
{
	short			myIndex;
		
	// initialize the bucket pointers in the hash table
	for (myIndex = 0; myIndex < kNumEntriesInTable; myIndex++)
		gHashTable[myIndex] = NULL;
	
	// insert entries into the hash table
	VRHash_PutCommandIntoTable("SetVerboseState", kSetVerboseState);
	VRHash_PutCommandIntoTable("OpenQTVRMovieFile", kOpenQTVRMovieFile);
	VRHash_PutCommandIntoTable("ReplaceMainMovie", kReplaceMainMovie);
	VRHash_PutCommandIntoTable("SetCurrentDirectory", kSetCurrentDirectory);
	VRHash_PutCommandIntoTable("SetBarState", kSetBarState);
	VRHash_PutCommandIntoTable("SetButtonState", kSetButtonState);
	VRHash_PutCommandIntoTable("SetResizeState", kSetResizeState);
	VRHash_PutCommandIntoTable("SetWindowSize", kSetWindowSize);
	VRHash_PutCommandIntoTable("SetMaxWindowSize", kSetMaxWindowSize);
	VRHash_PutCommandIntoTable("ReplaceCursor", kReplaceCursor);
	VRHash_PutCommandIntoTable("SetHotSpotIDCursors", kSetHotSpotIDCursors);
	VRHash_PutCommandIntoTable("SetHotSpotTypeCursors", kSetHotSpotTypeCursors);
	VRHash_PutCommandIntoTable("GoToNodeID", kGoToNodeID);
	VRHash_PutCommandIntoTable("ShowDefaultView", kShowDefaultView);
	VRHash_PutCommandIntoTable("OpenResourceFile", kOpenResourceFile);
	VRHash_PutCommandIntoTable("SetCorrection", kSetCorrection);
	VRHash_PutCommandIntoTable("SetQuality", kSetQuality);
	VRHash_PutCommandIntoTable("SetSwingSpeed", kSetSwingSpeed);
	VRHash_PutCommandIntoTable("SetSwingDirection", kSetSwingDirection);
	VRHash_PutCommandIntoTable("SetSwingState", kSetSwingState);
	VRHash_PutCommandIntoTable("SetPanAngle", kSetPanAngle);
	VRHash_PutCommandIntoTable("SetTiltAngle", kSetTiltAngle);
	VRHash_PutCommandIntoTable("SetPanTiltZoom", kSetPanTiltZoom);
	VRHash_PutCommandIntoTable("SetFieldOfView", kSetFieldOfView);
	VRHash_PutCommandIntoTable("SetViewCenter", kSetViewCenter);
	VRHash_PutCommandIntoTable("SetPanLimits", kSetPanLimits);
	VRHash_PutCommandIntoTable("SetTiltLimits", kSetTiltLimits);
	VRHash_PutCommandIntoTable("SetZoomLimits", kSetZoomLimits);
	VRHash_PutCommandIntoTable("SetHotSpotState", kSetHotSpotState);
	VRHash_PutCommandIntoTable("SetTranslateState", kSetTranslateState);
	VRHash_PutCommandIntoTable("SetClickRadius", kSetClickRadius);
	VRHash_PutCommandIntoTable("SetClickTimeout", kSetClickTimeout);
	VRHash_PutCommandIntoTable("SetPanTiltSpeed", kSetPanTiltSpeed);
	VRHash_PutCommandIntoTable("SetZoomSpeed", kSetZoomSpeed);
	VRHash_PutCommandIntoTable("SetMouseScale", kSetMouseScale);
	VRHash_PutCommandIntoTable("SetFrameRate", kSetFrameRate);
	VRHash_PutCommandIntoTable("SetViewRate", kSetViewRate);
	VRHash_PutCommandIntoTable("SetViewTime", kSetViewTime);
	VRHash_PutCommandIntoTable("SetViewState", kSetViewState);
	VRHash_PutCommandIntoTable("SetAnimationState", kSetAnimationState);
	VRHash_PutCommandIntoTable("SetControlState", kSetControlState);
	VRHash_PutCommandIntoTable("SetFrameAnimState", kSetFrameAnimState);
	VRHash_PutCommandIntoTable("SetViewAnimState", kSetViewAnimState);
	VRHash_PutCommandIntoTable("SetQTVRVisState", kSetQTVRVisState);
	VRHash_PutCommandIntoTable("SetCachePrefs", kSetCachePrefs);
	VRHash_PutCommandIntoTable("SetMovieVolume", kSetMovieVolume);
	VRHash_PutCommandIntoTable("SetTrackVolume", kSetTrackVolume);
	VRHash_PutCommandIntoTable("SetSoundVolume", kSetSoundVolume);
	VRHash_PutCommandIntoTable("SetSoundBalance", kSetSoundBalance);
	VRHash_PutCommandIntoTable("PlaySceneSound", kPlaySceneSound);
	VRHash_PutCommandIntoTable("PlaySceneQTMidi", kPlaySceneQTMidi);
	VRHash_PutCommandIntoTable("PlayNodeQTMidi", kPlayNodeQTMidi);
	VRHash_PutCommandIntoTable("PlayNodeSound", kPlayNodeSound);
	VRHash_PutCommandIntoTable("PlayNode3DSound", kPlayNode3DSound);
	VRHash_PutCommandIntoTable("HotSpotQTMidi", kHotSpotQTMidi);
	VRHash_PutCommandIntoTable("HotSpotSound", kHotSpotSound);
	VRHash_PutCommandIntoTable("HotSpot3DSound", kHotSpot3DSound);
	VRHash_PutCommandIntoTable("HotSpotMovie", kHotSpotMovie);
	VRHash_PutCommandIntoTable("TriggerHotSpot", kTriggerHotSpot);
	VRHash_PutCommandIntoTable("PlayQTMidi", kPlayQTMidi);
	VRHash_PutCommandIntoTable("PlaySndResource", kPlaySndResource);
	VRHash_PutCommandIntoTable("PlaySound", kPlaySndResource);				// synonym
	VRHash_PutCommandIntoTable("PlaySoundFile", kPlaySoundFile);
	VRHash_PutCommandIntoTable("Play3DSndResource", kPlay3DSndResource);
	VRHash_PutCommandIntoTable("Play3DSndResourceAngle", kPlay3DSndResourceAngle);
	VRHash_PutCommandIntoTable("ShowPicture", kShowPicture);
	VRHash_PutCommandIntoTable("ShowNodePicture", kShowNodePicture);
	VRHash_PutCommandIntoTable("AtTime", kAtTime);
	VRHash_PutCommandIntoTable("AtAppLaunch", kAtAppLaunch);
	VRHash_PutCommandIntoTable("AtAppQuit", kAtAppQuit);
	VRHash_PutCommandIntoTable("AtMouseOverHSID", kAtMouseOverHSID);
	VRHash_PutCommandIntoTable("AtMouseOverHSType", kAtMouseOverHSType);
	VRHash_PutCommandIntoTable("AtClickHSID", kAtClickHSID);
	VRHash_PutCommandIntoTable("AtClickHS", kAtClickHSID);					// synonym
	VRHash_PutCommandIntoTable("AtClickHSType", kAtClickHSType);
	VRHash_PutCommandIntoTable("AtClickCustomButton", kAtClickCustomButton);
	VRHash_PutCommandIntoTable("AtClickSprite", kAtClickSprite);
	VRHash_PutCommandIntoTable("AtTriggerWiredAction", kAtTriggerWiredAction);
	VRHash_PutCommandIntoTable("AtNodeEntry", kAtNodeEntry);
	VRHash_PutCommandIntoTable("AtNodeExit", kAtNodeExit);
	VRHash_PutCommandIntoTable("AtPanAngle", kAtPanAngle);
	VRHash_PutCommandIntoTable("AtTiltAngle", kAtTiltAngle);
	VRHash_PutCommandIntoTable("AtZoomAngle", kAtZoomAngle);
	VRHash_PutCommandIntoTable("DoBoth", kDoBoth);
	VRHash_PutCommandIntoTable("DoNothing", kDoNothing);
	VRHash_PutCommandIntoTable("PlayMovie", kPlayMovie);
	VRHash_PutCommandIntoTable("PlayTransMovie", kPlayTransMovie);
	VRHash_PutCommandIntoTable("PlayTransEffect", kPlayTransEffect);
	VRHash_PutCommandIntoTable("MoveScreen", kMoveScreen);
	VRHash_PutCommandIntoTable("Beep", kBeep);
	VRHash_PutCommandIntoTable("ProcessScript", kProcessScript);
	VRHash_PutCommandIntoTable("CreateBox", kCreateBox);
	VRHash_PutCommandIntoTable("CreateCone", kCreateCone);
	VRHash_PutCommandIntoTable("CreateCylinder", kCreateCylinder);
	VRHash_PutCommandIntoTable("CreateEllipsoid", kCreateEllipsoid);
	VRHash_PutCommandIntoTable("CreateTorus", kCreateTorus);
	VRHash_PutCommandIntoTable("CreateRectangle", kCreateRectangle);
	VRHash_PutCommandIntoTable("Open3DMFFile", kOpen3DMFFile);
	VRHash_PutCommandIntoTable("Set3DObjLocation", kSet3DObjLocation);
	VRHash_PutCommandIntoTable("Set3DObjColor", kSet3DObjColor);
	VRHash_PutCommandIntoTable("Set3DObjTransp", kSet3DObjTransp);
	VRHash_PutCommandIntoTable("Set3DObjInterp", kSet3DObjInterp);
	VRHash_PutCommandIntoTable("Set3DObjBackface", kSet3DObjBackface);
	VRHash_PutCommandIntoTable("Set3DObjFill", kSet3DObjFill);
	VRHash_PutCommandIntoTable("Set3DObjRotation", kSet3DObjRotation);
	VRHash_PutCommandIntoTable("Set3DObjRotState", kSet3DObjRotState);
	VRHash_PutCommandIntoTable("Set3DObjVisState", kSet3DObjVisState);
	VRHash_PutCommandIntoTable("Set3DObjTexture", kSet3DObjTexture);
	VRHash_PutCommandIntoTable("Destroy3DObject", kDestroy3DObject);
	VRHash_PutCommandIntoTable("Set3DSndLocation", kSet3DSndLocation);
	VRHash_PutCommandIntoTable("SetVariable", kSetVariable);
	VRHash_PutCommandIntoTable("If", kIf);
	VRHash_PutCommandIntoTable("SetSpriteVisState", kSetSpriteVisState);
	VRHash_PutCommandIntoTable("SetSpriteLayer", kSetSpriteLayer);
	VRHash_PutCommandIntoTable("SetSpriteGraphicsMode", kSetSpriteGraphicsMode);
	VRHash_PutCommandIntoTable("SetSpriteImageIndex", kSetSpriteImageIndex);
	VRHash_PutCommandIntoTable("SetSpriteMatrix", kSetSpriteMatrix);
	VRHash_PutCommandIntoTable("SetSpriteLocation", kSetSpriteLocation);
	VRHash_PutCommandIntoTable("SetTrackState", kSetTrackState);
	VRHash_PutCommandIntoTable("SetTrackLayer", kSetTrackLayer);
	VRHash_PutCommandIntoTable("SetMovieTime", kSetMovieTime);
	VRHash_PutCommandIntoTable("SetMovieRate", kSetMovieRate);
	VRHash_PutCommandIntoTable("SetMovieTimeScale", kSetMovieTimeScale);
}


//////////
//
// VRHash_Stop
// Destroy the hash table.
//
//////////

void VRHash_Stop (void)
{
	UInt32				myIndex;
	VRScriptHashPtr		myBucketPtr;
	VRScriptHashPtr		myNextBucketPtr;

	for (myIndex = 0; myIndex < kNumEntriesInTable; myIndex++) {
	
		// get the bucket pointer at this offset in the hash table
		myBucketPtr = gHashTable[myIndex];

		while (myBucketPtr != NULL) {
			myNextBucketPtr = myBucketPtr->fNextEntry;
			free(myBucketPtr->fCommandWord);
			DisposePtr((Ptr)myBucketPtr);
			myBucketPtr = myNextBucketPtr;
		}
	}
}


//////////
//
// VRHash_HashCommandWord
// Get the hash value for the specified command word.
//
// This hash function returns a value in the range 0 to kNumEntriesInTable - 1. It's
// a simple additive hash function. (As K&R put it, "[t]his is not the best possible
// algorithm, but it has the merit of extreme simplicity".)
//
//////////

UInt32 VRHash_HashCommandWord (char *theCommandWord)
{
	UInt32		myHashVal;

	for (myHashVal = 0; *theCommandWord != '\0'; myHashVal += *theCommandWord++)
		;

	return(myHashVal % kNumEntriesInTable);
}


//////////
//
// VRHash_PutCommandIntoTable
// Insert an entry for the specified command word and command code into the hash table.
//
//////////

void VRHash_PutCommandIntoTable (char *theCommandWord, UInt32 theCommandCode)
{
	UInt32				myHashVal;
	VRScriptHashPtr		myBucketPtr;
	
	if (theCommandWord == NULL)
		return;

	// get the hash value for the specified command word
	myHashVal = VRHash_HashCommandWord(theCommandWord);
	
	// create a new bucket and insert it at the head of the linked list
	myBucketPtr = (VRScriptHashPtr)NewPtrClear(sizeof(VRScriptHash));
	if (myBucketPtr != NULL) {
		myBucketPtr->fCommandCode = theCommandCode;
		myBucketPtr->fCommandWord = malloc(strlen(theCommandWord) + 1);
		strncpy(myBucketPtr->fCommandWord, theCommandWord, strlen(theCommandWord) + 1);
		myBucketPtr->fNextEntry = gHashTable[myHashVal];
		gHashTable[myHashVal] = myBucketPtr;
	}
}


//////////
//
// VRHash_GetCommandCode
// Get the command code associated with the specified command word.
//
//////////

UInt32 VRHash_GetCommandCode (char *theCommandWord)
{
	UInt32				myCode = kInvalidCommand;
	UInt32				myHashVal;
	VRScriptHashPtr		myBucketPtr;
	
	if (theCommandWord == NULL)
		return(myCode);

	// get the hash value for the specified command word
	myHashVal = VRHash_HashCommandWord(theCommandWord);
	
	for (myBucketPtr = gHashTable[myHashVal]; myBucketPtr != NULL; myBucketPtr = myBucketPtr->fNextEntry)
		if (strcmp(theCommandWord, myBucketPtr->fCommandWord) == 0)
			return(myBucketPtr->fCommandCode);

	return(myCode);
}


