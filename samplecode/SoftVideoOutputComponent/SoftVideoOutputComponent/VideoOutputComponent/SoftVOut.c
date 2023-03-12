/*
	File:		SoftVOut.c
	
	Description: This is an example video output component.

	Author:		QuickTime Engineering, QuickTime DTS

	Copyright: 	© Copyright 1998 - 2005 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first): <2> dts 08/02/2005 Added echo port support for 'raw ' source and filled out
                                                           implementation by adding sample routines for GetIndSoundOutput,
                                                           GetClock and CopyIndAudioOutputDeviceUID
                                                           Updated Xcode project for universal binary support
                                        <1> 12/20/2001 Initial Release
*/

/* 

A video output component receives QuickTime video data and delivers data to a video output
device for display. If the incoming data is in a format that the video output device can
display directly, the video output component can simply send the data to the video output device.
If the incoming data cannot be displayed directly, the video output component must use a transfer
codec or decompressor component to convert the data to a format that the video output device can display.

If a video output device cannot directly display 32-bit RGB data or data in one of the other
supported QuickTime pixel formats, the developers of the device are strongly encouraged to provide
a transfer codec that accepts data in one of the supported QuickTime pixel formats (preferably 32-bit RGB)
and converts it to data that can be displayed on the device. When this transfer codec is available,
any QuickTime video can be displayed on the video output device: the Image Compression Manager can convert
any QuickTime images to a supported QuickTime pixel format and then invoke the transfer codec to display
the result.

If any special decompressors, such as a transfer codec, are needed for a video output device, the
decompressors are included in the definitions of the component's display modes.

*/

#if __APPLE_CC__ || __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreAudio/CoreAudio.h>
#else
	#include <Components.h>
	#include <Movies.h>
	#include <ImageCodec.h>
	#include <ImageCompression.h>
	#include <QuickTimeComponents.h>
	#include <string.h>
#endif

// VOut component shared globas
typedef struct {
	CGrafPtr echoPort;
	Ptr      baseAddr;
	long	 rowBytes;
	short	 width;
	short	 height;
	Boolean  isEchoPortOn;
} SharedGlobalsRecord, *SharedGlobalsPtr;

// Per instance globals
typedef struct {
	ComponentInstance	  self;
	ComponentInstance	  target;
	ComponentInstance	  baseVideoOutput;
	Component			  soundOutput;
	ComponentInstance	  clock;
	SharedGlobalsPtr 	  sharedGlobals;
	QTAtomContainer		  displayModeList;
	long				  theCustomPixelFormat;
	short				  width;
	short				  height;
	Boolean				  ownsHardware;
} SoftVoutGlobalsRecord, *SoftVoutGlobalsPtr;

// Our Private Pixel Format
// To register a new fourCC please send mail to qtfourCC@apple.com.
// Include an email address (for future correspondence), the fourCC you
// would like to register, and a brief description of the fourCC format.
// For more information refer to IceFloe #20 - QuickTime Pixel Format FourCCs
// http://developer.apple.com/quicktime/icefloe/dispatch020.html
enum {
	kSoftPixelFormat = FOUR_CHAR_CODE('soft')
};

enum {
	kSoftCodecType = FOUR_CHAR_CODE('raw '),
	kSoftCodecManufacturer = FOUR_CHAR_CODE('dts ')
};

// Prototypes
static ComponentResult CreateVOutSharedGlobals(SoftVoutGlobalsPtr storage);
static OSStatus GetMyAudioDriverUID(CFStringRef *outUID);
static pascal ComponentResult CreateDisplayModeList(SoftVoutGlobalsPtr storage);
static pascal ComponentResult CreateDisplayModeAtom(SoftVoutGlobalsPtr storage, OSType pixelType, const char *name);
static pascal ComponentResult AddDecompressorsAtomToDisplayMode(SoftVoutGlobalsPtr storage, UInt8 index, ComponentDescription *cd, Boolean continuous);

/************************************************************************************/
// Setup required for ComponentDispatchHelper.c

#define CALLCOMPONENT_BASENAME()	SoftVout_
#define CALLCOMPONENT_GLOBALS()		SoftVoutGlobalsPtr storage

#define QTVIDEOOUTPUT_BASENAME()	CALLCOMPONENT_BASENAME()
#define	QTVIDEOOUTPUT_GLOBALS()		CALLCOMPONENT_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"SoftVOutDispatch.h"
#define	GET_DELEGATE_COMPONENT()	(storage->baseVideoOutput)
#define COMPONENT_UPP_SELECT_ROOT()	QTVideoOutput

