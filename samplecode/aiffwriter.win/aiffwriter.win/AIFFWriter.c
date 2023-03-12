//////////
//
//	File:		AIFFWriter.c
//
//	Contains:	Code for dispatching a sound hardware output ('sdev') component.
//
//	Written by:	Mark Cookson, Apple Developer Technical Support
//				Based on code by Kip Olson.
//	Revised by: Tim Monroe
//
//	Copyright:	� 1993-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	03/05/99	rtm		further work to bring into line with existing component sample code
//	   <1>	 	02/25/99	rtm		first inherited file; changes to support Windows compiles
//
//	You may incorporate this sample code into your applications without
//	restriction, though the sample code has been provided "AS IS" and the
//	responsibility for its operation is 100% yours. However, what you are
//	not permitted to do is to redistribute the source as "Apple Sample
//	Code" after having made changes. If you're going to re-distribute the
//	source, we require that you make it clear in the source that the code
//	was descended from Apple Sample Code, but that you've made changes.
//
//////////


//////////
//
// header files
//
//////////

#include "AIFFWriter.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Component dispatch helper defines
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#if TARGET_CPU_68K
	#define COMPONENT_C_DISPATCHER
	#define COMPONENT_DISPATCH_MAIN
#endif

#define SOUNDCOMPONENT_BASENAME() 		__SoundOutput
#define SOUNDCOMPONENT_GLOBALS() 		SoundOutputGlobalsPtr storage

#define CALLCOMPONENT_BASENAME() 		SOUNDCOMPONENT_BASENAME()
#define CALLCOMPONENT_GLOBALS() 		SOUNDCOMPONENT_GLOBALS()

#define COMPONENT_UPP_SELECT_ROOT()		SoundComponent
#define COMPONENT_DISPATCH_FILE			"SoundOutputDispatch.h"

#define	GET_DELEGATE_COMPONENT()		(storage->sourceComponent)

#include "Components.k.h"
#include "Sound.k.h"
#include "ComponentDispatchHelper.c"


// It is possible to debug this component using an application. To do this, you cannot call the Time
// Manager to generate interrupts, since source level debuggers are not re-entrant. This means you must call
// the FakeInterrupt routine described below during main event loop time and define the gGlobals variable.

