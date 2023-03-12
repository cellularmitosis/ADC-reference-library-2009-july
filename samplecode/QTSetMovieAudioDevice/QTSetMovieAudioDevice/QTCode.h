/*
*  File:    QTCode.h
* 
*  Contains:  Interface file for QTCode.cpp
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
*        to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

#ifndef STRICT
	#define STRICT
#endif

// If app hasn't choosen, set to work with Windows 98, Windows Me, Windows 2000, Windows XP and beyond
#ifndef WINVER
	#define WINVER         0x0410
#endif
#ifndef _WIN32_WINDOWS
	#define _WIN32_WINDOWS 0x0410 
#endif
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT   0x0500 
#endif

#include <windows.h>
#include <tchar.h>
#include <Movies.h>
#include <QTML.h>


OSStatus QT_CreateMovieWithProperties(CFStringRef inNativePath, QTAudioContextRef inAudioContext, Movie *outMovie);
OSStatus QT_CreateAudioContextFromDeviceName(TCHAR *inDeviceName,
											 QTAudioContextRef *outAudioContextRef);
OSStatus QT_CreateAudioContextFromGUID(GUID *inGUID, QTAudioContextRef *outAudioContextRef);
boolean	QT_QT704OrBetterInstalled();