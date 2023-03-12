/*
	File:		ColorSyncUtils.h
	
	Description: Interface file for ColorSyncUtils.m class

	Author:		Apple Developer Technical Support
	
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
	   <1>	 	11/2/03 	SRK		first file
*/

#import <Foundation/Foundation.h>
#import "ConcatProfileSet.h"
#import "Utils.h"
#import <QuickTime/QuickTime.h>


@interface ColorSyncUtils : NSObject {

}

+(CMError)embedProfileInImageFile:(FSSpec *)imageFile profileFile:(FSSpec *)profileFile;
+(CMError)closeProfileRef:(CMProfileRef)ref;
+(CMError)openFileBasedProfile:(FSSpec *)profileFSSpecPtr theProfileRef:(CMProfileRef *)aProfileRef;
+(CMError)openHandleBasedProfile:(Handle)profileHandle theProfile:(CMProfileRef *)aProfileRef;
+(NSString *)profileNameFromProfileRef:(CMProfileRef) profileRef;
+(NSString *)profileNameString:(Handle)profileHandle;
+(CMError)userGetNewProfile:(CMProfileRef *)aProfileRef;
+(CFStringRef)copyProfileDescriptionCFString:(CMProfileRef) prof;
+(OSErr)graphicsImporterForImageFileFromPath:(NSString *)imageFilePath graphicsImporter:(ComponentInstance *)anImporterRef;
+(CMError)embeddedProfileForImageFileFromPath:(NSString *)imageFilePath profileReference:(CMProfileRef *)aProfileRef;
+(CMError)defaultRGBProfile:(CMProfileRef *)aProfileRef;
+(CMError)embedDefaultRGBProfileIntoImageFile:(NSString *)imageFilePath;
+(CMError)colorMatchSrcOffscreenToDestOffscreen:(GWorldPtr)srcGWorld destGWorld:(GWorldPtr)aGWorld colorWorld:(CMWorldRef)aColorWorldRef;
+(CMError)PixMapToCMBitMap:(PixMapHandle)pm bm:(CMBitmap*)bm;
+(CMError)profileByUse:(OSType)use profileRefPtr:(CMProfileRef *)aProfileRef;
+(CMError)profileBySpace:(OSType)use profileRefPtr:(CMProfileRef *)aProfileRef;
+(NSString *)profilePathFromProfileRef:(CMProfileRef) profileRef;
+(CMError)openProfileFromNSStringPath:(NSString *)aProfilePath theProfile:(CMProfileRef *)aProfileRef;
+(Boolean)isPantherQTColorMatchingTurnedOnForComponent:(GraphicsImportComponent)aComponent;
+(CMError)setProfileByUseFromPath:(OSType)use theProfile:(NSString *)aProfilePath profileRef:(CMProfileRef *)aProfileRef;
+(CMError)setProfileBySpaceFromPath:(OSType)use theProfile:(NSString *)aProfilePath profileRef:(CMProfileRef *)aProfileRef;
+(OSErr)setPantherQTColorMatchingForComponent:(GraphicsImportComponent)aComponent;
+(OSErr)setPantherQTColorMatchingOffForComponent:(GraphicsImportComponent)aComponent;

@end
