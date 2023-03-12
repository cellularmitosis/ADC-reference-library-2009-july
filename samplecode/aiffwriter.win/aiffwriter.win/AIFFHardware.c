/*
**	Apple Macintosh Developer Technical Support
**
**	Routines for dealing with virtual sound hardware from a 'sdev' component.
**
**	by Mark Cookson, Apple Developer Technical Support
**	--based on code by Kip Olson
**
**	File:	Hardware.c
**
**	Copyright �1993-1996 Apple Computer, Inc.
**	All rights reserved.
**
**	You may incorporate this sample code into your applications without
**	restriction, though the sample code has been provided "AS IS" and the
**	responsibility for its operation is 100% yours.  However, what you are
**	not permitted to do is to redistribute the source as "Apple Sample
**	Code" after having made changes. If you're going to re-distribute the
**	source, we require that you make it clear in the source that the code
**	was descended from Apple Sample Code, but that you've made changes.
*/

#include <AIFF.h>
#include <Files.h>
#include <Devices.h>
#include <Timer.h>
#include <Sound.h>
#include <FixMath.h>
#include <Script.h>

#include "AIFFWriter.h"


#ifdef FakeInterrupts
extern SoundOutputGlobalsPtr					gGlobals;
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize the hardware.

OSErr SetupHardware (SoundOutputGlobalsPtr globals)
{
	IOBufferPtr			ioBuffer;
	ParmBlkPtr			iopb;
	unsigned long		samples;
	short				i;
	OSErr				result;

	// setup our output data which we'll request from the mixer and
	// setup the output file as well
	
	globals->outputData.flags = 0;
	if ((**globals->prefs).sampleSize == 8)
		globals->outputData.format = k8BitOffsetBinaryFormat;
	else
		globals->outputData.format = k16BitBigEndianFormat;
	globals->outputData.numChannels = (**globals->prefs).numChannels;
	globals->outputData.sampleSize = (**globals->prefs).sampleSize;
	globals->outputData.sampleRate = (**globals->prefs).sampleRate;
	globals->outputData.sampleCount = (**globals->prefs).sampleCount;
	globals->outputData.buffer = nil;
	globals->outputData.reserved = 0;

	// Setup hardware here. This example just calculates the interrupt interval
	// for the time manager task interrupt based on the current sample rate.

	globals->interruptInterval = UnsignedFixedMulDiv(((long)kHardwareSampleCount) << 16,
													1000000, globals->outputData.sampleRate);

	samples = UnsignedFixedMulDiv(globals->outputData.sampleRate, kSecondsInIOBuffer, fixed1);
	samples *= (globals->outputData.sampleSize >> 3) * globals->outputData.numChannels;
	globals->ioBufferSize = (samples + kHardwareSampleCount - 1) & (~(kHardwareSampleCount - 1));

	result = SetupOutputFile(globals);							// initialize output file
	FailIf(result != noErr, Exit);

	for (i = 0; i < 2; ++i)										// loop over both parameter blocks
	{
		ioBuffer = &globals->ioBuffers[i];

		// get some memory for i/o buffers
		ioBuffer->buffer = NewPtrClear(globals->ioBufferSize);
		FailIf(ioBuffer->buffer == nil, NewPtrFailed);
		iopb = &ioBuffer->iopb;									// get pointer to param block

		/* fill up the param block with the fields that never change after this. Note that
		   fields that are set to zero are commented out since we just zeroed things above. */

		iopb->ioParam.ioRefNum		= globals->fRefNum;			// refnum of file to write to
//		iopb->ioParam.ioCompletion	= nil;						// no completion routine
//		iopb->ioParam.ioReqCount	= 0;						// no request yet
		iopb->ioParam.ioBuffer		= ioBuffer->buffer;			// buffer to write to
		iopb->ioParam.ioPosMode		= (1 << noCacheBit);		// write w/o cache
//		iopb->ioParam.ioPosOffset	= 0;						// no offset yet
	}
	return (noErr);


NewPtrFailed:
	if (globals->ioBuffers[0].buffer != nil)
		DisposePtr(globals->ioBuffers[0].buffer);			// dispose of buffers
	if (globals->ioBuffers[1].buffer != nil)
		DisposePtr(globals->ioBuffers[1].buffer);			// dispose of buffers
Exit:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Release the hardware.
// BUG ALERT!  If we sample MicroSeconds at just the wrong time, we could be here for a long time!

void ReleaseHardware (SoundOutputGlobalsPtr globals)
{
	unsigned long		timeLimit;
	unsigned long		startTime;

	startTime = MicroSeconds();
	timeLimit = globals->interruptInterval << 1;				// wait for 2 interrupt periods
	while ((globals->hardwareOn) && ((MicroSeconds() - startTime) < timeLimit))
	{
		;	// wait until interrupts turn off, or we time out
	}

	StopHardware(globals);										// make sure hardware is off

	if (globals->fRefNum != 0)
	{
		CloseOutputFile(globals);								// close output file
		globals->fRefNum = 0;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Turn on hardware interrupts.

void StartHardware(SoundOutputGlobalsPtr globals)
{
	if (!globals->hardwareOn)
	{
		globals->hardwareOn = true;							// the hardware will soon be on

		// Turn hardware on here. The example uses the Time Manager for interrupts

#ifndef FakeInterrupts
		// start the time manager task going

		globals->tmTask.task.tmAddr = NewTimerProc(TMInterrupt);
		globals->tmTask.task.tmCount = 0;
		globals->tmTask.task.tmWakeUp = 0;
		globals->tmTask.task.tmReserved = 0;
		globals->tmTask.globals = globals;

		InsXTime((QElemPtr) &globals->tmTask);
		PrimeTime((QElemPtr) &globals->tmTask, -globals->interruptInterval);
#endif
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Turn off hardware interrupts.

void StopHardware(SoundOutputGlobalsPtr globals)
{
	if (globals->hardwareOn)
	{
		// Turn hardware off here. The example removes the Time Manager task

#ifndef FakeInterrupts
		RmvTime((QElemPtr) &globals->tmTask);
#endif
		globals->hardwareOn = false;						// the hardware is now off
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Resume hardware interrupts after they were suspended.

void ResumeHardware(SoundOutputGlobalsPtr globals)
{
	if (globals->hardwareOn)
	{
		// Resume hardware interrupts here. The example queues another Time Manager interrupt

#ifndef FakeInterrupts
		PrimeTime((QElemPtr) &globals->tmTask, -globals->interruptInterval);
#endif
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copy samples to the hardware buffer. In our case, write to a file.

#define SamplesToBytes(samples, shift)	(samples << shift)
#define BytesToSamples(bytes, shift)	(bytes >> shift)

void CopySamplesToHardware(SoundOutputGlobalsPtr globals, SoundComponentDataPtr siftPtr)
{
	long			samplesToCopy;
	long			bytesToCopy;
	long			bytesLeft;
	IOBufferPtr		currentBuffer;
	short			sourceShift;
	short			destShift;

	sourceShift = siftPtr->sampleSize / 16;
	if (siftPtr->numChannels == 2)
		sourceShift++;

	destShift = globals->outputData.sampleSize / 16;				// add 1 more for 16 bit, 2 for 32
	if (globals->outputData.numChannels == 2)
		destShift++;

	samplesToCopy = siftPtr->sampleCount;							// don't copy more than hardware buffer has
	if (samplesToCopy > kHardwareSampleCount)
		samplesToCopy = kHardwareSampleCount;

	currentBuffer = &globals->ioBuffers[globals->currentIndex];		// get current i/o buffer
	if (currentBuffer->iopb.ioParam.ioResult != 0)
	{																// oh, oh, write is still in progress
		FailMessage(currentBuffer->iopb.ioParam.ioResult != 0);
		siftPtr->buffer += SamplesToBytes(samplesToCopy, sourceShift);	// update source pointer
		siftPtr->sampleCount -= samplesToCopy;						// subtract amount copied from source and throw these bytes away!
		return;														// throw these bytes away!
	}

	bytesToCopy = SamplesToBytes(samplesToCopy, destShift);		// turn samples into bytes
	bytesLeft = globals->ioBufferSize - currentBuffer->byteCount;		// calc how much is left in i/o buffer

	if (bytesToCopy > bytesLeft)								// limit to amount we have left in buffer
		bytesToCopy = bytesLeft;
	samplesToCopy = BytesToSamples(bytesToCopy, destShift);		// turn back into samples

	if (globals->fRefNum) 										// only write out if file is open
		OutputToFile(siftPtr, currentBuffer->buffer + currentBuffer->byteCount, bytesToCopy);

	currentBuffer->byteCount += bytesToCopy;					// update dest pointer
	siftPtr->buffer += SamplesToBytes(samplesToCopy, sourceShift);	// update source pointer
	siftPtr->sampleCount -= samplesToCopy;						// subtract amount copied from source

	if (currentBuffer->byteCount == (long)globals->ioBufferSize)// current buffer is full - write it out
	{
		if (globals->fRefNum)									// only write out if file is open
		{
			currentBuffer->iopb.ioParam.ioReqCount = currentBuffer->byteCount;	// write this many bytes out
			currentBuffer->iopb.ioParam.ioPosOffset = 0;		// offset from current position

#ifdef FakeInterrupts
			PBWriteSync(&currentBuffer->iopb);					// write out the data synchronously
#else
			PBWriteAsync(&currentBuffer->iopb);					// start the asynch write
#endif
		}

		currentBuffer->byteCount = 0;							// buffer will be empty next time we use it
		globals->currentIndex ^= 1;								// switch to other buffer
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Slam the file.

void OutputToFile(SoundComponentDataPtr siftPtr, void *dest, long bytesToCopy)
{
	long	i;
	Byte	*sp;
	Byte	*dp;

	if (siftPtr->format == k8BitOffsetBinaryFormat)				// 8 bit offset source
	{
		// convert the data from offset binary to two's complement
		
		sp = (Byte *)siftPtr->buffer;
		dp = (Byte *)dest;
		for (i = bytesToCopy - 1; i >= 0; --i)
			*dp++ = *sp++ ^ 0x80;
	}
	else														// copy other formats
		BlockMove(siftPtr->buffer, dest, bytesToCopy);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Fake interrupt routine used when running the debugging application.

#ifdef FakeInterrupts
Boolean FakeInterrupt(void)
{
	SoundOutputGlobalsPtr	globals = gGlobals;					// get our globals

	if (globals->hardwareOn)
	{
		if (MicroSeconds() >= globals->nextTime)
		{
			globals->nextTime = MicroSeconds();
			InterruptRoutine(globals);
			globals->nextTime += globals->interruptInterval;
		}
	}
	return (globals->hardwareOn);
}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Entry point for the Time Manager task interrupt routine

//#if __MWERKS__ && TARGET_CPU_68K

//void TMInterrupt (myTMTaskPtr taskPtr:__a1)
//{
//	InterruptRoutine(taskPtr->globals);						// call interrupt routine with globals
//}

//#else

void TMInterrupt (myTMTaskPtr taskPtr)
{
#if TARGET_CPU_68K
	taskPtr = (myTMTaskPtr)GetRegisterA1();
#endif
	InterruptRoutine(taskPtr->globals);						// call interrupt routine with globals
}
//#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This routine is called every hardware interrupt to fill the hardware
// with audio data. First it should suspend interrupts so it will not
// be interrupted again (this example just queues up the next interrupt).
// Then it should get more data from the source mixer, and copy the data
// to the hardware. In the way out, if all data was copied, try to get
// some more so it will be available next time.

void InterruptRoutine(SoundOutputGlobalsPtr globals)
{
	SoundComponentDataPtr		siftPtr;

	siftPtr = GetMoreSource(globals);						// get source from mixer
	if (siftPtr == nil)										// no more source
		StopHardware(globals);								// turn hardware off
	else
	{
		CopySamplesToHardware(globals, siftPtr);			// fullfill hardware request

		if (siftPtr->sampleCount == 0)						// exhausted the source
			siftPtr = GetMoreSource(globals);				// get more for next time
	}
	ResumeHardware(globals);								// queue next interrupt while we are processing
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This routine returns the component data pointer to your mixer source. If there
// is no source or it is empty, it will call down the chain to fill it up.

SoundComponentDataPtr GetMoreSource(SoundOutputGlobalsPtr globals)
{
	ComponentResult			result;
	SoundComponentDataPtr	siftPtr;

	siftPtr = globals->sourceDataPtr;
	if ((siftPtr == nil) || (siftPtr->sampleCount == 0))	// no data - better get some
	{
		result = SoundComponentGetSourceData(globals->sourceComponent, &globals->sourceDataPtr);
		FailIf(result != noErr, Failure);

		siftPtr = globals->sourceDataPtr;
		if ((siftPtr == nil) || (siftPtr->sampleCount == 0)) // source has no more data
			return (nil);									// stop the presses
	}
	return (siftPtr);										// return pointer to source

Failure:
	return (nil);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This routine initializes the output file and sets up the i/o param block
// used to write the data to disk.

OSErr SetupOutputFile (SoundOutputGlobalsPtr globals)
{
	FSSpec			fspec;
	OSErr			err;

	err = FSMakeFSSpec(0, 0, kSndFileName, &fspec);			// make the FS spec
	if (err == fnfErr)										// file not found is OK
		err = noErr;
	else
		FailIf(FSpDelete(&fspec) != noErr, Failure);
	FailIf(err != noErr, Failure);

	err = FSpCreate(&fspec, kAIFFCreator, AIFFID, smSystemScript);	// create file
	FailIf(err != noErr, FSpCreateFailed);

	err = FSpOpenDF(&fspec, fsRdWrPerm, &globals->fRefNum);		// open file
	FailIf(err != noErr, FSpOpenDFFailed);

	err = SetupAIFFHeader(globals->fRefNum, globals->outputData.numChannels, globals->outputData.sampleRate,
						  globals->outputData.sampleSize, globals->outputData.format, 0, 0);
	FailIf(err != noErr, SetupAIFFFailed);

	err = GetFPos(globals->fRefNum, &globals->headerLen);		// get length of AIFF header
	FailIf(err != noErr, SetupAIFFFailed);

	return (noErr);

SetupAIFFFailed:
	if (globals->fRefNum != 0)
		FSClose(globals->fRefNum);

FSpOpenDFFailed:
FSpCreateFailed:
Failure:
	return (err);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This routine makes sure all asynchronous i/o is complete, writes out any
// remaining data, updates the AIFF header and closes the file.

OSErr CloseOutputFile(SoundOutputGlobalsPtr globals)
{
	long			filePos, bytesWritten;
	IOBufferPtr		currentBuffer;
	OSErr			err;

	while  ((globals->ioBuffers[globals->currentIndex].iopb.ioParam.ioResult > 0) ||	// wait for PBWrite to complete
			(globals->ioBuffers[globals->currentIndex^1].iopb.ioParam.ioResult > 0))	// for both buffers
		;

	currentBuffer = &globals->ioBuffers[globals->currentIndex];	// get current i/o buffer

	if (currentBuffer->byteCount != 0)					// still some data to write out
	{
		currentBuffer->iopb.ioParam.ioReqCount = currentBuffer->byteCount;	// write this many bytes out
		currentBuffer->iopb.ioParam.ioPosOffset = 0;	// offset from current position

		err = PBWriteSync(&currentBuffer->iopb);		// write this buffer synchronously
		FailIf(err != noErr, PBWriteSyncFailed);
	}

	err = GetFPos(globals->fRefNum, &filePos);			// get current file position
	FailIf(err != noErr, GetFPosFailed);

	bytesWritten = filePos - globals->headerLen;		// calc no. bytes written to file
	filePos = ++filePos & ~1;							// make sure file length is a word-aligned

	err = SetEOF(globals->fRefNum, filePos);			// set current file position to EOF
	FailIf(err != noErr, SetEOFFailed);

	err = SetFPos(globals->fRefNum, fsFromStart, 0);	// rewind file to beginning
	FailIf(err != noErr, SetFPosFailed);

	err = SetupAIFFHeader(globals->fRefNum, globals->outputData.numChannels, globals->outputData.sampleRate,
						  globals->outputData.sampleSize, globals->outputData.format, bytesWritten, 0);
	FailIf(err != noErr, SetupAIFFHeaderFailed);
	// all non-error conditions can fall through to the clean up code

SetupAIFFHeaderFailed:
SetFPosFailed:
SetEOFFailed:
GetFPosFailed:
PBWriteSyncFailed:
	FSClose(globals->fRefNum);							// close output file
	globals->fRefNum = 0;

	return (err);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSErr GetPreferences (SoundOutputGlobalsPtr globals)
{
	HardwarePrefsHandle		prefs;
	HardwarePrefsHandle		oldPrefs;
	OSErr					result;

	prefs = (HardwarePrefsHandle)GetComponentRefcon((Component)globals->self);
	if (prefs == nil)
	{	
		// no in memory preferences found, then create them
		prefs = (HardwarePrefsHandle)NewHandleSysClear(sizeof(HardwarePreferences));
		FailWithAction(prefs == nil, result = MemError(), NewPrefsFailed);

		// set our preferences to default values and put them in our refCon
		(**prefs).size = sizeof(HardwarePreferences);
		(**prefs).version = kSoundComponentVersion;
		(**prefs).volume = 0x00100100;
		(**prefs).sampleRate = rate22050hz;				// default sample rate
		(**prefs).sampleSize = 16;						// default sample size
		(**prefs).numChannels = 2;						// default num channels
		(**prefs).sampleCount = kHardwareSampleCount;	// default buffer count
		SetComponentRefcon((Component)globals->self, (unsigned long)prefs);
		globals->prefs = prefs;											// set global

		oldPrefs = (HardwarePrefsHandle)NewHandleSysClear(sizeof(HardwarePreferences));
		FailWithAction(oldPrefs == nil, result = MemError(), NewOldPrefsFailed);

		result = GetSoundPreference(kAIFFWriterSubType, kAIFFWriterName, (Handle)oldPrefs);
		if (result == noErr)
		{
			// we found our old preferences, check version and size of old preferences
			(**prefs).volume = (**oldPrefs).volume;
			(**prefs).sampleRate = (**oldPrefs).sampleRate;
			(**prefs).sampleSize = (**oldPrefs).sampleSize;
			(**prefs).numChannels = (**oldPrefs).numChannels;
			(**prefs).sampleCount = (**oldPrefs).sampleCount;
		}
		DisposeHandle((Handle)oldPrefs);
		
		result = InitFromPreferences(globals);
		FailIf(result != noErr, InitHWFailed);
	}
	return (noErr);
	
InitHWFailed:
NewOldPrefsFailed:
NewPrefsFailed:
	return (result);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup the hardware according to the user's preferences.
// Configure as much of the hardware as possible at this time, and use the
// InitOutputDevice() method call for the configuration that needs to be done
// everytime the component is being opened to play audio. If altering a setting
// would cause a click, then it would be better to do that here instead of everytime
// the user started a new sound.

OSErr InitFromPreferences(SoundOutputGlobalsPtr globals)
{
	globals; // suppress "unused variable" warning for all compilers
	return (noErr);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if !TARGET_CPU_68K
// for 68K, the macro we are using only returns a long, so the PPC version
// will just return the lo part of the number to be consistent
unsigned long MicroSeconds(void)
{
	UnsignedWide			microTickCount;

	Microseconds(&microTickCount);
	return (microTickCount.lo);
}
#endif
