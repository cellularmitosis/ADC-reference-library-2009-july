/*

File: psWindow.h

Abstract: Sample Code that Pulls in data from iPhoto and populates
albums the UI and also pulls in the data up in APIs.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import <Cocoa/Cocoa.h>

@interface psWindow : NSWindow
{
    IBOutlet NSTextField	*gSlideshowTitleField;			//name of FCP sequence and default name for XML file
    IBOutlet NSTextField	*gClipDurationField;			//duration of stills as clips
    IBOutlet NSTextField	*gTransitionDurationField;		//duration of transitions (will overlap with the clip duration)
    IBOutlet NSPopUpButton	*gTransitionSelectionField;		//lets user select which transition to use
    IBOutlet NSPopUpButton	*gFramerateSelectionField;		//choose your framerate for the project!
	IBOutlet NSButton		*gCreateNewProjectButton;		//if user selects, we create a project at import time.
															// Otherwise, we import into the top-most project
	int						gIndexOfRandom;					//stores off where I've put "random" in the transition popup.

    IBOutlet NSPopUpButton	*gAlbumPopup;					//lets user select which iphoto album to pull data from

	int						gSelectedAlbumIndex;			//index of the selected album
	NSArray					*gKeysForAlbum;					//the keys in the iphoto album file for the photos
	NSArray					*gAlbumsArray;					//array of all the albums in iphoto
	NSDictionary			*gMasterImageDictList;			//listing of all photos in iphoto
	NSDictionary			*gPlist;						//pointer to the iPhoto plist

}
/* awakeFromNib
	When we first open the nib with a psWindow, let's set up some default values... 
*/
-(void)awakeFromNib;
//-------------------------------- timebase functions ---------------------------------
/* getTimebase
	go check out the framerate popup and return a string value for the XML to use. This
	includes converting 29.97 to 30. NOTE: We will aslo return NTSCRate as "TRUE" for
	this case.
*/
-(NSString*)getTimebase;

/* getNTSCRate
	convert UI framerate output to what the NTSCRate should be.
	note... if you actually want 30FPS in addition to 29.97 you'll
	need to add a little more smarts to this so you can put both
	30, non-ntsc and 30, ntsc rate into the XML.
*/
-(NSString*)getNTSCRate;

/* getSeqPreset
	convert UI framerate output to what default sequence preset to use.
*/
-(NSString*)getSeqPreset;

//-------------------------------- durations functions ---------------------------------
/* getClipDuration
	convert the UI clip duration in seconds to some number
	of frames. Note that for 29.97 we will slowly drift out of 
	sync with timecode since we're using 30 for this calculation.
*/
-(int)getClipDuration;

/* getMediaDuration
	convert the UI clip duration in seconds to some number
	of frames. We want to setup the media duration to be long
	enough to cover the transitions on both before and after this clip
*/
-(NSString*)getMediaDuration;

/* getMediaIn
	pOut = the corresponding media out time to the returned	media in time.
	
	note that the media in time is offset from 0 to leave room for the transition
	to extend into.
*/
-(NSString*)getMediaIn:(int*)pOut;

/* getTransitionDuration
	pIntValue = the corresponding transition duration in int format for
	use in calculations of start, and end as well as total media
	
	convert the UI transition duration in seconds to some number
	of frames. Note that for 29.97 we will slowly drift out of 
	sync with timecode since we're using 30 for this calculation.
*/
-(NSString*)getTransitionDuration: (int*)dValue;

//-------------------------------- output settings functions ---------------------------------
/* getTransitionName
	effectCategory = filled in with the effect category of the return transition.

	returns the name of the selected transition or if "(random)" is selected,
	a random transition from the popup.
*/

-(NSString*)getTransitionName:(NSString**)effectCategory;

/* getFilesCount
	returns a count of the number of files in the selected iPhoto Album
*/
-(int)getFilesCount;

/* getClipFileURL
	index = the index of the file (in the file array) to get the URL and/or name for.
	andName = the name of the file at index.
	
	returns a the pathURL string of the file at index in the selected iPhoto Album
*/
-(NSString*)getClipFileURL:(int)index andName:(NSString**)clipName;

/* getSlideShowTitle
	returns the slide show title from UI.
*/
-(NSString*)getSlideShowTitle;

/* shouldCreateNewProject
	returns the state of UI checkbox
*/
-(BOOL)shouldCreateNewProject;

//-------------------------------- update UI ---------------------------------
/* setSelectedAlbum
	sender (unused)
	
	stores off the array of photo keys from the dictionary and 
	updates the UI default output name to match the selected album name
*/
- (IBAction)setSelectedAlbum:(id)sender;


/* refreshIPhotoData
	sender (unused)
	
	refresh the iPhotoData so that it is up to date
*/
- (IBAction)refreshIPhotoData:(id)sender;

/* setAlbumNames
	albumNames = array iPhoto album names

	update the album selection popup in the UI with those in albumNames
*/
-(void)setAlbumNames:(NSArray*)albumNames;

//-------------------------------- application delegate------------------------------

/* applicationWillBecomeActive
	aNotification (unused)
	
	Whenever the application becomes active, refresh the iPhotoData so that it is always up to date.
*/
- (void)applicationWillBecomeActive:(NSNotification *)aNotification;

@end
