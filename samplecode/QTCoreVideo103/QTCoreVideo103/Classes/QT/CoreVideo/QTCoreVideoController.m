//---------------------------------------------------------------------------
//
//	File: QTCoreVideoController.m
//
//  Abstract: Controller Class
// 			 
//  Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
//  Computer, Inc. ("Apple") in consideration of your agreement to the
//  following terms, and your use, installation, modification or
//  redistribution of this Apple software constitutes acceptance of these
//  terms.  If you do not agree with these terms, please do not use,
//  install, modify or redistribute this Apple software.
//  
//  In consideration of your agreement to abide by the following terms, and
//  subject to these terms, Apple grants you a personal, non-exclusive
//  license, under Apple's copyrights in this original Apple software (the
//  "Apple Software"), to use, reproduce, modify and redistribute the Apple
//  Software, with or without modifications, in source and/or binary forms;
//  provided that if you redistribute the Apple Software in its entirety and
//  without modifications, you must retain this notice and the following
//  text and disclaimers in all such redistributions of the Apple Software. 
//  Neither the name, trademarks, service marks or logos of Apple Computer,
//  Inc. may be used to endorse or promote products derived from the Apple
//  Software without specific prior written permission from Apple.  Except
//  as expressly stated in this notice, no other rights or licenses, express
//  or implied, are granted by Apple herein, including but not limited to
//  any patent rights that may be infringed by your derivative works or by
//  other works in which the Apple Software may be incorporated.
//  
//  The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//  
//  IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//  MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
//  STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// 
//  Copyright (c) 2008 Apple Inc., All rights reserved.
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#import "QTCoreVideoController.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

@implementation QTCoreVideoController

//---------------------------------------------------------------------------

- (void) openPanelDidEnd:(NSOpenPanel*)sheet 
			returnCode:(int)returnCode 
			contextInfo:(void*)contextInfo
{
	if ( returnCode ) 
	{
		[qtCVOpenGLView openMovie:[[sheet filenames] objectAtIndex:0]];
    } // if
	
    // Activate the display link
	
	CVDisplayLinkStart( [qtCVOpenGLView displayLink] );
} // openPanelDidEnd

//---------------------------------------------------------------------------

#pragma mark Actions

//---------------------------------------------------------------------------

- (IBAction) open:(id)sender
{
	// if the display link is active, stop it
	
	CVDisplayLinkRef  displayLinkRef = [qtCVOpenGLView displayLink];
	
	if ( CVDisplayLinkIsRunning( displayLinkRef ) ) 
	{
    	CVDisplayLinkStop( displayLinkRef );
    } // if
	
	[[NSOpenPanel openPanel] beginSheetForDirectory:nil 
												file:nil 
												types:[QTMovie 
												movieUnfilteredFileTypes] 
												modalForWindow:[qtCVOpenGLView window] 
												modalDelegate:self 
												didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) 
												contextInfo:nil];
} // open

//---------------------------------------------------------------------------
//
// This method is called when the user picks a different effect to 
// receive messages using the pop-up menu
//
//---------------------------------------------------------------------------

- (IBAction) switchGeometry:(id)sender
{
	// sender is the NSPopUpMenu containing geometry choices.
	// We ask the sender which popup menu item is selected and 
	// add one to compensate for counting from zero.
	
	int geometrySelected = [sender indexOfSelectedItem] + 1;

	if ( geometrySelected == kGeometryQuad )
	{
		[qtCVOpenGLView setGeometry:kGeometryQuad];
	} // if
	else
	{
		[qtCVOpenGLView setGeometry:kGeometryTeapot];
	} // else
} // switchEffects

//---------------------------------------------------------------------------

#pragma mark Delagates

//---------------------------------------------------------------------------

- (void) applicationDidFinishLaunching:(NSNotification *)notification
{
	[self open:self];
} // applicationDidFinishLaunching

//---------------------------------------------------------------------------
//
// It's important to clean up our rendering objects before we terminate -- 
// cocoa will not specifically release everything on application termination, 
// so we explicitly call our clean up routine ourselves
//
//---------------------------------------------------------------------------

- (void) applicationWillTerminate:(NSNotification *)notification
{
	[qtCVOpenGLView cleanUp];
} // applicationWillTerminate

//---------------------------------------------------------------------------

@end

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
