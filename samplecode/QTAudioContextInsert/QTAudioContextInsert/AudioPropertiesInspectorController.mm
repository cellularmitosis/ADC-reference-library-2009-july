/*

	File:		AudioPropertiesInspectorController.mm

	Abstract:	Implements the Audio Properties Inspector panel.
				This panel is used to inspect and change movie
				and audio track audio settings such as gain, track 
				enabled/disabled state and track channel assignments.
				
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

#import "AudioPropertiesInspectorController.h"

// Movie gain change property listener callback
// Used to update the movie gain slider in the movie settings view
static void movieGainChangeCallback(Movie m, QTPropertyClass propClass, QTPropertyID propID, id observer);

@implementation AudioPropertiesInspectorController

#pragma mark
#pragma mark ---- Init, Dealloc, Post-Nib Loading ---

- (id) init
{
	self = [self initWithWindowNibName:@"AudioPropertiesWindow"];
	if (self) 
	{
		mMovieDocument = [[NSDocumentController sharedDocumentController] currentDocument];
		mChannelLabelOptionsMenu = nil;
		mAudioTracks = [[NSMutableArray alloc] init];
		mCommonChannelLabelOptions = [[NSMutableArray alloc] init];
	}
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[self removeMovieGainPropertyListener:[self movie]];
	[self setChannelLabelOptionsMenu:nil];
	[mAudioTracks release];
	[mCommonChannelLabelOptions release];
    [super dealloc];
}

- (void)awakeFromNib
{
	// Populate the channel options menu with channel label names 
	// that make sense for the current movie
	[self synchronizeChannelLabelOptionsMenu];
}

- (void)windowDidLoad 
{	
	[super windowDidLoad];

	[[self window] setTitle:[NSString stringWithFormat:@"Audio Properties : %@", (NSString*)[[self movie] attributeForKey:QTMovieDisplayNameAttribute]]];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tableViewSelectionChanged:) name:NSTableViewSelectionDidChangeNotification object:nil];
	// Add an observor for movie gain changes
	[self addMovieGainPropertyListener:[self movie]];
	
	// Dispatched set-up
	[uiTrackTableView reloadData];

}

#pragma mark
#pragma mark --- Setter ----
- (void)setChannelLabelOptionsMenu:(NSMenu*)menu
{
	[menu retain];
	[mChannelLabelOptionsMenu release];
	mChannelLabelOptionsMenu = menu;
}

#pragma mark
#pragma mark --- Getter ----
-(QTMovie*)movie
{
	return [mMovieDocument movie];
}

#pragma mark
#pragma mark ---- IB Actions ----
- (IBAction)iaMovieGainSliderChanged:(id)sender
{
	float gain = [sender floatValue];
	(void)QTSetMovieProperty([[self movie] quickTimeMovie], 
									kQTPropertyClass_Audio,
									kQTAudioPropertyID_Gain, 
									sizeof(gain),  
									&gain);
}

- (IBAction)iaTrackGainSliderChanged:(id)sender
{
	float gain = [sender floatValue];
	int indexIntoTrackArray = [uiTrackTableView selectedRow] - 1;
	QTTrack *selectedTrack = [mAudioTracks objectAtIndex:indexIntoTrackArray];
	
	(void)QTSetTrackProperty([selectedTrack quickTimeTrack], 
											kQTPropertyClass_Audio, 
											kQTAudioPropertyID_Gain, 
											sizeof(gain),  
											&gain);
}


#pragma mark
#pragma mark ---- Panel UI Functions ----
// Depending on whether the movie or a track is selected
// in the upper 'track table', select which view -
// movie settings view or track settings view - to display
- (void)synchronizeSettingsView
{
	int trackTableSelectedRow = [uiTrackTableView selectedRow];
	float gain = 0.;
	
	if (trackTableSelectedRow == 0) 
	{
		// Switch to the Movie Settings Tab
		[uiSettingsTabView selectTabViewItemAtIndex:0];
		
		// Get the movie gain and update the gain slider
		[self synchronizeMovieGainSlider];										
	} 
	else 
	{
		int indexIntoTrackArray = trackTableSelectedRow - 1;
		if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count]) 
		{
			QTTrack *selectedTrack = [mAudioTracks objectAtIndex:indexIntoTrackArray];
			// Select the Track Settings Tab
			[uiSettingsTabView selectTabViewItemAtIndex:1];
			// Get the track's gain
			(void) QTGetTrackProperty([selectedTrack quickTimeTrack], 
										kQTPropertyClass_Audio, 
										kQTAudioPropertyID_Gain, 
										sizeof (gain), &gain, NULL);
			[uiTrackGainSlider setFloatValue:(float)gain];
							
			// Reload the channel assignment table
			[uiTrackChannelLayoutTableView reloadData];

		}
	}

}

// Get the current value for the movie gain and 
// set the movie gain slider to this value
- (void)synchronizeMovieGainSlider
{
	float gain = 0.;
	if ( [uiSettingsTabView indexOfTabViewItem:[uiSettingsTabView selectedTabViewItem]] == 0)
	{
		
		// If the current tab view is the movie settings view, 
		// get the movie gain and update the gain slider
		(void) QTGetMovieProperty([[self movie] quickTimeMovie], 
										kQTPropertyClass_Audio, 
										kQTAudioPropertyID_Gain, 
										sizeof(gain), &gain, NULL);
		[uiMovieGainSlider setFloatValue:(float)gain];	
	}
}

- (void)synchronizeChannelLabelOptionsMenu
{
	// Populates the channel label options array
	// with options that make sense for the selected track
	NSMenu *menu = [[NSMenu alloc] init];
	[self getNewChannelLabelOptions:menu];
	[self setChannelLabelOptionsMenu:menu];	//update our cached copy
	[menu release];	
}

- (void)getNewChannelLabelOptions:(NSMenu*)menu
{

	NSMenuItem *menuItem;	

	// This tag gets us most of the labels that we are interested in
	AudioChannelLayoutTag	tag = kAudioChannelLayoutTag_MPEG_7_1_A;
	UInt32					layoutSize = 0;
	AudioChannelLayout		*layout = NULL;
	UInt32					channel = 0;
	UInt32					numDiscreteChannels = 0;
	UInt32					numMovieAudioChannels = 0;
	UInt32					numDeviceChannels = 0;
	UInt32					index;
	
	// This array doesn't exist yet, so let's make it
	// it will contain some commonly used channel labels
	if ([mCommonChannelLabelOptions count] == 0) 
	{
		if (noErr != AudioFormatGetPropertyInfo (kAudioFormatProperty_ChannelLayoutForTag, sizeof(AudioChannelLayoutTag), 
												&tag, &layoutSize))
		{
			goto bail;									
		}
		layout = (AudioChannelLayout*)malloc(layoutSize);
		if (noErr != AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutForTag, sizeof(AudioChannelLayoutTag), 
												&tag, &layoutSize, layout))
		{
			goto bail;
		}										
		if (noErr != expandChannelLayout(&layout, &layoutSize))
		{
			goto bail;
		}	
		// Commonly used labels 
		for (channel = 0 ; channel < layout->mNumberChannelDescriptions; channel++)
		{
			AudioChannelLabel thisLabel = (AudioChannelLabel)(layout->mChannelDescriptions[channel]).mChannelLabel;
			[mCommonChannelLabelOptions addObject:[NSNumber numberWithInt:thisLabel]];
		}
		
		// Center Surround, Mono, Unused
		[mCommonChannelLabelOptions addObject:[NSNumber numberWithInt:kAudioChannelLabel_CenterSurround]];
		[mCommonChannelLabelOptions addObject:[NSNumber numberWithInt:kAudioChannelLabel_Mono]];
		[mCommonChannelLabelOptions addObject:[NSNumber numberWithInt:kAudioChannelLabel_Unused]];
	}
	
	// Add the commonly used labels to our menu
	for (index = 0; index < [mCommonChannelLabelOptions count]; index++)
	{
		menuItem = [[NSMenuItem alloc] init];
		[self fillInMenuItem:menuItem forChannelLabel:(AudioChannelLabel)[[mCommonChannelLabelOptions objectAtIndex:index] intValue]];
		[menu addItem:menuItem];
		[menuItem release];

	}
	 
	// Now add discrete channel labels to our menu
	// The number of discrete channels is the the greater number between 
	// the total number of device channels and the total number of discrete
	// movie channels	
	getDeviceNumberOfChannels([[self movie] quickTimeMovie], &numDeviceChannels);
	getNumMovieAudioChannels([[self movie] quickTimeMovie], &numMovieAudioChannels);
	numDiscreteChannels = (numMovieAudioChannels > numDeviceChannels) ? numMovieAudioChannels : numDeviceChannels;
	
	for (index = 0; index < numDiscreteChannels; index++)
	{
		menuItem = [[NSMenuItem alloc] init];
		[self fillInMenuItem:menuItem forChannelLabel:(AudioChannelLabel)((1L<<16) | index)];
		[menu addItem:menuItem];
		[menuItem release];
	}
	
bail:
	if (layout)
	{
		free(layout);
	}	
}

// Fills in a menu item with the channel name correspoding to the AudioChannelLabel
// specified. Also sets the menu item's tag to the AudioChannelLabel
- (void) fillInMenuItem:(NSMenuItem*)theMenuItem forChannelLabel:(AudioChannelLabel)theLabel
{
	AudioChannelDescription acd = {0};
	NSString* channelLabelStr;	
	UInt32 channelLabelStringSize = sizeof(NSString*);

	acd.mChannelLabel = theLabel;
	// Get the name for this channel
	if (noErr == AudioFormatGetProperty(kAudioFormatProperty_ChannelName, 
								sizeof(AudioChannelDescription),
								&acd,
								&channelLabelStringSize,
								&channelLabelStr))
	{						
		[theMenuItem setTitle:channelLabelStr];			// title = The channel name string
		[theMenuItem setTag:(int)acd.mChannelLabel];	// tag = AudioChannelLabel			
	}
	[channelLabelStr release];
}	

#pragma mark
#pragma mark ---- Wrappers to C Routines ----

// The movie gain property listener callback is called
// whenever the movie gain changes
- (void)addMovieGainPropertyListener:(QTMovie*)movie
{
	(void)QTAddMoviePropertyListener([movie quickTimeMovie],
									kQTPropertyClass_Audio, 
									kQTAudioPropertyID_Gain,
									(QTMoviePropertyListenerUPP)&movieGainChangeCallback, 
									self);

}

- (void)removeMovieGainPropertyListener:(QTMovie*)movie
{
	QTRemoveMoviePropertyListener([movie quickTimeMovie],
									kQTPropertyClass_Audio, 
									kQTAudioPropertyID_Gain,
									(QTMoviePropertyListenerUPP)&movieGainChangeCallback, 
									self);
}

#pragma mark
#pragma mark ---- NSTableView DataSource Methods ----------

-(int) numberOfRowsInTableView:(NSTableView*)tableView
{
	UInt32 numberOfRows = 0;
	
	if ( [mMovieDocument movie] ) 
	{
		if ( [tableView isEqual:uiTrackTableView] ) 
		{
			// Track Table
			UInt32	index;
			NSArray	*arrayOfMovieTracks = [(QTMovie*)[self movie] tracks];	
					
			[mAudioTracks removeAllObjects];
			for (index = 0; index < [arrayOfMovieTracks count]; index++) 
			{
				// Look for audio tracks that mix into the audio context 
				// (no streaming or MPEG tracks)
				if (trackMixesToAudioContext([[arrayOfMovieTracks objectAtIndex:index] quickTimeTrack])) 
				{
					[mAudioTracks addObject:[arrayOfMovieTracks objectAtIndex:index]];
				}
			}		
			// No. of rows = no. of tracks that mix into audio context + 1 (for the movie info) 
			numberOfRows = [mAudioTracks count] + 1;
			
		} else if ([tableView isEqual:uiTrackChannelLayoutTableView]) 
		{
			// Track Channel Layout table	
			QTTrack *selectedTrack = nil;
			AudioChannelLayout *trackLayout = NULL; 
			UInt32 trackLayoutSize = 0;
			int indexIntoTrackArray = [uiTrackTableView selectedRow] - 1;	//offset by one, since first row contains movie info
			
			if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count]) 
			{
				selectedTrack = [mAudioTracks objectAtIndex:indexIntoTrackArray];
				if (noErr == getTrackLayoutAndSize([selectedTrack quickTimeTrack], &trackLayoutSize, &trackLayout))
				{
					// No. of rows = no. of channels that the currently selected track has
					UInt32 numberOfRowsSize = sizeof(numberOfRows);
					(void) AudioFormatGetProperty(kAudioFormatProperty_NumberOfChannelsForLayout, 
												  trackLayoutSize, 
												  trackLayout, 
												  &numberOfRowsSize, 
												  &numberOfRows);	
				}
				if (trackLayout) 
				{
					free(trackLayout);
				}	
			}
		}
	}
	return numberOfRows;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	id objectValue = nil; 
	NSString *column_id = [tableColumn identifier];

	if ( [tableView isEqual:uiTrackTableView] ) 
	{
		// Track table 
		if ( [column_id isEqualToString:@"enabled"]) 
		{
			if (row == 0) 
			{
				objectValue = [NSString stringWithFormat:@""];
			} 
			else 
			{
				int indexIntoTrackArray = row-1;	//account for the fact that the first row in table corresponds to movie info
				if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count]) 
				{
					objectValue = (NSNumber*)[[mAudioTracks objectAtIndex:indexIntoTrackArray] attributeForKey:QTTrackEnabledAttribute];
				}
			}
		} 
		else if ( [column_id isEqualToString:@"name"]) 
		{
			if (row == 0) 
			{
				objectValue = (NSString*)[[self movie] attributeForKey:QTMovieDisplayNameAttribute];
			} 
			else 
			{
				int indexIntoTrackArray = row-1;	//account for the fact that the first row in table corresponds to movie info
				if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count]) 
				{
					objectValue = (NSString*)[[mAudioTracks objectAtIndex:indexIntoTrackArray] attributeForKey:QTTrackDisplayNameAttribute];
				}
			}
		} 
	}
	else if ([tableView isEqual:uiTrackChannelLayoutTableView]) 
	{
		// Track Channel Layout table
		QTTrack	 *selectedTrack = NULL;
		UInt32 trackLayoutSize = 0;
		AudioChannelLayout *trackLayout = NULL;
		int indexIntoTrackArray;	
		
		if ([column_id isEqualToString:@"channel"]) 
		{
			objectValue = [NSNumber numberWithInt:row+1];	// channel numbers are 1-based
		} 
		else if ([column_id isEqualToString:@"assignment"]) 
		{
			indexIntoTrackArray = [uiTrackTableView selectedRow] - 1;	// offset by one since the track table first row contains movie info
			if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count])
			{
				selectedTrack = [mAudioTracks objectAtIndex:indexIntoTrackArray];
				if (noErr == getTrackLayoutAndSize([selectedTrack quickTimeTrack], &trackLayoutSize, &trackLayout)) 	
				{
					int indexIntoMenu;
					AudioChannelLabel theChannelLabel = trackLayout->mChannelDescriptions[row].mChannelLabel;
					for (indexIntoMenu = 0; indexIntoMenu < [mChannelLabelOptionsMenu numberOfItems]; indexIntoMenu++)
					{
						if (theChannelLabel == (AudioChannelLabel)[[mChannelLabelOptionsMenu itemAtIndex:indexIntoMenu] tag]) 
						{
							objectValue = [NSNumber numberWithInt:indexIntoMenu];
						}
					}
				}
				if (trackLayout) 
				{
					free(trackLayout);
				}	
			} 			
		}
	}
	return objectValue;
}

-(void) tableView:(NSTableView*)tableView setObjectValue:(id)value forTableColumn:(NSTableColumn*)tableColumn row:(int)row
{
	NSString *column_id = [tableColumn identifier];

	if ( [tableView isEqual:uiTrackTableView] ) 
	{
		// Track table
		if ([column_id isEqualToString:@"enabled"]) 
		{
			int indexIntoTrackArray = row-1;
			if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count]) 
			{
				[[mAudioTracks objectAtIndex:indexIntoTrackArray] setAttribute:value forKey:QTTrackEnabledAttribute];
				[[NSNotificationCenter defaultCenter] postNotificationName:QTAudioContextInsertMovieTracksChangedNotification object:[mMovieDocument movie]];
				// Enabling/disabling a track changes the discrete channel label
				// options, so update these options, and reload the channel layout table
				[self synchronizeChannelLabelOptionsMenu];
				[uiTrackChannelLayoutTableView reloadData];
			}
		}
		
	}
	else if ([tableView isEqual:uiTrackChannelLayoutTableView]) 
	{
		// Track Channel Layout table
		if ([column_id isEqualToString:@"assignment"]) 
		{
			int indexIntoMenu;
			AudioChannelLabel newLabel;			
			QTTrack	 *selectedTrack = NULL;
			UInt32 trackLayoutSize = 0;
			AudioChannelLayout *trackLayout = NULL;
			int indexIntoTrackArray;	
			
			indexIntoMenu = [(NSNumber*)value intValue];
			newLabel = (AudioChannelLabel)[[mChannelLabelOptionsMenu itemAtIndex:indexIntoMenu] tag];	
			
			indexIntoTrackArray = [uiTrackTableView selectedRow] - 1;	// offset by one since the track table first row contains movie info
			if (indexIntoTrackArray >= 0 && indexIntoTrackArray < (int)[mAudioTracks count])
			{
				selectedTrack = [mAudioTracks objectAtIndex:indexIntoTrackArray];
				if (noErr == getTrackLayoutAndSize([selectedTrack quickTimeTrack], &trackLayoutSize, &trackLayout)) 
				{
					trackLayout->mChannelDescriptions[row].mChannelLabel = newLabel;
					if (noErr == setTrackLayout([selectedTrack quickTimeTrack],  trackLayoutSize, trackLayout))
					{
						[[NSNotificationCenter defaultCenter] postNotificationName:QTAudioContextInsertMovieTracksChangedNotification object:[mMovieDocument movie]];	
					}
				}
				if (trackLayout) 
				{
					free(trackLayout);
				}	
			}
		}
	}
}

#pragma mark
#pragma mark ---- NSTableColumn Delegate Method ----

	// Delegate method for custom NSTableColumn 
- (id)dataCellForRow:(int)row forTable:(NSTableView*)tableView
{
	id dataCell = nil;
	
	if ( [tableView isEqual:uiTrackTableView] ) 
	{
		// Track Table: Col 1 is a custom column. Its data cells are check-boxes.
		// However, the first row in the table shouldn't have a check-box cell.
		if (row == 0) 
		{
			dataCell = [[NSCell alloc] initTextCell:@""];
		} 
		else 
		{
			dataCell = [[NSButtonCell alloc] init];
			[dataCell setButtonType:NSSwitchButton];
			[dataCell setTitle:@""];
		}		
		[dataCell setControlSize:NSMiniControlSize];
		[dataCell setAlignment:NSCenterTextAlignment];

	} 
	else if ([tableView isEqual:uiTrackChannelLayoutTableView]) 
	{
		// Track Channel Layout Table: Col 2 is a custom column. 
		// Its cells are pop-up buttons, containing a menu of channel
		// label options
		dataCell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:NO];
		[dataCell setControlSize:NSMiniControlSize];
		[dataCell setFont:[NSFont menuFontOfSize:10]];
		[dataCell setMenu:mChannelLabelOptionsMenu];

	}
	return dataCell;
}


#pragma mark
#pragma mark ---- Notifications ----

- (void) windowWillClose:(NSNotification*)notification
{
	NSWindowController *controller = [[notification object] windowController];
	if ([controller isKindOfClass:[MovieWindowController class]]) 
	{
		// A movie window is being closed
		if ([controller window] == [[mMovieDocument movieWindowController] window])
		{
			// If that window belongs to the movie we're inspecting,
			// then close our own window
			[[self window] close];
		}
	}
}

-(void)tableViewSelectionChanged:(NSNotification*)notification
{
	NSTableView *changedTableView = [notification object];
	if ( [changedTableView isEqual:uiTrackTableView]) 
	{
		// A new row was selected in the Track Table
		// We must update the settings view 
		[self synchronizeSettingsView];
	}
}

		
@end

static void movieGainChangeCallback(Movie m, QTPropertyClass propClass, QTPropertyID propID, id observer)
{
	if ([observer isKindOfClass:[AudioPropertiesInspectorController class]]) 
	{
		AudioPropertiesInspectorController *trackInspector = (AudioPropertiesInspectorController*)observer;
		if ([[trackInspector movie] quickTimeMovie] == m &&
				propClass == kQTPropertyClass_Audio &&
				propID == kQTAudioPropertyID_Gain) 
		{
			[trackInspector synchronizeMovieGainSlider];
		}
	}
}