/*

File: psWindow.m

Abstract: Pull in data from the UI and from the source folders to pass back to
the psController.

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

*/ #import "psWindow.h"

@implementation psWindow

//-------------------------------- application delegate------------------------------
/* applicationWillBecomeActive
	aNotification (unused)
	
	Whenever the application becomes active, refresh the iPhotoData so that it is always up to date.
*/
- (void)applicationWillBecomeActive:(NSNotification *)aNotification{
	[self refreshIPhotoData: NULL];
}

/* awakeFromNib
	When we first open the nib with a psWindow, let's set up some default values... 
*/
-(void)awakeFromNib
{
	/* if we have a transition selection button, store off which index has the random
		transition selection...*/
	if (gTransitionSelectionField != nil)
		gIndexOfRandom = [gTransitionSelectionField indexOfItemWithTitle:@"(random)"];
	
	//update the iphoto album data
	[self refreshIPhotoData: NULL];

}

//----------------------------- update internal variables -------------------------------
/* getMasterImageDict
	albumData = CFDictionaryRef from an iPhoto Album plist
	
	get the dictionary in the iPhoto CFDictionary (albumData) which
	has each image's metadata (including fileurl) stored by key.
*/
-(NSDictionary *)getMasterImageDict:(CFDictionaryRef)albumData{
	if (albumData == nil)
		return nil;
	
	return ((NSDictionary *)CFDictionaryGetValue(albumData, @"Master Image List"));
}

/* getItemKeysForAlbum
	albumData = CFDictionaryRef from an iPhoto Album plist
	atIndex = the index of the specific album we're interested in
	
	returns the array of keys indicating which photos are in this album
*/
-(NSArray *)getItemKeysForAlbum:(CFDictionaryRef)albumData atIndex:(int)index{
	if (albumData == nil)
		return nil;
	
	if (gAlbumsArray==nil)
		return nil;
		
	//count how many albums we have
	CFIndex numAlbums = CFArrayGetCount((CFArrayRef)gAlbumsArray);
	if (numAlbums < 1){
		return nil;
	}
		
	//for indexed album dict, go get the array of item keys
	CFDictionaryRef curAlbumDict = CFArrayGetValueAtIndex ( (CFArrayRef)gAlbumsArray, index);
	CFArrayRef itemKeys = nil;
	if (!CFDictionaryGetValueIfPresent(curAlbumDict, @"KeyList", (const void**)&itemKeys)){
		return nil;
	}
	
	return ([(NSArray*)itemKeys retain]);
}

/* collectAlbumNames
	albumData = CFDictionaryRef from an iPhoto Album plist
	
	returns the array of album names
*/
-(NSArray *)collectAlbumNames:(CFDictionaryRef) albumData{
	if (albumData == nil)
		return nil;
	
	//get the list of albums
	if (!CFDictionaryGetValueIfPresent(albumData, @"List of Albums", (const void **)&gAlbumsArray))
		return nil;
	else [gAlbumsArray retain];
		
	//count how many albums we have
	CFIndex i = 0, numAlbums = CFArrayGetCount((CFArrayRef)gAlbumsArray);
	if (numAlbums < 1){
		return nil;
	}
	
	//make a mutable array to return
	CFMutableArrayRef albumNamesArray = CFArrayCreateMutable(kCFAllocatorDefault, numAlbums, NULL);
	if (albumNamesArray == nil){
		return nil;
	}
	
	//for each album dict, go get the name and store it in the return array
	for (i = 0; i< numAlbums; i++){
		CFDictionaryRef curAlbumDict = CFArrayGetValueAtIndex ( (CFArrayRef)gAlbumsArray, i);
		CFStringRef curAlbumName = nil;
		if (!CFDictionaryGetValueIfPresent(curAlbumDict, @"AlbumName", (const void**)&curAlbumName)){
			return nil;
		}
		CFArraySetValueAtIndex ( albumNamesArray, i, [(NSString *)curAlbumName retain]);
	}
	
	return ((NSArray*)albumNamesArray);
}

