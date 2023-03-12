//////////
//
//	File:		MyObject.m
//
//	Contains:	Implementation file for the MyObject class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	5/20/02	srk		first file
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

#import <QuickTime/QuickTime.h>
#import <Carbon/Carbon.h>

#import "MyObject.h"
#import "MyQuickDrawView.h"


NSTimer *theTimer;

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
    /* we'll want to be called when the application
     quits so we can do any cleanup */
     
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(applicationWillTerminate:)
        name:@"NSApplicationWillTerminateNotification" object:NSApp];

    return self;
}

//////////
//
// awakeFromNib
//
// Called after all our objects are unarchived and
// connected but just before the interface is made visible
// to the user. We will do custom initialization of our
// objects here.
//
//////////

- (void)awakeFromNib
{
    NSDictionary 	*dict;
    NSString 		*windowTitle;
    OSErr 			err;

    /* grab the name string for our window */
    NSString *filePath	= [[NSBundle mainBundle] pathForResource:@"InfoPlist" ofType:@"strings"];
    dict 				= [NSDictionary dictionaryWithContentsOfFile:filePath];
    windowTitle 		= [dict objectForKey:@"MiniMungWindowTitleString"];

    /* set the window title */
	[window setTitle:windowTitle];
    
    /* pass our view object to our MyQuickDrawView class so we can access methods
        in this class from our C code routines */
    saveQDViewObjectForCallback(view);

    /* now lets create a window and display the video data passed to us by the sequence grabber */
    err = [view doSeqGrab:[view bounds]];
    
    /* put up an error dialog to display any errors */
    if (err != noErr)
    {
        NSString *errorStr = [[NSString alloc] initWithFormat:@"%d" , err];
        int choice;

        /* now display error dialog and quit */
        choice = NSRunAlertPanel(@"Error", errorStr, @"OK", nil, nil);
        [errorStr release];
    }
    
}


//////////
//
// applicationWillTerminate
//
// We'll release any objects we initialized
// in our init method.
//
//////////

- (void)applicationWillTerminate:(NSNotification *)notification
{
    [view endGrab];    
}


@end

