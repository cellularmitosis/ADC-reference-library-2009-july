/*
	File:		ACInsertWindowController.mm

	Abstract:	Implements the Audio Context Insert Window Controller.
				
				This controller manages the UI related to the Audio
				Context Insert. It provides UI to inspect the movie
				and device summary mix, configure an insert's in/out channel
				layouts and hosts an AudioUnit view. The selected AudioUnit
				is used in the insert's processing backend. 

	Version:	1.0

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

	Copyright © 2006-2008 Apple Inc. All Rights Reserved.
*/

#import "ACInsertWindowController.h"

@implementation ACInsertWindowController 

#pragma mark
#pragma ---- Class methods ----
	// validate AU Cocoa View
+ (BOOL)plugInClassIsValid:(Class) pluginClass
{
	if ([pluginClass conformsToProtocol:@protocol(AUCocoaUIBase)]) 
	{
		if ([pluginClass instancesRespondToSelector:@selector(interfaceVersion)] &&
			[pluginClass instancesRespondToSelector:@selector(uiViewForAudioUnit:withSize:)]) 
		{
			return YES;
		}
	}
	
    return NO;
}

#pragma mark
#pragma mark ---- Init, Dealloc, Post-Nib Loading ---
- (id) init 
{
	self = [self initWithWindowNibName:@"ACInsertWindow"];
	if (self) 
	{
		mMovieDocument = [[NSDocumentController sharedDocumentController] currentDocument];
		mInsertLayoutPopUpsPopulated = false;
		mCurrentAUIndex = -1;
		mCurrentInsertDestinationIndex = -1;
		mMovieSummaryMixFormatString = [[NSMutableString alloc] init];
		mDeviceFormatString = [[NSMutableString alloc] init];
		mTrackFormatString = [[NSMutableString alloc] init];
	} 
	return self;
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
	if (mAUList != NULL) 
	{
		free(mAUList);
		mAUList = NULL;
	}
	if(mMovieSummaryLayout) 
	{
		free(mMovieSummaryLayout);
	}
	if(mDeviceLayout) 
	{
		free(mDeviceLayout);
    }
	[mMovieSummaryMixFormatString release];
	[mDeviceFormatString release];
	[mTrackFormatString release];
	[super dealloc];
}

- (void)awakeFromNib
{	
 	[self updateUIMovieSummaryMix];
	[self updateUIDeviceFormat];
	[self updateUITrackFormat];
	
	// create scroll-view
    NSRect frameRect = [[uiAUViewContainer contentView] frame];
    mScrollView = [[[NSScrollView alloc] initWithFrame:frameRect] autorelease];
    [mScrollView setDrawsBackground:NO];
    [mScrollView setHasHorizontalScroller:YES];
    [mScrollView setHasVerticalScroller:YES];
    [uiAUViewContainer setContentView:mScrollView];
    
	[self populateInsertDestinationPopUp];
			
    // Fill the AU selector pop-up with 
	// effect AU choices
	[self populateAUPopUpWithEffectAUs];
	
	// Select top-of-list AU & show its UI and bypass state
	// Set its in/out channel layouts to 'Select a Layout'
    [self iaAUPopUpButtonPressed:self];  
	
}

- (void)windowDidLoad 
{
	
	[super windowDidLoad];
	
	[[self window] setTitle:[NSString stringWithFormat:@"Audio Context Insert : %@", (NSString*)[[mMovieDocument movie] attributeForKey:QTMovieDisplayNameAttribute]]];
	
	// register for notifications
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(movieTracksChanged:) name:QTAudioContextInsertMovieTracksChangedNotification object:nil];
}

- (MovieDocument *)movieDocument
{
	return mMovieDocument;
}

#pragma mark
#pragma mark ---- IB Actions ----

// This action is called by the Audio Unit selector pop-up menu
// when a new Audio Unit is selected. We do the following:
// [1] Ask the ACInsertManager to create a new insert processor that
// uses the selected AU as its processing backend
// [2] Synchronize our UI - cocoa view, bypass button, in/out layout 
// choices - to the new AU.
- (IBAction)iaAUPopUpButtonPressed:(id)sender
{
	int index = [uiAUPopUpButton indexOfSelectedItem];
	
	if (index == mCurrentAUIndex)
	{
		// User re-selected the currently selected AU
		return;
	}
	
	mCurrentAUIndex = index;
	
	// The ACInsertManager needs to create a new insert processor that 
	// uses an instance of the selected AU for its back-end processing
	[[mMovieDocument acInsertManager] createInsertProcessorForAU:(Component)mAUList[index]];
	
	// Remove the old view first before closing the AU
	[[mScrollView documentView] removeFromSuperview];
	[self showCocoaViewForAU:[[mMovieDocument acInsertManager] currentAU]];

	// Sync the AU bypass button state
	[self syncInsertBypassButton]; 		

	// Update insert input layout pop-up with options that make sense for this AU,
	[self updateInsertACLPopUpButtonChoices];
}

