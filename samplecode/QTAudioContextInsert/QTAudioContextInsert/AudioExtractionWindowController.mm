/*
	File:		AudioExtractionWindowController.mm	

	Abstract:   Implements the Audio Extraction Window.
					
				1. Demonstrates the opening and configuration of an audio 
				extraction session (setting of layout, start time, duration,
				etc.) and how to preview and perform extraction on a worker
				thread, or alternatively, on a main thread.
				
				2. Demonstrates how an Audio Context Insert can be used 
				during extraction.
				
				3. Also shows how CoreAudio can be used for playback of 
				the extracted audio.
				
				
	Version:	1.0
				Contains modifies versions of code that is part of the
				QTAudioExtractionPanel sample code project, available
				online and introduced originally at WWDC 2005 Session 201:
				"Harnessing the Audio Capabilities of QuickTime 7" 

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

#import "AudioExtractionWindowController.h"

// Conditions for NSConditionLock
#define AUDIO_SLICE_RENDERED			1
#define WAIT_FOR_AUDIO_SLICE_RENDER		2

// Maximum size, in frames, of the MovieAudioExtractionFillBuffer calls
#define kMaxExtractionPacketCount		(4096)

NSString *QTAudioContextInsertMovieTracksChangedNotification = @"QTAudioContextInsertMovieTracksChanged";

// AUScheduledSoundPlayer callback called when a slice of scheduled audio has been played
// Used during extraction preview
static void previewAudioSliceCompletionProc(void *userData, 
												struct ScheduledAudioSlice *bufferList);
																									
@implementation AudioExtractionController

#pragma mark
#pragma mark ---- Init, Dealloc, Post-Nib Loading ---
- (id) init
{
	[super initWithWindowNibName:@"AudioExtractionWindow"];
	if (self) 
	{
		mMovieDocument = [[NSDocumentController sharedDocumentController] currentDocument];
		mClonedMovie = nil;
		[self setExtractionTime:nil isStartTime:YES isInit:YES];
		[self setExtractionTime:nil isStartTime:NO isInit:YES];
		mCommonChannelLabelOptions = [[NSMutableArray alloc] init];
		mSummaryLayout = NULL;
		mExtractionLayout = NULL;	
		mGraphUnit = nil;
		mExportFileID = nil;
	}
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	[mCommonChannelLabelOptions release];	
	if (mSummaryLayout) free(mSummaryLayout);
	if (mExtractionLayout) free(mExtractionLayout);
	[mCurrentExtractStartTime release];
	[mCurrentExtractEndTime release];
	[mClonedMovie release];
    [super dealloc];
}

- (void)awakeFromNib
{
	// Summary ASBD of the movie that
	// this window is associated with
	[self getSummaryASBD];
	
	// UI set-up
	[self populateExtractChannelsSelectorPopUpButton];
	[self refreshExtractionTableView];
	[self setExtractionTime:nil isStartTime:YES isInit:YES];
	[self setExtractionTime:nil isStartTime:NO isInit:YES];
}

- (void)windowDidLoad 
{
	[super windowDidLoad];
	
	[[self window] setTitle:[NSString stringWithFormat:@"Audio Extraction Panel : %@", (NSString*)[[self movie] attributeForKey:QTMovieDisplayNameAttribute]]];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(movieTracksChanged:) name:QTAudioContextInsertMovieTracksChangedNotification object:nil];

}

#pragma mark
#pragma mark ---- IB actions ----

// Action method invoked when user selects an extraction 
// layout in the extraction layout pop-up button menu 
- (IBAction) doSelectExtractionChannelLayout:(id)sender
{
	int		newTag;
	int		index;
	OSStatus	err = noErr;

	if ([mMovieDocument movie] == nil)
	{
		return;
	}
	// The tag is used to identify the AudioChannelLayout selected from
	// amongst the preset options available in the extraction layout 
	// pop-up button's menu
	newTag = [[uiExtractionLayoutSelectorPopUpButton selectedItem] tag];

	// Tag of -1 indicates a custom layout (which doesn't need to do anything).
	if (newTag ==  -1)
	{
		return;
	}
	// If we have a cached extraction layout, toss it and make a new one.
	if (mExtractionLayout != NULL)
	{
		free(mExtractionLayout);
		mExtractionLayout = NULL;
	}

	switch (newTag)
	{
		// Tag of 0 indicates the default (summary) layout
		case 0:
			err = getDefaultExtractionLayout([[mMovieDocument movie] quickTimeMovie], nil, &mExtractionLayout, nil);
			break;

		// Special value indicates All Channels Discrete extraction (no mixing)
		case kAudioChannelLayoutTag_DiscreteInOrder:
			err = getDiscreteExtractionLayout([[mMovieDocument movie] quickTimeMovie], nil, &mExtractionLayout);
			break;

		// Expand the layout tag associated with the menu item into a channel layout with descriptions
		default:
			err = getLayoutForTag((AudioChannelLayoutTag)newTag, nil, &mExtractionLayout);		
			break;
	}
	if (err)
	{
		return;
	}
	// If there is a Custom item in the popup, remove it now,
	// since we have just selected a layout preset.
	index = [uiExtractionLayoutSelectorPopUpButton indexOfItemWithTitle:@"Custom"];
	if (index != -1)
	{
		// We found a Custom item
		// Remove the item and the separator preceding it.
		[[uiExtractionLayoutSelectorPopUpButton menu] removeItemAtIndex:index];
		[[uiExtractionLayoutSelectorPopUpButton menu] removeItemAtIndex:index-1];
	}				

	// Since the extraction layout has changed, reload
	// the extraction channel layout table. 
	[uiExtractionChannelLayoutTableView reloadData];
}


// Action method invoked by the Preview Start button
- (IBAction) doStartPreview:(id)sender
{
	// Stop playback of current movie, so we can hear the preview
	[[mMovieDocument movie] stop];
	
	// If we are currently exporting, signal the export to stop.
	// We are allowed only one extraction session per movie.
	// If we are exporting, stop and wait for it to complete.
	if (mExportFileID != nil)
	{
		mStopExport = true;
		do 
		{
			[[NSRunLoop currentRunLoop] runUntilDate:nil];
		} while (mExportFileID != nil);
	}
	
	// Extract and play audio, on another thread if possible.
	// Else, fall back to playing on main thread.
	[self startPreview];
	
	// change the button state
	[sender setTitle:@"Stop Preview"];
	[sender setAction:@selector(doStopPreview:)];				
}

// Action method invoked by the Preview Stop button
- (IBAction) doStopPreview:(id)sender
{
	// Set the stop flag.  The rest of the cleanup will occur in previewCompletedNotification.
	mStopPreview = true;
}

// Notification for the Export Start button
- (IBAction) doStartExport:(id)sender
{
	NSSavePanel *savePanel;
	
	savePanel = [NSSavePanel savePanel];
	[savePanel setRequiredFileType:@"aiff"];

	// If we are previewing, stop and wait for it to complete.
	// We are allowed only one extraction session per movie,
	// so can't preview and extract at the same time.
	if (mGraphUnit != nil)
	{
		mStopPreview = true;
		do 
		{
			[[NSRunLoop currentRunLoop] runUntilDate:nil];
		} while (mGraphUnit != nil);
	}
	
    // Open a save panel to get a target file specification.
	// startExport will be invoked when 'OK' is pressed.
	[savePanel beginSheetForDirectory:nil
									file:nil
									modalForWindow:[self window]
									modalDelegate:self
									didEndSelector:@selector(startExport: returnCode: contextInfo:)
									contextInfo:nil];
}

// Action method for the Export Stop button
- (IBAction) doStopExport:(id)sender
{
	// Set the stop flag.  The rest of the cleanup will occur in exportCompletedNotification.
	mStopExport = true;
}


// Action method for text entry in the Start Time and End Time fields
- (IBAction) doChangeExtractionTime:(id)sender
{
	if ([mMovieDocument movie] == nil)
	{
		return;
	}
	QTTime startLimit;
	QTTime endLimit;
	QTTime theTime;

	// Set the valid ranges according to the field we are setting.
	if (sender == uiExtractionStartTimeTextField)
	{
		// Start Time can range from 0 to the just before the current End Time
		startLimit = QTMakeTime(0L, (long) mSummaryASBD.mSampleRate);
		endLimit = [mCurrentExtractEndTime QTTimeValue];
		endLimit.timeValue -= 1;
	}
	else // sender == uiExtractionEndTimeTextField
	{
		// End Time can range from just after the current Start Time to the end of the movie
		startLimit = [mCurrentExtractStartTime QTTimeValue];
		startLimit.timeValue += 1;
		endLimit = [[[mMovieDocument movie] attributeForKey:QTMovieDurationAttribute] QTTimeValue];
	}

	// Parse the user's string into a QTTime
	theTime = [self QTTimeFromString:[sender stringValue] timeScale:startLimit.timeScale];

	// If the time is not in the valid range, reset the text item from the value.
	if ((QTTimeCompare(theTime, startLimit) == NSOrderedAscending)
		/* || (QTTimeCompare(theTime, endLimit) == NSOrderedDescending) */)
	{
		if (sender == uiExtractionStartTimeTextField)
		{
			// Set the Start Time back to where it was.
			[self setExtractionTime:mCurrentExtractStartTime isStartTime:YES isInit:NO];
		}
		else if (sender == uiExtractionEndTimeTextField)
		{	// Re-init the End Time to the duration of the movie.
			[self setExtractionTime:mCurrentExtractEndTime isStartTime:NO isInit:YES];
		}	
	} 
	else 
	{
		// Set the current field to the parsed value.
		[self setExtractionTime:[NSValue valueWithQTTime:theTime]
					isStartTime:(sender == uiExtractionStartTimeTextField) isInit:NO];
	}	
}

