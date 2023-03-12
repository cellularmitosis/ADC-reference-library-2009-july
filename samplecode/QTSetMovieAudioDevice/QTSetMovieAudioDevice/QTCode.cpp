/*
*  File:    QTCode.cpp
* 
*  Contains:  QuickTime utility routines for opening movies and setting a movie audio
*             context.
*  
*  Version:  1.0
* 
*  Created:  2/15/06
*
*  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*        ("Apple") in consideration of your agreement to the following terms, and your
*        use, installation, modification or redistribution of this Apple software
*        constitutes acceptance of these terms.  If you do not agree with these terms,
*        please do not use, install, modify or redistribute this Apple software.
*
*        In consideration of your agreement to abide by the following terms, and subject
*        to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
*        copyrights in this original Apple software (the "Apple Software"), to use,
*        reproduce, modify and redistribute the Apple Software, with or without
*        modifications, in source and/or binary forms; provided that if you redistribute
*        the Apple Software in its entirety and without modifications, you must retain
*        this notice and the following text and disclaimers in all such redistributions of
*        the Apple Software.  Neither the name, trademarks, service marks or logos of
*        Apple Computer, Inc. may be used to endorse or promote products derived from the
*        Apple Software without specific prior written permission from Apple.  Except as
*        expressly stated in this notice, no other rights or licenses, express or implied,
*        are granted by Apple herein, including but not limited to any patent rights that
*        may be infringed by your derivative works or by other works in which the Apple
*        Software may be incorporated.
*
*        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*        COMBINATION WITH YOUR PRODUCTS.
*
*        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*  Copyright:  Copyright © 2006 Apple Computer, Inc, All Rights Reserved
*/


#include "QTCode.h"

//-----------------------------------------------------------------------------
// Name: QT_CreateMovieWithProperties
// Desc: Creates a movie using the NewMovieFromProperties function and the
//       specified movie file path and audio context (if any).
//-----------------------------------------------------------------------------

OSStatus QT_CreateMovieWithProperties(CFStringRef inNativePath, QTAudioContextRef inAudioContext, Movie *outMovie)
{
	// must pass a valid path
	if (!inNativePath) return -1;

    // Define a property array for NewMovieFromProperties

	QTNewMoviePropertyElement movieProps[5] = {0};
	ItemCount moviePropCount = 0;

    // Store the movie properties in the array

    movieProps[moviePropCount].propClass = kQTPropertyClass_DataLocation;
    movieProps[moviePropCount].propID = kQTDataLocationPropertyID_CFStringWindowsPath; 
    movieProps[moviePropCount].propValueSize = sizeof(inNativePath);
    movieProps[moviePropCount].propValueAddress = (void*)&inNativePath;
    movieProps[moviePropCount].propStatus = 0;

	moviePropCount++;

	bool boolTrue = true;
    movieProps[moviePropCount].propClass = kQTPropertyClass_MovieInstantiation;
    movieProps[moviePropCount].propID = kQTMovieInstantiationPropertyID_DontAskUnresolvedDataRefs;
    movieProps[moviePropCount].propValueSize = sizeof(boolTrue);
    movieProps[moviePropCount].propValueAddress = &boolTrue;
    movieProps[moviePropCount].propStatus = 0; 

	moviePropCount++;

    movieProps[moviePropCount].propClass = kQTPropertyClass_NewMovieProperty; 
    movieProps[moviePropCount].propID = kQTNewMoviePropertyID_Active;
    movieProps[moviePropCount].propValueSize = sizeof(boolTrue);
    movieProps[moviePropCount].propValueAddress = &boolTrue;
    movieProps[moviePropCount].propStatus = 0;

	moviePropCount++;

    movieProps[moviePropCount].propClass = kQTPropertyClass_NewMovieProperty;
    movieProps[moviePropCount].propID = kQTNewMoviePropertyID_DontInteractWithUser;
    movieProps[moviePropCount].propValueSize = sizeof(boolTrue);
    movieProps[moviePropCount].propValueAddress = &boolTrue;
    movieProps[moviePropCount].propStatus = 0;

	moviePropCount++;

	if (inAudioContext)
	{
		movieProps[moviePropCount].propClass = kQTPropertyClass_Context;
		movieProps[moviePropCount].propID = kQTContextPropertyID_AudioContext;
		movieProps[moviePropCount].propValueSize = sizeof(inAudioContext);
		movieProps[moviePropCount].propValueAddress = &inAudioContext;
		movieProps[moviePropCount].propStatus = 0;

		moviePropCount++;
	}

	return(NewMovieFromProperties(moviePropCount, movieProps, 0, NULL, outMovie));
}

//-----------------------------------------------------------------------------
// Name: QT_CreateAudioContextFromDeviceName()
// Desc: Create a QuickTime Audio Context from a Windows device name
//-----------------------------------------------------------------------------

OSStatus QT_CreateAudioContextFromDeviceName(TCHAR *inDeviceName,
											 QTAudioContextRef *outAudioContextRef)
{
	OSStatus status = -1;

	*outAudioContextRef = NULL;

	CFIndex deviceNameSize = _tcslen(inDeviceName);
	require (deviceNameSize != 0, bail);

	CFStringRef deviceNameStrRef = NULL;
	
	// Create CFString representation of device name
	deviceNameStrRef = CFStringCreateWithCharacters(kCFAllocatorDefault, 
													inDeviceName, 
													deviceNameSize);
	require(deviceNameStrRef != NULL, bail);

	// Create the QT Audio Context for the device
	status = QTAudioContextCreateForAudioDevice(
												NULL,
												deviceNameStrRef,
												NULL,
												outAudioContextRef);
bail:
	if (deviceNameStrRef)
	{
		CFRelease(deviceNameStrRef);
	}
	return status;
}

//-----------------------------------------------------------------------------
// Name: QT_CreateAudioContextFromGUID()
// Desc: Create a QuickTime Audio Context from a Windows GUID
//-----------------------------------------------------------------------------

OSStatus QT_CreateAudioContextFromGUID(GUID *inGUID, QTAudioContextRef *outAudioContextRef)
{
	OSStatus status = -1;

	*outAudioContextRef = NULL;

	CFStringRef guidStrRef = NULL;

	if (inGUID)
	{
		TCHAR strGuid[39];
		int numBytes = StringFromGUID2(*inGUID, strGuid, 39);
		require(numBytes > 0, bail);

		// Create a CFString representation of the GUID

		// NOTE: you should pass (numBytes - 1) for the numChars parameter
		// to CFStringCreateWithCharacters since StringFromGUID2 returns 
		// the number of characters *plus* the terminating null character.

		guidStrRef = CFStringCreateWithCharacters(kCFAllocatorDefault, 
													strGuid, 
													(CFIndex)(numBytes - 1));
		require(guidStrRef != NULL, bail);
	}
	else
	{
		// A NULL GUID means use the default device

		guidStrRef = NULL;
	}

	// Create a QT Audio Context from the GUID for the device
	status = QTAudioContextCreateForAudioDevice(
												NULL,
												guidStrRef,
												NULL,
												outAudioContextRef);
bail:
	if (guidStrRef)
	{
		CFRelease(guidStrRef);
	}
	return status;
}

//-----------------------------------------------------------------------------
// Name: QT_QT704OrBetterInstalled()
// Desc: Determines if QuickTime Version 7.0.4 or better is installed
//-----------------------------------------------------------------------------

boolean	QT_QT704OrBetterInstalled()
{
	long version;
	OSErr result;

	result = Gestalt(gestaltQuickTime, &version);
	if ((result == noErr) && (version >= 0x07048000))
	{
		return true;
	}

	return false;
}
