/*
 File:		AppDelegate.m

 Description: 	This is the implementation for the main controller class.  Most of the interesting work of this sample is done here.

 Author:	MCF

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

#import "AppDelegate.h"
#include <Carbon/Carbon.h>


// This sample shows how to have many dock menu items hooked up to one action method.
// Because of bug #2751274, on Mac OS X 10.1.x the sender for this action method when called from
// a dock menu item is always the NSApplication object, not the actual menu item.  This ordinarily 
// makes it impossible to take action based on which menu item was selected, because we don't know
// which menu item called our action method. We have a workaround
// in this sample for the bug (using NSInvocations), but we set up a #define here for the AppKit
// version that is fixed (in a future release of Mac OS X) so that the code can automatically switch
// over to doing things the right way when the time comes.
#define kFixedDockMenuAppKitVersion 632

extern OSErr SetDesktopPicture(NSString *picturePath,SInt32 pIndex);

@implementation AppDelegate

// We save out as a preference the path to the folder to search for pictures in,
// and so we want to register some default settings to use in case this is the first
// time we've ever run.  +initialize will be called when the class initializes at program
// launch time, before any instances of AppDelegate are created.
+(void)initialize
{
    NSMutableDictionary *dict;
    NSUserDefaults *defaults;

    defaults=[NSUserDefaults standardUserDefaults];

    dict=[NSMutableDictionary dictionary];
    
    // We pick the Pictures folder in the user's home directory as the default
    // when DesktopPictureAppMenu is run for the first time - the pref set up here
    // will be automatically overridden by anything saved out to a pref file from previous runs
    [dict setObject:@"~/Pictures" forKey:@"picturesFolderPath"];
    [defaults registerDefaults:dict];
}

// initialize some things
- (void) awakeFromNib
{
    // setup the path to the folder to scan for pictures by checking what's been
    // saved in preferences - if this is the first run of this app, the settings
    // from +initialize (above) will be used automatically.  Note that
    // we expand "~" and follow symlinks when working with the path we saved by calling
    // -stringByResolvingSymlinksInPath.
    picturesFolderPath=[[[[NSUserDefaults standardUserDefaults] stringForKey:@"picturesFolderPath"] stringByResolvingSymlinksInPath] retain];
    
    // create the appDockMenu and build it for the first time.  We want to offload as much
    // work as possible from the -applicationDockMenu routine, because that is the most timebound
    // (the user is waiting to see their menu).  Thus, we generate the menu initially here
    // when the app first launches.
    appDockMenu=[[NSMenu alloc] initWithTitle:@"DockMenu"];
    // We want all menu items to be valid, and we can't rely on -validateMenuItem: being 
    // implemented on the target (if an NSInvocation is used, it won't be), so we turn
    // off autoenabling so that we can just enable everything by fiat.
    [appDockMenu setAutoenablesItems:NO];
    [self buildMenu];
}

// Before we quit, we save out the picturesFolderPath in preferences again so it'll be there 
// the next time
- (void)applicationWillTerminate:(NSNotification *)notification
{
    [[NSUserDefaults standardUserDefaults] setObject:picturesFolderPath forKey:@"picturesFolderPath"];
}

// Respond to the user chosing the Preferences menu item by setting up and displaying our
// preferences panel
- (IBAction)preferences:(id)sender
{
    [picturesFolderField setStringValue:picturesFolderPath];
    [prefWin makeKeyAndOrderFront:nil];
}

// This action method is called when the user clicks the "Set Folder..." button in prefs
- (IBAction)pickPictureFolder:(id)sender
{
    NSOpenPanel *op=[NSOpenPanel openPanel];

    [op setCanChooseDirectories:YES];
    [op setCanChooseFiles:NO];
    // get a sheet going to let the user pick a folder to scan for pictures
    [op beginSheetForDirectory:[picturesFolderField stringValue] file:NULL types:NULL modalForWindow:prefWin modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

// The picture folder selection sheet just went away, so let's setup the pictureFolderPath
// again
- (void)openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{
    [picturesFolderPath release];
    picturesFolderPath=[[[sheet filenames] objectAtIndex:0] retain];
    [picturesFolderField setStringValue:picturesFolderPath];
}

// This NSApplication delegate method is called when the user clicks and holds on the application
// icon in the dock.  You can also instead use a static NSMenu in your nib and wire it up to the
// NSApplication object if your dock menu doesn't change, but if you need a dynamic dock menu,
// you'll need to implement this method.  If at all possible, avoid calculating/creating the menu
// on the fly here (esp. if it'll be slow), because the user is waiting to see your menu.  
// Have your menu sitting around and just return it unless you really need just-in-time content
// to be there.
- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
    NSString *newModDate;

    // The only time we want to actually go and recreate the menu of pictures here
    // is if the folder we scan for pictures has changed (content or otherwise).  So we
    // do a quick mod date check.
    newModDate=[[[NSFileManager defaultManager] fileAttributesAtPath:picturesFolderPath traverseLink:YES] objectForKey:NSFileModificationDate];

    // If we're out of date with the folder, go rebuild the menu
    if (![newModDate isEqual:picturesFolderModDate])
        [self buildMenu];

    return appDockMenu;
}

// This is the action method that will be called whenever one of our dock menu items is selected.
// The sender will be the menu item that was picked.
- (IBAction)menuSelected:(id)sender
{
    OSErr err;

    // We go set the desktop picture to the picture whose path we stored in the menu item's 
    // representedObject field.
    err=SetDesktopPicture([sender representedObject],0);
    if (err!=noErr)
    NSLog(@"Failed to set the desktop picture!");
}


// This method is called when we actually have to go through and rebuild the dock menu.
// It can be fairly time consuming, so we try not to call it unless we really need to.
- (void)buildMenu
{
    NSDirectoryEnumerator *picturesFolderEnum;
    NSString *relativeFilePath,*fullPath;
    // grab all picture formats NSImage knows about - we'll assume that if we can read them,
    // we can set them to be the desktop picture
    NSArray *imageFormats=[NSImage imageFileTypes];
    
    // clear out old menu items first, since we're rebuilding the menu
    int numItems = [appDockMenu numberOfItems];
    while (numItems--)
    {
        // if we're running on a broken OS version, then we've been using NSInvocations
        // to get around the sender-is-NSApplication bug and we need to go release them here
        if (NSAppKitVersionNumber<kFixedDockMenuAppKitVersion)
        [[[appDockMenu itemAtIndex: 0] target] release];
        
        [appDockMenu removeItemAtIndex: 0];
    }    

    if (picturesFolderModDate)
    [picturesFolderModDate release];
    // store off the modification date of the folder, so that if the user adds new pictures to it,
    // changes the name of one of the pictures, or changes picture folders, we'll notice
    picturesFolderModDate=[[[[NSFileManager defaultManager] fileAttributesAtPath:picturesFolderPath traverseLink:YES] objectForKey:NSFileModificationDate] retain];

    // now we need to go scan the folder chosen, enumerating through to find all picture files
    picturesFolderEnum=[[NSFileManager defaultManager] enumeratorAtPath:picturesFolderPath];

    relativeFilePath=[picturesFolderEnum nextObject];
    while (relativeFilePath)
    {
        fullPath=[NSString stringWithFormat:@"%@/%@",picturesFolderPath,relativeFilePath];
        
        // If the file's extension or type matches a format that NSImage understands,
        // then we're good to go, and we add a new menu item, using the display name
        // (which may have a hidden extension) for the menu item's title and passing
        // the full path to the picture to store with the menu item
        if ([imageFormats containsObject:[relativeFilePath pathExtension]] ||
            [imageFormats containsObject:NSHFSTypeOfFile(fullPath)])        
        [appDockMenu addItem:[self constructMenuItem:[[NSFileManager defaultManager] displayNameAtPath:fullPath] action:@selector(menuSelected:) realPath:fullPath]];
        relativeFilePath=[picturesFolderEnum nextObject];
    }
}

// This method constructs a new menu item for our dock menu, using NSInvocations if we're on
// an OS version that has broken Cocoa app dock menu support.  An NSInvocation is basically
// a freeze-dried complete method call to a particular object, parameters and all, encapsulated into an object.
// Using NSInvocations allows us to set up the sender parameter to -menuSelected: to be the 
// NSMenuItem as it should be.
- (NSMenuItem *)constructMenuItem:(NSString *)title action:(SEL)aSelector realPath:(NSString *)path
{
    NSMenuItem *item;

    // This is the "correct" way to create the menu items - this code is used if we're
    // running on a fixed system
    if (NSAppKitVersionNumber>=kFixedDockMenuAppKitVersion)
    {
        item=[[[NSMenuItem alloc] initWithTitle:title action:aSelector keyEquivalent:@""] autorelease];
        [item setTarget:self];
        // store the real path to the picture in the menu item so we can fetch it later
        [item setRepresentedObject:path];
        [item setEnabled:YES];
    }
    else //we're running on an OS version that isn't fixed; use NSInvocation
    {
        //This invocation is going to be of the form aSelector
        NSInvocation *myInv=[NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:aSelector]];

        item=[[[NSMenuItem alloc] initWithTitle:title action:@selector(invoke) keyEquivalent:@""] autorelease];

        //This invocation is going to be an invocation of aSelector
        [myInv setSelector:aSelector];
        //This invocation is going to send its message to self
        [myInv setTarget:self];
        // The parameter to this invocation will be item, i.e. the NSMenuItem that should
        // be the sender to -menuSelected:  Note that we pass it at index 2 - indices 0 and 1
        // are taken by the hidden arguments self and _cmd, accessible through -setTarget:
        // and -setSelector:
        [myInv setArgument:&item atIndex:2];
        // here we retain the invocation so it won't go away.  Later on we'll have to
        // release it to avoid leaking
        [item setTarget:[myInv retain]];
        // store the real path to the picture in the menu item so we can fetch it later
        [item setRepresentedObject:path];
        [item setEnabled:YES];
    }

    return item;
}

@end