// This action is called when the Bypass check-box is
// checked or unchecked. We pass on the bypass state change
// message to the ACInsertManager.
- (IBAction)iaAUBypassButtonPressed:(id)sender
{
	UInt32 isBypassed = ([sender state] == NSOffState) ? 0 : 1;
	[[mMovieDocument acInsertManager] setInsertBypassed:isBypassed];
}	

// This action is called when an insert's input channel layout is changed.
// Use the ACInsertManager to set the new input layout on the insert, and update 
// the output layout pop-up button with options that make sense for the selected
// input layout.
- (IBAction)iaInsertInputACLPopUpButtonPressed:(id)sender
{		
	if (sender != self)		
	{
		// User re-selected the currently selected item
		if ([[uiInsertInputACLPopUpButton selectedItem] tag] == mCurrentInLayoutTag) 
		{
			return;
		}
	}
	if ( [[uiInsertInputACLPopUpButton selectedItem] tag] == 0 ) 
	{
		// Special tag associated with the 'Select Layout' menu item
		// Nothing to do here
		mCurrentInLayoutTag = 0;
		return;
	}	
	if ([[uiInsertInputACLPopUpButton itemAtIndex:0] tag] == 0) 
	{
		// Remove the 'Select Layout' menu item if it exists
		[uiInsertInputACLPopUpButton removeItemAtIndex:0];
	}
	mCurrentInLayoutTag = [[uiInsertInputACLPopUpButton selectedItem] tag];
	[[mMovieDocument acInsertManager] setInsertInputLayout:(AudioChannelLayoutTag)[[uiInsertInputACLPopUpButton selectedItem] tag]];

	// Enable the output layout button
	[uiInsertOutputACLPopUpButton setEnabled:YES];
	
	// Enable/disable menu items in the ouput layout button depending on  
	// whether they are valid output layouts for the given input layout
	for (UInt32 index=0; index < (UInt32)[uiInsertOutputACLPopUpButton numberOfItems]; index++)
	{
		if ([[uiInsertOutputACLPopUpButton itemAtIndex:index] tag] == 0) 
		{
			// The 'Select Layout' item should always be enabled
			[[uiInsertOutputACLPopUpButton itemAtIndex:index] setEnabled:YES];
			continue;
		}	
		if ([[mMovieDocument acInsertManager] insertCanDoOutputChannels:AudioChannelLayoutTag_GetNumberOfChannels([[uiInsertOutputACLPopUpButton itemAtIndex:index] tag])]) 
		{
			// The selected input layout, and this output layout make a valid combination
			[[uiInsertOutputACLPopUpButton itemAtIndex:index] setEnabled:YES];
		} else {
			[[uiInsertOutputACLPopUpButton itemAtIndex:index] setEnabled:NO];		
		}
	}
	
	if ( ([[uiInsertOutputACLPopUpButton selectedItem] tag] == 0) ||
		 ([[uiInsertOutputACLPopUpButton selectedItem] isEnabled] == 0) ) 
	{
		// If no output layout selected yet or output layout
		// is incompatible with the new input layout, try and select the
		// output layout that is equal to the input layout. For most effect
		// units this is a valid combination. If not, select the 'Select
		//  Layout' item, and let the user decide.
		AudioChannelLayoutTag inputLayoutTag = [[uiInsertInputACLPopUpButton selectedItem] tag];
		BOOL foundValidOutLayout = false;
		for (UInt32 index=0; index < (UInt32)[uiInsertOutputACLPopUpButton numberOfItems]; index++)
		{
			if ([[uiInsertOutputACLPopUpButton itemAtIndex:index] tag] == (int)inputLayoutTag) 
			{
				if ([[uiInsertOutputACLPopUpButton itemAtIndex:index] isEnabled]) 
				{
					[uiInsertOutputACLPopUpButton selectItemAtIndex:index];
					foundValidOutLayout = true;
				} 
				break;
			}	
		}
		if (foundValidOutLayout == false) 
		{
			// Let the user select a layout
			if ([[uiInsertOutputACLPopUpButton itemAtIndex:0] tag] != 0)
			{
				[uiInsertOutputACLPopUpButton insertItemWithTitle:@"Select Layout" atIndex:0];
				[[uiInsertOutputACLPopUpButton itemAtIndex:0] setTag:0];
			}
			[uiInsertOutputACLPopUpButton selectItemAtIndex:0];
		} 
	}
	[self iaInsertOutputACLPopUpButtonPressed:self];
}

