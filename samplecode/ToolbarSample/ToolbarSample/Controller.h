/*
File:		Controller.h

Description: 	This is the header file for the Controller class, which implements the object used to
		control/initialize this application and as the NSToolbar delegate.

Author:		MCF

Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.

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

05/2001 - MCF - initial version

*/

#import <Cocoa/Cocoa.h>

@interface Controller : NSObject
{
    IBOutlet id theWindow;
    IBOutlet NSView *popUpView; //the font style changing view (ends up in an NSToolbarItem)
    IBOutlet NSView *fontSizeView; //the font size changing view (ends up in an NSToolbarItem)
    IBOutlet id fontSizeStepper;
    IBOutlet id fontSizeField;
    IBOutlet NSMenu *fontStyleMenu; //this menu and the fontSizeMenu are the menuFormRepresentations of the toolbar items
    IBOutlet NSMenu *fontSizeMenu;
    IBOutlet NSTextView * contentView; //The NSTextView that holds all our text
    NSMutableDictionary *toolbarItems; //The dictionary that holds all our "master" copies of the NSToolbarItems
    int fontStylePicked; //a state variable that keeps track of what style has been picked (plain, bold, italic)
}

//Required NSToolbar delegate methods
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag;    
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar;

//actions to handle the various toolbar items and what happens when you click the popups, etc.
-(IBAction) changeFontSize:(id)sender;
-(IBAction) changeFontStyle:(id)sender;
-(IBAction) blueText:(id)sender;
-(IBAction) fontSizeBigger:(id)sender;
-(IBAction) fontSizeSmaller:(id)sender;

@end
