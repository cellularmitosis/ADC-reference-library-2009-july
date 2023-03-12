/*
	File:		AudioPropInfo.m

	Description:    Demonstrates how audio channel layouts and track 
					and movie properties can be manipulated.
				    Demonstrates the opening and configuration of an audio 
					extraction sessions (setting of layout, starttime, duration, etc.)
					Shows how CoreAudio can be used for playback of the extracted audio.
					Provides examples of performing the preview and extraction tasks 
					on a worker thread, and alternatively on the main thread

		Originally introduced at WWDC 2005 at Session 201:
			"Harnessing the Audio Capabilities of QuickTime 7" 


	Copyright:   © Copyright 2004, 2005 Apple Computer, Inc.
				 All rights reserved.

	Disclaimer: IMPORTANT:  This Apple software is supplied to you by
	Apple Computer, Inc. ("Apple") in consideration of your agreement to the
	following terms, and your use, installation, modification or
	redistribution of this Apple software constitutes acceptance of these
	terms.  If you do not agree with these terms, please do not use,
	install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple’s copyrights in this original Apple software (the
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

*/
 
#import "MovieDocument.h"

// Conditions for NSConditionLock
#define AUDIO_SLICE_RENDERED			1
#define WAIT_FOR_AUDIO_SLICE_RENDER		2

// Maximum size, in frames, of the MovieAudioExtractionFillBuffer calls
#define kMaxExtractionPacketCount		(4096)

// Table of channel name abbreviations
struct channelLabelToLetter
{
	AudioChannelLabel		label;
	NSString				*letter;
} typedef channelLabelToLetter;

static const channelLabelToLetter channelLabelToLetterArray[] = {
						{ kAudioChannelLabel_Left, @"L "},
						{ kAudioChannelLabel_Right, @"R "},
						{ kAudioChannelLabel_Center, @"C "},
						{ kAudioChannelLabel_LFEScreen, @"Lfe "},
						{ kAudioChannelLabel_LeftSurround, @"Ls "},
						{ kAudioChannelLabel_RightSurround, @"Rs "},
						{ kAudioChannelLabel_LeftCenter, @"Lc "},
						{ kAudioChannelLabel_RightCenter, @"Rc "},
						{ kAudioChannelLabel_CenterSurround, @"Cs "},
						{ kAudioChannelLabel_Mono, @"Mono "},
						{ kAudioChannelLabel_Unused, @"- "},
						{ kAudioChannelLabel_Discrete_0, @"D0 "},           
						{ kAudioChannelLabel_Discrete_1, @"D1 "},             
						{ kAudioChannelLabel_Discrete_2, @"D2 "},            
					    { kAudioChannelLabel_Discrete_3, @"D3 "},          
					    { kAudioChannelLabel_Discrete_4, @"D4 "},         
					    { kAudioChannelLabel_Discrete_5, @"D5 "},      
					    { kAudioChannelLabel_Discrete_6, @"D6 "},    
					    { kAudioChannelLabel_Discrete_7, @"D7 "},   
					    { kAudioChannelLabel_Discrete_8, @"D8 "}, 
					    { kAudioChannelLabel_Discrete_9, @"D9 "},
					    { kAudioChannelLabel_Discrete_10, @"D10 " },
					    { kAudioChannelLabel_Discrete_11, @"D11 " },
					    { kAudioChannelLabel_Discrete_12, @"D12 " },  
					    { kAudioChannelLabel_Discrete_13, @"D13 " },
					    { kAudioChannelLabel_Discrete_14, @"D14 " },
					    { kAudioChannelLabel_Discrete_15, @"D15 "},
					    { kAudioChannelLabel_Discrete_0 | 16, @"D16 "},
					    { kAudioChannelLabel_Discrete_0 | 17, @"D17 "},
					    { kAudioChannelLabel_Discrete_0 | 18, @"D18 "},
					    { kAudioChannelLabel_Discrete_0 | 19, @"D19 "},
					    { kAudioChannelLabel_Discrete_0 | 20, @"D20 "},
					    { kAudioChannelLabel_Discrete_0 | 21, @"D21 "},
					    { kAudioChannelLabel_Discrete_0 | 22, @"D22 "},
					    { kAudioChannelLabel_Discrete_0 | 23, @"D23 "},
					    { kAudioChannelLabel_Discrete_0 | 24, @"D24 "},
					    { kAudioChannelLabel_Discrete_0 | 25, @"D25 "},
					    { kAudioChannelLabel_Discrete_0 | 26, @"D26 "},
					    { kAudioChannelLabel_Discrete_0 | 27, @"D27 "},
					    { kAudioChannelLabel_Discrete_0 | 28, @"D28 "},
					    { kAudioChannelLabel_Discrete_0 | 29, @"D29 "},
					    { kAudioChannelLabel_Discrete_0 | 30, @"D30 "},
					    { kAudioChannelLabel_Discrete_0 | 31, @"D31 "},
					    { kAudioChannelLabel_Discrete_0 | 32, @"D32 "},
};


// Movie/Track gain change property listener callbacks 
// Used by the movie/track gain sliders on the panel
static void movieGainChangeCallback(Movie m, QTPropertyClass propClass, QTPropertyID propID, id observer); 
static void trackGainChangeCallback(Track t, QTPropertyClass propClass, QTPropertyID propID, id observer);

// AUScheduledSoundPlayer callback called when a slice of scheduled audio has been played
// Used during extraction preview
static void previewAudioSliceCompletionProc(void *userData, 
												struct ScheduledAudioSlice *bufferList);

																									
@implementation AudioPropInfo

	// Class method
+ (AudioPropInfo *) audioPropInfo
{
	AudioPropInfo	*_audioPropInfo;
	_audioPropInfo = [(AudioPropInfo *)[self alloc] init];
	return _audioPropInfo;
}

	// initialize the various member variables 
	// of this class
- (id) init
{
	[super init];
	
	_trackChannelLabelsMenusArray = [[[NSMutableArray alloc] init] retain];
	_trackChannelLabelsIndexOfSelectedMenuItemArray = [[[NSMutableArray alloc] init] retain];
	_trackChannelLabelNames = [[[NSMutableArray alloc] init] retain];
	_extractionChannelLabelNames = [[[NSMutableArray alloc] init] retain];
	_extractionLayoutMenuList = nil;
	_clonedMovie = nil;
	_summaryLayout = nil;
	_deviceLayout = nil;
	_extractionLayout = nil;	
	_graphUnit = nil;
	_exportFileID = nil;
	_currentDocFileName = nil;
	[self setMovie:nil fileName:nil];
	[self setTrack:nil];
	[self setExtractionTime:nil isStartTime:YES isInit:YES];
	[self setExtractionTime:nil isStartTime:NO isInit:YES];

	return self;
}

- (void) dealloc
{
	[_trackChannelLabelsMenusArray release];
	[_trackChannelLabelsIndexOfSelectedMenuItemArray release];
	[_trackChannelLabelNames release];
	[_extractionChannelLabelNames release];
	[_extractionLayoutMenuList release];
    
	[self setTrack:nil];
	[self setMovie:nil fileName:nil];
	
	if (_summaryLayout)
		free(_summaryLayout);
	if (_deviceLayout)
		free(_deviceLayout);
	if (_extractionLayout)
		free(_extractionLayout);
	[_currentExtractStartTime release];
	[_currentExtractEndTime release];
	[_currentDocFileName release];

    [super dealloc];
}


#pragma mark ---- Track Layout View actions ----

// Method invoked by the track selector pop-up when its value changes
- (IBAction) doSelectTrack:(id)sender
{
	if (_currentTrack == [[sender selectedItem] representedObject]) 
		return;

	[self setTrack:[[sender selectedItem] representedObject]];
	
	// Since the track has changed, throw out the channel popup values
	// and repopulate them according to the current state.
	[_trackChannelLabelsMenusArray removeAllObjects];
	[_trackChannelLabelsIndexOfSelectedMenuItemArray removeAllObjects];
	[_audTrackChannelLayoutTableView reloadData];

	// Make sure the summary layout text string is displayed, if this is a specific track
	[self setSummaryChannelLayoutTextField];
}


// Method invoked by the enable/disable checkbox when it's state is changed
- (IBAction) doChangeTrackEnabled:(id)sender
{
	if (_currentTrack == nil)
		return;
	
	SetTrackEnabled( [_currentTrack quickTimeTrack],  ([sender state] == NSOnState)); 
	
	// Enabling/Disabling a track changes the movie's summary mix,
	// so update the cached summary layout
	[self getSummaryChannelLayout];
	[self setSummaryChannelLayoutTextField];

	// If the Default (Summary) or All Discrete extraction type is selected,
	// it should be refreshed now.
	[self refreshExtractionTableView];
}


// Method invoked by the track Gain slider when its value changes
- (IBAction) doChangeTrackGain:(id)sender
{
	[self setTrackGain:[sender floatValue] forTrack:_currentTrack];
}


#pragma mark ---- Device Layout View actions ----

// Method invoked by the movie Gain slider when its value changes
- (IBAction) doChangeMovieGain:(id)sender
{
	[self setMovieGain:[sender floatValue]];
}