#if __APPLE_CC__ || __MACH__
	#include <CoreServices/Components.k.h>				// Standard .k.h
	#include <QuickTime/QuickTimeComponents.k.h>		// Type & SubType .k.h
	#include <QuickTime/ComponentDispatchHelper.c>		// Make the dispatcher and canDo
#else
	#include "Components.k.h"				// Standard .k.h
	#include "QuickTimeComponents.k.h"		// Type & SubType .k.h
	#include "ComponentDispatchHelper.c"	// Make the dispatcher and canDo
#endif

/************************************************************************************/
// Component Manager Calls

// Component Open Request - Required
pascal ComponentResult SoftVout_Open(SoftVoutGlobalsPtr storage, ComponentInstance self)
{
	ICMPixelFormatInfo pixelInfo;

	ComponentResult	   err;

	// Allocate globals
	storage = (SoftVoutGlobalsPtr)NewPtrClear(sizeof(SoftVoutGlobalsRecord));
	if ((err = MemError())) goto bail;

	SetComponentInstanceStorage(self, (Handle)storage);
	storage->self	= self;
	storage->target	= self;
	storage->soundOutput = 0;
	storage->clock = 0;
	storage->sharedGlobals = NULL;
	
	// Open the base VOut for future delegation
	// To use the services of the base video output component,
	// your video output component must open a connection to the base video output component
	err	= OpenADefaultComponent(QTVideoOutputComponentType, QTVideoOutputComponentBaseSubType, &storage->baseVideoOutput);
	if (err) goto bail;

	// Set us as the base component's target
	err	= ComponentSetTarget(storage->baseVideoOutput, self);
	if (err) goto bail;	

	// SoftVOut a.k.a Mr. Fake Video Output is 320 x 240
	storage->width	= 320;
	storage->height	= 240;
	storage->theCustomPixelFormat = kSoftPixelFormat;
	
	// Register information about our custom pixel format with the ICM, this is
	// needed only for the case where the VOut is using a transfer codec (SoftCodec)
	BlockZero(&pixelInfo, sizeof(pixelInfo));
	pixelInfo.size			  = sizeof(ICMPixelFormatInfo);
	pixelInfo.formatFlags	  = 0;
	pixelInfo.bitsPerPixel[0] = 32;
	
	// Ignore any errors as this could be a duplicate registration
	ICMSetPixelFormatInfo(storage->theCustomPixelFormat, &pixelInfo);
	
    // Create our shared globals with the transfer codec so our echo port stuff will work
	err = CreateVOutSharedGlobals(storage);
	
bail:
	if (err) {
		SoftVout_Close(storage, self);
	}
	
	return err;
}