#pragma mark
#pragma mark ---- Getter ----
- (QTMovie *)movie 
{
	return [mMovieDocument movie];
}

#pragma mark
#pragma mark ---- Setters ----

// Set the extraction start and end time textfields, and update the cached start and end times
// If isInit, a default start time of 0 and end time of duration is set
// Else, the time is set to theTimeValue that is passed to this method
- (void) setExtractionTime:(NSValue *)theTimeValue isStartTime:(BOOL)isStart isInit:(BOOL)isInit
{
	[uiExtractionEndTimeTextField setStringValue:@""];
	
	if ([mMovieDocument movie] == nil)
	{
		return;
	}
	// Set to default start and end times
	if (isInit) 
	{
		// start time text field
		if (isStart) 
		{
			QTTime startTime = QTMakeTime(0L, (long) mSummaryASBD.mSampleRate);
			[mCurrentExtractStartTime release];
			mCurrentExtractStartTime = [[NSValue valueWithQTTime:startTime] retain];
			[uiExtractionStartTimeTextField setStringValue:[self StringFromQTTime:startTime]];
		} 
		else  
		{ // end time text field
			if ([[mMovieDocument movie] attributeForKey:QTMovieHasDurationAttribute])
			{
				QTTime endTime = [[[mMovieDocument movie] attributeForKey:QTMovieDurationAttribute] QTTimeValue];
				[mCurrentExtractEndTime release];
				mCurrentExtractEndTime = [[NSValue valueWithQTTime:endTime] retain];
				[uiExtractionEndTimeTextField setStringValue:[self StringFromQTTime:endTime]];
			}
			mUseExtractionEndTime = false;
		}	
	} 
	// Set to the time that is passed to this method
	else  
	{
		// start time text field
		if (isStart) 
		{
			if (theTimeValue == nil)
			{
				return;
			}
			[theTimeValue retain];
			[mCurrentExtractStartTime release];
			mCurrentExtractStartTime = theTimeValue; 
			[uiExtractionStartTimeTextField setStringValue:[self StringFromQTTime:[mCurrentExtractStartTime QTTimeValue]]];
		} 
		else 
		{ // end time text field
			if (theTimeValue == nil)
			{
				return;
			}
			[theTimeValue retain];
			[mCurrentExtractEndTime release];
			mCurrentExtractEndTime = theTimeValue; 
			[uiExtractionEndTimeTextField setStringValue:[self StringFromQTTime:[mCurrentExtractEndTime QTTimeValue]]];
			mUseExtractionEndTime = true;
		}
	}
}

// Make a string representation of the time 
// represented by a QTTime
- (NSString*) StringFromQTTime:(QTTime) time
{
    NSString 	*timeString = nil;
	
    // construct the time string
    if ((time.flags & kQTTimeIsIndefinite) == 0)
    {
        NSString* sign = @"";
        if (time.timeValue < 0) 
		{
            sign = @"-";
            time.timeValue = -time.timeValue;
        }
		
        // calculate the time in days, hours, minutes, seconds
        long long divisor = 24LL * 60LL * 60LL * (long long)time.timeScale;
        
        long long days = time.timeValue / divisor;
        time.timeValue -= days * divisor;
		
        divisor /= 24;
        long long hours = time.timeValue / divisor;
        time.timeValue -= hours * divisor;
		
        divisor /= 60;
        long long minutes = time.timeValue / divisor;
        time.timeValue -= minutes * divisor;
		
        divisor /= 60;
        double seconds = (double) time.timeValue / (double) divisor;
		
        // note that we don't care how many digits the day & timescale is
        timeString = [NSString stringWithFormat:@"%@%lld:%02lld:%02lld:%02.2f",
            sign, days, hours, minutes, seconds];
    }
    return timeString;
}

// Construct a QTTime representation of timeString 
- (QTTime) QTTimeFromString:(NSString*)timeString timeScale:(long)scale
{
	NSScanner		*scanner;
    NSString		*string;
    NSTimeInterval 	times[3] = {0};
	double			secs, tempsecs;
    QTTime			rettime;

	// init
	scanner = [NSScanner scannerWithString:timeString];
	[scanner setCharactersToBeSkipped:[NSCharacterSet characterSetWithCharactersInString:@":"]];
		
	// parse the time (days:hr:min:sec.ttt)
	if ([scanner scanUpToString:@":" intoString:(&string)] && ![scanner isAtEnd])
	{
		times[0] = [string intValue];

		if ([scanner scanUpToString:@":" intoString:(&string)] && ![scanner isAtEnd])
		{
			times[1] = [string intValue];

			if ([scanner scanUpToString:@":" intoString:(&string)] && ![scanner isAtEnd])
			{
				times[2] = [string intValue];
				times[1] *= 60;					// hrs to minutes
				times[0] *= 60 * 24;			// days to minutes
				times[0] += times[1] + times[2];
			}
			else
			{
				times[0] *= 60;					// hrs to minutes
				times[0] += times[1];
			}
		}
	}
	secs = (double) times[0] * 60.;				// minutes to seconds

	// If we consumed everything, the last string was our sec.ttt segment,
	// so reset the scanner to parse just that.
	if ([scanner isAtEnd])
	{
		scanner = [NSScanner scannerWithString:string];
	}	
	if ([scanner scanDouble:&tempsecs])
	{
		secs += tempsecs;
	}
	
	rettime.timeValue = (long long)(secs * (double) scale);
    rettime.timeScale = scale;
	rettime.flags = 0;
	
    return rettime;
}

#pragma mark
#pragma mark ---- UI refresh/population ----

