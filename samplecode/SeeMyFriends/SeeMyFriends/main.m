/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.
 */

/*  
	main.m
	SeeMyFriends
	
	Version: 1.1

	Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
	
	SeeMyFriend is a very simple example that shows how to integrate SyncServices into a Carbon based application.
	
	It displays all user's Contact data in a very simple way ( first name, last name, picture if present) in a custom 
	HIView, which is updated when the contacts are changed by another sync client. SeeMyFriend itself does not allow 
	modification of data, and is a read only sync client. Therefore it does not have an explicit sync button and will be 
	triggered only when other clients syncing the same kind of data will synchronize. It triggers a sync only at launch to
	update its data. A way to test SeeMyFriends is to change some data (like a frist name or lats name attribute) in 
	Address Book and wait for the change to be synchronized. It will auomatically appear in SeeMyFriends.
	
	SeeMyFriends sync data are archived between two launches of the app in a subfolder of ~/Library/Application Support.
	The archiving is done using the HIArchive mechanism.

	SeeMyFriend is using Model-View-Controller paradigm in a Carbon application. A controller object (SMFWindowController) 
	is created and is managing the content of the main window, as well as interaction with SyncServices.
	
*/


#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>
#import <SyncServices/SyncServices.h>

#import "SMFWindowController.h"

SMFWindowController *gMainWindowController;

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
	NSAutoreleasePool	*pool;
    OSStatus		err;
	
	//Create an autorelease pool for Foundation objects. Note that this is not the most optimized way 
	//to manage memory as objects in the pool won't be released before this one is destroyed. Use of local
	//pools is certainly more cleaver.
	pool = [[NSAutoreleasePool alloc] init];
	
	//Register our custom HIView to display the Address Book content
	err = SMFPeopleViewRegister();
	require_noerr( err, EXIT );
	
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, EXIT );
    
    // Once the nib reference is created, set the menu bar. 
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, EXIT);
    DisposeNibReference(nibRef);
	
	//Create the controller for the main window. See SMFWindowController.h/SMFWindowController.m
	gMainWindowController = [[SMFWindowController alloc] init];
	[gMainWindowController showWindow];
    
    // Call the event loop
    RunApplicationEventLoop();

EXIT:
	//release everything
	[gMainWindowController release];
	[pool release];
	return err;
}