/* initIPhotoData
	find the iPhoto Library album data
	read out the album names to update the UI
	update the UI appropriately
	also update the cached gMasterImageDictList
*/
-(void)initIPhotoData{
	// first clear out all the globals that we might be holding on to
	// since we can get called to init multiple times and the world
	// might have changed out from underneath us.
	if (gPlist!=nil)
		gPlist = nil;
	if (gMasterImageDictList!=nil)
		[gMasterImageDictList release];
	if (gAlbumsArray!=nil)[gAlbumsArray release];
	if (gKeysForAlbum!=nil)[gKeysForAlbum release];
	gSelectedAlbumIndex = 0;
	
	//make a path to the iphoto library ("~/Pictures/iPhoto Library/AlbumData.xml")
	NSString *albumDataPath = [[[NSHomeDirectory()
			stringByAppendingPathComponent:@"Pictures"]
			stringByAppendingPathComponent:@"iPhoto Library"]
			stringByAppendingPathComponent:@"AlbumData.xml"];

	//if there is an iPhoto Library
	NSFileManager *manager = [NSFileManager defaultManager];
	if ([manager fileExistsAtPath:albumDataPath]){
		NSData *plistData;
		NSString *error;
		NSPropertyListFormat format;
		
		//suck in all the data from the iPhoto Library's album plist
		plistData = [NSData dataWithContentsOfFile:albumDataPath];
		gPlist = [NSPropertyListSerialization propertyListFromData:plistData
										mutabilityOption:NSPropertyListImmutable
										format:&format
										errorDescription:&error];
		if(!gPlist)
		{
			NSLog(error);
			[error release];
		} else {
			//pull out the album names and update the UI with them
			NSArray *albumNames = [self collectAlbumNames:(CFDictionaryRef)gPlist];
			[self setAlbumNames:albumNames];

			//update our cached globals
			gMasterImageDictList = [[self getMasterImageDict:(CFDictionaryRef)gPlist] retain];

			//update the default sequence name with the UI selected value.
			[self setSelectedAlbum:NULL];
		}
	}
}

//-------------------------------- update UI ---------------------------------
/* setAlbumNames
	albumNames = array iPhoto album names

	update the album selection popup in the UI with those in albumNames
*/
-(void)setAlbumNames:(NSArray*)albumNames{
	//store off which item is selected
	NSString* selectedTitle = nil;
	if ([gAlbumPopup numberOfItems] > 1)
		selectedTitle = [gAlbumPopup titleOfSelectedItem];

	//then rebuild the list
	[gAlbumPopup removeAllItems];
	[gAlbumPopup addItemsWithTitles:albumNames];

	//then restore the selection if it is there
	if (selectedTitle != nil){
		int selectedIndex = [gAlbumPopup indexOfItemWithTitle:selectedTitle];
		if (selectedIndex < 0)
			selectedIndex = 0;
		[gAlbumPopup selectItemAtIndex:selectedIndex];
	}
}

/* refreshIPhotoData
	sender (unused)
	
	refresh the iPhotoData so that it is up to date
*/
- (IBAction)refreshIPhotoData:(id)sender{
	[self initIPhotoData];
}


/* setSelectedAlbum
	sender (unused)
	
	stores off the array of photo keys from the dictionary and 
	updates the UI default output name to match the selected album name
*/
- (IBAction)setSelectedAlbum:(id)sender{
	gSelectedAlbumIndex = [gAlbumPopup indexOfSelectedItem];
	if (gSelectedAlbumIndex != -1){
	
		//cache off the keys for this album
		gKeysForAlbum = [self getItemKeysForAlbum:(CFDictionaryRef)gPlist atIndex:gSelectedAlbumIndex];
		
		//update UI
		[gSlideshowTitleField setStringValue:[gAlbumPopup titleOfSelectedItem]];
	}
}

//-------------------------------- timebase functions ---------------------------------
/* getFPS
	go check out the framerate popup and return an int value for the XML to use. This
	includes converting 29.97 to 30. NOTE: We will aslo return NTSCRate as "TRUE" for
	this case.
*/
-(int)getFPS{
	NSString *curFramerate =[gFramerateSelectionField titleOfSelectedItem];
	if (NSOrderedSame == [curFramerate compare:@"29.97"]){
		return 30;
	} else return [curFramerate intValue];
}

/* getTimebase
	convert getFPS output to a string
*/
-(NSString*)getTimebase{
	return[[NSNumber numberWithInt:[self getFPS]] stringValue];
}

/* getNTSCRate
	convert getFPS output to what the NTSCRate should be.
	note... if you actually want 30FPS in addition to 29.97 you'll
	need to add a little more smarts to this so you can put both
	30, non-ntsc and 30, ntsc rate into the XML.
*/
-(NSString*)getNTSCRate{
	if ([self getFPS]==30){
		return @"TRUE";
	} 
	return @"FALSE";
}

/* getSeqPreset
	convert getFPS output to what default sequence preset to use.
*/
-(NSString*)getSeqPreset{
	if ([self getFPS]==30){
		return @"DV NTSC 48 kHz";
	} else return @"DV PAL 48 kHz";
}

//-------------------------------- durations functions ---------------------------------
/* getClipDuration
	convert the UI clip duration in seconds to some number
	of frames. Note that for 29.97 we will slowly drift out of 
	sync with timecode since we're using 30 for this calculation.
*/
-(int)getClipDuration{

	return ([gClipDurationField intValue]*[self getFPS]);
}

/* getTransitionDuration
	convert the UI clip duration in seconds to some number
	of frames. Note that for 29.97 we will slowly drift out of 
	sync with timecode since we're using 30 for this calculation.
*/
-(int)getTransitionDuration{
	return 	([gTransitionDurationField intValue] *[self getFPS]);
}

/* getMediaDuration
	convert the UI clip duration in seconds to some number
	of frames. We want to setup the media duration to be long
	enough to cover the transitions on both before and after this clip
*/
-(NSString*)getMediaDuration{
	int value = [self getClipDuration]+(4*[self getTransitionDuration]);
	return [[NSNumber numberWithInt:value] stringValue];
}

