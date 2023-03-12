/*

File: AtomBrowserController.m

Author: QuickTime DTS, some code originally from QTComponents

Change History (most recent first): <1> 10/20/04 initial release

© Copyright 2001-2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import "AtomBrowserController.h"
#import "AtomContainer.h"

@implementation AtomBrowserController

+ (AtomBrowserController *)withQTAtomContainer:(QTAtomContainer)inContainer
{
    AtomBrowserController *control;
    AtomContainer *theContainer;
    BOOL loaded;
    
    control = [AtomBrowserController new];
    if (nil == control) return nil;
    
    theContainer = [AtomContainer withQTAtomContainer:inContainer];
    if (nil == theContainer) return nil;
    
    control->container = [theContainer retain];
    
    loaded = [NSBundle loadNibNamed: @"AtomBrowser" owner:control];
    if (!loaded) return nil;
    
    return control;
}

- (id)window
{
    return atomListWindow;
}

- (void)windowWillClose:(NSNotification *)aNotification
{
    [container release];
    [self release];
}

- (void)setWindowTitle:(NSString *)inString
{
    [atomListWindow setTitle:inString];
    // bring the atom window to the front if you want, I don't bother
    //[atomListWindow makeKeyAndOrderFront:self];
}

#pragma mark **** Data Source ****
// implement the outline view, uncomment the NSLog lines to see what's going on
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
    //NSLog(@"outlineView:(NSOutlineView *)outlineView child:%d ofItem:%@", index, [item description]);

    if (item == nil) item = container;

    return [item objectAtIndex:index];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    //NSLog(@"outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item:%@", [item description]);

    return [item count] > 0;
}

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    //NSLog(@"outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:%@", [item description]);
    
    if(item == nil) item = container;
    
    return [item count];
}

- (id)outlineView:(NSOutlineView *)outlineView 	objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    NSString *key = [tableColumn identifier];
    
    //NSLog(@"outlineView:(NSOutlineView *)outlineView 	objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item");
    
    return [item valueForKey:key];

}

@end