// Populate the extraction layout pop-up button with a menu that
// contains some commonly used AudioChannelLayout options as presets
-(void) populateExtractChannelsSelectorPopUpButton
{
	[uiExtractionLayoutSelectorPopUpButton setAutoenablesItems:NO];
	[uiExtractionLayoutSelectorPopUpButton removeAllItems];

	if ([mMovieDocument movie] == nil)
	{
		return;
	}
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"Mono"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_Mono];
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"Stereo (L R)"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_Stereo];
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"Quadraphonic (L R Ls Rs)"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_Quadraphonic];
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"5.0 (L R C Ls Rs)"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_MPEG_5_0_A];
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"5.1 (L R C LFE Ls Rs)"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_MPEG_5_1_A];
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"7.1 (L R C LFE Ls Rs Lc Rc)"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_MPEG_7_1_A];
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"All Discrete"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)kAudioChannelLayoutTag_DiscreteInOrder];
	// Use tag == 0 to flag the default (summary) layout.
	// We will use tag == -1 to flag a custom layout.
	[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"Default"];
	[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:(int)0];

	[uiExtractionLayoutSelectorPopUpButton selectItem:[uiExtractionLayoutSelectorPopUpButton lastItem]];

}

// Refresh the extraction layout table if the underlying channel layouts may have changed
-(void) refreshExtractionTableView
{
	// Throw away the cached extraction layout and reload to refresh it.
	if (mExtractionLayout != nil)
	{
		int	tag;

		// Tag of 0 indicates the default (summary) layout
		// Tag of kAudioChannelLayoutTag_DiscreteInOrder indicates All Channels Discrete extraction
		tag = [[uiExtractionLayoutSelectorPopUpButton selectedItem] tag];
		if ((tag == 0) || (tag == (int)kAudioChannelLayoutTag_DiscreteInOrder))
		{
			free(mExtractionLayout);
			mExtractionLayout = nil;
		}
	}

	if (mExtractionLayout == nil)
	{
		// Refresh the panel if it is being displayed
		[uiExtractionChannelLayoutTableView reloadData];
	}
}


// Get the summary ASBD of the current movie.
-(void)getSummaryASBD
{	
	if ([self movie] == nil)
	{
		return;
	}	
	(void) getMovieSummaryASBD([[self movie] quickTimeMovie], &mSummaryASBD);
}

// Create the channel labels array that is used to populate the
// channel assignment options menu in the extraction layout table
- (void)getCommonChannelLabelOptions
{

	// This tag gets us most of the labels that we are interested in
	AudioChannelLayoutTag	tag = kAudioChannelLayoutTag_MPEG_7_1_A;
	UInt32					layoutSize = 0;
	AudioChannelLayout		*layout = NULL;
	UInt32					channel = 0;	

	// This array doesn't exist yet, so let's make it it will contain some commonly used channel labels
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
		
		// Center Surround
		[mCommonChannelLabelOptions addObject:[NSNumber numberWithInt:kAudioChannelLabel_CenterSurround]];
		
		if (layout) 
		{
			free(layout);
		}	
	}
bail:;	
}

// Fills in a menu item with the channel name correspoding to the AudioChannelLabel
// specified. Also sets the menu item's tag to the AudioChannelLabel.
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
#pragma mark ---- Extraction and Write To File ----------

// Fill in all the parameters necessary to configure an audio extraction
// from the panel UI and cached values.
-(OSStatus) getExtractionParameters:(AudioChannelLayout**)layout 
					outLayoutSize:(UInt32*)layoutSize
					outASBD:(AudioStreamBasicDescription*)asbd 
					startTime:(TimeRecord*)startTime 
					duration:(Float64*)duration 
					allDiscrete:(Boolean*)allDiscrete
					insertRegInfo:(InsertRegistryInfo*)regInfoRef					
{
	OSStatus err = noErr;
	AudioChannelLayoutTag selectedLayoutTag;
	QTTime durationInQTTime;
	TimeRecord durationTimeRecord;
	
	// Set all the parameters to safe values 
	// in the event that this method has to bail on an error
	*layout = nil;
	*layoutSize = 0;
	*duration = 0;	// setting to 0 indicates that the entire movie is to be extracted
	*allDiscrete = NO;
	
	// Get the extraction layout that has been selected in the extraction layout pop-up,
	// just to check for All Channels Discrete.
	selectedLayoutTag = (AudioChannelLayoutTag)[[uiExtractionLayoutSelectorPopUpButton selectedItem] tag];

	// Start Time
	(void) QTGetTimeRecord([mCurrentExtractStartTime QTTimeValue], startTime);
	
	// Duration
	// Just extract the entire file if the UI end time wasn't changed.
	if (mUseExtractionEndTime) {
		durationInQTTime = QTTimeDecrement([mCurrentExtractEndTime QTTimeValue], [mCurrentExtractStartTime QTTimeValue]);
		(void) QTGetTimeRecord(durationInQTTime, &durationTimeRecord);
		
		// Convert to floating-point seconds
		*duration = *((TimeValue64*) &durationTimeRecord.value) / (Float64) durationTimeRecord.scale;
	}

	// ASBD
	*asbd = mSummaryASBD;
	asbd->mChannelsPerFrame = mExtractionLayout->mNumberChannelDescriptions;

	// All channels discrete 
	if (selectedLayoutTag == kAudioChannelLayoutTag_DiscreteInOrder)
	{
		*allDiscrete = YES;
	}
	// Create an Audio Context insert processor for use during extraction
	// The registration information returned is set as a property on the extraction sesssion during MovieAudioExtraction
	// session configuration
	[[mMovieDocument acInsertManager] createProcessorForExtractionAndGetRegistrationInfo:(InsertRegistryInfo*)regInfoRef];
		
	// Return a copy of the cached extraction layout, which should always be current
	*layoutSize = fieldOffset(AudioChannelLayout, mChannelDescriptions[mExtractionLayout->mNumberChannelDescriptions]);
	*layout = (AudioChannelLayout*) calloc(1, *layoutSize);
	if (*layout == nil) 
	{
		goto bail;
	}
	memcpy(*layout, mExtractionLayout, *layoutSize);

bail:
	if (err)
	{
		if (*layout)
		{
			free(*layout);
		}
		if (regInfoRef)
		{
			if (regInfoRef->theInsertRegistryInfo) 
			{
				free(regInfoRef->theInsertRegistryInfo);
			}
		}	
	}
	return (err);
}