// This action is called when an insert's output channel layout is changed.
// Use the ACInsertManager to set the new output layout on the insert. 
- (IBAction)iaInsertOutputACLPopUpButtonPressed:(id)sender
{
	if (sender != self) 
	{
		// User re-selected the same item
		if (mCurrentOutLayoutTag == [[uiInsertOutputACLPopUpButton selectedItem] tag])
		{
			return;
		}	
	}
	if ( [[uiInsertOutputACLPopUpButton selectedItem] tag] == 0 ) 
	{
		// Special tag that indicates the 'Select Layout' item
		// Nothing to do.
		mCurrentOutLayoutTag = 0;	
		return;
	}	
	if ([[uiInsertOutputACLPopUpButton itemAtIndex:0] tag] == 0) 
	{
		// Remove the 'Select Layout' menu item if it exists
		[uiInsertOutputACLPopUpButton removeItemAtIndex:0];
	}
	mCurrentOutLayoutTag = [[uiInsertOutputACLPopUpButton selectedItem] tag];
	[[mMovieDocument acInsertManager] setInsertOutputLayout:(AudioChannelLayoutTag)[[uiInsertOutputACLPopUpButton selectedItem] tag]];
}

- (IBAction)iaInsertDestinationPopUpButtonPressed:(id)sender
{
	// User re-selected the same item
	if (mCurrentInsertDestinationIndex == [uiInsertDestinationPopUpButton indexOfSelectedItem])
	{
		return;
	}	 
	id oldDestination = nil;
	if (mCurrentInsertDestinationIndex != -1) 
	{
		oldDestination = [[uiInsertDestinationPopUpButton 
									itemAtIndex:mCurrentInsertDestinationIndex] representedObject];
	}
	[[mMovieDocument acInsertManager] insertDestinationChangedFrom:oldDestination
									to:[[uiInsertDestinationPopUpButton selectedItem] representedObject]];
	mCurrentInsertDestinationIndex = [uiInsertDestinationPopUpButton indexOfSelectedItem];
	[self updateUIMovieSummaryMix];
	[self updateUIDeviceFormat];
	[self updateUITrackFormat];
}

#pragma mark
#pragma mark ---- UI Updating and syncing ----
// Called when loading from nib. This method populates
// the menu that contains the possible points that
// an insert effect can be hooked to.
- (void)populateInsertDestinationPopUp
{
	UInt32 index;
	NSArray	*arrayOfMovieTracks;
	
	// Empty the menu
	[uiInsertDestinationPopUpButton removeAllItems];
	[uiInsertDestinationPopUpButton setAutoenablesItems:NO];
	
	// Add 'Movie' as the first item in the list
	[uiInsertDestinationPopUpButton addItemWithTitle:@"Movie"];
	[[uiInsertDestinationPopUpButton lastItem] setRepresentedObject:(QTMovie*)[mMovieDocument movie]];
	
	// Add a menu item for each audio track
	arrayOfMovieTracks = [(QTMovie*)[mMovieDocument movie] tracks];
	for (index = 0; index < [arrayOfMovieTracks count]; index++) 
	{
		// Look for audio tracks that mix into the audio context 
		// (no streaming or MPEG tracks)
		if (trackMixesToAudioContext([[arrayOfMovieTracks objectAtIndex:index] quickTimeTrack])) 
		{
			BOOL trackIsEnabled = GetTrackEnabled ([[arrayOfMovieTracks objectAtIndex:index] quickTimeTrack]);
			[uiInsertDestinationPopUpButton addItemWithTitle:[[arrayOfMovieTracks objectAtIndex:index] 
														attributeForKey:QTTrackDisplayNameAttribute]];
			[[uiInsertDestinationPopUpButton lastItem] setRepresentedObject:[arrayOfMovieTracks objectAtIndex:index]];
			[[uiInsertDestinationPopUpButton lastItem] setEnabled:(trackIsEnabled == YES)];
			
		}
	}
	
	// Select the first item in the menu
	[uiInsertDestinationPopUpButton selectItemAtIndex:0];
	[self iaInsertDestinationPopUpButtonPressed:self];
}

// Called when loading from nib. Populates
// the Audio Unit selector pop-up with names of Effect AU's
// available on the system.
- (void)populateAUPopUpWithEffectAUs
{
	ComponentDescription cd = {0};
	Component last = NULL;
	int componentCount = 0;
	UInt32 dataByteSize = 0;

    // get Effect AU list
	if (mAUList != NULL) 
	{
		free (mAUList);
		mAUList = NULL;
	}	
	cd.componentType = kAudioUnitType_Effect;
	componentCount = CountComponents(&cd);
	dataByteSize = componentCount * sizeof(Component);
	mAUList = (Component *) calloc(1, dataByteSize);
	for (int i = 0; i < componentCount; ++i) 
	{
		mAUList[i] = FindNextComponent (last, &cd);
		last = mAUList[i];
	}
	
	// populate pop-up with names of the AU's
    [uiAUPopUpButton removeAllItems];
	for (int i = 0; i < componentCount; ++i) 
	{
		NSString *nameString = [self GetAUNameForEffectComponentAtIndex:i];
		[uiAUPopUpButton addItemWithTitle:nameString];
		[nameString release];
	}
}