// Component Close Request - Required
pascal ComponentResult SoftVout_Close(SoftVoutGlobalsPtr storage, ComponentInstance self)
{
	SharedGlobalsPtr mySharedGlobals = NULL;
	
	if (storage) {
		if (storage->baseVideoOutput) {
			// When your video output component closes, it must close its connection
			// to the base video output component
			CloseComponent(storage->baseVideoOutput);
			storage->baseVideoOutput = 0;
		}
		
		if (storage->displayModeList) {
			QTDisposeAtomContainer(storage->displayModeList);
			storage->displayModeList = NULL;
		}
		
		if (storage->sharedGlobals) {
			mySharedGlobals = (SharedGlobalsPtr)GetComponentRefcon((Component)storage->self);
			if (mySharedGlobals) {
				DisposePtr((Ptr)mySharedGlobals);
				SetComponentRefcon((Component)storage->self, (long)NULL);
			}
		}
						
		DisposePtr((Ptr)storage);
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult SoftVout_Version(SoftVoutGlobalsPtr storage)
{
#pragma unused(storage)

	const int videoOutputInterfaceVersion = 1;
	
	return (videoOutputInterfaceVersion << 16) + 1;
}

// Component Target Request
pascal ComponentResult SoftVout_Target(SoftVoutGlobalsPtr storage, ComponentInstance target)
{
	ComponentResult	err;

	// Tell the base video output to target the instance
	err	= CallComponentTarget(storage->baseVideoOutput, target);
	if (err) goto bail;

	// Remember our target
	storage->target	= target;

bail:
	return err;
}

// Component Register Request
pascal ComponentResult SoftVout_Register(SoftVoutGlobalsPtr storage)
{
#pragma unused(storage)

	// Always register
	return noErr;
}

#pragma mark-
/************************************************************************************/
// Video Output Component Calls

// QTVideoOutputGetDisplayModeList
// 		Return a copy of the list to the caller, they are responsible for disposing of it.
//		See below for more information.
pascal ComponentResult SoftVout_GetDisplayModeList(SoftVoutGlobalsPtr storage, QTAtomContainer* outputs)
{
	ComponentResult	err	= noErr;

	// Create our list of display modes if not done yet
	if (NULL == storage->displayModeList) {
		err	= CreateDisplayModeList(storage);
		if (err) goto bail;
	}

	// Copy it all into a new atom container
	err	= QTNewAtomContainer(outputs);
	if (noErr == err) {
		err	= QTCopyAtom(storage->displayModeList, kParentAtomIsContainer, outputs);
		if (noErr != err) {
			QTDisposeAtomContainer(*outputs);
		}
	}
	
bail:
	return err;
}

// QTVideoOutputBegin
// 		This is basically when you should turn your Video Output hardware on.
pascal ComponentResult SoftVout_Begin(SoftVoutGlobalsPtr storage)
{
	long			mode;
	
	ComponentResult	err;

	// Call the default implementation
	// The default implementation of the QTVideoOutputBegin function ensures that
	// the hardware is not currently in use by other software. It also ensures that a
	// valid display mode has been set with either the QTVideoOutputSetDisplayMode function or the
	// QTVideoRestoreSettings function.
	err	= QTVideoOutputBegin(storage->baseVideoOutput);
	if (noErr == err) {
		// Get the selected mode
		err	= QTVideoOutputGetDisplayMode(storage->self, &mode);
		if (noErr == err) {
			// NOTE: -> This is a good place to switch your hardware to the selected mode

			// Remember that we now own the hardware
			storage->ownsHardware = true;
		}
	}

	if (noErr != err  &&  storage->ownsHardware) {
		QTVideoOutputEnd(storage->baseVideoOutput);
	}

	return err;
}

// QTVideoOutputEnd
// 		This is basically when you should turn your Video Output hardware off.
// In the implementation of QTVideoOutputEnd, your component should also display a
// default image on the video output device to indicate that the device is no longer in use
// by other software.
pascal ComponentResult SoftVout_End(SoftVoutGlobalsPtr storage)
{
	ComponentResult	err;

	// Check that Begin has been called
	if (!storage->ownsHardware) return paramErr;

	// NOTE: -> This is a good place to release your hardware

	// Call the default implementation
	err	= QTVideoOutputEnd(storage->baseVideoOutput);
	
	if (storage->clock) {
		CloseComponent(storage->clock);
		storage->clock = 0;
	}

	// Remember that we no longer own the hardware
	storage->ownsHardware = false;

	return err;
}

// QTVideoOutputGetGWorldParameters - Required
//		Return the parameters of the Video Output GWorld.
// 		Your video output component must implement this function, it is not called by applications or other
//		clients of your component; it is called by the base video output component as part of the implementation
//		of the QTVideoOutputGetGWorld function.
// 		baseAddr - return the address at which to display pixels. If your component does not display
//				   pixels, return 0.
//		rowBytes - return the width of each scan line in bytes. If your component does not display
//				   pixels, return the width of the current display mode.
//		colorTable - return the color table to be used. If your component does not use a color table, return NULL.
// 		In the software case, we are simply returning a pointer to a portion of the main screen.
pascal ComponentResult SoftVout_GetGWorldParameters(SoftVoutGlobalsPtr storage, Ptr *baseAddr, long *rowBytes, CTabHandle *colorTable)
{
	GDHandle		mainGD = GetMainDevice();
	
	ComponentResult	err = internalComponentErr;
	
	// The VOut is going to use the lower right hand corner of the main device
	if (mainGD) {
		PixMapHandle gdPMap	= (*mainGD)->gdPMap;

		// Get the rowBytes from the main device's PixMap
		*rowBytes = QTGetPixMapPtrRowBytes(*gdPMap);

		// Get the color table from the main device
		if ((*gdPMap)->pixelSize < 16) {
			*colorTable = (*gdPMap)->pmTable;
		} else {
			*colorTable = NULL;
		}

		// Offset the screen's baseAddr to the top of our VOut
		*baseAddr = (*gdPMap)->baseAddr
					+ ((*gdPMap)->bounds.bottom - storage->height) * (*rowBytes)
					+ ((*gdPMap)->bounds.right - storage->width) * ((*gdPMap)->pixelSize / 8);
		
        // Save the required information for the transfer codec in our shared globals
		storage->sharedGlobals->baseAddr = *baseAddr;
		storage->sharedGlobals->rowBytes = *rowBytes;
		storage->sharedGlobals->width = storage->width;
		storage->sharedGlobals->height = storage->height;
		
		err	= noErr;
	}
	
	return err;
}

#if TARGET_OS_MAC
// QTVideoOutputGetIndSoundOutput
// 		Return the sound output component associated with the video output component
//      specified by the index parameter. The index of the first component should be 1.
pascal ComponentResult SoftVout_GetIndSoundOutput(SoftVoutGlobalsPtr storage, long index, Component *outputComponent)
{
	Component 				c = 0;
	ComponentDescription 	cd = { kSoundOutputDeviceType, kHALCustomComponentSubType, 0, 0, 0 };
	CFStringRef 			myUID = NULL;
	CFStringRef 			halUID = NULL;
	SMStatus 				ignore;

	ComponentResult 		err;
	
	*outputComponent = NULL;
	
	// we only have one Sound Output Component
	// so if index is anything other than 1 it's an error
	if (index != 1 ) return paramErr;
			
	// already have it, we're done
	if (storage->soundOutput) {
		*outputComponent = storage->soundOutput;
		return noErr;
	}

	// find our audio drivers UID using core audio
	err = GetMyAudioDriverUID(&myUID);
	if (err) goto bail;
	
	// cheaply init the Sound Manager - must be done so the Sound Manager will
	// synthesize Sound Output Components for each Core Audio Driver on the system
	SndManagerStatus(sizeof(SMStatus), &ignore);
	
	// find our synthesized Sound Output Component
	while (c = FindNextComponent(c, &cd)) {
		// don't release the returned CFString here, it's not retained as in the property case
		err = SoundComponentGetInfo((ComponentInstance)c, 0, siHALAudioDeviceUniqueID, &halUID);
		if (err) goto bail;
		
		// if it's us, we're done
		if (CFEqual(myUID, halUID)) {
			*outputComponent = c;
			storage->soundOutput = c;
			break;
		}
	}
	
	// we didn't find the corresponding sDev, that's bad!
	if (NULL == *outputComponent) err = badComponentType;

bail:
	if (myUID) CFRelease(myUID);
	
	return err;
}

// QTVideoOutputGetClock
// 		Returns a pointer to the clock component associated with the video output component.
pascal ComponentResult SoftVout_GetClock(SoftVoutGlobalsPtr storage, ComponentInstance *clock )
{
	ComponentDescription cd = { clockComponentType, systemMicrosecondClock, kAppleManufacturer, 0, 0 };
	
	ComponentResult err;
	
	*clock = 0;
		
	// Check that Begin has been called
	if (!storage->ownsHardware) return badCallOrderErr;
		
	if (storage->clock) {
		*clock = storage->clock;
		return noErr;
	}
	
	err = OpenAComponent(FindNextComponent(0, &cd), &storage->clock);
	
	if (noErr == err)
		*clock = storage->clock;
	
	return err;
}

// QTVideoOutputSetEchoPort
//		Specifies a window on the desktop in which to display video sent to the device.
pascal ComponentResult SoftVout_SetEchoPort(SoftVoutGlobalsPtr storage, CGrafPtr echoPort)
{
	ComponentResult err = noErr;
	
	// QuickTime 6 - Mach-O only 
	err = QTVideoOutputBaseSetEchoPort(storage->baseVideoOutput, echoPort);

    // Set the information in the shared globals for the transfer codec
	if (err == noErr) {
		storage->sharedGlobals->echoPort = echoPort;
		storage->sharedGlobals->isEchoPortOn = (NULL != echoPort);
	} else {
		storage->sharedGlobals->echoPort = NULL;
		storage->sharedGlobals->isEchoPortOn = false;
	}
	
	return err;
}

// QTVideoOutputCopyIndAudioOutputDeviceUID
//      Identifies the audio device being used by a video output component.
pascal ComponentResult SoftVout_CopyIndAudioOutputDeviceUID(SoftVoutGlobalsPtr storage, long index, CFStringRef *audioDeviceUID)
{
#pragma unused (storage)

     CFStringRef     myUID = NULL;
     ComponentResult err = paramErr;

     // if we were passed a CFStringRef
     if ((index == 1) && (NULL != audioDeviceUID)) {
     
         // find our audio driver UID using core audio
         err = GetMyAudioDriverUID(&myUID);

         // if there was no error and we got the UID
         if ((noErr == err) && myUID) {
         
             // copy the UID for the caller
             *audioDeviceUID = CFStringCreateCopy(kCFAllocatorDefault, myUID);
            
             // done with this UID, release our reference
             CFRelease(myUID);
         }
     }
    
     return err;
}
#endif
	
#pragma mark-
/************************************************************************************/
// Local Component Calls

// Create some globals to be shared by the transfer codec
ComponentResult CreateVOutSharedGlobals(SoftVoutGlobalsPtr storage)
{
	SharedGlobalsPtr mySharedGlobals = NULL;
	
	ComponentResult err = noErr;

	// Create shared variables if they don’t exist
	mySharedGlobals = (SharedGlobalsPtr)GetComponentRefcon((Component)storage->self);
	if (mySharedGlobals == NULL) {
		mySharedGlobals = (SharedGlobalsPtr)NewPtrClear(sizeof(SharedGlobalsRecord));
		if (mySharedGlobals == NULL) {
			err = MemError();
			goto bail;
		}
		
		SetComponentRefcon((Component)storage->self, (long)mySharedGlobals);
	}
	
	storage->sharedGlobals = (SharedGlobalsPtr)mySharedGlobals;
	
bail:

	return err;
}

#define kMaxStringSize 1024

#if TARGET_OS_MAC
// GetMyAudioDriverUID
//		Find your drivers UID and return it in a CFString the calling
// function is responsible for releasing.
OSStatus GetMyAudioDriverUID(CFStringRef *outUID)
{
	UInt32			theSize;
	char			theString[kMaxStringSize];
	UInt32			theNumberDevices;
	AudioDeviceID   *theDeviceList = NULL;
	UInt32			theDeviceIndex;
	CFStringRef     theCFString = NULL;
	
	// this is us
	const char		*nameString = "Built-in Audio";
	const char 		*manufacturerString = "Apple";
	
	OSStatus		theStatus = noErr;
	
	*outUID = NULL;
	
	// device list size
	theSize = 0;
	theStatus = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &theSize, NULL);
	if (theStatus) goto done;
	
	theNumberDevices = theSize / sizeof(AudioDeviceID);
	
	// allocate the device list
	theDeviceList = (AudioDeviceID*)malloc(theNumberDevices * sizeof(AudioDeviceID));
	
	// get the device list
	theSize = theNumberDevices * sizeof(AudioDeviceID);
	theStatus = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &theSize, theDeviceList);

	// iterate through the device list, find our device and return the UID
	for(theDeviceIndex = 0; theDeviceIndex < theNumberDevices; ++theDeviceIndex) {
		// get a name
		theSize = kMaxStringSize;
		theStatus = AudioDeviceGetProperty(theDeviceList[theDeviceIndex], 0, 0, kAudioDevicePropertyDeviceName, &theSize, theString);
		if (theStatus) goto done;
		
		// is it me?
		if (strncmp(theString, nameString, strlen(nameString)) == 0) {
			
			// get a manufacturer
			theSize = kMaxStringSize;
			theStatus = AudioDeviceGetProperty(theDeviceList[theDeviceIndex], 0, 0, kAudioDevicePropertyDeviceManufacturer, &theSize, theString);
			if (theStatus) goto done;
			
			// is it really me?
			if (strncmp(theString, manufacturerString, strlen(manufacturerString)) == 0) {
				
				// get device UID
                // Core Audio retains the returned CFString so make sure and release it when you're done
				theSize = sizeof(CFStringRef);
				theStatus = AudioDeviceGetProperty(theDeviceList[theDeviceIndex], 0, 0, kAudioDevicePropertyDeviceUID, &theSize, &theCFString);
				if (theStatus) goto done;
				
				*outUID = theCFString;
				break;
			}
		}
	}
	
	// we didn't find ourselves, that's bad!
	if (NULL == *outUID) theStatus = badComponentType;
	
done:	
	// free the device list
	if (theDeviceList) free(theDeviceList);

	return theStatus;
}
#endif