// This method checks if the export can be done on a worker thread. If so, it spawns a thread, 
// else it does the export on the main thread
- (void) startExport:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	AudioStreamBasicDescription asbd;
	AudioChannelLayout *layout = nil;
	UInt32 layoutSize = 0;
	TimeRecord startTime;
	Boolean discrete = NO;
	InsertRegistryInfo regInfo = { 0 };	
	Float64 duration = 0.;					// extraction duration, in seconds
	Boolean extractionOnWorkerThread = NO;
	FSRef parentRef, fileRef;
	UInt32 fileType = kAudioFileAIFFType;
	NSString *directory = [[[NSString alloc] initWithString:[sheet directory]] retain]; 
	NSString *fileName = [[[NSString alloc] initWithString:[[sheet filename] lastPathComponent ]] retain];	
	InfoForCallback *info = [[InfoForCallback alloc] init];
	OSStatus err = noErr;

    if (returnCode == NSOKButton)
	{ 
		// Close the Save As sheet
		[sheet close];

		// Change the button state
		[uiExtractionExportButton setTitle:@"Stop Export"];
		[uiExtractionExportButton setAction:@selector(doStopExport:)];				
			
		//-------------------------------------------------------
		
		// Clone the movie, test if clone can be migrated to a 
		// worker thread for extraction, and set a flag accordingly 
		
		mStopExport = false;	// when this is set true, export stops on the next cycle

		mClonedMovie = [[mMovieDocument movie] copyWithZone:nil];
		if (!mClonedMovie)
		{
			err = memFullErr;
			goto bail;
		}
		if ([mClonedMovie detachFromCurrentThread])
		{
			extractionOnWorkerThread = YES;
		}
		else
		{
			// If we could not migrate this movie, dispose the clone and
			// export from the original movie on the main thread.
			[mClonedMovie release];
			mClonedMovie = nil;
		}

		// Read the UI, get the required extraction layout, layoutsize, 
		// extraction startTime, duration and whether we need to 
		// extract in the "All Channels Discrete" mode
		err = [self getExtractionParameters:(AudioChannelLayout**)&layout 
											outLayoutSize:(UInt32*)&layoutSize
											outASBD:&asbd
											startTime:(TimeRecord*)&startTime 
											duration:(Float64*)&duration 
											allDiscrete:(Boolean*)&discrete
											insertRegInfo:(InsertRegistryInfo*)&regInfo];
		if (err)
		{
			goto bail;
		}
		// Set the output ASBD to 16-bit interleaved PCM big-endian integers
		asbd.mFormatID = kAudioFormatLinearPCM;
		asbd.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger |
								kAudioFormatFlagIsBigEndian |
								kAudioFormatFlagIsPacked;
		asbd.mFramesPerPacket = 1;
		asbd.mBitsPerChannel = 16;
		asbd.mBytesPerFrame = 2 * asbd.mChannelsPerFrame;
		asbd.mBytesPerPacket = 2 * asbd.mChannelsPerFrame;

		//---------------------------------------------------------------
		// Open the AIFF file, configure it for out extraction output layout
		// If the file we want to save to exists, delete it
		err = FSPathMakeRef((const UInt8*)[[sheet filename] fileSystemRepresentation], &fileRef, NULL);
		if (err == noErr) 
		{
			FSDeleteObject(&fileRef);
		}	
		FSPathMakeRef((const UInt8*)[directory fileSystemRepresentation], &parentRef, NULL);
		err = AudioFileCreate(&parentRef, (CFStringRef)fileName, fileType, &asbd, 0, &fileRef, &mExportFileID);
		if (err) 
		{
			goto bail;
		}

		// If we do an All Channels Discrete extraction, create an unlabeled multi-channel file.
		// Otherwise, set the channel labels that we've specified for the extraction.
		if (!discrete)
		{
			err = AudioFileSetProperty(mExportFileID, 
										kAudioFilePropertyChannelLayout,
										layoutSize,
										(void*) layout);
			if (err)
			{
				goto bail;
			}	
		}
		//---------------------------------------------------------------------
		
		// We are now ready to extract the audio and write to the AIFF file.
		// Package the extraction information in an object.
		// If we cannot migrate this job to the worker thread, do the extraction
		// on the main thread. Else spawn an extraction thread to do the extraction.
				
		QTTime startTimeInQTTime = QTMakeTimeWithTimeRecord(startTime);
		[info setASBD:asbd];
		[info setDiscrete:discrete];
		[info setLayout:layout];				
		[info setLayoutSize:layoutSize];
		[info setStartTime:startTimeInQTTime];
		[info setSamplesRemaining:duration ? (SInt64)(duration * asbd.mSampleRate) : -1];
		[info setLocationInFile:0];
		[info setInsertRegInfo:regInfo];

		// If we can export on a worker thread, go do it
		if (extractionOnWorkerThread == YES) 
		{
			[NSThread detachNewThreadSelector:@selector(exportExtractionThread:) toTarget:self withObject:info];
		} 
		else 
		{
			// Otherwise, since we're on the main thread we can just call the main-thread worker method
			[self exportOnMainThreadCallBack:(id)info];
		}
	}

bail:
	[info release];
	if (fileName)
	{
		[fileName release];
	}
	if (directory)
	{
		[directory release];
	}
	// If there was an error, we never spawned the worker thread to close the cloned movie
	if (err) 
	{
		if (layout)
		{
			free(layout);
		}
		if (regInfo.theInsertRegistryInfo)
		{
			free(regInfo.theInsertRegistryInfo);
		}
		if (extractionOnWorkerThread == YES) 
		{	
			[mClonedMovie release];
			mClonedMovie = nil;
		}
		if (mExportFileID) 
		{
			(void) AudioFileClose(mExportFileID);
			mExportFileID = nil;
			FSDeleteObject(&fileRef);
		}
	}
}

// Called when export is completed
// Changes the button state
- (void) exportCompletedNotification:(id)object
{
	// Set the Preview button back to its original state
	[uiExtractionExportButton setTitle:@"Export"];
	[uiExtractionExportButton setAction:@selector(doStartExport:)];
}

-(void) exportExtractionThread:(id)theObject
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	OSStatus err = noErr;

	[NSThread setThreadPriority:[NSThread threadPriority]+.1];
	
	// Unpack the information passed to this thread
	InfoForCallback *info = (InfoForCallback*) theObject;
	AudioStreamBasicDescription asbd = [info asbd];
	AudioChannelLayout *layout = [info layout];
	UInt32 layoutSize = [info layoutSize];
	QTTime startTimeInQTTime = [info startTime];
	SInt64	samplesRemaining;
	Boolean discrete = [info discrete]; 
	SInt64 ploc = [info locationInFile];
	TimeRecord startTime;
	MovieAudioExtractionRef extraction = nil;
	InsertRegistryInfo regInfo = [info insertRegInfo];
	
	QTGetTimeRecord(startTimeInQTTime, &startTime);

	Boolean enterMoviesSucceeded=NO, attachedToThread = NO;
	
	// -----------------------------------------------------
	// Attach the movie to this thread.
	[QTMovie enterQTKitOnThread];
	enterMoviesSucceeded = YES;	

	if (![mClonedMovie attachToCurrentThread])
	{
		goto bail;
	}
	attachedToThread = YES;

	// ---------------------------------------------------------
	// Prepare for extraction	
	// Open a session, configure the session
	err = prepareMovieForExtraction([mClonedMovie quickTimeMovie], 
									regInfo,
									&extraction, 
									discrete, 
									asbd, 
									&layout, 
									&layoutSize, 
									startTime,
									&samplesRemaining);
	if ([info samplesRemaining] != -1) {
		samplesRemaining = [info samplesRemaining];
	}
	if (layout) 
	{
		free(layout);
	}		
	if (regInfo.theInsertRegistryInfo)
	{
		free(regInfo.theInsertRegistryInfo);
	}
	if (err)
	{
		goto bail;
	}
	
	//------------------------------------------------------------
	
	// Start exporting slices and loop until everything is written out

	Boolean			extractionComplete;
	extractionComplete = NO;

	// Loop until stopped from an external event, or we've finished the entire extraction
	while (!mStopExport && !extractionComplete)
	{
		// If there are any samples left to export...		
		if (samplesRemaining == 0)
		{
			extractionComplete = YES;
		}
		if (!extractionComplete)
		{
			// We will read numSamplesThisSlice number of samples
			SInt64 numSamplesThisSlice = samplesRemaining;
			if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
			{
				numSamplesThisSlice = kMaxExtractionPacketCount;
			}
			err = extractAudioToFile(extraction, mExportFileID, asbd, &numSamplesThisSlice, &ploc, &extractionComplete);	
			if (err)
			{
				break;
			}
			if (samplesRemaining != -1)
			{
				samplesRemaining -= numSamplesThisSlice;
			}	
		}
	}

bail:
	(void) AudioFileClose(mExportFileID);
	mExportFileID = nil;
	if (extraction)
	{
		(void) MovieAudioExtractionEnd(extraction);
	}
	[mClonedMovie release];
	mClonedMovie = nil;
	if (enterMoviesSucceeded)
	{
		[QTMovie exitQTKitOnThread];
	}	
	// Schedule the completion routine to execute on the main thread
	[self performSelectorOnMainThread:@selector(exportCompletedNotification:)
									withObject:(id)nil
									waitUntilDone:NO];
	[pool release];
}

