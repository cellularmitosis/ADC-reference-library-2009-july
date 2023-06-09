//////////
//
//	File:		VRSound.c
//
//	Contains:	Sound support for QuickTime VR movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <11>	 	03/13/98	rtm		removed support for chunk-based files (AIFF and WAVE)
//	   <10>	 	09/18/97	rtm		added parameter to VRSound_CheckForCompletedSounds
//	   <9>	 	08/30/97	rtm		begun adding support for chunk-based files (AIFF and WAVE)
//	   <8>	 	08/11/97	rtm		added support for sound files (of type 'sfil')
//	   <7>	 	06/19/97	rtm		added support for scene-wide sounds
//	   <6>	 	04/30/97	rtm		added distance attenuation to VRSound_SetBalanceAndVolume
//	   <5>	 	04/25/97	rtm		added VRSound_SetBalanceAndVolume to substitute for SoundSprocket
//									(and to support 68K Macs)
//	   <4>	 	04/24/97	rtm		merged VRSound.c with previous VR3DSound.c
//	   <3>	 	04/21/97	rtm		implemented fade-out option when stopping sounds
//	   <2>	 	04/18/97	rtm		implemented callback procedure
//	   <1>	 	04/17/97	rtm		first file
//	 
//
// This file provides functions to play both ambient and localized sounds in QTVR
// panoramas and objects.
//
// Some of this code is straight out of Inside Macintosh: Sound and the SoundSprocket
// documentation (by yours truly!) or the SoundSprocket sample source (by Dan Venolia).
//
//////////

// TODO:

//////////
//
// header files
//
//////////

#include "VRSound.h"
#include "QTVRUtilities.h"


//////////
//
// global variables
//
//////////

SndCallBackUPP			gSndCallBackProc = NULL; 	// a routine descriptor for our sound callback
Boolean					gHasSoundManager30;			// is Sound Manager version 3.0 (or greater) available?

extern Boolean			gHasSoundSprockets;			// is SoundSprockets available?


//////////
//
// constants
//
//////////

#define kBlockIsLocked	0x80						// the block-is-locked bit in the SignedByte returned by HGetState


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sound utility functions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_CheckVersionNumber
// Returns true if the given version number is compatible with
// (that is, not older than) version theMajor.theMinor.theBug.
//
//////////

Boolean VRSound_CheckVersionNumber (
	const NumVersion		*theVersion,
	UInt8					theMajor,
	UInt8					theMinor,
	UInt8					theBug)
{
	if (theVersion->majorRev != theMajor)
		return(theVersion->majorRev > theMajor);
	else
		return(theVersion->minorAndBugRev >= theMinor << 4 | theBug);
}


//////////
//
// VRSound_GetSoundHeader
// Returns a pointer to the sound header in a sampled sound resource.
//
//////////

SoundHeaderPtr VRSound_GetSoundHeader (Handle theSndHandle)
{
	SoundHeaderPtr		mySndHeader = NULL;
	long				myOffset = 0;
	OSErr				myErr = noErr;
	
	myErr = GetSoundHeaderOffset((SndListHandle)theSndHandle, &myOffset);
	if (myErr == noErr)
		mySndHeader = (SoundHeaderPtr)(*theSndHandle + myOffset);

	return(mySndHeader);
}


//////////
//
// VRSound_GetSndBaseFrequency
// Returns the base frequency of a sampled sound.
// We need this when installing the sound as a voice, in VRSound_PlayResource.
//
//////////

long VRSound_GetSndBaseFrequency (Handle theSndHandle)
{
	SoundHeaderPtr		mySndHeader;
	long				myBaseFreq = kMiddleC;		// a reasonable default
	
	mySndHeader = VRSound_GetSoundHeader(theSndHandle);
	if (mySndHeader != NULL)
		myBaseFreq = mySndHeader->baseFrequency;

	return(myBaseFreq);
}


//////////
//
// VRSound_GetVolume
// Get the current left and right volumes of a sound channel.
//
//////////

