/*
	File:		MyDocumentController.m
	
	Description: Implementation file for the MyDocumentController.m class. Contains code
				 to validate menu items and check for Image Capture devices.

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

#import "MyDocumentController.h"

//////////
//
// ImageWasDownloaded:
// called when an image is downloaded from an Image Capture device.
// This routine is called by code in our SimpleDownloadFile.c
// file.
//
//////////

void ImageWasDownloaded(UInt32 refcon, char* path)
{
    NSDocumentController *docController = [NSDocumentController sharedDocumentController];
    [docController openDocumentWithContentsOfFile:[NSString stringWithCString:path] display:YES];
}

//////////
//
// NoCameraDevicesFound:
// error dialog displayed when there are no Image Capture devices connected, and the user
// user tries to download an image.
//
//////////

void NoCameraDevicesFound(UInt32 refcon)
{
    NSRunAlertPanel(@"Error", @"No Image Capture Devices found. Please connect a device and try again.", @"OK", nil, nil);
}

@implementation MyDocumentController


//////////
//
// sharedImageProfiles:
// returns the object instance.
//
//////////

- (IBAction)openDigitalCameraImage:(id)sender
{
// this will cause our function ImageWasDownloaded above to get called to kick off things...
    SimpleDownloadFirstImage((UInt32)self);
}

//////////
//
// validateMenuItem:
// Set the menus based on the current document state.
//
//////////

- (BOOL)validateMenuItem:(NSMenuItem *)item 
{
	BOOL isValid = NO;
    SEL action = [item action];
    
    if (action == @selector(openDocument:)) 
    {
        isValid = YES;
    }
    // always enable Save, Save As, and Select None
    else if (action == @selector(saveDocument:)) 
    {
        isValid = YES;
    }
    else if (action == @selector(saveDocumentAs:)) 
    {
        isValid = NO;
    }
    else if (action == @selector(selectNone:)) 
    {
        isValid = YES;
    }
    // enable Revert if the document has changed
    else if (action == @selector(revertDocumentToSaved:)) 
    {
        isValid = NO;
    }
    else if (action == @selector(openDigitalCameraImage:)) 
    {
        isValid = YES;
    }

    return isValid;
}

@end