// Called by the populateAUPopUpWithEffectAUs: method to 
// get the display name for a particular AU
- (NSString*)GetAUNameForEffectComponentAtIndex:(UInt32)index
{
	OSStatus err = noErr;
	Handle name = NewHandle(4);
	CFStringRef	compName = NULL;
	CFStringRef	aUName = NULL;
	char* ptr1;
	int len;
	char* displayStr;
	NSString *retVal;
	ComponentDescription cd = {0};
	cd.componentType = kAudioUnitType_Effect;
	
	GetComponentInfo (mAUList[index], &cd, name, NULL, NULL);

	if (err) 
	{ 
		goto bail; 
	}
		
	HLock(name);
	ptr1 = *name;
	// Get the manufacturer's name... look for the ':' character convention
	len = *ptr1++;
	displayStr = 0;

	compName = CFStringCreateWithPascalString(NULL, (const unsigned char*)*name, kCFStringEncodingMacRoman);
			
	for (int i = 0; i < len; ++i) 
	{
		if (ptr1[i] == ':') 
		{ // found the name
			ptr1[i] = 0;
			displayStr = ptr1;
			break;
		}
	}
	
	if (displayStr)
	{					
		// move displayStr ptr past the manu, to the name
		// we move the characters down a index, because the handle doesn't have any room
		// at the end for the \0
		int i = strlen(displayStr), j = 0;
		while (displayStr[++i] == ' ' && i < len)
				;
		while (i < len)
		{
			displayStr[j++] = displayStr[i++];
		}
		displayStr[j] = 0;

		aUName = CFStringCreateWithCString(NULL, displayStr, kCFStringEncodingMacRoman);
	} 
bail:		
	DisposeHandle (name);
	retVal = (aUName ? (NSString*)aUName : (NSString*)compName);
	return retVal;
}