#ifdef FakeInterrupts
SoundOutputGlobalsPtr					gGlobals;
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Required component calls
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/*	SoundOutputOpen

	This routine is called when the Component Manager creates an instance of this
	component. The routine should allocate global variables in the appropriate heap
	and call SetComponentInstanceStorage() so the Component Manager can remember
	the globals and pass them to all the method calls.

	Determining the heap to use can be tricky. The Component Manager will normally
	load the component code into the system heap, which is good, since many applications
	will be sharing this component to play sound. In this case, the component's global
	variable storage should also be created in the system heap.

	However, if system heap memory is tight, the Component Manager will load
	the component into the application heap of the first application that plays sound.
	When this happens, the component should create global storage in the application heap
	instead. The Sound Manager will make sure that other applications will not try
	to play sound while the component is in this application heap.

	To determine the proper heap to use, call GetComponentInstanceA5(). If the value
	returned is 0, then the component was loaded into the system heap, and all storage
	should be allocated there. If the value returned is non-zero, the component is in
	the application heap specified by returned A5 value, and all storage should be
	allocated in this application heap.

	NOTE: If the component is loaded into the application heap, the value returned by
	GetComponentRefCon() will be 0.
	NOTE: Do not attempt to initialize or access hardware in this call, since the
	Component Manager will call SoundOutputOpen() BEFORE calling RegisterComponent().
	Instead, initialize the hardware during InitOutputDevice(), described below.
	NOTE: This routine is never called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputOpen (SoundOutputGlobalsPtr globals, ComponentInstance self)
{
	OSErr		result;

	// get space for globals in appropriate heap
	globals = (SoundOutputGlobalsPtr)NewPtrSysClear(sizeof(SoundOutputGlobals));
	FailWithAction(globals == nil, result = MemError(), Failure);

	// If a component is loaded into an application's heap, then it gets an A5 value.
	// This also means that the component's refCon is 0. All of this means we need
	// to go and get the preferences just like we're being registered for the first time.
	
	if (GetComponentInstanceA5(self) != 0)
	{
		globals->inAppHeap = true;
		result = GetPreferences(globals);
		FailIf(result != noErr, NoPrefs);
	}

	globals->self = self;
	globals->prefs = (HardwarePrefsHandle)GetComponentRefcon((Component)self);


#ifdef FakeInterrupts
	gGlobals = globals;
#endif

	SetComponentInstanceStorage(self, (Handle) globals); 	// save pointer to our globals
	return (noErr);

NoPrefs:
	DisposePtr((Ptr)globals);
Failure:
	return (result);
}


/*	SoundOutputClose

	This routine is called when the Component Manager is closing the instance of
	this component. The routine should make sure all remaining data is written
	to the hardware and that the hardware is completely turned off. It should
	delete all global storage and close any other components that were opened.

	NOTE: Be sure to check that the globals pointer passed in to this routine is
	not set to NIL. If the SoundOutputOpen() routine fails for any reason, the Component
	Manager will call this routine passing in a NIL for the globals.
	NOTE: This routine is never called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputClose (SoundOutputGlobalsPtr globals, ComponentInstance self)
{
	if (globals != nil)											// we have some globals
	{
		ReleaseHardware(globals);								// make sure the hardware is off and release it

		if (globals->sourceComponent)
			CloseMixerSoundComponent(globals->sourceComponent);	// close mixer

		if (globals->prefsChanged)
			ErrorMessage(SetSoundPreference(kAIFFWriterSubType, kAIFFWriterName, (Handle)globals->prefs));

		if (globals->inAppHeap)
			DisposeHandle((Handle)globals->prefs);

		if (globals->ioBuffers[0].buffer != nil)
			DisposePtr(globals->ioBuffers[0].buffer);			// dispose of buffers
		if (globals->ioBuffers[1].buffer != nil)
			DisposePtr(globals->ioBuffers[1].buffer);			// dispose of buffers

		DisposePtr((Ptr)globals);								// torch our storage
	}
	return (noErr);
}


/*	SoundOutputRegister

	This routine is called once, usually at boot time, when the Component Manager
	is first registering this component. This routine should check to see if the proper
	hardware is installed and return 0 if it is. If the hardware is not installed,
	the routine should return 1 and this component will not be registered. This is
	also an opportunity to do one-time initializations and perhaps register this
	component again if more than one hardware device is available. Global state information
	can also be saved in the component refcon by calling SetComponentRefCon();

	NOTE: The cmpWantsRegisterMessage bit must be set in the component flags of the
	component in order for this routine to be called.
	NOTE: This routine is never called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputRegister (SoundOutputGlobalsPtr globals)
{
	NumVersion			installedVersion;
	ComponentResult		result;
	OSErr				err;

	// we can only run if version 3.0 or greater of the Sound Manager is running;
	// we can check the entire long because the format of NumVersion is BCD data
	installedVersion = SndSoundManagerVersion();
	if (installedVersion.majorRev < 3)
		result = 1;									// component doesn't want to be registered
	else
	{
		result = 0;									// component wants to be registered
		err = GetPreferences(globals);
		FailWithAction(err != noErr, result = 1, Exit);
	}

Exit:
	return result;
}


PASCAL_RTN ComponentResult __SoundOutputVersion (SoundOutputGlobalsPtr globals)
{
	globals; // suppress "unused variable" warning for all compilers

	return (kSoundComponentVersion);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sound component functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/*	InitOutputDevice

	This routine is called once when the Sound Manager first opens this component.
	The routine should initialize the hardware to default values, allocate the
	appropriate mixer component and create any other memory that is required.

	NOTE: This routine is never called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputInitOutputDevice (SoundOutputGlobalsPtr globals, long actions)
{
	ComponentResult		result;

	actions;			// suppress "unused variable" warning for all compilers
	
	result = SetupHardware(globals);
	FailIf(result != noErr, Failure);

	// if any of the preferences are needed during the interrupts,
	// then locked the preferences handle now

	// first create a mixer and tell it the type of data it should output. The
	// description includes sample format, sample rate, sample size, number of channels
	// and the size of your optimal interrupt buffer. If a mixer cannot be found that
	// will output this type of data, an error will be returned.

	result = OpenMixerSoundComponent(&globals->outputData, 0, &globals->sourceComponent);
	FailIf(result != noErr, Failure);
	return (noErr);

Failure:
	return (result);
}


/*	SoundOutputGetInfo

	This routine returns information about this component to the Sound Manager. A
	4-byte OSType selector is used to determine the type and size of the information
	to return. If the component does not support a selector, it should delegate this
	call on up the component chain.

	NOTE: This can be called at interrupt time. However, selectors that return
	a handle will not be called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputGetInfo (SoundOutputGlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	SoundInfoListPtr	listPtr;
	UnsignedFixed		*lp;
	Handle				infoHandle;
	ComponentResult		result;
	short				*sp;

	result = noErr;
	switch (selector)
	{
		case siSampleSize:								// return current sample size
			*((short *) infoPtr) = (**globals->prefs).sampleSize;
			break;

		case siSampleSizeAvailable:						// return samples sizes available
			infoHandle = NewHandle(sizeof(short) * 2);	// space for sample sizes
			FailWithAction(infoHandle == nil, result = MemError(), Exit);

			listPtr = (SoundInfoListPtr)infoPtr;
			listPtr->count = 2;							// no. sample sizes in handle
			listPtr->infoHandle = infoHandle;			// handle to be returned

			sp = (short *) *infoHandle;					// store sample sizes in handle
			*sp++ = 8;
			*sp++ = 16;
			break;

		case siSampleRate:								// return current sample rate
			*((Fixed *) infoPtr) = (**globals->prefs).sampleRate;
			break;

		case siSampleRateAvailable:						// return sample rates available
			infoHandle = NewHandle(sizeof(UnsignedFixed) * 5);	// space for sample rates
			FailWithAction(infoHandle == nil, result = MemError(), Exit);

			listPtr = (SoundInfoListPtr)infoPtr;
			listPtr->count = 5;							// no. sample rates in handle
			listPtr->infoHandle = infoHandle;			// handle to be returned

			// If the hardware supports a limited set of sample rates, then the list count
			// should be set to the number of sample rates and this list of rates should be
			// stored in the handle.
			lp = (UnsignedFixed *) *infoHandle;
			*lp++ = rate48khz;
			*lp++ = rate44khz;
			*lp++ = rate22050hz;
			*lp++ = rate11025hz;
			*lp++ = 0x1F400000;							// 8kHz sample rate not defined
			break;

		case siNumberChannels:							// return current no. channels
			*((short *) infoPtr) = (**globals->prefs).numChannels;
			break;

		case siChannelAvailable:						// return channels available
			infoHandle = NewHandle(sizeof(short) * 2);	// space for channels
			FailWithAction(infoHandle == nil, result = MemError(), Exit);

			listPtr = (SoundInfoListPtr)infoPtr;
			listPtr->count = 2;							// no. channels in handle
			listPtr->infoHandle = infoHandle;			// handle to be returned

			sp = (short *) *infoHandle;					// store channels in handle
			*sp++ = 1;									// mono
			*sp++ = 2;									// stereo
			break;

		case siHardwareVolume:
			*((long *)infoPtr) = (**globals->prefs).volume;
			break;

		case siHardwareVolumeSteps:
			*((short *) infoPtr) = kHardwareVolumeSteps;
			break;

		case siHardwareMute:
			*((short *) infoPtr) = (**globals->prefs).muteState;
			break;
			
		case siHardwareBusy:
			*((short *)infoPtr) = globals->hardwareOn;
			break;

		// if you do not handle this selector, then delegate it up the chain
		default:
			result = SoundComponentGetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
			break;
	}

Exit:
	return (result);
}


/*	SoundOutputSetInfo

	This routine sets information about this component. A 4-byte OSType selector is
	used to determine the type and size of the information to apply. If the component
	does not support a selector, it should delegate this call on up the component chain.

	NOTE: This can be called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputSetInfo (SoundOutputGlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult			result;

	result = noErr;
	switch (selector)
	{
		case siSampleSize:								// set sample size
			switch ((short)infoPtr)
			{
				case 8:									// valid sample sizes
					(**globals->prefs).sampleSize = (short)infoPtr;
					globals->outputData.format = k8BitOffsetBinaryFormat;
					globals->prefsChanged = true;
					break;				
				
				case 16:
					(**globals->prefs).sampleSize = (short)infoPtr;
					globals->outputData.format = k16BitBigEndianFormat;
					globals->prefsChanged = true;
					break;
				
				default:
					result = siInvalidSampleSize;
			}
			break;

		case siSampleRate:								// set sample rate
			switch ((UnsignedFixed)infoPtr)
			{
				case rate48khz:							// valid sample rates
				case rate44khz:
				case rate22050hz:
				case rate11025hz:
				case 0x1F400000:
					(**globals->prefs).sampleRate = (UnsignedFixed)infoPtr;
					globals->prefsChanged = true;
					break;
				
				default:
					result = siInvalidSampleRate;
			}
			break;

		case siNumberChannels:							// set no. channels
			if ( ((short)infoPtr == 1) || ((short)infoPtr == 2) )
			{
				(**globals->prefs).numChannels = (short)infoPtr;
				globals->prefsChanged = true;
			}
			else
				result = notEnoughHardware;
			break;

		case siHardwareVolume:
			// the volume is two 16 bit values combined into a long
			// the range is 0x0000 - 0x0100, where 0x0100 is the max volume level
			(**globals->prefs).volume = (long)infoPtr;
			globals->prefsChanged = true;
			break;

		case siHardwareMute:
			(**globals->prefs).muteState = (short)infoPtr;
			globals->prefsChanged = true;
			break;
			
		// if you do not handle this selector, then call up the chain
		default:
			result = SoundComponentSetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
			break;
	}
	return (result);
}


/*	StartSource

	This routine is used to start sounds playing that are currently paused. It should
	first delegate this call up the component chain so the rest of the chain can prepare
	to play this sound. Then, if the hardware is not already started it should be
	turned on.

	NOTE: This can be called at interrupt time.
*/

