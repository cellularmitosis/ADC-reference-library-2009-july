/*
 File:		DesktopPicture.m

 Description: 	This file contains the SetDesktopPicture function, which tells the Finder to set a given picture as the desktop picture.

 Author:		MCF

 Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.

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

 Version History:
 
 1.0 - 03/2002 Initial Release

 */


#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>


// Setting the desktop picture involves a lot of Apple Event and Carbon code.
// It's all self-contained in this one function.  This Apple Event isn't guarrenteed to
// work forever, and real APIs to set the desktop picture should be coming down the road,
// but in the meantime this is how you do it.
OSErr SetDesktopPicture(NSString *picturePath,SInt32 pIndex)
{
    AEDesc tAEDesc = {typeNull, nil};	//	always init AEDescs
    OSErr		anErr = noErr;
    AliasHandle		aliasHandle=nil;
    FSRef		pictRef;
    OSStatus		status;

// Someday pIndex will hopefully determine on which monitor to set the desktop picture.
// This doesn't work in Mac OS X right now, so we don't do anything with the parameter.
#pragma unused (pIndex)

    // Let's make an FSRef from the NSString picture path that was passed in
    status=FSPathMakeRef([picturePath fileSystemRepresentation],&pictRef,NULL);

    // Now we create an alias to the picture from that FSRef
    if (status==noErr)
    anErr = FSNewAlias( nil, &pictRef, &aliasHandle);
    
    if ( noErr == anErr  &&  aliasHandle == nil )
        anErr = paramErr;
    
    // Now we create an AEDesc containing the alias to the picture
    if ( noErr == anErr )
    {
        char	handleState = HGetState( (Handle) aliasHandle );
        HLock( (Handle) aliasHandle);
        anErr = AECreateDesc( typeAlias, *aliasHandle, GetHandleSize((Handle) aliasHandle), &tAEDesc);
        HSetState( (Handle) aliasHandle, handleState );
        DisposeHandle( (Handle)aliasHandle );
    }
    if (noErr == anErr)
    {
        // Now we need to build the actual Apple Event that we're going to send the Finder
        AppleEvent tAppleEvent;
        OSType sig = 'MACS'; // The app signature for the Finder
        AEBuildError tAEBuildError;
        anErr = AEBuildAppleEvent(kAECoreSuite,kAESetData,typeApplSignature,&sig,sizeof(OSType),
                                  kAutoGenerateReturnID,kAnyTransactionID,&tAppleEvent,&tAEBuildError,
                                  "'----':'obj '{want:type(prop),form:prop,seld:type('dpic'),from:'null'()},data:(@)",&tAEDesc);
                                  
        // Finally we can go ahead and send the Apple Event using AESend                          
        if (noErr == anErr)
        {
            AppleEvent    theReply = {typeNull, nil};
            anErr = AESend(&tAppleEvent,&theReply,kAENoReply,kAENormalPriority,kNoTimeOut,nil,nil);
            AEDisposeDesc(&tAppleEvent);    // Always dispose ASAP
        }
    }
    return anErr;
}