// This callback is scheduled on the main thread.
// In order to keep from locking up the UI, it does one slice of export,
// writes it to file and then reschedule itself.
-(void) exportOnMainThreadCallBack:(id)object
{
	OSStatus err = noErr;
	
	// Unpack the information passed to this method
	InfoForCallback *info = (InfoForCallback *) object;
	AudioStreamBasicDescription asbd = [info asbd];
	AudioChannelLayout *layout = [info layout];
	UInt32 layoutSize = [info layoutSize];
	QTTime startTimeInQTTime = [info startTime];
	SInt64	samplesRemaining;
	Boolean discrete = [info discrete]; 
	SInt64 ploc = [info locationInFile];
	TimeRecord startTime;
	MovieAudioExtractionRef extraction = [info extractionSessionRef];
	InsertRegistryInfo regInfo = [info insertRegInfo];

	QTGetTimeRecord(startTimeInQTTime, &startTime);

	// ---------------------------------------------------------
	// Prepare for extraction if this is the first entry

	if (extraction == nil)	
	{	
		// Open a session, configure audio format 
		err = prepareMovieForExtraction([[mMovieDocument movie] quickTimeMovie], 
										regInfo,
										&extraction, 
										discrete, 
										asbd, 
										&layout, 
										&layoutSize, 
										startTime,
										&samplesRemaining);						
		if ([info samplesRemaining] != -1) {
			samplesRemaining = [info samplesRemaining];
		}
		if (layout)
		{
			free(layout);
		}
		if (regInfo.theInsertRegistryInfo)
		{
			free(regInfo.theInsertRegistryInfo);
		}
		if (err)
		{
			goto bail;
		}
		// Save for future entries
		[info setExtractionSession:extraction];	
	}

	// Export a slice now and then reschedule for another entry soon.
	Boolean extractionComplete;
	extractionComplete = NO;
	
	if (!mStopExport)
	{
		// If there are any samples left to export...		
		if (samplesRemaining == 0)
		{
			extractionComplete = YES;
		}
		if (!extractionComplete)
		{
			// We will read numSamplesThisSlice number of samples
			SInt64 numSamplesThisSlice = samplesRemaining;
			if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
			{
				numSamplesThisSlice = kMaxExtractionPacketCount;
			}
			err = extractAudioToFile(extraction, mExportFileID, asbd, &numSamplesThisSlice, &ploc, &extractionComplete);	
			if (err)
			{
				goto bail;
			}
			if (samplesRemaining != -1) 
			{
				samplesRemaining -= numSamplesThisSlice;
				if (samplesRemaining == 0)
				{
					extractionComplete = YES;
				}	
			}
		}
	}
	// Save updated state for the next time through this method
	[info setLocationInFile:ploc];
	[info setSamplesRemaining:samplesRemaining];

bail:
	if (err || extractionComplete || mStopExport)
	{
		err = AudioFileClose(mExportFileID);
		mExportFileID = nil;
		if (extraction)
		{
			(void) MovieAudioExtractionEnd(extraction);
		}
		// Call the completion routine to reset the UI
		[self exportCompletedNotification:(id)nil];
	}
	else
	{
		// Reschedule to perform this routine again on the next run loop cycle
		[self performSelectorOnMainThread:@selector(exportOnMainThreadCallBack:)
										withObject:(id)info
										waitUntilDone:NO];
	}
}


#pragma mark
#pragma mark ---- Extraction Preview (Playback through CoreAudio) ------------- 

// Start a preview playback
- (void) startPreview
{
	AudioStreamBasicDescription asbd;
	AudioChannelLayout *layout = nil;
	UInt32 layoutSize = 0;
	TimeRecord startTime;
	Boolean discrete = NO;
	Float64 duration = 0.;
	Boolean extractionOnWorkerThread = NO;
	OSStatus err = noErr;
	InsertRegistryInfo regInfo = { 0 };
	InfoForCallback *info = [[InfoForCallback alloc] init];

	// Clone the movie, test if clone can be migrated to a 
	// worker thread for extraction and playback, and set a flag accordingly

	mStopPreview = false;	// when this is set true, preview stops on the next cycle

	mClonedMovie = [[mMovieDocument movie] copyWithZone:nil];
	if (!mClonedMovie)
	{
		err = memFullErr;
		goto bail;
	}
	if ([mClonedMovie detachFromCurrentThread])
	{
		extractionOnWorkerThread = YES;
	}
	else
	{
		// If we could not migrate this movie, dispose the clone and
		// extract from the original movie on the main thread.
		[mClonedMovie release];
		mClonedMovie = nil;
	}

	// Read the UI, get the required extraction layout, layoutsize, 
	// extraction startTime, duration and whether we need to 
	// extract in the "All Channels Discrete" mode
	err = [self getExtractionParameters:(AudioChannelLayout**)&layout 
										outLayoutSize:(UInt32*)&layoutSize
										outASBD:&asbd
										startTime:(TimeRecord*)&startTime 
										duration:(Float64*)&duration 
										allDiscrete:(Boolean*)&discrete
										insertRegInfo:(InsertRegistryInfo*)&regInfo];
	if (err)
	{
		goto bail;
	}
	// Build an AU Graph with a scheduled sound player unit and
	// an output unit for playback
	err = BuildAUGraphPlayer(layout, layoutSize, &asbd, &mGraphUnit, &mPlayerAudioUnit);
	if (err)
	{
		goto bail;
	}
	// Package the refCon information that will be sent to the preview thread/timer
	QTTime startTimeInQTTime = QTMakeTimeWithTimeRecord(startTime);
	[info setASBD:asbd];
	[info setDiscrete:discrete];
	[info setLayout:layout];				
	[info setLayoutSize:layoutSize];
	[info setStartTime:startTimeInQTTime];
	[info setSamplesRemaining:duration ? (SInt64)(duration * asbd.mSampleRate) : -1];
	[info setInsertRegInfo:regInfo];
	
	// If we can preview on a worker thread, go do it
	if (extractionOnWorkerThread == YES) 
	{
		[NSThread detachNewThreadSelector:@selector(previewExtractionThread:) toTarget:self withObject:info];
	}
	else 
	{
		// Otherwise, since we're on the main thread we can just call the main-thread worker method
		[self previewOnMainThreadCallBack:(id)info];
	}

bail:
	[info release];

	// If there was an error, we never spawned the worker thread to close the cloned movie
	if (err) 
	{
		if (layout)
		{
			free(layout);
		}
		if (regInfo.theInsertRegistryInfo)
		{
			free(regInfo.theInsertRegistryInfo);
		}
		if (extractionOnWorkerThread == YES) 
		{
			[mClonedMovie release];
			mClonedMovie = nil;
		}
	}
}


- (void) previewCompletedNotification:(id)object
{
	// Stop and dispose the playback AudioUnit Graph
	(void) CloseAUGraphPlayer(mGraphUnit);
	mGraphUnit = nil;

	// Set the Preview button back to its original state
	[uiExtractionPreviewButton setTitle:@"Preview"];
	[uiExtractionPreviewButton setAction:@selector(doStartPreview:)];
}


const UInt32 numSlices = 3;		// number of slices we try to keep in the play queue


// Slice Rendered Completion Callback
void previewAudioSliceCompletionProc(void *userData, struct ScheduledAudioSlice *bufferList)
{
	id condLock = (NSConditionLock*)userData;

	// Signal the sleeping preview thread that a slice has been rendered.
	if (condLock != nil) 
	{
		[condLock lock];
		[condLock unlockWithCondition:AUDIO_SLICE_RENDERED];
	}
}										