/*

A video output device has one or more display modes. The characteristics of each mode determine
how video is displayed. When any software displays video on a video output device, it must select
which of the device's display modes to use.

The characteristics of a display mode include

+      the height of the displayed image, in pixels 
+      the width of the displayed image, in pixels 
+      the horizontal resolution of the display, in pixels per inch 
+      the vertical resolution of the display, in pixels per inch 
+      the refresh rate of the display, in Hertz 
+      the pixel type of the display 
+      a text description of the display mode 

The characteristics can also include a list of decompressor components required for the mode that are
provided specifically for the video output device. If a video output device cannot directly display any of
the pixel formats supported by QuickTime, the vendor of the device must provide one or more special decompressors
to convert supported pixel formats to a format the device can display. If a video output device can display one or
more of the pixel formats supported by QuickTime, the Image Compression Manager can use standard decompressors that
are included with QuickTime, and the list of special decompressor components can be empty.

These characteristics, returned by the QTVideoOutputGetDisplayModeList function, are stored in a QT atom container.

For a description of this QT atom container, see:
http://developer.apple.com/techpubs/quicktime/qtdevdocs/REF/refVidOutComp.1e.htm#23578

*/

// CreateDisplayModeList
//		Builds the entire display list hierarchy and returns the QTAtom if successful.
//
// 		SoftVOut supports two modes:
// 		1) The first mode provides a GWorld by wrapping a portion of the screen.
// 		   Codecs write directly to the GWorld since QuickTime understands the pixel format.
// 		2) The second mode uses a custom pixel type (kSoftPixelFormat). The fake VOut transfer
//		   codec (SoftCodec) will take care of the drawing.
//
//	Below is the structure of the display mode list. It contains two displays modes.
//  At the root of the QT atom container are one or more atoms of type kQTVODisplayModeItem.
//
//	kQTVODisplayModeItem						Mode 1
//		kQTVODimensions							320 x 240
//		kQTVOResolution							??, ??
//		kQTVORefreshRate						60
//		kQTVOPixelType							??
//		kQTVOName								SoftVOut
//
//	kQTVODisplayModeItem						Mode 2
//		kQTVODimensions							320 x 240
//		kQTVOResolution							??, ??
//		kQTVORefreshRate						60
//		kQTVOPixelType							soft
//		kQTVOName								SoftVOut using Codec
//		kQTVODecompressors
//			kQTVODecompressorType				'raw '
//			kQTVODecompressorContinuous			true
//			kQTVODecompressorComponent			????
pascal ComponentResult CreateDisplayModeList(SoftVoutGlobalsPtr storage)
{
	const char * const 	 mode1Name = "SoftVOut";
	const char * const   mode2Name = "SoftVOut using Codec";
	ComponentDescription cd = { decompressorComponentType, kSoftCodecType, kSoftCodecManufacturer, 0, 0L };	
	GDHandle 			 mainGD = GetMainDevice();
	
	ComponentResult		 err = noErr;

	// Create the container
	err	= QTNewAtomContainer(&storage->displayModeList);
	if (err) goto bail;
	
	// Create the 'SoftVOut' mode 1
	err	= CreateDisplayModeAtom(storage, GETPIXMAPPIXELFORMAT(*(*mainGD)->gdPMap), mode1Name);
	if (err) goto bail;

	// Create the 'SoftVOut using Codec' mode 2
	err	= CreateDisplayModeAtom(storage, storage->theCustomPixelFormat, mode2Name);
	
	// Add the kQTVODecompressors atom for mode 2
	err = AddDecompressorsAtomToDisplayMode(storage, 2, &cd, true);

bail:
	
	if (err && storage->displayModeList) {
		QTDisposeAtomContainer(storage->displayModeList);
		storage->displayModeList = NULL;
	}
	
	return err;
}