OSErr VRSound_GetVolume (SndChannelPtr theChannel, unsigned short *theLeftVol, unsigned short *theRightVol)
{
	SndCommand			mySndCommand;
	unsigned long		myVolume;
	OSErr				myErr = noErr;

	if (!gHasSoundManager30)
		return(paramErr);
		
	mySndCommand.cmd = getVolumeCmd;
	mySndCommand.param1 = 0;
	mySndCommand.param2 = (long)&myVolume;
	myErr = SndDoImmediate(theChannel, &mySndCommand);
	
	if (myErr == noErr) {
		*theLeftVol = myVolume & 0x0000ffff;
		*theRightVol = myVolume >> 16;
	}
	
	return(myErr);
}


//////////
//
// VRSound_SetVolume
// Set the left and right volumes of a sound channel.
//
//////////

OSErr VRSound_SetVolume (SndChannelPtr theChannel, unsigned short theLeftVol, unsigned short theRightVol)
{
	SndCommand			mySndCommand;
	OSErr				myErr = noErr;

	if (!gHasSoundManager30)
		return(paramErr);
		
	mySndCommand.cmd = volumeCmd;
	mySndCommand.param1 = 0;
	mySndCommand.param2 = (theRightVol << 16) | theLeftVol;
	
	myErr = SndDoImmediate(theChannel, &mySndCommand);
	return(myErr);
}


//////////
//
// VRSound_CreateSoundChannel
// Create a sound channel.
//
//////////

SndChannelPtr VRSound_CreateSoundChannel (Boolean theSoundIsLocalized)
{
	SndChannelPtr		mySndChannel = NULL;
	SoundComponentLink	myLink;
	OSStatus			myErr;
	
	// create storage for a new sound channel
	mySndChannel = (SndChannelPtr)NewPtrClear(sizeof(SndChannel));
	if (mySndChannel == NULL)
		return(mySndChannel);
	
	// set the number of commands in the sound channel queue
	mySndChannel->qLength = kVRSound_NumCmdsInQueue;
	
	// create the sound channel
	myErr = SndNewChannel(&mySndChannel, sampledSynth, initMono, gSndCallBackProc);
	if (myErr != noErr) {
		DisposePtr((Ptr)mySndChannel);
		return(NULL);
	}
	
	if (theSoundIsLocalized) {
		if (gHasSoundSprockets) {
			// install the 3D sound filters
			myLink.description.componentType			= kSoundEffectsType;
			myLink.description.componentSubType			= kSSpLocalizationSubType;
			myLink.description.componentManufacturer	= 0;
			myLink.description.componentFlags			= 0;
			myLink.description.componentFlagsMask		= 0;
			myLink.mixerID								= NULL;
			myLink.linkID								= NULL;
			
			myErr = SndSetInfo(mySndChannel, siPreMixerSoundComponent, &myLink);
		}
	}
	
	if (myErr != noErr) {
		DisposePtr((Ptr)mySndChannel);
		mySndChannel = NULL;
	}
	
	return(mySndChannel);
}


//////////
//
// VRSound_CreateLocalizedSource
// Create a localized sound source.
//
//////////

SSpSourceReference VRSound_CreateLocalizedSource (void)
{
	SSpSourceReference		mySource = 0;
	
	// create a new sound source
#if SOUNDSPROCKET_AVAIL
	if (gHasSoundSprockets)
		SSpSource_New(&mySource);
#endif

	return(mySource);
}


//////////
//
// VRSound_SetLocation
// Set the location of a localized sound.
//
//////////

void VRSound_SetLocation (WindowObject theWindowObject, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions)
{
	VRScriptSoundPtr	myPointer;

	myPointer = (VRScriptSoundPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_Sound, theEntryID);
	if (myPointer != NULL) {
		if (theOptions == kVRValue_Relative) {
			myPointer->fLocation.x += theX;
			myPointer->fLocation.y += theY;
			myPointer->fLocation.z += theZ;
		} else {
			myPointer->fLocation.x = theX;
			myPointer->fLocation.y = theY;
			myPointer->fLocation.z = theZ;
		}
		
#if SOUNDSPROCKET_AVAIL
		if (gHasSoundSprockets)
			SSpSource_SetPosition(myPointer->fSource, &(myPointer->fLocation));
#endif
	}
}


//////////
//
// VRSound_GetSndResourceID
// Get the resource ID of the first 'snd ' resource in the specified resource file.
//
//////////