// This method is called whenever the selected Audio Unit changes.
// It shows the cocoa view (that contains controls for the Audio Unit's
// adjustable parameters) for the selected Audio Unit. This method
// checks to see if the selected Audio Unit has a custom view. If
// it does, it displays the custom view. Else, it displays a generic
// view.
- (void)showCocoaViewForAU:(AudioUnit)inAU
{
	// get AU's Cocoa view property
    UInt32 						dataSize = 0;
    Boolean 					isWritable;
    AudioUnitCocoaViewInfo *	cocoaViewInfo = NULL;
    UInt32						numberOfClasses;
    
    OSStatus result = AudioUnitGetPropertyInfo(	inAU,
                                                kAudioUnitProperty_CocoaUI,
                                                kAudioUnitScope_Global, 
                                                0,
                                                &dataSize,
                                                &isWritable );
    
    numberOfClasses = (dataSize - sizeof(CFURLRef)) / sizeof(CFStringRef);
    
    NSURL 	 *	CocoaViewBundlePath = nil;
    NSString *	factoryClassName = nil;
    
	// Does view have custom Cocoa UI?
    if ((result == noErr) && (numberOfClasses > 0) ) 
	{
        cocoaViewInfo = (AudioUnitCocoaViewInfo *)malloc(dataSize);
        if(AudioUnitGetProperty(		inAU,
                                        kAudioUnitProperty_CocoaUI,
                                        kAudioUnitScope_Global,
                                        0,
                                        cocoaViewInfo,
                                        &dataSize) == noErr) 
		{
            CocoaViewBundlePath	= (NSURL *)cocoaViewInfo->mCocoaAUViewBundleLocation;
			
			// we only take the first view in this example.
            factoryClassName	= (NSString *)cocoaViewInfo->mCocoaAUViewClass[0];
        } 
		else 
		{
            if (cocoaViewInfo != NULL) 
			{
				free (cocoaViewInfo);
				cocoaViewInfo = NULL;
			}
        }
    }
	
	NSView *AUView = nil;
	BOOL wasAbleToLoadCustomView = NO;
	
	// Show custom UI if view has it
	if (CocoaViewBundlePath && factoryClassName) 
	{
		NSBundle *viewBundle  	= [NSBundle bundleWithPath:[CocoaViewBundlePath path]];
		if (viewBundle == nil) 
		{
			NSLog (@"Error loading AU view's bundle");
		} 
		else 
		{
			Class factoryClass = [viewBundle classNamed:factoryClassName];
			NSAssert (factoryClass != nil, @"Error getting AU view's factory class from bundle");
			
			// make sure 'factoryClass' implements the AUCocoaUIBase protocol
			NSAssert(	[ACInsertWindowController plugInClassIsValid:factoryClass],
						@"AU view's factory class does not properly implement the AUCocoaUIBase protocol");
			
			// make a factory
			id factoryInstance = [[[factoryClass alloc] init] autorelease];
			NSAssert (factoryInstance != nil, @"Could not create an instance of the AU view factory");
			// make a view
			AUView = [factoryInstance	uiViewForAudioUnit:inAU
										withSize:[[mScrollView contentView] bounds].size];
			
			// cleanup
			[CocoaViewBundlePath release];
			if (cocoaViewInfo) 
			{
				UInt32 i;
				for (i = 0; i < numberOfClasses; i++)
				{
					CFRelease(cocoaViewInfo->mCocoaAUViewClass[i]);
				}
				free (cocoaViewInfo);
			}
			wasAbleToLoadCustomView = YES;
		}
	}
	
	if (!wasAbleToLoadCustomView) 
	{
		// No custom view, show generic Cocoa view
		AUView = [[AUGenericView alloc] initWithAudioUnit:inAU];
		[(AUGenericView *)AUView setShowsExpertParameters:NO];
    }
	
	// Display view
	// Get the size of the new AU View's frame 
	NSRect viewFrame = [AUView frame];
	NSSize frameSize = [NSScrollView	frameSizeForContentSize:viewFrame.size
										hasHorizontalScroller:[mScrollView hasHorizontalScroller]
										hasVerticalScroller:[mScrollView hasVerticalScroller]
										borderType:[mScrollView borderType]];

	// Create a new frame with same origin as current
	// frame but size equal to the size of the new view
	NSRect newFrame;
	NSRect currentFrame = [mScrollView frame];
	newFrame.origin = currentFrame.origin;
	newFrame.size = frameSize;

	// Set the new frame and document views on the scroll view
	[mScrollView setFrame:newFrame];
	[mScrollView setDocumentView:AUView];

	NSRect origWindowFrame = [[self window] frame];
	NSSize oldContentSize = [[[self window] contentView] frame].size;
	NSSize newContentSize = oldContentSize;
	newContentSize.width += (newFrame.size.width - currentFrame.size.width);
	newContentSize.height += (newFrame.size.height - currentFrame.size.height);

	[[self window] setContentSize:newContentSize];
	[[self window] setFrameTopLeftPoint:NSMakePoint(origWindowFrame.origin.x, NSMaxY(origWindowFrame))];
}


// This method is called whenever the selected Audio Unit changes.
// It enables/disables the Bypass button depending on whether
// the selected AU is bypassable. If bypassable, it sets the state
// of the button to the current bypass state of the AU.
- (void)syncInsertBypassButton
{
	UInt32 isBypassed;
	BOOL enableButton = YES;
		
	[[mMovieDocument acInsertManager] insertIsBypassable:&enableButton currentBypassState:&isBypassed];
	
	if (enableButton) 
	{
		// Enable button, set bypass state
		[uiAUBypassButton setEnabled:YES];
		[uiAUBypassButton setState:(isBypassed ? NSOnState : NSOffState)];
	} 
	else 
	{
		// Disable button
		[uiAUBypassButton setState:NSOffState];
		[uiAUBypassButton setEnabled:NO];
	}
}

// This method is called when loading from nib and
// whenever the movie's tracks' enabled state or channel
// layout changes. Creates and displays a string to describe 
// the format (data format, sample rate, num of channels, 
// channel layout) of the movie's summary mix. 
- (void)updateUIMovieSummaryMix
{
	OSStatus err = noErr;
	UInt32 movieSummaryLayoutSize = 0;
	NSString *movieSummaryLayoutNameStr = nil;
	UInt32 size = sizeof(NSString *);
			
	if (mMovieSummaryLayout) 
	{
		free(mMovieSummaryLayout);
	}	
	
	if (mMovieDocument == nil)
	{
		goto bail;
	}
	// Get the new movie summary asbd and layout
	err = getMovieSummaryLayoutAndASBD( [[mMovieDocument movie] quickTimeMovie], 
							&movieSummaryLayoutSize, &mMovieSummaryLayout, &mMovieSummaryASBD);
	
	if (err)
	{
		[mMovieSummaryMixFormatString setString:@"Movie Summary Mix: - "];
		goto bail;
	}
	
	// Append data format, sample rate, number of channels and channel layout to 
	// the display string
	[mMovieSummaryMixFormatString setString:@"Movie Summary Mix: "];
	[mMovieSummaryMixFormatString appendFormat:@"%.0f Hz", mMovieSummaryASBD.mSampleRate];
	[mMovieSummaryMixFormatString appendFormat:@", %d channels", mMovieSummaryASBD.mChannelsPerFrame];
	
	err = AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutName, 
									movieSummaryLayoutSize, mMovieSummaryLayout, &size, (void *)&movieSummaryLayoutNameStr);
	if (noErr == err) 
	{
		[mMovieSummaryMixFormatString appendFormat:@", %@", movieSummaryLayoutNameStr];
	}

	[movieSummaryLayoutNameStr release];
	