// CreateDisplayModeAtom
//		Add an atom for a given display mode to our current list of display modes.
pascal ComponentResult CreateDisplayModeAtom(SoftVoutGlobalsPtr storage, OSType pixelType, const char *name)
{
	QTAtom 	 displayModeAtom;
	long   	 dimensions[2];
	Fixed  	 resolution[2];
	Fixed  	 rate = (60 << 16);   // make up a refresh rate -> 60Hz
	OSType 	 nPixelType;	
	GDHandle mainGD = GetMainDevice();
	
	ComponentResult	 err;

	dimensions[0] = EndianU32_NtoB(storage->width);				// width
	dimensions[1] = EndianU32_NtoB(storage->height);			// height
	resolution[0] = EndianU32_NtoB((*(*mainGD)->gdPMap)->hRes);	// horizontal
	resolution[1] = EndianU32_NtoB((*(*mainGD)->gdPMap)->vRes);	// vertical
	rate		  = EndianU32_NtoB(rate);                       // refresh rate
	nPixelType	  = EndianU32_NtoB(pixelType);					// pixel format

	// Create an atom to describe our display mode...
	err = QTInsertChild(storage->displayModeList,
						kParentAtomIsContainer,
						kQTVODisplayModeItem,
						0,						// ask QTInsertChild to assign a unique ID
						0,						// insert at the beginning
						0, NULL,				// no data in this atom
						&displayModeAtom);
	if (err) goto bail;

	// ...with given dimensions...
	err = QTInsertChild(storage->displayModeList,
						displayModeAtom,
						kQTVODimensions,
						1,						// ID
						0,						// insert at the beginning
						sizeof(dimensions), dimensions,
						NULL);
	if (err) goto bail;

	// ...and given resolutions...
	err = QTInsertChild(storage->displayModeList,
						displayModeAtom,
						kQTVOResolution,
						1,						// ID
						0,						// insert at the beginning
						sizeof(resolution), resolution,
						NULL);
	if (err) goto bail;

	// ...and with a given refresh rate...
	err = QTInsertChild(storage->displayModeList,
						displayModeAtom,
						kQTVORefreshRate,
						1,						// ID
						0,						// insert at the beginning
						sizeof(rate), &rate,
						NULL);
	if (err) goto bail;

	// ...and a given pixel type...
	err = QTInsertChild(storage->displayModeList,
						displayModeAtom,
						kQTVOPixelType,
						1,						// ID
						0,						// insert at the beginning
						sizeof(nPixelType), &nPixelType,
						NULL);
	if (err) goto bail;

	// ...and a given name.
	// NOTE: Name is added without the trailing length byte. This is
	// to spec. Clients should determine the string length by examining
	// the length of the atom data.
	err = QTInsertChild(storage->displayModeList,
						displayModeAtom,
						kQTVOName,
						1,						// ID
						0,						// insert at the beginning
						strlen(name), (void *)name,
						NULL);
bail:
	return err;
}