short VRSound_GetSndResourceID (short theRefNum)
{
	Handle		myResource;
	short		myResID = 0;
	short		myCurRefNum;
	ResType		myResType;
	Str255		myResName;
	
	// make sure the specified file is the current resource file
	myCurRefNum = CurResFile();
	if (theRefNum != myCurRefNum)
		UseResFile(theRefNum);
	
	if (theRefNum > 0) {
		// find the resource ID of the single 'snd ' resource in the resource file;
		// we do this by loading the resource and then getting its info.
		myResource = Get1IndResource(soundListRsrc, 1);
		if (myResource != NULL)
			GetResInfo(myResource, &myResID, &myResType, myResName);
	}
	
	// restore the current resource file
	if (theRefNum != myCurRefNum)
		UseResFile(myCurRefNum);
	
	return(myResID);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sound list utility functions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_Update3DSoundEnv
// Update the virtual audio environment.
//
//////////

void VRSound_Update3DSoundEnv (WindowObject theWindowObject)
{
#if SOUNDSPROCKET_AVAIL
	SSpLocalizationData		myData;
#endif
	ApplicationDataHdl		myAppData;
	VRScriptSoundPtr		myPointer;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
	
	// walk our linked list and update this node's virtual audio environment
	myPointer = (VRScriptSoundPtr)(**myAppData).fListArray[kVREntry_Sound];
	if (gHasSoundSprockets) {
#if SOUNDSPROCKET_AVAIL
		while (myPointer != NULL) {
			if (myPointer->fSoundIsLocalized) {
				SSpSource_CalcLocalization(myPointer->fSource, (**myAppData).fListener, &myData);
				SndSetInfo(myPointer->fChannel, siSSpLocalization, &myData);
			}
			myPointer = myPointer->fNextEntry;
		}
#endif
	} else {
		// VRSound_SetBalanceAndVolume is triggered by the prescreen routine, so do an update
		(**myAppData).fSoundHasChanged = true;
		QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
	}
}


//////////
//
// VRSound_SetBalanceAndVolume
// Set the balance and volume attenuation of any localized sounds.
//
// This is a low-budget SoundSprocket replacement: we use the Sound Manager's volumeCmd 
// to adjust the volume and balance of a sound channel according to the given pan angle.
//
//////////

void VRSound_SetBalanceAndVolume (WindowObject theWindowObject, float thePan, float theTilt)
{
#pragma unused(theTilt)

	ApplicationDataHdl		myAppData;
	VRScriptSoundPtr		myPointer;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	// walk our linked list and set the balance and volume of any localized sounds
	myPointer = (VRScriptSoundPtr)(**myAppData).fListArray[kVREntry_Sound];
	while (myPointer != NULL) {
		if (myPointer->fSoundIsLocalized) {
			float			myPan;
			float			myPanDelta;
			float			myCosLimit;
			float			myCosDeltaPlus, myCosDeltaMinus;
			unsigned short	myLeftVol, myRightVol;
			float			myDistance;
			
			// get the pan angle of the localized sound;
			myPan = QTVRUtils_Point3DToPanAngle(myPointer->fLocation.x, myPointer->fLocation.y, myPointer->fLocation.z);
				
			myPanDelta = thePan - myPan;
			myCosLimit = cos(myPointer->fProjAngle);
			myCosDeltaPlus = cos(myPanDelta + QTVRUtils_DegreesToRadians(kVRSound_SpeakerAngle));
			myCosDeltaMinus = cos(myPanDelta - QTVRUtils_DegreesToRadians(kVRSound_SpeakerAngle));
			
			// inside cone of attenuation, volume scales from 1.0 (at center) to 0.0 (at cone edge)
			// outside cone of attenuation, volume is 0.0;
			myLeftVol = (myCosDeltaPlus >= myCosLimit) ? (kQTMaxSoundVolume * ((myCosDeltaPlus - myCosLimit) / (1 - myCosLimit))) : kNoVolume;
			myRightVol = (myCosDeltaMinus >= myCosLimit) ? (kQTMaxSoundVolume * ((myCosDeltaMinus - myCosLimit) / (1 - myCosLimit))) : kNoVolume;

			// now adjust the volume for the distance of the object from the listener;
			// we could use many other algorithms here....
			myDistance = QTVRUtils_GetDistance(myPointer->fLocation);
			if (myDistance > 1.0) {
				myLeftVol /= myDistance;
				myRightVol /= myDistance;
			}
			
			VRSound_SetVolume(myPointer->fChannel, myLeftVol, myRightVol);
		}
		
		myPointer = myPointer->fNextEntry;
	}
}
	

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Application initialization and shutdown.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_Init
// Initialize for sound.
//
//////////

void VRSound_Init (void)
{
	NumVersion			myVersion;

	// check the Sound Manager version
	myVersion = SndSoundManagerVersion();
	
	// for doing any sound, we require version 3.0 or later
	gHasSoundManager30 = VRSound_CheckVersionNumber(&myVersion, 3, 0, 0);

	// for doing localized sound using SoundSprockets, we require version 3.2.1 or later
	if (!VRSound_CheckVersionNumber(&myVersion, 3, 2, 1))
		gHasSoundSprockets = false;
	
	// now see whether SoundSprockets is available;
	// there is no Gestalt selector for this package, so we use an alternate strategy:
#if SOUNDSPROCKET_AVAIL
	gHasSoundSprockets = ((short)SSpListener_New != (short)kUnresolvedCFragSymbolAddress);
#else
	gHasSoundSprockets = false;
#endif

	// allocate a routine descriptor for our sound callback routine
	if (gSndCallBackProc == NULL)
		gSndCallBackProc = NewSndCallBackUPP(VRSound_CallbackProc);
}


//////////
//
// VRSound_Stop
// Close down for sound.
//
//////////

void VRSound_Stop (void)
{
	// deallocate routine descriptor
	if (gSndCallBackProc != NULL)
		DisposeSndCallBackUPP(gSndCallBackProc);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Window-specific initialization and clean-up.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_InitWindowData
// Initialize window-specific data for sounds.
//
//////////

void VRSound_InitWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

#if SOUNDSPROCKET_AVAIL
	if (gHasSoundSprockets) {
		OSStatus		myErr;
	
		// create a listener and set listener units to feet
		myErr = SSpListener_New(&(**myAppData).fListener);
		if (myErr == noErr)
			SSpListener_SetMetersPerUnit((**myAppData).fListener, 0.3048);
	}
#endif
}


//////////
//
// VRSound_DumpWindowData
// Get rid of window-specific data for sounds.
//
//////////

void VRSound_DumpWindowData (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	VRSound_DumpNodeSounds(theWindowObject);
	VRSound_DumpSceneSounds(theWindowObject);

#if SOUNDSPROCKET_AVAIL
	// shut down SoundSprockets, if enabled
	if (gHasSoundSprockets && ((**myAppData).fListener != NULL)) 
		SSpListener_Dispose((**myAppData).fListener);
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Manipulating multiple sound channels.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_DumpNodeSounds
// Stop playing all sounds and release all sound resources enlisted for the current node.
//
//////////

void VRSound_DumpNodeSounds (WindowObject theWindowObject)
{
	VRSound_DumpSelectedSounds(theWindowObject, kVRSelect_Node);
}


//////////
//
// VRSound_DumpSceneSounds
// Stop playing all sounds and release all sound resources enlisted for the current scene.
//
//////////

void VRSound_DumpSceneSounds (WindowObject theWindowObject)
{
	VRSound_DumpSelectedSounds(theWindowObject, kVRSelect_Scene);
}


//////////
//
// VRSound_DumpSelectedSounds
// Stop playing selected sounds and release selected sound resources.
//
//////////

void VRSound_DumpSelectedSounds (WindowObject theWindowObject, UInt32 theOptions)
{
	ApplicationDataHdl	myAppData;
	VRScriptSoundPtr	myPointer;
	VRScriptSoundPtr	myNext;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	// stop playing and delist any sounds associated with this node
	myPointer = (VRScriptSoundPtr)(**myAppData).fListArray[kVREntry_Sound];
	while (myPointer != NULL) {		
		myNext = myPointer->fNextEntry;
		if (((myPointer->fNodeID != kVRAnyNode) && (theOptions == kVRSelect_Node)) ||
			((myPointer->fNodeID == kVRAnyNode) && (theOptions == kVRSelect_Scene))) {
			if (myPointer->fFadeWhenStopping)
				VRSound_FadeSilence(theWindowObject, myPointer->fChannel);
			else
				VRSound_PlaySilence(theWindowObject, myPointer->fChannel);
				
			VRScript_DelistEntry(theWindowObject, (VRScriptGenericPtr)myPointer);
		}
		myPointer = myNext;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Manipulating a single sound channel.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_FadeSilence
// Fade out the sound playing on a particular sound channel.
//
//////////

void VRSound_FadeSilence (WindowObject theWindowObject, SndChannelPtr theChannel)
{
#pragma unused(theWindowObject)

	unsigned short		myLeftVol;
	unsigned short		myRightVol;
	short				myCount;
	unsigned long		myTicks;
	float				myFactor;
	OSErr				myErr = noErr;

	if (theChannel == NULL)
		return;
	
	// get the current right and left volumes
	myErr = VRSound_GetVolume(theChannel, &myLeftVol, &myRightVol);
	
	if (myErr == noErr) {
		// now gradually reduce the volume of each channel to silence
		for (myCount = 1; myCount < kVRSound_NumFadeSteps; myCount++) {
			Delay(kVRSound_FadeStepWait, &myTicks);			// wait a small bit, to make fade perceptible
			myFactor = (float)(kVRSound_NumFadeSteps - myCount) / (float)kVRSound_NumFadeSteps;
			myLeftVol *= myFactor;
			myRightVol *= myFactor;
			VRSound_SetVolume(theChannel, myLeftVol, myRightVol);
		}
	}	
}


//////////
//
// VRSound_PlaySilence
// Stop the sound playing on a particular sound channel.
//
//////////

void VRSound_PlaySilence (WindowObject theWindowObject, SndChannelPtr theChannel)
{
#pragma unused(theWindowObject)
	SndCommand			mySndCommand;

	if (theChannel == NULL)
		return;
		
	mySndCommand.cmd = quietCmd;
	mySndCommand.param1 = 0;
	mySndCommand.param2 = 0;
	SndDoImmediate(theChannel, &mySndCommand);
}


//////////
//
// VRSound_PlayResource
// Play the specified sound resource on the specified sound channel.
//
//////////

void VRSound_PlayResource (WindowObject theWindowObject, SndChannelPtr theChannel, SndListHandle theResHandle, UInt32 theOptions)
{
	SndCommand			mySndCommand;
	long				myOffset;
	ApplicationDataHdl	myAppData;
	SignedByte			myHState;
	OSErr				myErr = noErr;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	if (theResHandle == NULL)
		return;

	// silence the sound channel
	VRSound_PlaySilence(theWindowObject, theChannel);
		
	// lock the resource data down, if it isn't already locked
	myHState = HGetState((Handle)theResHandle);
	if (!(myHState & kBlockIsLocked))
		HLock((Handle)theResHandle);
	
	GetSoundHeaderOffset(theResHandle, &myOffset);
	
	switch (theOptions) {
		case kVRPlay_Loop:
			// loop the sound indefinitely
			// TO DO: need to rewirte this without freqCmd, which ain't in Carbon
#if !TARGET_API_MAC_CARBON
			mySndCommand.cmd = soundCmd;
			mySndCommand.param1 = 0;
			mySndCommand.param2 = (long)*(theResHandle) + myOffset;
			myErr = SndDoImmediate(theChannel, &mySndCommand);

			mySndCommand.cmd = freqCmd;
			mySndCommand.param1 = 0;
			mySndCommand.param2 = VRSound_GetSndBaseFrequency((Handle)theResHandle);
			myErr = SndDoImmediate(theChannel, &mySndCommand);
#endif
			break;
			
		case kVRPlay_Once:
			// play the sound once thru, asynchronously
			mySndCommand.cmd = bufferCmd;
			mySndCommand.param1 = 0;
			mySndCommand.param2 = (long)*(theResHandle) + myOffset;
			myErr = SndDoImmediate(theChannel, &mySndCommand);
			break;
		
		default:
			break;
	}
}


//////////
//
// VRSound_PlaySound
// Start a sound playing or stop one from playing.
//
//////////

void VRSound_PlaySound (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theResID, UInt32 theEntryID, float theX, float theY, float theZ, float theProjAngle, UInt32 theSourceMode, UInt32 theMode, UInt32 theFade, UInt32 theOptions)
{
	ApplicationDataHdl		myAppData;
	Handle					myHandle;
	Boolean					myNeedPlaySound = false;
	Boolean					myNeedNewChannel = false;
	SndChannelPtr			mySndChannel;
	VRScriptSoundPtr		myPointer = NULL;
	SSpSourceReference		mySource = 0;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
	
	// see if this sound is already in our list of playing sounds				
	myPointer = (VRScriptSoundPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_Sound, theEntryID);
	if (myPointer == NULL) {
		// this sound isn't in our list yet, so we'll need to add it to the list
		myNeedPlaySound = true;
		myNeedNewChannel = true;
	} else {
		// this sound is already in our list; theOptions determines how we handle this request:
		
		mySource = myPointer->fSource;

		switch (theOptions) {
		
			case kVRMedia_PlayNew:
				// start the sound playing (whether it's already playing or not)
				myNeedPlaySound = true;
				myNeedNewChannel = true;
				break;
				
			case kVRMedia_Restart:
				// stop the current sound and then restart it; use the existing sound channel
				// (PlayResource automatically stops the current sound on this channel)
				myNeedPlaySound = true;
				myNeedNewChannel = false;
				break;
				
			case kVRMedia_ToggleStop:
			case kVRMedia_TogglePause:
				// toggle the sound's current play/stop state
				myNeedNewChannel = false;
				if (myPointer->fSoundIsPlaying) {
					// stop the sound
					if (myPointer->fFadeWhenStopping)
						VRSound_FadeSilence(theWindowObject, myPointer->fChannel);
					else
						VRSound_PlaySilence(theWindowObject, myPointer->fChannel);
					myPointer->fSoundIsPlaying = false;
					myNeedPlaySound = false;
				} else {
					// start the sound; use the existing sound channel
					myNeedPlaySound = true;
				}
				break;
				
			case kVRMedia_Continue:
				// just let the sound already in progress continue
				myNeedPlaySound = false;
				myNeedNewChannel = false;
				break;
				
			case kVRMedia_Stop:
				// stop the sound
				if (myPointer->fFadeWhenStopping)
					VRSound_FadeSilence(theWindowObject, myPointer->fChannel);
				else
					VRSound_PlaySilence(theWindowObject, myPointer->fChannel);
				myPointer->fSoundIsPlaying = false;
				myNeedPlaySound = false;
				myNeedNewChannel = false;
				break;
				
			default:
				// unrecognized option
				// start the specified resource playing; use the existing sound channel
				myNeedPlaySound = true;
				myNeedNewChannel = false;
				break;
		}
		
	}
	
	if (myNeedNewChannel) {
		if (theSourceMode == kSSpSourceMode_Ambient) {
			mySndChannel = VRSound_CreateSoundChannel(false);
		} else {
			mySource = VRSound_CreateLocalizedSource();
			mySndChannel = VRSound_CreateSoundChannel(true);
		}
		
		if (mySndChannel == NULL) 
			return;		// couldn't allocate a new channel, so we cannot play sound....
		
		myHandle = GetResource(soundListRsrc, theResID);
		if (myHandle != NULL) {
			HNoPurge((Handle)myHandle);
			if (theSourceMode == kSSpSourceMode_Ambient) {
				myPointer = VRScript_EnlistSound(theWindowObject, theNodeID, theResID, theEntryID, theMode, theFade, theOptions, mySndChannel, (SndListHandle)myHandle);
			} else {
				myPointer = VRScript_EnlistLocalizedSound(theWindowObject, theNodeID, theResID, theEntryID, theX, theY, theZ, theProjAngle, theMode, theFade, theOptions, mySndChannel, (SndListHandle)myHandle, mySource);
			}
		} else {
			SndDisposeChannel(mySndChannel, true);
			DisposePtr((Ptr)mySndChannel);
			return;
		}
		
	}

	if (myNeedPlaySound) {
		if (myPointer != NULL) {
		
			if (theSourceMode != kSSpSourceMode_Ambient) {
				if (gHasSoundSprockets) {
					TQ3Point3D			myPoint;
					TQ3Vector3D			myOrient;
					
					// install this sound using SoundSprocket's high-level interfaces
					myPoint.x = theX;
					myPoint.y = theY;
					myPoint.z = theZ;
					
					myOrient.x = -myPoint.x;
					myOrient.y = -myPoint.y;
					myOrient.z = -myPoint.z;
					
#if SOUNDSPROCKET_AVAIL
					SSpSource_SetPosition(mySource, &myPoint);
					SSpSource_SetOrientation(mySource, &myOrient);
					SSpSource_SetAngularAttenuation(mySource, kVRPi/8, 20.0);
					SSpSource_SetMode(mySource, theSourceMode);
					SSpSource_SetReferenceDistance(mySource, 1.0);
					SSpSource_SetSize(mySource, 0.0, 0.0, 0.0);
#endif
				}
				
				VRSound_Update3DSoundEnv(theWindowObject);
			}
			
			VRSound_PlayResource(theWindowObject, myPointer->fChannel, myPointer->fResourceData, theMode);
			myPointer->fSoundIsPlaying = true;
			if (theMode == kVRPlay_Once)
				VRSound_InstallCallbackMessage(myPointer, kVRSound_Complete);
		}	
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callback procedure functions and utilities.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// VRSound_InstallCallbackMessage
// Clean up after a sound is finished playing.
//
//////////

void VRSound_InstallCallbackMessage (VRScriptSoundPtr theEntry, short theMessage)
{
	if (theEntry != NULL) {
		SndCommand			myCommand;
		
		myCommand.cmd = callBackCmd;
		myCommand.param1 = theMessage;
		myCommand.param2 = (long)theEntry;
		SndDoCommand(theEntry->fChannel, &myCommand, false);
	}
}


//////////
//
// VRSound_CallbackProc
// Handle callback messages.
//
//////////

PASCAL_RTN void VRSound_CallbackProc (SndChannelPtr theChannel, SndCommand *theCommand)
{
#pragma unused(theChannel)

	VRScriptSoundPtr	myPointer = NULL;
	
	switch(theCommand->param1) {
		case kVRSound_Complete:
			myPointer = (VRScriptSoundPtr)(theCommand->param2);
			if (myPointer != NULL)
				myPointer->fSoundIsPlaying = false;
			break;
			
		default:
			break;
	}

}


//////////
//
// VRSound_GetFinishedSound
// Get the first enlisted sound that is done playing.
//
//////////

VRScriptSoundPtr VRSound_GetFinishedSound (WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;
	VRScriptSoundPtr	myPointer = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return(NULL);
	
	// walk our linked list of sounds to find the target sound
	myPointer = (VRScriptSoundPtr)(**myAppData).fListArray[kVREntry_Sound];
	while (myPointer != NULL) {
		if (myPointer->fSoundIsPlaying == false)
			return(myPointer);

		myPointer = myPointer->fNextEntry;
	}
	
	return(NULL);
}


//////////
//
// VRSound_CheckForCompletedSounds
// Clean up any sounds that are finished playing.
//
//////////

void VRSound_CheckForCompletedSounds (WindowObject theWindowObject)
{
	VRScriptSoundPtr		myPointer = NULL;

	// delist any completed sounds for the specified movie window
	if (theWindowObject != NULL)
		while ((myPointer = VRSound_GetFinishedSound(theWindowObject)) != NULL)
			VRScript_DelistEntry(theWindowObject, (VRScriptGenericPtr)myPointer);
}


//////////
//
// VRSound_DumpEntryMem
// Release any memory associated with the specified list entry.
//
//////////

void VRSound_DumpEntryMem (VRScriptSoundPtr theEntry)
{
	if (theEntry == NULL)
		return;

	// how we release memory depends on the original container of the sound data
	if (theEntry->fSoundContainer == kVRSound_SoundResource) {
		HUnlock((Handle)(theEntry->fResourceData));
		ReleaseResource((Handle)(theEntry->fResourceData));
	}
	
#if SOUNDSPROCKET_AVAIL
	if (gHasSoundSprockets && (theEntry->fSource != NULL))
		SSpSource_Dispose(theEntry->fSource);
#endif

	SndDisposeChannel(theEntry->fChannel, true);
	DisposePtr((Ptr)theEntry->fChannel);			// because we allocated the storage ourselves....
}