bail:
	[self updatePreAndPostInsertFormatTextFields];
	return;
}

// This method is called when loading from nib.
// Creates and displays a string to describe the format 
// (data format, sample rate, channel layout, etc.) of the 
// device that the movie is playing to
- (void)updateUIDeviceFormat
{
	OSStatus err = noErr;
	UInt32 deviceLayoutSize = 0;
	NSString *deviceLayoutNameStr = nil;
	UInt32 size = sizeof(NSString *);
	
	if (mDeviceLayout) 
	{
		free(mDeviceLayout);
	}	
	
	if (mMovieDocument == nil)
	{
		goto bail;
	}
	// get the new device asbd and layout
	err = getDeviceLayoutAndASBD( [(QTMovie*)[mMovieDocument movie] quickTimeMovie], 
							&deviceLayoutSize, &mDeviceLayout, &mDeviceASBD);
	
	if (err)
	{
		[mDeviceFormatString setString:@"Device: - "];
		goto bail;
	}

	// Append data format, sample rate, number of channels and channel layout to 
	// the display string
	[mDeviceFormatString setString:@"Device: "];
	[mDeviceFormatString appendFormat:@"%.0f Hz", mDeviceASBD.mSampleRate];
	[mDeviceFormatString appendFormat:@", %d channels", mDeviceASBD.mChannelsPerFrame];
	err = AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutName, 
									deviceLayoutSize, mDeviceLayout, &size, (void *)&deviceLayoutNameStr);
	if (noErr == err) 
	{
		[mDeviceFormatString appendFormat:@", %@", deviceLayoutNameStr];
	}

	[deviceLayoutNameStr release];
bail:
	[self updatePreAndPostInsertFormatTextFields];
	return;
}

- (void)updateUITrackFormat
{
	OSStatus err = noErr;
	UInt32 trackLayoutSize = 0;
	NSString *trackLayoutNameStr = nil;
	UInt32 size = sizeof(NSString *);
	Track theTrack;
	
	if (mCurrentInsertDestinationIndex <= 0) // insert is attached to the movie
	{
		return;
	}
	
	if (mTrackLayout)
	{
		free(mTrackLayout);
	}
	
	if (mMovieDocument == nil)
	{
		goto bail;
	} 
	
	theTrack = [[[uiInsertDestinationPopUpButton itemAtIndex:mCurrentInsertDestinationIndex] representedObject] quickTimeTrack];
	
	// get track format
	err = getTrackASBD(theTrack, &mTrackASBD);
	if (err)
	{
		[mTrackFormatString setString:@"Track: -"];
		goto bail;
	}
	
	// get the track layout
	err = getTrackLayoutAndSize(theTrack,  &trackLayoutSize, &mTrackLayout);
	
	if (err)
	{
		[mTrackFormatString setString:@"Track: -"];
		goto bail;
	}
	
	// Append data format, sample rate, number of channels and channel layout to 
	// the display string
	[mTrackFormatString setString:@"Track: "];
	//[self appendFloatPCMFormatTextFor:mTrackASBD intoString:mTrackFormatString];
	[mTrackFormatString appendFormat:@"%.0f Hz", mTrackASBD.mSampleRate];
	[mTrackFormatString appendFormat:@", %d channels", mTrackASBD.mChannelsPerFrame];
	err = AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutName, 
									trackLayoutSize, mTrackLayout, &size, (void *)&trackLayoutNameStr);
	if (noErr == err) 
	{
		[mTrackFormatString appendFormat:@", %@", trackLayoutNameStr];
	}

	[trackLayoutNameStr release];

bail:
	[self updatePreAndPostInsertFormatTextFields];
	return;	
}

- (void) updatePreAndPostInsertFormatTextFields
{
	if ([uiInsertDestinationPopUpButton indexOfSelectedItem] == 0) // insert is currently attached to a movie
	{
		[uiPreInsertFormatTextField setStringValue:mMovieSummaryMixFormatString];
		[uiPostInsertFormatTextField setStringValue:mDeviceFormatString];
	}
	else // insert attached to a track
	{
		[uiPreInsertFormatTextField setStringValue:mTrackFormatString]; // replace with track format
		[uiPostInsertFormatTextField setStringValue:[NSString stringWithFormat:@"%@\n%@", mMovieSummaryMixFormatString, mDeviceFormatString]];
	}
}