/* getMediaIn
	pOut = the corresponding media out time to the returned	media in time.
	
	note that the media in time is offset from 0 to leave room for the transition
	to extend into.
*/
-(NSString*)getMediaIn:(int*)pOut{
	int value = [self getTransitionDuration];
	if (pOut != nil)
		*pOut = value+[self getClipDuration]+2*[self getTransitionDuration];
	return [[NSNumber numberWithInt:value] stringValue];
}

/* getTransitionDuration
	pIntValue = the corresponding transition duration in int format for
	use in calculations of start, and end as well as total media
	
	convert the UI transition duration in seconds to some number
	of frames. Note that for 29.97 we will slowly drift out of 
	sync with timecode since we're using 30 for this calculation.
*/
-(NSString*)getTransitionDuration: (int*)pIntValue{
	int intValue = 0;
	intValue = ([self getTransitionDuration]);
	if (pIntValue != nil)
		*pIntValue = intValue;
	return [[NSNumber numberWithInt:intValue] stringValue];
}

//-------------------------------- output settings functions ---------------------------------
/* shouldCreateNewProject
	returns the state of UI checkbox
*/
-(BOOL)shouldCreateNewProject{
	if (gCreateNewProjectButton != nil)
		if ([gCreateNewProjectButton state] == NSOnState)
			return TRUE;
		else return FALSE;
	return TRUE;
}


/* getSlideShowTitle
	returns the slide show title from UI.
*/
-(NSString*)getSlideShowTitle{
	return [gSlideshowTitleField stringValue];
}

/* getTransitionName 
	fxCategory = filled in with the effect category of the return transition.
	index = index in the popup for which you want the name and fx category.

	returns the name of the transition at index in the popup
*/
-(NSString*)getTransitionName: (NSString**)fxCategory forIndex:(int)index{
	NSString *curTrans =[gTransitionSelectionField itemTitleAtIndex: index];
	if (NSOrderedSame == [curTrans compare:@"random"]){
		NSLog(@"we got the index for random in getTransitionName:forIndex");
		
		*fxCategory =@"Dissolve";
		return @"Cross Dissolve";
	}
	if (NSOrderedSame == [curTrans compare:@"Cross Dissolve"]){
		*fxCategory =@"Dissolve";
	} else if (NSOrderedSame == [curTrans compare:@"Edge Wipe"]){
		*fxCategory =@"Wipe";
	} else *fxCategory =@"Iris";
	return curTrans;
}

/* getTransitionName
	effectCategory = filled in with the effect category of the return transition.

	returns the name of the selected transition or if "(random)" is selected,
	a random transition from the popup.
*/
-(NSString*)getTransitionName: (NSString**)fxCategory{
	int index = [gTransitionSelectionField indexOfSelectedItem];
	if (index == gIndexOfRandom){
		index = random()%(gIndexOfRandom);//+1?
	}
	return [self getTransitionName:fxCategory forIndex:index];
}

/* getFilesCount
	returns a count of the number of files in the selected iPhoto Album
*/
-(int)getFilesCount{
	CFIndex fileCount = CFArrayGetCount((CFArrayRef)gKeysForAlbum);
	return (int)fileCount;
}

/* getClipFileURL
	index = the index of the file (in the file array) to get the URL and/or name for.
	andName = the name of the file at index.
	
	returns a the pathURL string of the file at index in the selected iPhoto Album
*/
-(NSString*)getClipFileURL:(int)index andName:(NSString**)clipName{

	//get the key value to lookup
	CFStringRef currentKey = CFArrayGetValueAtIndex((CFArrayRef)gKeysForAlbum, index);
	
	//find the keyed entry in the master image dictionary
	CFDictionaryRef currClipDataDict = nil;
	if (!CFDictionaryGetValueIfPresent((CFDictionaryRef)gMasterImageDictList, currentKey, (const void**)&currClipDataDict)){
		return nil;
	}
	
	//get the fileurl stored at "imagepath"
	NSString *filePath = nil,*fileURL = nil;
	if (!CFDictionaryGetValueIfPresent((CFDictionaryRef)currClipDataDict, @"ImagePath", (const void**)&filePath)){
		return nil;
	}

	//get the name to use in finalcut for this image stored at "Caption"
	if (!CFDictionaryGetValueIfPresent((CFDictionaryRef)currClipDataDict, @"Caption", (const void**)clipName)){
		NSFileManager *fileManager = [NSFileManager defaultManager];
		*clipName = [fileManager displayNameAtPath:filePath];
	}
	
	//make sure the url is properly percent escaped by making a URL and then converting that back to a string.
	if (filePath != nil){
		NSURL *urlurl = [NSURL fileURLWithPath:filePath];
		fileURL = (NSString*)CFURLGetString((CFURLRef)urlurl);
	}
	
	return fileURL;
}


@end