// This is the method that executes on a spawned thread to do the preview.
- (void) previewExtractionThread:(id)theObject
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	OSStatus err = noErr;

	//[NSThread setThreadPriority:[NSThread threadPriority]+.1];
	id condLock = [[NSConditionLock alloc] initWithCondition:AUDIO_SLICE_RENDERED];

	// Unpack the information passed to this thread
	InfoForCallback *info = (InfoForCallback*) theObject;
	AudioStreamBasicDescription asbd = [info asbd];
	AudioChannelLayout *layout = [info layout];
	UInt32 layoutSize = [info layoutSize];
	QTTime startTimeInQTTime = [info startTime];
	SInt64	samplesRemaining;
	Boolean discrete = [info discrete];
	TimeRecord startTime;
	MovieAudioExtractionRef extraction = nil;
	Boolean			extractionComplete = NO;
	UInt32			numSlicesFree = numSlices;
	ScheduledAudioSlice sliceList[numSlices];
	Float64			sampleTimeStamp = 0.;
	Boolean			playerUnitStarted = false;
	InsertRegistryInfo regInfo = [info insertRegInfo];

	QTGetTimeRecord(startTimeInQTTime, &startTime);

	Boolean enterMoviesSucceeded=NO, attachedToThread = NO;
	
	// -----------------------------------------------------
	// Attach the movie to this thread.
	[QTMovie enterQTKitOnThread];
	enterMoviesSucceeded = YES;	
	
	if (![mClonedMovie attachToCurrentThread])
	{
		goto bail;
	}
	attachedToThread = YES;
	
	// ---------------------------------------------------------
	// Prepare for extraction
	// Open a session, configure audio format 
	err = prepareMovieForExtraction([mClonedMovie quickTimeMovie], 
									regInfo,
									&extraction, 
									discrete, 
									asbd, 
									&layout, 
									&layoutSize, 
									startTime,
									&samplesRemaining);						
	if ([info samplesRemaining] != -1) {
		samplesRemaining = [info samplesRemaining];
	}
	if (layout) 
	{
		free(layout);
	}
	if (regInfo.theInsertRegistryInfo)
	{
		free(regInfo.theInsertRegistryInfo);
	}
	if (err)
	{
		goto bail;
	}
	//-----------------------------------------------------------
	
	// Start scheduling slices and loop until everything is played out
	err = [self previewBufferAllocate:sliceList
										numSlices:numSlices
										asbd:asbd
										lock:(void *)condLock];
	if (err)
	{
		goto bail;
	}
	// Loop until stopped from an external event, or we've played out the entire extraction
	while (!mStopPreview && !(extractionComplete && (numSlicesFree == numSlices)))
	{
		// The first time, this does not block.
		// Thereafter, it blocks until a completion callback has set it.
		// Once acquired, unlock it immediately, but clear the state.
		[condLock lockWhenCondition:AUDIO_SLICE_RENDERED];
		[condLock unlockWithCondition:WAIT_FOR_AUDIO_SLICE_RENDER];

		// Iterate through our slice list.
		// For each completed (or never used) slice, fill and schedule it.
		numSlicesFree = [self previewBufferScheduleSlices:sliceList
											numSlices:(UInt32)numSlices
											extractionSession:(MovieAudioExtractionRef)extraction
											asbd:(AudioStreamBasicDescription)asbd
											timeStamp:(Float64*)&sampleTimeStamp
											remaining:(SInt64*)&samplesRemaining
											complete:(Boolean*)&extractionComplete];

		// If all the slices are free now, we didn't succeed in queueing any more
		if (numSlicesFree == numSlices)
		{
			break;
		}
		// Start the AUGraph, it is wasn't already running.
		// Then the next time around the loop, we'll wait until the slice completion callback
		// has set the condition lock state.
		if (playerUnitStarted == false)
		{
			// Start the AUGraph player
			err = StartAUGraphPlayer(mGraphUnit);
			if (err) 
			{
				goto bail;
			}		
			playerUnitStarted = true;
		}
	}		
		
bail:
	if (extraction)
	{
		(void) MovieAudioExtractionEnd(extraction);
	}
	
	(void) StopAUGraphPlayer(mGraphUnit);
	[self previewBufferDeallocate:sliceList numSlices:numSlices];
	[mClonedMovie release];
	mClonedMovie = nil;
	if (enterMoviesSucceeded)
	{
		[QTMovie exitQTKitOnThread];
	}
	// Schedule the completion routine to execute on the main thread
	[self performSelectorOnMainThread:@selector(previewCompletedNotification:)
									withObject:(id)nil
									waitUntilDone:NO];
	[condLock release];
	[pool release];
}

// This callback is scheduled on the main thread.
// In order to keep from locking up the UI, schedules what it can, and then reschedules itself.
-(void) previewOnMainThreadCallBack:(id)object
{
	OSStatus err = noErr;

	// Unpack the information passed to this thread
	InfoForCallback *info = (InfoForCallback *) object;
	AudioStreamBasicDescription asbd = [info asbd];
	AudioChannelLayout *layout = [info layout];
	UInt32 layoutSize = [info layoutSize];
	QTTime startTimeInQTTime = [info startTime];
	SInt64	samplesRemaining;
	Boolean discrete = [info discrete];
	TimeRecord startTime;
	Boolean			playerUnitStarted = [info playerUnitStarted];
	Float64			sampleTimeStamp = [info sampleTimeStamp];
	ScheduledAudioSlice *sliceList = [info sliceList];
	UInt32			numSlicesFree = 0;
	MovieAudioExtractionRef extraction = [info extractionSessionRef];
	InsertRegistryInfo regInfo = [info insertRegInfo];

	QTGetTimeRecord(startTimeInQTTime, &startTime);

	// ---------------------------------------------------------
	// Prepare for extraction if this is the first entry

	if (extraction == nil)	
	{	
		// Open a session, configure audio format 
		err = prepareMovieForExtraction([[mMovieDocument movie] quickTimeMovie], 
										regInfo,
										&extraction, 
										discrete, 
										asbd, 
										&layout, 
										&layoutSize, 
										startTime,
										&samplesRemaining);						
		if ([info samplesRemaining] != -1) {
			samplesRemaining = [info samplesRemaining];
		}
		if (layout) 
		{
			free(layout);
		}
		if (regInfo.theInsertRegistryInfo)
		{
			free(regInfo.theInsertRegistryInfo);
		}
		if (err)
		{
			goto bail;
		}
		// Save for future entries
		[info setExtractionSession:extraction];	
	}
	if (sliceList == nil)
	{
		sliceList = (ScheduledAudioSlice *) calloc(numSlices, sizeof(ScheduledAudioSlice));
		if (sliceList == nil) 
		{
			err = memFullErr;
			goto bail;
		}
		err = [self previewBufferAllocate:sliceList
									numSlices:numSlices
									asbd:asbd
									lock:(void *)nil];
		if (err)
		{
			goto bail;
		}
		[info setSliceList:sliceList];
	}

	//-----------------------------------------------------------
	
	Boolean			extractionComplete;
	extractionComplete = NO;
	
	if (!mStopPreview)
	{
		// Iterate through our slice list.
		// For each completed (or never used) slice, fill and schedule it.
		numSlicesFree = [self previewBufferScheduleSlices:sliceList
											numSlices:(UInt32)numSlices
											extractionSession:(MovieAudioExtractionRef)extraction
											asbd:(AudioStreamBasicDescription)asbd
											timeStamp:(Float64*)&sampleTimeStamp
											remaining:(SInt64*)&samplesRemaining
											complete:(Boolean*)&extractionComplete];
	}

	if (!playerUnitStarted)
	{
		// Start the AUGraph player
		err = StartAUGraphPlayer(mGraphUnit);
		if (err) 
		{
			goto bail;
		}
		[info setPlayerUnitStarted:YES];
	}

	// Save updated state for the next time through this method
	[info setSamplesRemaining:samplesRemaining];
	[info setSampleTimeStamp:sampleTimeStamp];

	// Set the complete flag if we could not queue anything, for what ever reason
	// (ie, we finished playing everything or we never queued anything).
	extractionComplete = (numSlicesFree == numSlices);

bail:
	if (err || extractionComplete || mStopPreview)
	{
		if (extraction)
		{
			(void) MovieAudioExtractionEnd(extraction);
		}
		(void) StopAUGraphPlayer(mGraphUnit);
		if (sliceList != nil) 
		{
			[self previewBufferDeallocate:sliceList numSlices:numSlices];
			free (sliceList);
		}

		// Call the completion routine to reset the UI
		[self previewCompletedNotification:(id)nil];
	}
	else
	{
		// Reschedule to perform this routine again on the next run loop cycle
		[self performSelectorOnMainThread:@selector(previewOnMainThreadCallBack:)
										withObject:(id)info
										waitUntilDone:NO];
	}
}