// Method invoked by the Speaker Configuration button
- (IBAction) doLaunchAMS:(id)sender
{
	// Launch Audio MIDI Setup
	[[NSWorkspace sharedWorkspace] launchApplication:@"Audio MIDI Setup"];
}

#pragma mark ---- Extraction View actions ----

// Method invoked by the extraction channel layout popup when its value changes
- (IBAction) doSelectExtractionChannelLayout:(id)sender
{
	AudioChannelLayoutTag	newTag;
	UInt32					index;
	OSStatus				err;

	if (_currentMovie == nil)
		return;

	// Select the output channel layout for audio extraction from the preset list. 
	newTag = (AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item];

	// Tag of -1 indicates a custom layout (which doesn't need to do anything).
	if (newTag == -1)
		return;

	// If we have a cached extraction layout, toss it and make a new one.
	if (_extractionLayout != nil)
	{
		free(_extractionLayout);
		_extractionLayout = nil;
	}

	switch (newTag)
	{
		// Tag of 0 indicates the default (summary) layout
		case 0:
			err = getDefaultExtractionLayout([_currentMovie quickTimeMovie], nil, &_extractionLayout, nil);
			break;

		// Special value indicates All Channels Discrete extraction (no mixing)
		case kAudioChannelLayoutTag_DiscreteInOrder:
			err = getDiscreteExtractionLayout([_currentMovie quickTimeMovie], nil, &_extractionLayout);
			break;

		// Expand the layout tag associated with the menu item into a channel layout with descriptions
		default:
			err = getLayoutForTag((AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item],
								  nil, &_extractionLayout);		
			break;
	}
	if (err)
		return;

	// If there is a Custom item in the popup, remove it now,
	// since we have just selected a layout preset.
	index = [_audExtractLayoutSelectorPopUpButton indexOfItemWithTitle:@"Custom"];
	if (index != -1)
	{
		// Remove the item and the separator preceding it.
		[[_audExtractLayoutSelectorPopUpButton menu] removeItemAtIndex:index];
		[[_audExtractLayoutSelectorPopUpButton menu] removeItemAtIndex:index-1];
	}				

	// Since the extraction layout has changed, go and reload everything. 
	[_audExtractChannelLayoutTableView reloadData];
}


// Method invoked by the Preview Start button
- (IBAction) doStartPreview:(id)sender
{
	// Stop playback of current movie, so we can hear the preview
	[_currentMovie stop];
	
	// If we are currently exporting, stop the export
	// We are allowed only one extraction session per movie
	// If we are exporting, stop and wait for it to complete.
	if (_exportFileID != nil)
	{
		_stopExport = true;
		do {
			[[NSRunLoop currentRunLoop] runUntilDate:nil];
		}
		while (_exportFileID != nil);
	}
	
	// Extract and play audio, on another thread if possible.
	[self startPreview];
	
	// change the button state
	[sender setTitle:@"Stop Preview"];
	[sender setAction:@selector(doStopPreview:)];				
}

// Method invoked by the Preview Stop button
- (IBAction) doStopPreview:(id)sender
{
	// Set the stop flag.  The rest of the cleanup will occur in previewCompletedNotification.
	_stopPreview = true;
}

// Notification for the Export Start button
- (IBAction) doStartExport:(id)sender
{
	NSSavePanel *savePanel;
	
	savePanel = [NSSavePanel savePanel];
	[savePanel setRequiredFileType:@"aiff"];

	// If we are previewing, stop and wait for it to complete.
	if (_graphUnit != nil)
	{
		_stopPreview = true;
		do {
			[[NSRunLoop currentRunLoop] runUntilDate:nil];
		}
		while (_graphUnit != nil);
	}
	
    // Open a save panel to get a target file specification.
	// startExport will be invoked when 'OK' is pressed.
	[savePanel beginSheetForDirectory:nil
									file:_currentDocFileName
									modalForWindow:_audioPanel
									modalDelegate:self
									didEndSelector:@selector(startExport: returnCode: contextInfo:)
									contextInfo:nil];
}

// Notification for the Export Stop button
- (IBAction) doStopExport:(id)sender
{
	// Set the stop flag.  The rest of the cleanup will occur in exportCompletedNotification.
	_stopExport = true;
}


// Callback for text entry in the Start Time and End Time fields
- (IBAction) doChangeExtractionTime:(id)sender
{
	if (_currentMovie == nil)
		return;
	
	QTTime startLimit;
	QTTime endLimit;
	QTTime theTime;

	// Set the valid ranges according to the field we are setting.
	if (sender == _audExtractStartTimeTextField)
	{
		// Start Time can range from 0 to the just before the current End Time
		startLimit = QTMakeTime(0L, (long) _summaryASBD.mSampleRate);
		endLimit = [_currentExtractEndTime QTTimeValue];
		endLimit.timeValue -= 1;
	}
	else if (sender == _audExtractEndTimeTextField)
	{
		// End Time can range from just after the current Start Time to the end of the movie
		startLimit = [_currentExtractStartTime QTTimeValue];
		startLimit.timeValue += 1;
		endLimit = [[_currentMovie attributeForKey:QTMovieDurationAttribute] QTTimeValue];
	}

	// Parse the user's string into a QTTime
	theTime = [self QTTimeFromString:[sender stringValue] timeScale:startLimit.timeScale];

	// If the time is not in the valid range, reset the text item from the value.
	if ((QTTimeCompare(theTime, startLimit) == NSOrderedAscending) ||
		(QTTimeCompare(theTime, endLimit) == NSOrderedDescending))
	{
		if (sender == _audExtractStartTimeTextField)
			// Set the Start Time back to where it was.
			[self setExtractionTime:_currentExtractStartTime isStartTime:YES isInit:NO];
		else if (sender == _audExtractEndTimeTextField)
			// Re-init the End Time to the duration of the movie.
			[self setExtractionTime:_currentExtractEndTime isStartTime:NO isInit:YES];
	} else {
		// Set the current field to the parsed value.
		[self setExtractionTime:[NSValue valueWithQTTime:theTime]
					isStartTime:(sender == _audExtractStartTimeTextField) isInit:NO];
	}	
}

#pragma mark
#pragma mark ---- getters ----

- (NSPanel *)audioPropInfoPanel
{
	return _audioPanel;
}

- (QTMovie *)movie 
{
	return _currentMovie;
}

-(QTTrack *)track
{
	return _currentTrack;
}

- (NSSlider *)movieGainSlider
{
	return _audMovieGainSlider;
}

- (NSSlider *)trackGainSlider
{
	return _audTrackGainSlider;
}

// Get the movie's gain
-(float) movieGain
{
	float gain = 0.0;
	if (_currentMovie != nil) 
	{
		(void) QTGetMovieProperty((Movie)[_currentMovie quickTimeMovie], 
										kQTPropertyClass_Audio, 
										kQTAudioPropertyID_Gain, 
										sizeof (gain), &gain, NULL);
	}
	return gain;
}

// Get the track's gain
- (float) trackGainForTrack:(QTTrack *)track
{
	float gain = 0.0;
	if (track != nil) {
		(void) QTGetTrackProperty((Track)[track quickTimeTrack], 
										kQTPropertyClass_Audio, 
										kQTAudioPropertyID_Gain, 
										sizeof (gain), &gain, NULL);
	}
	return gain;
}


#pragma mark
#pragma mark ---- setters ----

- (void) setMovie:(QTMovie *)theMovie fileName:(NSString *)name
{   
	if (_currentMovie == theMovie)
		return;

	// If we had set up a gain property listener, remove it here
	if (_currentMovie != nil) {
		[self removeMovieGainPropertyListener:[_currentMovie quickTimeMovie]];
	}

	[theMovie retain];
	[_currentMovie release];
    _currentMovie = theMovie;

	[_currentDocFileName release];
	_currentDocFileName = [[[name stringByDeletingPathExtension] lastPathComponent] retain];

	// Add a listener to the new movie's gain changes
	if (_currentMovie != nil) {
		[self addMovieGainPropertyListener:[_currentMovie quickTimeMovie]];
	}

	[_audMovieGainSlider setEnabled:(_currentMovie != nil)];
}

- (void)setTrack:(QTTrack *)theTrack
{
	if (_currentTrack == theTrack)
		return;
	
	// If we had set up a gain property listener for this track, remove it here
	if (_currentTrack != nil) {
		[self removeTrackGainPropertyListener:[_currentTrack quickTimeTrack]];
	}
	
	[theTrack retain];
	[_currentTrack release];
	_currentTrack = theTrack;

	if (_currentTrack != nil) {
		// Add a listener for the Track-level Gain property
		[self addTrackGainPropertyListener:[_currentTrack quickTimeTrack]];
		// Enable the slider and track-enable check box, set the state of the check-box
		[_audTrackGainSlider setEnabled:YES];
		[_audTrackEnabledCheckBox setEnabled:YES];
		[_audTrackEnabledCheckBox setState:GetTrackEnabled([_currentTrack quickTimeTrack])];
	} else {
		// Disable the slider and the check-box for this track
		[_audTrackGainSlider setEnabled:NO];
		[_audTrackEnabledCheckBox setState:NSOffState];
		[_audTrackEnabledCheckBox setEnabled:NO];
		
	}	
}