- (void)updateTrackEnabledDisabledStatus
{
	int ndx;
	
	for (ndx = 1; ndx < [uiInsertDestinationPopUpButton numberOfItems]; ndx++)
	{
		Track theTrack = [[[uiInsertDestinationPopUpButton itemAtIndex:ndx] representedObject] quickTimeTrack];
		Boolean trackIsEnabled = GetTrackEnabled(theTrack);
		
		if (trackIsEnabled &&  ([[uiInsertDestinationPopUpButton itemAtIndex:ndx] isEnabled] == NO))
		{
			// Previously disabled track is now enabled. Enable menu item associated with it.
			[[uiInsertDestinationPopUpButton itemAtIndex:ndx] setEnabled:YES];
		} 
		else if (!trackIsEnabled && ([[uiInsertDestinationPopUpButton itemAtIndex:ndx] isEnabled] == YES))
		{
			// Previously enabled track is now disabled. Disable menu item associated with it.
			[[uiInsertDestinationPopUpButton itemAtIndex:ndx] setEnabled:NO];
			if (mCurrentInsertDestinationIndex == ndx)
			{
				// If an insert was attached to the track that has been disabled, 
				// move the insert to the movie.
				[uiInsertDestinationPopUpButton selectItemAtIndex:0]; 
				[self iaInsertDestinationPopUpButtonPressed:self];
			}	
		}
	}
}


// Helper function called by updateUIMovieSummaryMix: and updateUIDeviceFormat:
// Creates and appends to given string a textual representation of the Float32
// PCM format (big/little endian)
-(void) appendFloatPCMFormatTextFor:(AudioStreamBasicDescription)asbd intoString:(NSMutableString *)formatStr
{
	// This function is called only for Float32 LPCM data formats, check that this is true
	if ( (asbd.mFormatID == 'lpcm') && (asbd.mFormatFlags & kLinearPCMFormatFlagIsFloat) && (asbd.mBitsPerChannel == 32.))
	{
		// Data is 32-bit Float lpcm, determine endianness
		// format:  Floating Point | Unsigned Integer | Integer [(Little Endian) | (Big Endian)]
		[formatStr appendString:@"32-bit Float"];
		// append big or little endian
		if (asbd.mBitsPerChannel > 8)
		{
			[formatStr appendString:@" "]; 
			if (asbd.mFormatFlags & kLinearPCMFormatFlagIsBigEndian)
				[formatStr appendString:@"(Big Endian)"];
			else
				[formatStr appendString:@"(Little Endian)"];
		}
	}
}

// This method is called whenever the selected Audio Unit changes.
// It updates the insert input layout menu with options that are  
// valid for the selected AU
- (void)updateInsertACLPopUpButtonChoices
{
	if (!mInsertLayoutPopUpsPopulated)
	{
		// We get in here only once, right after the window is loaded
		
		// Populate the pop-ups with channel layout option presets
		[self populateInsertLayoutPopUps];
		mInsertLayoutPopUpsPopulated = true;

		// Insert a 'Select Layout' item at the top of both
		// pop-up lists and select it that item
		[uiInsertInputACLPopUpButton insertItemWithTitle:@"Select Layout" atIndex:0];
		[[uiInsertInputACLPopUpButton itemAtIndex:0] setTag:0];
		[uiInsertInputACLPopUpButton selectItemAtIndex:0];
		mCurrentInLayoutTag = 0;
		
		[uiInsertOutputACLPopUpButton insertItemWithTitle:@"Select Layout" atIndex:0];
		[[uiInsertOutputACLPopUpButton itemAtIndex:0] setTag:0];
		[uiInsertOutputACLPopUpButton selectItemAtIndex:0];
		[uiInsertOutputACLPopUpButton setEnabled:NO];	//Enabled once a valid input layout is selected
		mCurrentInLayoutTag = 0;
	}
	
	// Enable/disable items in the input layout menu based on this AU's capabilities
	for (UInt32 index=0; index < (UInt32)[uiInsertInputACLPopUpButton numberOfItems]; index++)
	{
		UInt32 inputNumChannels = AudioChannelLayoutTag_GetNumberOfChannels([[uiInsertInputACLPopUpButton itemAtIndex:index] tag]);
		if ([[uiInsertInputACLPopUpButton itemAtIndex:index] tag] == 0) 
		{
			// Special 'Select Layout' item should always be enabled
			[[uiInsertInputACLPopUpButton itemAtIndex:index] setEnabled:YES];
			continue;
		}
		if ([[mMovieDocument acInsertManager] insertCanDoInputChannels:inputNumChannels outputNumChannels:inputNumChannels]) 
		{
			// If the insert can do a combination of n input and n output channels,
			// then n input channels is a valid option, enable it
			[[uiInsertInputACLPopUpButton itemAtIndex:index] setEnabled:YES];
		} else 
		{
			[[uiInsertInputACLPopUpButton itemAtIndex:index] setEnabled:NO];
		} 	
	}
	
	if ([[uiInsertInputACLPopUpButton selectedItem] tag] != 0)
	{
		// The menu item selected in the input layout menu is something
		// other than 'Select Layout'
		
		// If the currently selected item is not valid for the 
		// currently select AU, pick the first valid item
		if ( [[uiInsertInputACLPopUpButton selectedItem] isEnabled] == false)
		{
			for (UInt32 index=0; index < (UInt32)[uiInsertInputACLPopUpButton numberOfItems]; index++)
			{
				if ([[uiInsertInputACLPopUpButton itemAtIndex:index] isEnabled]) 
				{
					[uiInsertInputACLPopUpButton selectItemAtIndex:index];
					break;
				}
			}
		}
		[self iaInsertInputACLPopUpButtonPressed:self];
	}
}