// Allocate and initialize audio slice buffers for the AUScheduledSoundPlayer
-(OSStatus) previewBufferAllocate:(ScheduledAudioSlice *)sliceList
									numSlices:(UInt32)numSlices
									asbd:(AudioStreamBasicDescription)asbd
									lock:(void *)condLock
{
	UInt32			sliceNumber;
	OSStatus		err = noErr;

	bzero(sliceList, numSlices * sizeof(ScheduledAudioSlice));

	for (sliceNumber = 0; sliceNumber < numSlices ; sliceNumber++)
	{
		AudioBufferList	*bufList = nil;
		UInt32			bufNumber = 0;
		UInt32			bufSize, listSize;
		UInt32			mallocSize;

		// Accumulate the size of all the memory objects we need for this slice.
		// Then allocate it and parcel it out.
		// Make sure all sizes are rounded up to Altivec boundaries;
		listSize = fieldOffset(AudioBufferList, mBuffers[asbd.mChannelsPerFrame]);
		listSize = ((listSize + 15) / 16) * 16;
		mallocSize = listSize;

		bufSize = kMaxExtractionPacketCount * asbd.mBytesPerPacket;
		bufSize = ((bufSize + 15) / 16) * 16;
		mallocSize += (bufSize * asbd.mChannelsPerFrame);

		bufList = (AudioBufferList*) calloc(1, mallocSize);
		if (bufList == nil) 
		{
			err = memFullErr;
			goto bail;
		}
		bufList->mNumberBuffers = asbd.mChannelsPerFrame;
		for (bufNumber = 0; bufNumber < bufList->mNumberBuffers; bufNumber++)
		{
			bufList->mBuffers[bufNumber].mNumberChannels = 1;
			bufList->mBuffers[bufNumber].mData = (char *) bufList + listSize + (bufNumber * bufSize);
			bufList->mBuffers[bufNumber].mDataByteSize = bufSize;
		}
		sliceList[sliceNumber].mBufferList = bufList;
		sliceList[sliceNumber].mNumberFrames = kMaxExtractionPacketCount;  
		sliceList[sliceNumber].mTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
		sliceList[sliceNumber].mCompletionProcUserData = condLock; 
		sliceList[sliceNumber].mCompletionProc = (ScheduledAudioSliceCompletionProc)
										((condLock == nil) ? nil : previewAudioSliceCompletionProc);
		sliceList[sliceNumber].mFlags = kScheduledAudioSliceFlag_Complete;
		sliceList[sliceNumber].mReserved = 0;
	}
bail:
	return (err);
}

// Free the audio slice buffers
-(void) previewBufferDeallocate:(ScheduledAudioSlice *)sliceList
									numSlices:(UInt32)numSlices
{
	UInt32			sliceNumber;

	for (sliceNumber = 0; sliceNumber < numSlices ; sliceNumber++) 
	{
		if (sliceList[sliceNumber].mBufferList != nil)
		{
			free (sliceList[sliceNumber].mBufferList);
		}		
	}
}

// Fill and schedule the audio slice buffers.
// Return the number of slices free once we're done.
-(UInt32) previewBufferScheduleSlices:(ScheduledAudioSlice *)sliceList
									numSlices:(UInt32)numSlices
									extractionSession:(MovieAudioExtractionRef)extraction
									asbd:(AudioStreamBasicDescription)asbd
									timeStamp:(Float64*)ioSampleTimeStamp
									remaining:(SInt64*)ioSamplesRemaining
									complete:(Boolean*)outExtractionComplete
{
	UInt32		sliceNumber;
	UInt32		numSlicesFree = 0;
	OSStatus	err;

	// Iterate through our slice list.
	// For each completed (or never used) slice, fill and schedule it.
	for (sliceNumber = 0; sliceNumber < numSlices ; sliceNumber++)
	{
		if (sliceList[sliceNumber].mFlags & kScheduledAudioSliceFlag_Complete)
		{
			// Count this slice as free until we fill it up
			numSlicesFree++;

			// If there are any samples left to extract...		
			if (*ioSamplesRemaining == 0)
			{
				*outExtractionComplete = true;
			}
			if (!*outExtractionComplete)
			{
				// We will read numSamplesThisSlice number of samples
				SInt64 numSamplesThisSlice = *ioSamplesRemaining;
				if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
				{
					numSamplesThisSlice = kMaxExtractionPacketCount;
				}
				sliceList[sliceNumber].mTimeStamp.mSampleTime = *ioSampleTimeStamp;
				err = extractSliceAndScheduleToPlay(extraction,
													asbd, 
													mPlayerAudioUnit,
													&sliceList[sliceNumber], 
													&numSamplesThisSlice,
													outExtractionComplete);
				if (!err && (numSamplesThisSlice > 0)) 
				{
					numSlicesFree--;
				} else 
				{
					sliceList[sliceNumber].mFlags = kScheduledAudioSliceFlag_Complete;
				}
				// Whether there was an error or not, numSamplesThisSlice is valid
				*ioSampleTimeStamp += numSamplesThisSlice;
				if (*ioSamplesRemaining != -1)
				{
					*ioSamplesRemaining -= numSamplesThisSlice;
				}	
			}
		}
	}
	return (numSlicesFree);
}

#pragma mark
#pragma mark ---- NSTableView DataSource Methods ----------

-(int) numberOfRowsInTableView:(NSTableView*)tableView
{
	OSStatus	err;
	UInt32		numberOfRows = 0;

	if ([mMovieDocument movie]) 
	{
		// Extraction Layout table
		if ([tableView isEqual:uiExtractionChannelLayoutTableView])
		{
			// If we have a cached extraction layout, use it.
			// Otherwise, initialize and cache it here.
			if (mExtractionLayout == NULL)
			{
				// Tag of -1 indicates a custom layout (which we won't have with a nil mExtractionLayout)

				// Tag of 0 indicates the default (summary) layout
				if ([[uiExtractionLayoutSelectorPopUpButton selectedItem] tag] == 0)
				{
					err = getDefaultExtractionLayout([[mMovieDocument movie] quickTimeMovie], nil, &mExtractionLayout, nil);
				}
				// If we are doing an AllChannelsDiscrete extraction, construct an appropriate channel layout
				else if ((AudioChannelLayoutTag)[[uiExtractionLayoutSelectorPopUpButton selectedItem] tag] ==
									kAudioChannelLayoutTag_DiscreteInOrder)
				{
					err = getDiscreteExtractionLayout([[mMovieDocument movie] quickTimeMovie], nil, &mExtractionLayout);		
				}
				// Otherwise, get the layout for the tag identified in the menu item
				else
				{
					err = getLayoutForTag((AudioChannelLayoutTag)[[uiExtractionLayoutSelectorPopUpButton selectedItem] tag],
												nil, &mExtractionLayout);		
				}
				if (err == noErr) 
				{
					numberOfRows = mExtractionLayout->mNumberChannelDescriptions;
				}	
			} 
			else 
			{
				numberOfRows = mExtractionLayout->mNumberChannelDescriptions;			
			}
		}
	}	
	return numberOfRows;																								
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	id objectValue = nil; 
	NSString *column_id = [tableColumn identifier];
	
	if ([tableView isEqual:uiExtractionChannelLayoutTableView])
	{
		if ([column_id isEqualToString:@"channel"]) 
		{
			objectValue = [NSNumber numberWithInt:row];
		} 
		else if ([column_id isEqualToString:@"assignment"]) 
		{
			// If we are in All Channels Discrete mode, you cannot select anything
			if ((AudioChannelLayoutTag)[[uiExtractionLayoutSelectorPopUpButton selectedItem] tag] == kAudioChannelLayoutTag_DiscreteInOrder)
			{
				objectValue = (id)[NSNumber numberWithInt:0];
			}
			else 
			{
				UInt32 index;

				// The channel selectors are always in the same order, with the custom value, if any, at the end
				for (index = 0; index < [mCommonChannelLabelOptions count]; index++)
				{
					if (mExtractionLayout->mChannelDescriptions[row].mChannelLabel == 
						(AudioChannelLabel)[[mCommonChannelLabelOptions objectAtIndex:index] intValue])
					{
						objectValue = [NSNumber numberWithInt:index];
						break;
					}
				}
				// If we didn't find the label, then the last item (which index now reflects) is the right one.
				if (objectValue == nil)
				{
					objectValue = [NSNumber numberWithInt:index];
				}
			}
		} 
	}
	return objectValue;
}