PASCAL_RTN ComponentResult __SoundOutputStartSource (SoundOutputGlobalsPtr globals, short count, SoundSource *sources)
{
	ComponentResult		result;

	// tell the mixer to start these sources
	result = SoundComponentStartSource(globals->sourceComponent, count, sources);
	FailIf(result != noErr, Exit);

	// make sure hardware interrupts are running
	StartHardware(globals);

Exit:
	return (result);
}


/*	PlaySourceBuffer

	This routine is used to specify a new sound to play and conditionally start
	the hardware playing that sound. It should first delegate this call up the component
	chain so the rest of the chain can prepare to play this sound. Then, if the
	hardware is not already started it should be turned on.

	NOTE: This can be called at interrupt time.  */

PASCAL_RTN ComponentResult __SoundOutputPlaySourceBuffer (SoundOutputGlobalsPtr globals, SoundSource sourceID, SoundParamBlockPtr pb, long actions)
{
	ComponentResult		result;

	// tell mixer to we'll start playing this new buffer
	result = SoundComponentPlaySourceBuffer(globals->sourceComponent, sourceID, pb, actions);
	FailIf(result != noErr, Exit);

	// if the kSourcePaused bit is set, then do not turn on your hardware just yet
	// (the assumption is that StartSource() will later be used to start this sound playing).
	// If this bit is not set, turn your hardware interrupts on.

	if ( !(actions & kSourcePaused) )
		StartHardware(globals);

Exit:
	return (result);
}