// Set the movie gain
-(void) setMovieGain:(float) gain
{
	if (_currentMovie == nil)
		return;
		
	(void)QTSetMovieProperty((Movie)[_currentMovie quickTimeMovie], 
									kQTPropertyClass_Audio,
									kQTAudioPropertyID_Gain, 
									sizeof(gain),  
									&gain);
	
}

// Set the track gain
- (void) setTrackGain:(float) gain forTrack:(QTTrack *)track
{
	if (track==nil)
		return;
	(void)QTSetTrackProperty((Track)[track quickTimeTrack], 
											kQTPropertyClass_Audio, 
											kQTAudioPropertyID_Gain, 
											sizeof(gain),  
											&gain);
	
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
        if (time.timeValue < 0) {
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
		scanner = [NSScanner scannerWithString:string];
		
	if ([scanner scanDouble:&tempsecs])
	{
		secs += tempsecs;
	}
	
	rettime.timeValue = (long long)(secs * (double) scale);
    rettime.timeScale = scale;
	rettime.flags = 0;
	
    return rettime;
}

// Set the extraction start and end time textfields, and update the cached start and end times
// If isInit, a default start time of 0 and end time of duration is set
// Else, the time is set to theTimeValue that is passed to this method
- (void) setExtractionTime:(NSValue *)theTimeValue isStartTime:(BOOL)isStart isInit:(BOOL)isInit
{
	if (_currentMovie == nil)
		return;
	
	// Set to default start and end times
	if (isInit) 
	{
		// start time text field
		if (isStart) {
			QTTime startTime = QTMakeTime(0L, (long) _summaryASBD.mSampleRate);
			[_currentExtractStartTime release];
			_currentExtractStartTime = [[NSValue valueWithQTTime:startTime] retain];
			[_audExtractStartTimeTextField setStringValue:[self StringFromQTTime:startTime]];
		} else  { // end time text field
			if ([_currentMovie attributeForKey:QTMovieHasDurationAttribute])
			{
				QTTime endTime = [[_currentMovie attributeForKey:QTMovieDurationAttribute] QTTimeValue];
				[_currentExtractEndTime release];
				_currentExtractEndTime = [[NSValue valueWithQTTime:endTime] retain];
				[_audExtractEndTimeTextField setStringValue:[self StringFromQTTime:endTime]];
			}
		}	
	} 
	// Set to the time that is passed to this method
	else  
	{
		// start time text field
		if (isStart) {
			if (theTimeValue == nil)
				return;
			[theTimeValue retain];
			[_currentExtractStartTime release];
			_currentExtractStartTime = theTimeValue; 
			[_audExtractStartTimeTextField setStringValue:[self StringFromQTTime:[_currentExtractStartTime QTTimeValue]]];
		} else { // end time text field
			if (theTimeValue == nil)
					return;
			[theTimeValue retain];
			[_currentExtractEndTime release];
			_currentExtractEndTime = theTimeValue; 
			[_audExtractEndTimeTextField setStringValue:[self StringFromQTTime:[_currentExtractEndTime QTTimeValue]]];
		}
	}
}

#pragma mark
#pragma mark ---- delegates ----

// load the audio info panel
- (void) loadPanel
{		
  [NSBundle loadNibNamed:@"AudioPropInfoPanel.nib" owner:self];
}

- (void) showPanel
{	
    // load the panel
    if (_audioPanel == nil)
    {
        [self loadPanel];
		[self rebuildInfoPanel];

		// Get the movie name, if any.  Otherwise, use the fileName.
		NSString	*name = [_currentMovie attributeForKey:QTMovieDisplayNameAttribute];
		if (name == nil)
			name = _currentDocFileName;
		[_audioPanel setTitle:[NSString stringWithFormat:@"%@- Audio Extraction", name]];

		// add the AudioProp Info window to the windows menu (automatic removal)
		[[NSApplication sharedApplication] addWindowsItem:_audioPanel title:[_audioPanel title] filename:false];
	}

	// show the panel
    [_audioPanel makeKeyAndOrderFront:nil];
}

- (void) hidePanel
{
	// If we are previewing, stop and wait for it to complete.
	if (_graphUnit != nil)
	{
		_stopPreview = true;
		do {
			[[NSRunLoop currentRunLoop] runUntilDate:nil];
		}
		while (_graphUnit != nil);
	}

	// If we are exporting, stop and wait for it to complete.
	if (_exportFileID != nil)
	{
		_stopExport = true;
		do {
			[[NSRunLoop currentRunLoop] runUntilDate:nil];
		}
		while (_exportFileID != nil);
	}
	
	// close the panel 
    if (_audioPanel != nil)
    {
		[_audioPanel close];
	}
}


- (BOOL) isVisible
{
    return [_audioPanel isVisible];
}

// Populate the various menus and tables in the panel
- (void) rebuildInfoPanel
{
	[self getSummaryChannelLayout];
	[self populateTrackSelectorPopUpButton];
	[self getDeviceLayout];
	[self populateExtractChannelsSelectorPopUpButton];
	[self createLabelsArray];
 
	[_audTrackChannelLayoutTableView reloadData];
	[_audDeviceChannelLayoutTableView reloadData];
	[_audExtractChannelLayoutTableView reloadData];
	
	[self setExtractionTime:nil isStartTime:YES isInit:YES];
	[self setExtractionTime:nil isStartTime:NO isInit:YES];
	
	[_audMovieGainSlider setEnabled:(_currentMovie != nil)];
	[_audTrackGainSlider setEnabled:(_currentTrack != nil)];
}

// Populate the track selector pop-up button  
- (void) populateTrackSelectorPopUpButton
{
	UInt32 index;
	NSArray	*arrayOfMovieTracks;
	
	[_audTrackSelectorPopUpButton removeAllItems];
	[self setTrack:nil];

	if (_currentMovie == nil)
		return;
	
	// First, add the special item associated with the movie summary mix. 
	// This item does not have a track associated with it
	[_audTrackSelectorPopUpButton insertItemWithTitle:@"Summary Channel Layout" atIndex:0];
	[[_audTrackSelectorPopUpButton itemAtIndex:0] setRepresentedObject:nil];
	[_audTrackSelectorPopUpButton selectItemAtIndex:0];
	[_audTrackEnabledCheckBox setEnabled:NO]; 
	
	// Add to the pop-up menu, all the sound tracks of this movie
	arrayOfMovieTracks = [_currentMovie tracks];
	for (index = 0; index < [arrayOfMovieTracks count]; index++) 
	{
		if (trackMixesToAudioContext([[arrayOfMovieTracks objectAtIndex:index] quickTimeTrack]))
		{
			NSMutableString	*trackNumber;
			trackNumber = [[NSMutableString alloc] initWithFormat:@"Track %d: %@", index + 1,
												[[arrayOfMovieTracks objectAtIndex:index] attributeForKey:@"QTTrackDisplayNameAttribute"]];
			[_audTrackSelectorPopUpButton addItemWithTitle:trackNumber];
			[[_audTrackSelectorPopUpButton lastItem] setRepresentedObject:(QTTrack*)[arrayOfMovieTracks objectAtIndex:index]];
			[trackNumber release];	
		}
	}
}

// Populate the pop-up button used to select the movie's
// audio extraction output layout 
-(void) populateExtractChannelsSelectorPopUpButton
{
	UInt32 index;
	[_audExtractLayoutSelectorPopUpButton setAutoenablesItems:NO];
	[_audExtractLayoutSelectorPopUpButton removeAllItems];

	if (_currentMovie == nil)
		return;
		
	_extractionLayoutMenuList = [[[NSMutableArray alloc] initWithObjects:
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_Mono andItemName:@"Mono"],
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_Stereo andItemName:@"Stereo (L R)"],
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_Quadraphonic andItemName:@"Quadraphonic (L R Ls Rs)"],
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_MPEG_5_0_A andItemName:@"5.0 (L R C Ls Rs)"],
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_MPEG_5_1_A andItemName:@"5.1 (L R C LFE Ls Rs)"],
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_MPEG_7_1_A andItemName:@"7.1 (L R C LFE Ls Rs Lc Rc)"],
						[InfoObject infoObjectWithItem:kAudioChannelLayoutTag_DiscreteInOrder andItemName:@"All Discrete"],
						// Use tag == 0 to flag the default (summary) layout.
						// We will use tag == -1 to flag a custom layout.
						[InfoObject infoObjectWithItem:0 andItemName:@"Default"], nil] retain];
	
	for (index = 0; index < [_extractionLayoutMenuList count]; index++)
	{
		[_audExtractLayoutSelectorPopUpButton addItemWithTitle:(NSString*)[[_extractionLayoutMenuList objectAtIndex:index] itemName]];
		[[_audExtractLayoutSelectorPopUpButton lastItem] setRepresentedObject:[_extractionLayoutMenuList objectAtIndex:index]];
	}
	[_audExtractLayoutSelectorPopUpButton selectItem:[_audExtractLayoutSelectorPopUpButton lastItem]];
}


