//////////
//
//	File:		MyObject.m
//
//	Contains:	Implementation file for the MyObject class.
//
//  This is a controller class which drives our custom MyQuickDrawView class as follows:
//  
//  1) prompts the user for a movie file to display
//  2) when the "Next Frame" button is pressed, calls the MyQuickDrawView class to handle
//     advancing and displaying the next video frame in the window
//  3) handles application clean-up when the application is terminated
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/18/02	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "MyObject.h"
#import "MyQuickDrawView.h"

@implementation MyObject


//////////
//
// init
//
// Our controller's initialization method. Well
// add our self as an observer for application
// termination notifications, so we can perform
// cleanup when the application quits.
//
//////////

- (id)init
{

    movieFilePath = [self promptUserForMovieFile];
    if (movieFilePath != NULL)
    {
        [movieFilePath retain];
        
        /* we'll want to be called when the application
        quits so we can do any cleanup */
        
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(applicationWillTerminate:)
            name:@"NSApplicationWillTerminateNotification" object:NSApp];
    }
    else
    {
        [NSApp terminate:self];
    }

    return self;
}

//////////
//
// applicationWillTerminate
//
// We'll release any objects we initialized earlier
//
//////////

- (void)applicationWillTerminate:(NSNotification *)notification
{
    [movieFilePath release];
    [myView freeAllocatedObjects];
}

//////////
//
// nextFrame
//
// Called when the user presses the
// "Next Frame" button to advance the
// movie to the next frame.
//
//////////

-(IBAction)nextFrame:(id)sender
{
    /* advance to next movie frame */
    [myView nextFrame];
    /* force redisplay of view - this will call our drawRect routine */
    [myView setNeedsDisplay:YES];
}

//////////
//
// promptUserForMovieFile
//
// Ask the user for a movie file to open
//
//////////

-(NSString *)promptUserForMovieFile
{
    NSOpenPanel *opanel 		= nil;
    NSArray 	*fileExtensions = nil;
    NSString 	*fileExtension 	= @"mov", *fileExtension2 = @"mp4", *fileExtension3 = @"dv";
    int 		result;
	
    fileExtensions = [NSArray arrayWithObjects:fileExtension, fileExtension2, fileExtension3, nil];
    NSAssert(fileExtensions != nil, @"arrayWithObjects failed");

    opanel = [NSOpenPanel openPanel];
    NSAssert(opanel != nil, @"openPanel failed");
    result = [opanel runModalForDirectory:nil file:nil types:fileExtensions];
    if (result == NSOKButton)
    {
        return [opanel filename];            
    }
    else
    {
        return nil;
    }
}

//////////
//
// movieFilePath
//
// Getter for the movie path string object
//
//////////

-(NSString *)movieFilePath
{
    return (movieFilePath);
}

@end