// AddDecompressorsAtomToDisplayMode
//		Add a kQTVODecompressors atom for a given display mode item.
pascal ComponentResult AddDecompressorsAtomToDisplayMode(SoftVoutGlobalsPtr storage, UInt8 index, ComponentDescription *cd, Boolean continuous)
{
	DecompressorComponent c = 0;
	QTAtom 				  theParent = 0;
	OSType 				  theDecompressorType;
	long   				  theComponent;
	
	ComponentResult 	  err;
	
	theDecompressorType = EndianU32_NtoB(cd->componentSubType);	// decompressor type

	// Find the asked for kQTVODisplayModeItem atom
	theParent = QTFindChildByIndex(storage->displayModeList,
								   kParentAtomIsContainer,
								   kQTVODisplayModeItem,
								   index,
								   NULL);								   
	if (theParent == 0) goto bail;
									   
	// Create an atom to describe our special decompressor
	err = QTInsertChild(storage->displayModeList,
						theParent,
						kQTVODecompressors,
						0,								// ask QTInsertChild to assign a unique ID
						0,								// insert at the beginning
						0, NULL,						// no data in this atom
						&theParent);
	if (err) goto bail;
	
	// ...with a decompression type...
	err = QTInsertChild(storage->displayModeList,
						theParent,
						kQTVODecompressorType,
						1,								// ID
						0,								// insert at the beginning
						sizeof(theDecompressorType), &theDecompressorType,						
						NULL);
	if (err) goto bail;

	// ...the continuous display setting...
	err = QTInsertChild(storage->displayModeList,
						theParent,
						kQTVODecompressorContinuous,
						1,								// ID
						0,								// insert at the beginning
						sizeof(continuous), &continuous,						
						NULL);
	if (err) goto bail;
						
	c = FindNextComponent(c, cd);
	if (c) {
		// ...and a decompression component value.
		theComponent = EndianU32_NtoB((long)c);
		err = QTInsertChild(storage->displayModeList,
							theParent,
							kQTVODecompressorComponent,
							1,							// ID
							0,							// insert at the beginning
							sizeof(theComponent), &theComponent,						
							NULL);
	}
	
bail:
	return err;
}