// Displays a string that describes the movie's summary layout
-(void)setSummaryChannelLayoutTextField
{
	NSMutableString *stringForSummaryLayout = [NSMutableString stringWithString:@""];
	
	// If the track channel layout table is not currently displaying the summary channel layout,
	// and we have a summary layout construct the summary layout string and display it	
	// Else display an empty string
	if ([_audTrackSelectorPopUpButton indexOfSelectedItem] != 0 &&  _summaryLayout)
	{
		UInt32 channel = 0, index = 0;
		
		[stringForSummaryLayout appendString:@"Summary Layout:  "];
		// Iterate through the channel descriptions in the summary layout
		// Find the abbreviation of the channel name of each channel, and concatenate 
		// it to the string that we will be displaying
		for (channel=0; channel <  _summaryLayout->mNumberChannelDescriptions; channel++) 
		{
			for (index=0; index < (sizeof(channelLabelToLetterArray) / sizeof(channelLabelToLetter)); index++)
			{
				if ((_summaryLayout->mChannelDescriptions[channel]).mChannelLabel == (channelLabelToLetterArray[index]).label)
				{
					[stringForSummaryLayout appendString:(NSString*)(channelLabelToLetterArray[index]).letter];
					break;
				}		
			
			} 
			// Didn't find an abbreviation for this channel label
			if (index == (sizeof(channelLabelToLetterArray) / sizeof(channelLabelToLetter)))
				[stringForSummaryLayout appendString:@"? "]; 
		}
	}
	// Set the string value
	[_audSummaryChannelLayoutTextField setStringValue:stringForSummaryLayout];

}


// Refresh the extraction view if the underlying channel layouts may have changed
-(void) refreshExtractionTableView
{
	// Throw away the cached extraction layout and reload to refresh it.
	if (_extractionLayout != nil)
	{
		AudioChannelLayoutTag	tag;

		// Tag of 0 indicates the default (summary) layout
		// Tag of kAudioChannelLayoutTag_DiscreteInOrder indicates All Channels Discrete extraction
		tag = (AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item];
		if ((tag == 0) || (tag == kAudioChannelLayoutTag_DiscreteInOrder))
		{
			free(_extractionLayout);
			_extractionLayout = nil;
		}
	}

	if (_extractionLayout == nil)
	{
		// Refresh the panel if it is being displayed
		[_audExtractChannelLayoutTableView reloadData];
	}
}

// Create and add an InfoObject to the provided array
- (void) addLabelToLabelNamesArray:(NSMutableArray *)namesArray label:(AudioChannelLabel) thisLabel
{
	InfoObject *labelInfo = [[InfoObject alloc] init];
	NSString *labelStr = [NSString stringWithString:@""];
	UInt32 labelSize = sizeof(NSString*);
	AudioChannelDescription acd = {0};
	acd.mChannelLabel = thisLabel;
	(void)AudioFormatGetProperty(kAudioFormatProperty_ChannelName, 
								sizeof(AudioChannelDescription), 
								&acd, 
								&labelSize, 
								&labelStr);
	[labelInfo setItem:thisLabel andItemName:labelStr];
	[labelStr release];
	[namesArray addObject:labelInfo];
	[labelInfo release];
}

// Create an array of pairs of AudioChannelLabels and their names
// for the commonly requires channel labels that we need. 
- (void) createLabelsArray
{
	// This tag gets us most of the labels that we are interested in
	AudioChannelLayoutTag	tag = kAudioChannelLayoutTag_MPEG_7_1_A;
	UInt32					layoutSize;
	AudioChannelLayout		*layout;
	UInt32					channel = 0;
	UInt32					numDiscreteChannels = 0;
	UInt32					numDeviceChannels;
	UInt32					index;
	OSStatus				err;
	
	if (noErr != AudioFormatGetPropertyInfo (kAudioFormatProperty_ChannelLayoutForTag, sizeof(AudioChannelLayoutTag), 
											&tag, &layoutSize))
		goto bail;									
	
	layout = (AudioChannelLayout*)malloc(layoutSize);
	if (noErr != AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutForTag, sizeof(AudioChannelLayoutTag), 
											&tag, &layoutSize, layout))
		goto bail;
											
	if (noErr != expandChannelLayout(&layout, &layoutSize))
		goto bail;
		
	// Commonly used labels 
	for (channel = 0 ; channel < layout->mNumberChannelDescriptions; channel++)
	{
		AudioChannelLabel thisLabel = (AudioChannelLabel)(layout->mChannelDescriptions[channel]).mChannelLabel;
		[self addLabelToLabelNamesArray:_trackChannelLabelNames label:thisLabel];
		[self addLabelToLabelNamesArray:_extractionChannelLabelNames label:thisLabel];
	}

	// Center Surround
	[self addLabelToLabelNamesArray:_trackChannelLabelNames label:kAudioChannelLabel_CenterSurround];
	[self addLabelToLabelNamesArray:_extractionChannelLabelNames label:kAudioChannelLabel_CenterSurround];

	// Mono
	[self addLabelToLabelNamesArray:_trackChannelLabelNames label:kAudioChannelLabel_Mono];

	// Discrete Labels
	if (_deviceLayout)
		numDeviceChannels = _deviceLayout->mNumberChannelDescriptions;

	AudioChannelLayout *localDiscreteLayout;	
	err = getDiscreteExtractionLayout([_currentMovie quickTimeMovie], nil, &localDiscreteLayout);		
	if (!err)
		numDiscreteChannels = localDiscreteLayout->mNumberChannelDescriptions;
	if (localDiscreteLayout)
		free(localDiscreteLayout);

	numDiscreteChannels = (numDeviceChannels > numDiscreteChannels) ? numDeviceChannels : numDiscreteChannels;

	for (index = 0; index < numDiscreteChannels; index++)
	{
		AudioChannelLabel discreteLabel = (1L<<16) | index;
		[self addLabelToLabelNamesArray:_trackChannelLabelNames label:discreteLabel];
	}	
	// Unused (to disable output from this channel)
	[self addLabelToLabelNamesArray:_trackChannelLabelNames label:kAudioChannelLabel_Unused];
	
	bail:
	if (layout)
		free(layout);
}

// Get the movie's summary layout
- (void) getSummaryChannelLayout
{
	OSStatus err = noErr;
	
	if (_summaryLayout)
		free(_summaryLayout);
	_summaryLayout = nil;
		
	if (_currentMovie == nil)
		return;
	// Get the extraction default (summary) layout and ASBD,
	// which reflects the highest sample rate among the sound tracks
	err = getDefaultExtractionLayout([_currentMovie quickTimeMovie], nil, &_summaryLayout, &_summaryASBD);

	// If for some reason we didn't get a valid ASBD, set a default sample rate.
	// This will avoid divide-by-zero errors later.
	if (_summaryASBD.mSampleRate == 0.)
		_summaryASBD.mSampleRate = 48000.;
}


// Get thde device layout
- (void) getDeviceLayout
{
	if (_deviceLayout)
		free(_deviceLayout);
	_deviceLayout = nil;	

	if (_currentMovie == nil)
		return;	
		
	(void) getDeviceLayout([_currentMovie quickTimeMovie], nil, &_deviceLayout);
}

// Get the discrete layout
- (void) getAllDiscreteLayout
{	
	if (_extractionLayout)
		free(_extractionLayout);
	_extractionLayout = nil;
	
	if (_currentMovie == nil)
		return;
		
	(void) getDiscreteExtractionLayout([_currentMovie quickTimeMovie], nil, &_extractionLayout);		
}

#pragma mark
#pragma mark ---- Movie/Track Gain Listening ---------------------

// Add and remove movie and track gain property listeners
- (void)addMovieGainPropertyListener:(Movie)movie
{
	(void)QTAddMoviePropertyListener(movie,
									kQTPropertyClass_Audio, 
									kQTAudioPropertyID_Gain,
									(QTMoviePropertyListenerUPP)&movieGainChangeCallback, 
									self);

}

- (void)addTrackGainPropertyListener:(Track)track
{
	(void) QTAddTrackPropertyListener(track,
										kQTPropertyClass_Audio, 
										kQTAudioPropertyID_Gain,
										(QTTrackPropertyListenerUPP)&trackGainChangeCallback, 
										self);
}

- (void)removeMovieGainPropertyListener:(Movie)movie
{
	QTRemoveMoviePropertyListener(movie,
									kQTPropertyClass_Audio, 
									kQTAudioPropertyID_Gain,
									(QTMoviePropertyListenerUPP)&movieGainChangeCallback, 
									self);
}

- (void)removeTrackGainPropertyListener:(Track)track
{
	QTRemoveTrackPropertyListener(track,
									kQTPropertyClass_Audio, 
									kQTAudioPropertyID_Gain,
									(QTTrackPropertyListenerUPP)&trackGainChangeCallback, 
									self);
}

// Callback function for movie gain change
static void movieGainChangeCallback(Movie m, QTPropertyClass propClass, QTPropertyID propID, id observer)
{
	if ([[(AudioPropInfo*)observer movie] quickTimeMovie] != m) {
		// Callback for the wrong movie??
	} else if (propID == kQTAudioPropertyID_Gain) {
		[[observer movieGainSlider] setFloatValue:(float)[observer movieGain]];
	}
			
}

