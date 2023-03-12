/* File: Controller.m

Abstract: implementation for main window's controller.

Version: 1.0

(c) Copyright 2007 Apple, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.
*/


#import "Controller.h"




@implementation Controller


	/* tell the compiler to synthasize the property accessors
	for our instance variables. */
	
@synthesize applications, finder;



	/* called after our nib has been loaded */
- (void)awakeFromNib {
	
		/* route double-clicks on our table to the same message
		handler as clicks on the launch button. */
	[applicationsTable setDoubleAction:@selector(launchSelected:)];
	[applicationsTable setTarget:self];

		/* allocate our scripting bridge object */
	self.finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];

}



	/* we set ourself to the NSApplication's delegate in the .nib file.  Adding
	this method is a minor convenience so the sample will quit when the
	window is closed. */
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
	return YES;
}




	/* applicationDidBecomeActive: is an NSApplication delegate method
	that is called whenever our application is switched into the forground.
	We take this opportunity to re-sync our list so it is accurate
	and up to date with the current state of the applications folder.  Since we're
	using the scripting bridge, re-populating our list is very fast so it's not a
	problem to do it every time our application is switched in to the forground.   */
- (void)applicationDidBecomeActive:(NSNotification *)notification {

		/* spin the progress indicator */
	[applicationsTable setEnabled:NO];
	[progressIndicator startAnimation:self];
	
		/* create a new array. */
	NSMutableArray* currentApps = [NSMutableArray arrayWithCapacity:100];
	
		/* if the applications folder exists... */
	if ( [[[[self.finder startupDisk] folders] objectWithName:@"Applications"] exists] ) {
		
			/* first, retrieve all of the applications in the main applications folder.  */
		SBElementArray* topLevelApps = [[[[self.finder startupDisk] folders]
				objectWithName:@"Applications"] applicationFiles];
		for ( FinderApplicationFile* nthApplication in topLevelApps ) {

				/* add each application to our list of applications */
			[currentApps addObject: nthApplication];
		}
		
			/* second, search every folder inside of the applications folder for
			applications and add all of the applications we find into our list. */
			
			/* get all of the folders inside of the Application's folder on the startup disk */
		SBElementArray* appFolders =[[[[self.finder startupDisk] folders]
				objectWithName:@"Applications"] folders];
		for ( FinderFolder* nthFolder in appFolders ) {
		
				/* get all of the applications in the ith folder */
			SBElementArray* secondLevelApps = [nthFolder applicationFiles];
			for ( FinderApplicationFile* nthSecondLevelApp in secondLevelApps ) {
					
					/* add each of those applications to our list of applications */
				[currentApps addObject: nthSecondLevelApp];
			}
		}
	}
	
		/* set the new table contents */
	self.applications = currentApps;
	
		/* hide the progress indicator */
	[progressIndicator stopAnimation:self];
	[applicationsTable setEnabled:YES];
}





	/* our interface builder action method for the launch button.  Here, if
	an application is selected then we use the scripting bridge to ask the
	Finder to open the application */

- (IBAction)launchSelected:(id)sender {

		/* get the selected row from the applications table. */
	NSInteger theRow = [appTableContent selectionIndex];

	if ( theRow != NSNotFound ) {  /* if there is not no selection.... */
	
			/* retrieve the selected application from our list of applications. */
		FinderApplicationFile* selectedApplication =
			(FinderApplicationFile*) [[appTableContent arrangedObjects]
						objectAtIndex: theRow];
			
			/* ask the finder to open the application.  */
		[selectedApplication openUsing:self.finder withProperties:nil];
				
	}
}


@end