-(void) tableView:(NSTableView*)tableView setObjectValue:(id)value forTableColumn:(NSTableColumn*)tableColumn row:(int)row
{
	
	if ([tableView isEqual:uiExtractionChannelLayoutTableView])
	{
		int				index;
		AudioChannelLabel	channelLabel;

		// The extraction layout should never be nil here
		if (mExtractionLayout == nil)
		{
			return;			
		}
		// Get the channel label for this popup selector index.
		// If it is not in the table of names, it is the custom item.
		index = [(NSNumber*)value intValue];
		if (index < (int)[mCommonChannelLabelOptions count])
		{
			channelLabel = (AudioChannelLabel)[[mCommonChannelLabelOptions objectAtIndex:index] intValue];
		}
		else
		{
			// This should never happen, since there is no callback when the selected item doesn't change.
			// Once a custom tag is set to one from the table, the custom menu item is removed.
			channelLabel = mExtractionLayout->mChannelDescriptions[row].mChannelLabel;
		}

		// If the channel label changed, then we make a new layout and associate it with a channel tag, if possible
		if (channelLabel != mExtractionLayout->mChannelDescriptions[row].mChannelLabel)
		{
			AudioChannelLayout		*newLayout = NULL;
			AudioChannelLayoutTag	newTag;
			size_t					size;
			Boolean					foundMatch = false;

			// Make an updated layout and replace the cached copy
			size = fieldOffset(AudioChannelLayout, mChannelDescriptions[mExtractionLayout->mNumberChannelDescriptions]);
			newLayout = (AudioChannelLayout *) calloc(1, size);
			if (newLayout == NULL)
			{
				return;
			}
			memcpy(newLayout, mExtractionLayout, size);
			newLayout->mChannelDescriptions[row].mChannelLabel = channelLabel;
			free(mExtractionLayout);
			mExtractionLayout = newLayout;

			// See if this new layout matches any of the top-level popup selector tags
			(void) getTagForLayout(mExtractionLayout, size, &newTag);
			if (newTag != kAudioChannelLayoutTag_UseChannelDescriptions)
			{
				for (index = 0; index < [uiExtractionLayoutSelectorPopUpButton numberOfItems]; index++)
				{
					// If the channel tag matches one in the top-level list, then set the top-level item to it.
					if (newTag == (AudioChannelLayoutTag)[[uiExtractionLayoutSelectorPopUpButton itemAtIndex:index] tag])
					{
						foundMatch = true;
						[uiExtractionLayoutSelectorPopUpButton selectItemAtIndex:index];
					}
				}
			}
			// If we found a match, make sure "Custom" is no longer in the popup, otherwise make sure it is.
			index = [uiExtractionLayoutSelectorPopUpButton indexOfItemWithTitle:@"Custom"];
			if (foundMatch)
			{
				if (index != -1)
				{
					// Remove the item and the separator preceding it.
					[[uiExtractionLayoutSelectorPopUpButton menu] removeItemAtIndex:index];
					[[uiExtractionLayoutSelectorPopUpButton menu] removeItemAtIndex:index-1];
				}
			}
			else
			{
				if (index == -1)
				{
					// Add a separating line and "Custom" menu item
					[[uiExtractionLayoutSelectorPopUpButton menu] addItem:[NSMenuItem separatorItem]];
					[uiExtractionLayoutSelectorPopUpButton addItemWithTitle:@"Custom"];
					[[uiExtractionLayoutSelectorPopUpButton lastItem] setTag:-1];
					index = [uiExtractionLayoutSelectorPopUpButton indexOfItemWithTitle:@"Custom"];
				}
				[uiExtractionLayoutSelectorPopUpButton selectItemAtIndex:index];
			}
		}
	}
}

#pragma mark
#pragma mark ---- NSTableColumn Delegate Method ----

// Delegate method for NSTableColumn subclass PopUpTableColumn
							
- (id)dataCellForRow:(int)row forTable:(NSTableView*)tableView
{
	NSPopUpButtonCell *dataCell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:NO];
	NSMenu *menu = [[NSMenu alloc] init];
	NSMenuItem *menuItem;
	int index;
	
	[dataCell setControlSize:NSMiniControlSize];
	[dataCell setFont:[NSFont menuFontOfSize:10]];
	
	// All Discrete is effectively immutable: we only populate the menu with the specific discrete channel label
	if ((AudioChannelLayoutTag)[[uiExtractionLayoutSelectorPopUpButton selectedItem] tag] ==
			 kAudioChannelLayoutTag_DiscreteInOrder)
	{
		menuItem = [[NSMenuItem alloc] init];
		[self fillInMenuItem:menuItem forChannelLabel:(mExtractionLayout->mChannelDescriptions[row].mChannelLabel)];
		[menu addItem:menuItem];
		[menuItem release];
		[dataCell setEditable:NO];	// make sure setObjectValue never gets called
	}	
	else 
	{
		// Populate the popup with a standard set of labels,
		// but make sure the current label name is in the list (at the end, if necessary).
		Boolean	labelFound = false;
		// Add the commonly used labels to our menu
		[self getCommonChannelLabelOptions];
		for (index = 0; index < (int)[mCommonChannelLabelOptions count]; index++)
		{
			menuItem = [[NSMenuItem alloc] init];
			[self fillInMenuItem:menuItem forChannelLabel:(AudioChannelLabel)[[mCommonChannelLabelOptions objectAtIndex:index] intValue]];
			if (mExtractionLayout && 
				(mExtractionLayout->mChannelDescriptions[row].mChannelLabel == (AudioChannelLabel)[[mCommonChannelLabelOptions objectAtIndex:index] intValue]))
			{
				labelFound = true;
			}
			[menu addItem:menuItem];
			[menuItem release];

		}
			
		if (mExtractionLayout && !labelFound)
		{
			// Make sure the current channel label is in the popup
			menuItem = [[NSMenuItem alloc] init];
			[self fillInMenuItem:menuItem forChannelLabel:mExtractionLayout->mChannelDescriptions[row].mChannelLabel];
			[menu addItem:menuItem];
			[menuItem release];
		}
	}

	[dataCell setMenu:menu];
	[menu release];
	return dataCell;
}

#pragma mark
#pragma mark ---- Window Notifications ----

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
	// The extraction panel is being closed
	if ([controller window] == [self window])
	{
		// the extraction window is closing,
		// stop any ongoing preview 
		[self doStopPreview:self];
	}
	
}

// listen to the track changed callback, and refresh the extraction layout and table
- (void)movieTracksChanged:(NSNotification *)notification
{
	QTMovie *changedMovie = (QTMovie*)[notification object];
	if (changedMovie == [mMovieDocument movie])
	{
		[self getSummaryASBD];
		[self refreshExtractionTableView];
		
	}
}
@end