// Callback function for track gain change
static void trackGainChangeCallback(Track t, QTPropertyClass propClass, QTPropertyID propID, id observer)
{
	if ([[(AudioPropInfo*)observer track] quickTimeTrack] != t) {
		// Callback for the wrong track??
	} else if (propID == kQTAudioPropertyID_Gain) {
		[[observer trackGainSlider] setFloatValue:(float)
						[observer trackGainForTrack:[(AudioPropInfo*)observer track]]];
	}

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
	selectedLayoutTag = (AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item];

	// Start Time
	(void) QTGetTimeRecord([_currentExtractStartTime QTTimeValue], startTime);
	
	// Duration		
	durationInQTTime = QTTimeDecrement([_currentExtractEndTime QTTimeValue], [_currentExtractStartTime QTTimeValue]);
	(void) QTGetTimeRecord(durationInQTTime, &durationTimeRecord);

	// Convert to floating-point seconds
	*duration = *((TimeValue64*) &durationTimeRecord.value) / (Float64) durationTimeRecord.scale;

	// ASBD
	*asbd = _summaryASBD;
	asbd->mChannelsPerFrame = _extractionLayout->mNumberChannelDescriptions;

	// All channels discrete 
	if (selectedLayoutTag == kAudioChannelLayoutTag_DiscreteInOrder)
		*allDiscrete = YES;

	// Return a copy of the cached extraction layout, which should always be current
	*layoutSize = fieldOffset(AudioChannelLayout, mChannelDescriptions[_extractionLayout->mNumberChannelDescriptions]);
	*layout = (AudioChannelLayout*) calloc(1, *layoutSize);
	if (*layout == nil) 
		goto bail;
	memcpy(*layout, _extractionLayout, *layoutSize);

bail:
	if (err)
	{
		if (*layout)
			free(*layout);
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
	Float64 duration = 0.;					// extraction duration, in seconds
	Handle cloneHandle = nil;
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
		[_audExtractExportButton setTitle:@"Stop Export"];
		[_audExtractExportButton setAction:@selector(doStopExport:)];				
			
		//-------------------------------------------------------
		
		// Clone the movie, test if clone can be migrated to a 
		// worker thread for extraction, and set a flag accordingly 
		
		_stopExport = false;	// when this is set true, export stops on the next cycle

		cloneHandle = NewHandle(0);
		if (cloneHandle == nil) {
			err = memFullErr;
			goto bail;
		}
		err = PutMovieIntoHandle([_currentMovie quickTimeMovie], cloneHandle);
		if (err != noErr)
			goto bail;
		err = NewMovieFromHandle(&_clonedMovie, cloneHandle, newMovieActive, NULL);
		if (err != noErr || _clonedMovie == nil)
			goto bail;
		if (DetachMovieFromCurrentThread(_clonedMovie) == noErr)
		{
			extractionOnWorkerThread = YES;
		}
		else
		{
			// If we could not migrate this movie, dispose the clone and
			// export from the original movie on the main thread.
			DisposeMovie(_clonedMovie);
			_clonedMovie = nil;
		}

		// Read the UI, get the required extraction layout, layoutsize, 
		// extraction startTime, duration and whether we need to 
		// extract in the "All Channels Discrete" mode
		err = [self getExtractionParameters:(AudioChannelLayout**)&layout 
											outLayoutSize:(UInt32*)&layoutSize
											outASBD:&asbd
											startTime:(TimeRecord*)&startTime 
											duration:(Float64*)&duration 
											allDiscrete:(Boolean*)&discrete];
		if (err)
			goto bail;

		// Set the output ASBD to 16-bit interleaved PCM native-endian integers
		asbd.mFormatID = kAudioFormatLinearPCM;
		asbd.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger |
								kAudioFormatFlagsNativeEndian |
								kAudioFormatFlagIsPacked;
		asbd.mFramesPerPacket = 1;
		asbd.mBitsPerChannel = 16;
		asbd.mBytesPerFrame = 2 * asbd.mChannelsPerFrame;
		asbd.mBytesPerPacket = 2 * asbd.mChannelsPerFrame;

		//---------------------------------------------------------------
		// Open the AIFF file, configure it for out extraction output layout
		// If the file we want to save to exists, delete it
		err = FSPathMakeRef((const UInt8*)[[sheet filename] fileSystemRepresentation], &fileRef, NULL);
		if (err == noErr) {
			FSDeleteObject(&fileRef);
		}	
		FSPathMakeRef((const UInt8*)[directory fileSystemRepresentation], &parentRef, NULL);
		err = AudioFileCreate(&parentRef, (CFStringRef)fileName, fileType, &asbd, 0, &fileRef, &_exportFileID);
		if (err) {
			goto bail;
		}

		// If we do an All Channels Discrete extraction, create an unlabeled multi-channel file.
		// Otherwise, set the channel labels that we've specified for the extraction.
		if (!discrete)
		{
			err = AudioFileSetProperty(_exportFileID, 
										kAudioFilePropertyChannelLayout,
										layoutSize,
										(void*) layout);
			if (err)
				goto bail;
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
		[info setSamplesRemaining:duration ? (duration * asbd.mSampleRate) : -1];
		[info setLocationInFile:0];

		// If we can export on a worker thread, go do it
		if (extractionOnWorkerThread == YES) {
			[NSThread detachNewThreadSelector:@selector(exportExtractionThread:) toTarget:self withObject:info];
		} else {
			// Otherwise, since we're on the main thread we can just call the main-thread worker method
			[self exportOnMainThreadCallBack:(id)info];
		}
	}

bail:
	// Once the cloned movie has been opened, the handle can be disposed
	if (cloneHandle)
		DisposeHandle(cloneHandle);
	[info release];
	if (fileName)
		[fileName release];
	if (directory)
		[directory release];

	// If there was an error, we never spawned the worker thread to close the cloned movie
	if (err) {
		if (layout)
			free(layout);
		if (extractionOnWorkerThread == YES) {
			DisposeMovie(_clonedMovie);
			_clonedMovie = nil;
		}
		if (_exportFileID) {
			(void) AudioFileClose(_exportFileID);
			_exportFileID = nil;
			FSDeleteObject(&fileRef);
		}
	}
}

// Called when export is completed
// Changes the button state
- (void) exportCompletedNotification:(id)object
{
	// Set the Preview button back to its original state
	[_audExtractExportButton setTitle:@"Export"];
	[_audExtractExportButton setAction:@selector(doStartExport:)];
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
	SInt64	samplesRemaining = [info samplesRemaining];
	Boolean discrete = [info discrete]; 
	SInt64 ploc = [info locationInFile];
	TimeRecord startTime;
	MovieAudioExtractionRef extraction = nil;

	QTGetTimeRecord(startTimeInQTTime, &startTime);

	Boolean enterMoviesSucceeded=NO, attachedToThread = NO;
	
	// -----------------------------------------------------
	// Attach the movie to this thread.
	err = EnterMoviesOnThread(0);
	if (err)
		goto bail;
	enterMoviesSucceeded = YES;	
	
	err = AttachMovieToCurrentThread(_clonedMovie);
	if (err)
		goto bail;
	attachedToThread = YES;

	// ---------------------------------------------------------
	// Prepare for extraction
	
	// Open a session, configure audio format 
	err = prepareMovieForExtraction(_clonedMovie, 
									&extraction, 
									discrete, 
									asbd, 
									&layout, 
									&layoutSize, 
									startTime);						
	if (layout)
		free(layout);
	if (err)
		goto bail;

	//------------------------------------------------------------
	
	// Start exporting slices and loop until everything is written out

	Boolean			extractionComplete = NO;

	// Loop until stopped from an external event, or we've finished the entire extraction
	while (!_stopExport && !extractionComplete)
	{
		// If there are any samples left to export...		
		if (samplesRemaining == 0)
			extractionComplete = YES;
		if (!extractionComplete)
		{
			// We will read numSamplesThisSlice number of samples
			SInt64 numSamplesThisSlice = samplesRemaining;
			if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
				numSamplesThisSlice = kMaxExtractionPacketCount;

			err = extractAudioToFile(extraction, _exportFileID, asbd, &numSamplesThisSlice, &ploc, &extractionComplete);	
			if (err)
				break;
			if (samplesRemaining != -1)
				samplesRemaining -= numSamplesThisSlice;
		}
	}

bail:
	(void) AudioFileClose(_exportFileID);
	_exportFileID = nil;
	if (extraction)
		(void) MovieAudioExtractionEnd(extraction);

	DisposeMovie(_clonedMovie);
	_clonedMovie = nil;
	if (enterMoviesSucceeded)
		ExitMoviesOnThread();
		
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
	SInt64	samplesRemaining = [info samplesRemaining];
	Boolean discrete = [info discrete]; 
	SInt64 ploc = [info locationInFile];
	TimeRecord startTime;
	MovieAudioExtractionRef extraction = [info extractionSessionRef];

	QTGetTimeRecord(startTimeInQTTime, &startTime);

	// ---------------------------------------------------------
	// Prepare for extraction if this is the first entry

	if (extraction == nil)	
	{	
		// Open a session, configure audio format 
		err = prepareMovieForExtraction([_currentMovie quickTimeMovie], 
										&extraction, 
										discrete, 
										asbd, 
										&layout, 
										&layoutSize, 
										startTime);						
		if (layout)
			free(layout);
		if (err)
			goto bail;
		// Save for future entries
		[info setExtractionSession:extraction];	
	}

	// Export a slice now and then reschedule for another entry soon.
	Boolean extractionComplete = NO;
	
	if (!_stopExport)
	{
		// If there are any samples left to export...		
		if (samplesRemaining == 0)
			extractionComplete = YES;
		if (!extractionComplete)
		{
			// We will read numSamplesThisSlice number of samples
			SInt64 numSamplesThisSlice = samplesRemaining;
			if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
				numSamplesThisSlice = kMaxExtractionPacketCount;

			err = extractAudioToFile(extraction, _exportFileID, asbd, &numSamplesThisSlice, &ploc, &extractionComplete);	
			if (err)
				goto bail;
			if (samplesRemaining != -1) {
				samplesRemaining -= numSamplesThisSlice;
				if (samplesRemaining == 0)
					extractionComplete = YES;
			}
		}
	}
	// Save updated state for the next time through this method
	[info setLocationInFile:ploc];
	[info setSamplesRemaining:samplesRemaining];

bail:
	if (err || extractionComplete || _stopExport)
	{
		err = AudioFileClose(_exportFileID);
		_exportFileID = nil;
		if (extraction)
			(void) MovieAudioExtractionEnd(extraction);

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
	Handle cloneHandle = nil;
	Boolean extractionOnWorkerThread = NO;
	OSStatus err = noErr;

	// Clone the movie, test if clone can be migrated to a 
	// worker thread for extraction and playback, and set a flag accordingly

	_stopPreview = false;	// when this is set true, preview stops on the next cycle

	cloneHandle = NewHandle(0);
	if (cloneHandle == nil) {
		err = memFullErr;
		goto bail;
	}
	err = PutMovieIntoHandle([_currentMovie quickTimeMovie], cloneHandle);
	if (err != noErr)
		goto bail;
	err = NewMovieFromHandle(&_clonedMovie, cloneHandle, newMovieActive, NULL);
	if (err != noErr || _clonedMovie == nil)
		goto bail;
	if (DetachMovieFromCurrentThread(_clonedMovie) == noErr)
	{
		extractionOnWorkerThread = YES;
	}
	else
	{
		// If we could not migrate this movie, dispose the clone and
		// extract from the original movie on the main thread.
		DisposeMovie(_clonedMovie);
		_clonedMovie = nil;
	}

	// Read the UI, get the required extraction layout, layoutsize, 
	// extraction startTime, duration and whether we need to 
	// extract in the "All Channels Discrete" mode
	err = [self getExtractionParameters:(AudioChannelLayout**)&layout 
										outLayoutSize:(UInt32*)&layoutSize
										outASBD:&asbd
										startTime:(TimeRecord*)&startTime 
										duration:(Float64*)&duration 
										allDiscrete:(Boolean*)&discrete];
	if (err)
		goto bail;

	// Build an AU Graph with a scheduled sound player unit and
	// an output unit for playback
	err = BuildAUGraphPlayer(layout, layoutSize, &asbd, &_graphUnit, &_thePlayerUnit);
	if (err)
		goto bail;
	
	// Package the refCon information that will be sent to the preview thread/timer
	QTTime startTimeInQTTime = QTMakeTimeWithTimeRecord(startTime);
	InfoForCallback *info = [[InfoForCallback alloc] init];
	[info setASBD:asbd];
	[info setDiscrete:discrete];
	[info setLayout:layout];				
	[info setLayoutSize:layoutSize];
	[info setStartTime:startTimeInQTTime];
	[info setSamplesRemaining:duration ? (duration * asbd.mSampleRate) : -1];
	
	// If we can preview on a worker thread, go do it
	if (extractionOnWorkerThread == YES) {
		[NSThread detachNewThreadSelector:@selector(previewExtractionThread:) toTarget:self withObject:info];
	} else {
		// Otherwise, since we're on the main thread we can just call the main-thread worker method
		[self previewOnMainThreadCallBack:(id)info];
	}

bail:
	// Once the cloned movie has been opened, the handle can be disposed
	if (cloneHandle)
		DisposeHandle(cloneHandle);
	[info release];

	// If there was an error, we never spawned the worker thread to close the cloned movie
	if (err) {
		if (layout)
			free(layout);
		if (extractionOnWorkerThread == YES) {
			DisposeMovie(_clonedMovie);
			_clonedMovie = nil;
		}
	}
}


- (void) previewCompletedNotification:(id)object
{
	// Stop and dispose the playback AudioUnit Graph
	(void) CloseAUGraphPlayer(_graphUnit);
	_graphUnit = nil;

	// Set the Preview button back to its original state
	[_audExtractPreviewButton setTitle:@"Preview"];
	[_audExtractPreviewButton setAction:@selector(doStartPreview:)];
}


const UInt32 numSlices = 3;		// number of slices we try to keep in the play queue


// Slice Rendered Completion Callback
void previewAudioSliceCompletionProc(void *userData, struct ScheduledAudioSlice *bufferList)
{
	id condLock = (NSConditionLock*)userData;

	// Signal the sleeping preview thread that a slice has been rendered.
	if (condLock != nil) {
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
	SInt64	samplesRemaining = [info samplesRemaining];
	Boolean discrete = [info discrete];
	TimeRecord startTime;
	MovieAudioExtractionRef extraction = nil;
	Boolean			extractionComplete = NO;
	UInt32			numSlicesFree = numSlices;
	ScheduledAudioSlice sliceList[numSlices];
	Float64			sampleTimeStamp = 0.;
	Boolean			playerUnitStarted = false;


	QTGetTimeRecord(startTimeInQTTime, &startTime);

	Boolean enterMoviesSucceeded=NO, attachedToThread = NO;
	
	// -----------------------------------------------------
	// Attach the movie to this thread.
	err = EnterMoviesOnThread(0); 
	if (err)
		goto bail;
	enterMoviesSucceeded = YES;	
	
	err = AttachMovieToCurrentThread(_clonedMovie);
	if (err)
		goto bail;
	attachedToThread = YES;
	
	// ---------------------------------------------------------
	// Prepare for extraction
	
	// Open a session, configure audio format 
	err = prepareMovieForExtraction(_clonedMovie, 
									&extraction, 
									discrete, 
									asbd, 
									&layout, 
									&layoutSize, 
									startTime);						
	if (layout)
		free(layout);
	if (err)
		goto bail;
	//-----------------------------------------------------------
	
	// Start scheduling slices and loop until everything is played out
	err = [self previewBufferAllocate:sliceList
										numSlices:numSlices
										asbd:asbd
										lock:(void *)condLock];
	if (err)
		goto bail;

	// Loop until stopped from an external event, or we've played out the entire extraction
	while (!_stopPreview && !(extractionComplete && (numSlicesFree == numSlices)))
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
			break;

		// Start the AUGraph, it is wasn't already running.
		// Then the next time around the loop, we'll wait until the slice completion callback
		// has set the condition lock state.
		if (playerUnitStarted == false)
		{
			// Start the AUGraph player
			err = StartAUGraphPlayer(_graphUnit);
			if (err) {
				goto bail;
			}		
			playerUnitStarted = true;
		}
	}		
		
bail:
	if (extraction)
		(void) MovieAudioExtractionEnd(extraction);

	(void) StopAUGraphPlayer(_graphUnit);
	[self previewBufferDeallocate:sliceList numSlices:numSlices];

	DisposeMovie(_clonedMovie);
	_clonedMovie = nil;
	if (enterMoviesSucceeded)
		ExitMoviesOnThread();

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
	SInt64	samplesRemaining = [info samplesRemaining];
	Boolean discrete = [info discrete];
	TimeRecord startTime;
	Boolean			playerUnitStarted = [info playerUnitStarted];
	Float64			sampleTimeStamp = [info sampleTimeStamp];
	ScheduledAudioSlice *sliceList = [info sliceList];
	MovieAudioExtractionRef extraction = [info extractionSessionRef];

	QTGetTimeRecord(startTimeInQTTime, &startTime);

	// ---------------------------------------------------------
	// Prepare for extraction if this is the first entry

	if (extraction == nil)	
	{	
		// Open a session, configure audio format 
		err = prepareMovieForExtraction([_currentMovie quickTimeMovie], 
										&extraction, 
										discrete, 
										asbd, 
										&layout, 
										&layoutSize, 
										startTime);						
		if (layout)
			free(layout);
		if (err)
			goto bail;
		// Save for future entries
		[info setExtractionSession:extraction];	
	}
	if (sliceList == nil)
	{
		sliceList = (ScheduledAudioSlice *) calloc(numSlices, sizeof(ScheduledAudioSlice));
		if (sliceList == nil) {
			err = memFullErr;
			goto bail;
		}
		err = [self previewBufferAllocate:sliceList
									numSlices:numSlices
									asbd:asbd
									lock:(void *)nil];
		if (err)
			goto bail;

		[info setSliceList:sliceList];
	}

	//-----------------------------------------------------------
	
	UInt32			numSlicesFree;
	Boolean extractionComplete = NO;
	
	if (!_stopPreview)
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
		err = StartAUGraphPlayer(_graphUnit);
		if (err) {
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
	if (err || extractionComplete || _stopPreview)
	{
		if (extraction)
			(void) MovieAudioExtractionEnd(extraction);

		(void) StopAUGraphPlayer(_graphUnit);
		if (sliceList != nil) {
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
		if (bufList == nil) {
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
			free (sliceList[sliceNumber].mBufferList);
				
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
				*outExtractionComplete = true;
			if (!*outExtractionComplete)
			{
				// We will read numSamplesThisSlice number of samples
				SInt64 numSamplesThisSlice = *ioSamplesRemaining;
				if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
					numSamplesThisSlice = kMaxExtractionPacketCount;

				sliceList[sliceNumber].mTimeStamp.mSampleTime = *ioSampleTimeStamp;
				err = extractSliceAndScheduleToPlay(extraction,
													asbd, 
													_thePlayerUnit,
													&sliceList[sliceNumber], 
													&numSamplesThisSlice,
													outExtractionComplete);
				if (!err && (numSamplesThisSlice > 0)) {
					numSlicesFree--;
				} else {
					sliceList[sliceNumber].mFlags = kScheduledAudioSliceFlag_Complete;
				}
				// Whether there was an error or not, numSamplesThisSlice is valid
				*ioSampleTimeStamp += numSamplesThisSlice;
				if (*ioSamplesRemaining != -1)
					*ioSamplesRemaining -= numSamplesThisSlice;
			}
		}
	}
	return (numSlicesFree);
}

#pragma mark
#pragma mark ---- NSTableView DataSource Methods ----------

// Return the number of channels in a particular view.
// This is always called before tableView:objectValueForTableColumn:row
-(int) numberOfRowsInTableView:(NSTableView*)tableView
{
	OSStatus	err;
	UInt32		numberOfRows = 0;

	if (_currentMovie == nil)
		return numberOfRows;
	
	// Table that displays track information
	if ([tableView isEqual:_audTrackChannelLayoutTableView])
	{
		if ([_audTrackSelectorPopUpButton indexOfSelectedItem] != 0 && _currentTrack == nil)	
				return numberOfRows;
				
		// If summary layout needs to be displayed
		if ([_audTrackSelectorPopUpButton indexOfSelectedItem] == 0) {
			if (_summaryLayout) 
				numberOfRows = _summaryLayout->mNumberChannelDescriptions;			
		} else {				
			AudioChannelLayout *trackLayout; 
			UInt32 trackLayoutSize;
			if (noErr == getTrackLayoutAndSize([_currentTrack quickTimeTrack], &trackLayoutSize, &trackLayout))
			{
				UInt32 numberOfRowsSize = sizeof(numberOfRows);
				(void) AudioFormatGetProperty(kAudioFormatProperty_NumberOfChannelsForLayout, 
															trackLayoutSize, 
															trackLayout, 
															&numberOfRowsSize, 
															&numberOfRows);	
				if (trackLayout)
					free(trackLayout);											
			}
			
		}			
	}
	// Table that displays device channel information
	else if ([tableView isEqual:_audDeviceChannelLayoutTableView])
	{
		if (_deviceLayout)
			numberOfRows = (int)_deviceLayout->mNumberChannelDescriptions;
	}
	// Table that displays extraction information
	else if ([tableView isEqual:_audExtractChannelLayoutTableView])
	{
		// If we have a cached extraction layout, use it.
		// Otherwise, initialize and cache it here.
		if (_extractionLayout == nil)
		{
			// Tag of -1 indicates a custom layout (which we won't have with a nil _extractionLayout)

			// Tag of 0 indicates the default (summary) layout
			if ([[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item] == 0)
			{
				err = getDefaultExtractionLayout([_currentMovie quickTimeMovie], nil, &_extractionLayout, nil);
			}
			// If we are doing an AllChannelsDiscrete extraction, construct an appropriate channel layout
			else if ((AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item] ==
								kAudioChannelLayoutTag_DiscreteInOrder)
			{
				err = getDiscreteExtractionLayout([_currentMovie quickTimeMovie], nil, &_extractionLayout);		
			}
			// Otherwise, get the layout for the tag identified in the menu item
			else
			{
				err = getLayoutForTag((AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item],
											nil, &_extractionLayout);		
			}
			if (err)
				return numberOfRows;
		}
		numberOfRows = _extractionLayout->mNumberChannelDescriptions;
	}
		
	return numberOfRows;																								
	
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	NSString *column_id = [tableColumn identifier];
	id returnVal = nil; 
	
	if ([tableView isEqual:_audTrackChannelLayoutTableView])
	{
		if ([_audTrackSelectorPopUpButton indexOfSelectedItem] == 0)
		{
			if (_summaryLayout) {
				// Row 'n' contains info on channel 'n+1'
				if ([column_id isEqualToString:@"channel"]) {
					returnVal = (id)[NSNumber numberWithInt:row+1];
				} 
				// Since the pop-up button for a summary layout
				// will contain a menu with only 1 item, we always
				// return the index of this first and only item
				else if ([column_id isEqualToString:@"assignment"]) {
					returnVal = (id) [NSNumber numberWithInt:0];
				}
			}
			return returnVal;
		}
		else if (_currentTrack != nil)
		{	
			if ([column_id isEqualToString:@"channel"]) {
				returnVal = (id)[NSNumber numberWithInt:row+1];
			} else if ([column_id isEqualToString:@"assignment"]) {
				
				UInt32 trackLayoutSize = 0;
				AudioChannelLayout *trackLayout = NULL;
				if (noErr == getTrackLayoutAndSize([_currentTrack quickTimeTrack], &trackLayoutSize, &trackLayout)) 	
				{
					int index;
					for (index=0; index < [_trackChannelLabelNames count]; index++)
					{
						if (trackLayout->mChannelDescriptions[row].mChannelLabel == [(InfoObject*)[_trackChannelLabelNames objectAtIndex:index] item]) 
						{
							returnVal = (id) [NSNumber numberWithInt:index];
								
							if (row < [_trackChannelLabelsIndexOfSelectedMenuItemArray count])
								[_trackChannelLabelsIndexOfSelectedMenuItemArray replaceObjectAtIndex:row withObject:[NSNumber numberWithInt:index]];
							else
								[_trackChannelLabelsIndexOfSelectedMenuItemArray insertObject:[NSNumber numberWithInt:index] atIndex:row];
							break;
						}
					}
				}
				if (trackLayout)
					free(trackLayout);
			} 			
		}
	}
	else if ([tableView isEqual:_audDeviceChannelLayoutTableView])
	{
		if ([column_id isEqualToString:@"channel"]) {
			returnVal = (id)[NSNumber numberWithInt:row];
		} else if ([column_id isEqualToString:@"assignment"]) {
			NSString *labelStr = [NSString stringWithString:@""];
			UInt32 labelSize = sizeof(NSString*);
			AudioChannelDescription acd = {0};
			if (_deviceLayout) 
			{
				acd.mChannelLabel = (AudioChannelLabel)(_deviceLayout->mChannelDescriptions[row]).mChannelLabel;
				(void)AudioFormatGetProperty(kAudioFormatProperty_ChannelName, 
													sizeof(AudioChannelDescription), 
													&acd, 
													&labelSize, 
													&labelStr);
			}
			returnVal = (id)labelStr;
		
		} 
	}
	else if ([tableView isEqual:_audExtractChannelLayoutTableView])
	{
		if ([column_id isEqualToString:@"channel"]) {
			returnVal = (id)[NSNumber numberWithInt:row];
		} else if ([column_id isEqualToString:@"assignment"]) {

			// If we are in All Channels Discrete mode, you cannot select anything
			if ((AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item] ==
				kAudioChannelLayoutTag_DiscreteInOrder)
			{
				returnVal = (id)[NSNumber numberWithInt:0];
			}
			else {
				int index;

				// The channel selectors are always in the same order, with the custom value, if any, at the end
				for (index = 0; index < [_extractionChannelLabelNames count]; index++)
				{
					if (_extractionLayout->mChannelDescriptions[row].mChannelLabel ==
							[(InfoObject*)[_extractionChannelLabelNames objectAtIndex:index] item])
					{
						returnVal = (id)[NSNumber numberWithInt:index];
						break;
					}
				}
				// If we didn't find the label, then the last item (which index now reflects) is the right one.
				if (returnVal == nil)
				{
					returnVal = (id)[NSNumber numberWithInt:index];
				}
			}
		} 
	}

	return returnVal;
}

-(void) tableView:(NSTableView*)tableView setObjectValue:(id)value forTableColumn:(NSTableColumn*)tableColumn row:(int)row
{
	if ([tableView isEqual:_audTrackChannelLayoutTableView])
	{
		AudioChannelLayout* trackChannelLayout = NULL;
		UInt32 i;
		OSStatus err = noErr;
		UInt32 trackChannelLayoutSize;
		
		// Allocate a layout of the required size
		trackChannelLayoutSize = fieldOffset(AudioChannelLayout, mChannelDescriptions[[tableView numberOfRows]]);
		trackChannelLayout = (AudioChannelLayout*)calloc(1, trackChannelLayoutSize);
		trackChannelLayout->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
		trackChannelLayout->mNumberChannelDescriptions = [tableView numberOfRows];
	
		// Set the channel labels for each channel in the layout
		[_trackChannelLabelsIndexOfSelectedMenuItemArray replaceObjectAtIndex:row withObject:(NSNumber*)value];
		for (i = 0; i < trackChannelLayout->mNumberChannelDescriptions; i++)
		{
			trackChannelLayout->mChannelDescriptions[i].mChannelLabel =  [[[_trackChannelLabelsMenusArray objectAtIndex:i] itemAtIndex:[[_trackChannelLabelsIndexOfSelectedMenuItemArray objectAtIndex:i] intValue]] tag];
		}
		
		// Set the track layout
		err = setTrackLayout([_currentTrack quickTimeTrack],  trackChannelLayoutSize, trackChannelLayout);
		
		if (trackChannelLayout)
			free(trackChannelLayout);
		if (err == noErr) {	
			[self getSummaryChannelLayout];
			[self setSummaryChannelLayoutTextField];

			// If the Default (Summary) or All Discrete extraction type is selected,
			// it should be refreshed now.
			[self refreshExtractionTableView];
		}	
	}
	else if ([tableView isEqual:_audExtractChannelLayoutTableView])
	{
		UInt32				index;
		AudioChannelLabel	channelLabel;

		// The extraction layout should never be nil here
		if (_extractionLayout == nil)
			return;			

		// Get the channel label for this popup selector index.
		// If it is not in the table of names, it is the custom item.
		index = [(NSNumber*)value intValue];
		if (index < [_extractionChannelLabelNames count])
		{
			channelLabel = [(InfoObject*)[_extractionChannelLabelNames objectAtIndex:index] item];
		}
		else
		{
			// This should never happen, since there is no callback when the selected item doesn't change.
			// Once a custom tag is set to one from the table, the custom menu item is removed.
			channelLabel = _extractionLayout->mChannelDescriptions[row].mChannelLabel;
		}

		// If the channel label changed, then we make a new layout and associate it with a channel tag, if possible
		if (channelLabel != _extractionLayout->mChannelDescriptions[row].mChannelLabel)
		{
			AudioChannelLayout		*newLayout = nil;
			AudioChannelLayoutTag	newTag;
			size_t					size;
			Boolean					foundMatch = false;

			// Make an updated layout and replace the cached copy
			size = fieldOffset(AudioChannelLayout, mChannelDescriptions[_extractionLayout->mNumberChannelDescriptions]);
			newLayout = calloc(1, size);
			if (newLayout == nil)
				return;
			memcpy(newLayout, _extractionLayout, size);
			newLayout->mChannelDescriptions[row].mChannelLabel = channelLabel;
			free(_extractionLayout);
			_extractionLayout = newLayout;

			// See if this new layout matches any of the top-level popup selector tags
			(void) getTagForLayout(_extractionLayout, size, &newTag);
			if (newTag != kAudioChannelLayoutTag_UseChannelDescriptions)
			{
				for (index = 0; index < [_audExtractLayoutSelectorPopUpButton numberOfItems]; index++)
				{
					// If the channel tag matches one in the top-level list, then set the top-level item to it.
					if (newTag == [[[_audExtractLayoutSelectorPopUpButton itemAtIndex:index] representedObject] item])
					{
						foundMatch = true;
						[_audExtractLayoutSelectorPopUpButton selectItemAtIndex:index];
					}
				}
			}
			// If we found a match, make sure "Custom" is no longer in the popup, otherwise make sure it is.
			index = [_audExtractLayoutSelectorPopUpButton indexOfItemWithTitle:@"Custom"];
			if (foundMatch)
			{
				if (index != -1)
				{
					// Remove the item and the separator preceding it.
					[[_audExtractLayoutSelectorPopUpButton menu] removeItemAtIndex:index];
					[[_audExtractLayoutSelectorPopUpButton menu] removeItemAtIndex:index-1];
				}
			}
			else
			{
				if (index == -1)
				{
					// Add a separating line and "Custom" menu item
					[[_audExtractLayoutSelectorPopUpButton menu] addItem:[NSMenuItem separatorItem]];
					[_audExtractLayoutSelectorPopUpButton addItemWithTitle:@"Custom"];
					[[_audExtractLayoutSelectorPopUpButton lastItem]
								setRepresentedObject:[InfoObject infoObjectWithItem:-1 andItemName:@"Custom"]];
					index = [_audExtractLayoutSelectorPopUpButton indexOfItemWithTitle:@"Custom"];
				}
				[_audExtractLayoutSelectorPopUpButton selectItemAtIndex:index];
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
	UInt32 index;
	AudioChannelDescription acd = {0};
	NSString* labelStr;
	UInt32 labelSize = sizeof(NSString*);
	
	[dataCell setControlSize:NSMiniControlSize];
	[dataCell setFont:[NSFont menuFontOfSize:10]];
	
	// Track/Summary Layout Table
	if ([tableView isEqual:_audTrackChannelLayoutTableView]) 
	{			
		if ([_audTrackSelectorPopUpButton indexOfSelectedItem] == 0 && _summaryLayout) 
		{
				[dataCell setEditable:NO];
				for (index=0; index < [_trackChannelLabelNames count]; index++) 
				{
					if (_summaryLayout->mChannelDescriptions[row].mChannelLabel == [(InfoObject *)[_trackChannelLabelNames objectAtIndex:index] item])
					{
						acd.mChannelLabel = [(InfoObject *)[_trackChannelLabelNames objectAtIndex:index] item];
						if (noErr == AudioFormatGetProperty(kAudioFormatProperty_ChannelName, 
												sizeof(AudioChannelDescription), 
												&acd, 
												&labelSize, 
												&labelStr))
						{
							menuItem = [[NSMenuItem alloc] init];
							[menuItem setTitle:labelStr];
							[labelStr release];
							[menuItem setTag:acd.mChannelLabel];
							[menu addItem:menuItem];
							[menuItem release];
							break;
						}
					}
					
				}
		}
		
		else // Regular track 
		{
			for (index = 0; index < [_trackChannelLabelNames count]; index++)
			{
					menuItem = [[NSMenuItem alloc] init];
					[menuItem setTitle:[(InfoObject*)[_trackChannelLabelNames objectAtIndex:index] itemName]];
					[menuItem setTag:[(InfoObject*)[_trackChannelLabelNames objectAtIndex:index] item]];				
					[menu addItem:menuItem];
					[menuItem release];

			}
		}
		[_trackChannelLabelsMenusArray insertObject:dataCell atIndex:row];
	} 
	// Extraction layout table
	else if ([tableView isEqual:_audExtractChannelLayoutTableView]) 
	{
		// All Discrete is effectively immutable: we only populate the menu with the specific discrete channel label
		if ((AudioChannelLayoutTag)[[[_audExtractLayoutSelectorPopUpButton selectedItem] representedObject] item] ==
				 kAudioChannelLayoutTag_DiscreteInOrder)
		{
			AudioFormatGetProperty(kAudioFormatProperty_ChannelName, 
								   sizeof(AudioChannelDescription), 
								   &_extractionLayout->mChannelDescriptions[row], 
								   &labelSize, 
								   &labelStr);
			menuItem = [[NSMenuItem alloc] init];
			[menuItem setTitle:labelStr];
			[labelStr release];
			[menuItem setTag:_extractionLayout->mChannelDescriptions[row].mChannelLabel];
			[menu addItem:menuItem];
			[menuItem release];
			[dataCell setEditable:NO];	// make sure that the setObjectValue callback is never called
		}
		else
		{
			// Populate the popup with a standard set of labels,
			// but make sure the current label name is in the list (at the end, if necessary).
			Boolean	labelFound = false;
			
			for (index = 0; index < [_extractionChannelLabelNames count]; index++)
			{
				menuItem = [[NSMenuItem alloc] init];
				[menuItem setTitle:[(InfoObject*)[_extractionChannelLabelNames objectAtIndex:index] itemName]];
				[menuItem setTag:[(InfoObject*)[_extractionChannelLabelNames objectAtIndex:index] item]];				
				if (_extractionLayout->mChannelDescriptions[row].mChannelLabel ==
								[(InfoObject*)[_extractionChannelLabelNames objectAtIndex:index] item])
					labelFound = true;
				[menu addItem:menuItem];
				[menuItem release];
			}
			if (!labelFound)
			{
				// Make sure the current channel label is in the popup
				AudioFormatGetProperty(kAudioFormatProperty_ChannelName, 
									   sizeof(AudioChannelDescription), 
									   &_extractionLayout->mChannelDescriptions[row], 
									   &labelSize, 
									   &labelStr);
				
				menuItem = [[NSMenuItem alloc] init];
				[menuItem setTitle:labelStr];
				[menuItem setTag:_extractionLayout->mChannelDescriptions[row].mChannelLabel];
				[menu addItem:menuItem];
				[menuItem release];

				[labelStr release];
			}
		}
	}

	[dataCell setMenu:menu];
	[menu release];	
	return dataCell;
}

@end