// This method is called by the updateInsertACLPopUpButtonChoices: method
// It is called only once, and it populates the insert input and output layout
// menus with some channel layout presets
- (void)populateInsertLayoutPopUps
{
	//[1] Empty the pop up buttons' menus
	// Make sure that the menu items will not be autoenabled
	[uiInsertInputACLPopUpButton setAutoenablesItems:NO];
	[uiInsertOutputACLPopUpButton setAutoenablesItems:NO];
	[uiInsertInputACLPopUpButton removeAllItems];
	[uiInsertOutputACLPopUpButton removeAllItems];
	
	[self addToInserLayoutPopUpsItemWithLayoutTag:kAudioChannelLayoutTag_Mono andTitle:@"Mono"];
	[self addToInserLayoutPopUpsItemWithLayoutTag:kAudioChannelLayoutTag_Stereo andTitle:@"Stereo (L, R)"];
	[self addToInserLayoutPopUpsItemWithLayoutTag:kAudioChannelLayoutTag_Quadraphonic andTitle:@"Quadraphonic (L R Ls Rs)"];
	[self addToInserLayoutPopUpsItemWithLayoutTag:kAudioChannelLayoutTag_MPEG_5_0_A andTitle:@"5.0 (L R C Ls Rs)"];
	[self addToInserLayoutPopUpsItemWithLayoutTag:kAudioChannelLayoutTag_MPEG_5_1_A andTitle:@"5.1 (L R C LFE Ls Rs)"];
	[self addToInserLayoutPopUpsItemWithLayoutTag:kAudioChannelLayoutTag_MPEG_7_1_A andTitle:@"7.1 (L R C LFE Ls Rs Lc Rc)"];

}

// This method is a helper method called by the populateInsertLayoutPopUps: method. Adds 
// a menu item (and sets the menu item's tag) to the input and output layout menus. 
- (void) addToInserLayoutPopUpsItemWithLayoutTag:(AudioChannelLayoutTag)tag andTitle:(NSString*)title
{
	[uiInsertInputACLPopUpButton addItemWithTitle:title];
	[[uiInsertInputACLPopUpButton lastItem] setTag:(int)tag];
	[uiInsertOutputACLPopUpButton addItemWithTitle:title];	
	[[uiInsertOutputACLPopUpButton lastItem] setTag:(int)tag];
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
#pragma mark 
#pragma mark ---- Notifications ----

// This notification callback is called whenever a window
// is being closed. If the window is a movie window, this
// is a signal to close our own window.
- (void)windowWillClose:(NSNotification *)notification
{
	NSWindowController *controller = [[notification object] windowController];
	if ([controller isKindOfClass:[MovieWindowController class]]) 
	{
		// A movie window is being closed
		if ([controller window] == [[mMovieDocument movieWindowController] window])
		{
			// If that window belongs to the movie we're associated with
			[[self window] close];
		}
	} 
}


// This notification callback is called whenever the movie's tracks'
// enabled state or channel assignment changes. Such a change likely
// means that the movie's summary mix has changed, so we call
// updateUIMovieSummaryMix: which gets the (possibly) new movie summary
// and updates the movie summary information text fields.
- (void)movieTracksChanged:(NSNotification *)notification
{
	QTMovie *changedMovie = (QTMovie*)[notification object];
	if (changedMovie == [mMovieDocument movie])
	{
		[self updateUIMovieSummaryMix];
		[self updateTrackEnabledDisabledStatus];
	}
}

@end