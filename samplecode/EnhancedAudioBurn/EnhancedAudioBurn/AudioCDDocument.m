/*
    File:  AudioCDDocument.m
    
    Contains:  Document class for handling creation of RedBook audio CDs
     
    Copyright:  (c) Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
              
    Change History (most recent first):
            1.0 (July 5th, 2005)
*/

#import "AudioCDDocument.h"
#import "AudioCDWindowController.h"
#import "EABDictionaryHelpers.h"
#import "EABQuickTimeHelpers.h"

/* Keys for the disc info dictionary */
NSString * const EABDiscCDTextEnabled			= @"EABDiscCDTextEnabled";
NSString * const EABDiscMCNEnabled				= @"EABDiscMCNEnabled";
NSString * const EABDiscMCN						= @"EABDiscMCN";

/* Keys for the track info dictionaries */
NSString * const EABTrackFilePath				= @"EABTrackFilePath";
NSString * const EABTrackLength					= @"EABTrackLength";
NSString * const EABTrackPreGap					= @"EABTrackPreGap";
NSString * const EABTrackPreEmphasisEnabled		= @"EABTrackPreEmphasisEnabled";
NSString * const EABTrackIndexPointsEnabled		= @"EABTrackIndexPointsEnabled";
NSString * const EABTrackIndexPoints			= @"EABTrackIndexPoints";
NSString * const EABTrackISRCEnabled			= @"EABTrackISRCEnabled";
NSString * const EABTrackISRC					= @"EABTrackISRC";
NSString * const EABTrackISRCInCDText			= @"EABTrackISRCInCDText";

/* Notifications */
NSString * const EABAudioCDTrackListChanged		= @"EABAudioCDTrackListChanged";
NSString * const EABAudioCDDiscPropertyChanged	= @"EABAudioCDDiscPropertyChanged";
NSString * const EABAudioCDTrackPropertyChanged	= @"EABAudioCDTrackPropertyChanged";
NSString * const EABWhichPropertyKey			= @"EABWhichPropertyKey";
NSString * const EABWhichTrackKey				= @"EABWhichTrackKey";


/* Localized strings */
NSString * EABUntitledCD = nil;



@interface AudioCDDocument (PrivateHelpers)
// Bottlenecks used for undo/redo
- (void)addTrackDictionary:(NSMutableDictionary*)track atIndex:(unsigned)index;
- (void)removeTrackDictionary:(NSMutableDictionary*)track atIndex:(unsigned)index;
- (void)moveTrackDictionary:(NSMutableDictionary*)track from:(unsigned)oldIndex to:(unsigned)newIndex;

// Called when the track list changes (tracks added, removed, reorganized)
- (void)trackListChanged:(int)trackIndex;

// Called when a disc property changes
- (void)discPropertyChanged:(NSString*)property;

// Called when a track property changes
- (void)track:(unsigned)index propertyChanged:(NSString*)property;

// Returns properly formatted MCN or ISRC data
- (NSData*)mcnDataForDisc;
- (NSData*)isrcDataForTrack:(unsigned)index;
@end



#pragma mark -



@implementation AudioCDDocument

// --------------------------------------------------------------------------
//	init
// --------------------------------------------------------------------------
//	NSObject - initializes a new instance.
//
- (id)init
{
    [super init];
    if (self) {
    
		// Load the appropriate untitled string.
		if (EABUntitledCD == nil)
			EABUntitledCD = NSLocalizedString(@"Untitled CD",@"Name for new document");
		
		// Allocate an array to hold both disc and track info dictionaries.
		trackArray = [[NSMutableArray alloc] initWithCapacity:100];
		
		// There's always at least one entry in the track array, the entry for the disc.
		NSMutableDictionary *discInfo = [NSMutableDictionary dictionary];
		[trackArray addObject:discInfo];
		
		// Initialize an untitled document.
		[discInfo setObject:[NSNumber numberWithBool:NO] forKey:EABDiscCDTextEnabled];
		[discInfo setObject:[NSNumber numberWithBool:NO] forKey:EABDiscMCNEnabled];
		[discInfo setObject:EABUntitledCD forKey:DRCDTextTitleKey];
    }
    return self;
}


// --------------------------------------------------------------------------
//	dealloc
// --------------------------------------------------------------------------
//	NSObject - releases memory for an instance.
//
- (void)dealloc
{
	// Log deallocations to make sure they're happening when we expect them to.
	//NSLog(@"-[AudioCDDocument dealloc]");
	
	// We're going away, so nobody should be listening to notifications for us anymore.
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter] removeObserver:nil name:nil object:self];
	
	// Release our objects.
	[windowController release];
	[trackArray release];
	[_menuSaveDocument release];
	
	[super dealloc];
}


// --------------------------------------------------------------------------
//	displayName
// --------------------------------------------------------------------------
//	NSDocument - overridden to use a custom untitled string.
//
- (NSString *)displayName
{
	if ([self fileName] == nil)
		return EABUntitledCD;
	else
		return [super displayName];
}


// --------------------------------------------------------------------------
//	makeWindowControllers
// --------------------------------------------------------------------------
//	NSDocument - loads our custom window controller.
//
- (void)makeWindowControllers
{
	// Load the window controller.
	windowController = [[AudioCDWindowController alloc] initWithWindowNibName:@"AudioCDDocument"];
	[self addWindowController:windowController];
	
	// Get the "save" menu item since we're going to need it.
	NSMutableArray	*items = [NSMutableArray arrayWithArray:[[NSApp mainMenu] itemArray]];
	NSMenuItem		*item;
	while ((item = [items objectAtIndex:0]) != nil)
	{
		NSMenu *menu = [item submenu];
		if (menu != nil)
			[items addObjectsFromArray:[menu itemArray]];
		
		SEL	action = [item action];
		if (action == @selector(saveDocument:)) {
			_menuSaveDocument = [item retain];
			break;
		}
		
		[items removeObjectAtIndex:0];
	}
}


// --------------------------------------------------------------------------
//	validateMenuItem:
// --------------------------------------------------------------------------
//	NSDocument - overridden to transform "Save" on a new document into "Save As".
//
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
	BOOL	result = [super validateMenuItem:item];
	if (result == NO)
	{
		SEL	action = [item action];
		if (action == @selector(saveDocument:))
			result = YES;
	}
	return result;
}


// --------------------------------------------------------------------------
//	saveDocument:
// --------------------------------------------------------------------------
//	NSDocument - if the "Save" command is disabled, map it to "Save As".
//
- (IBAction)saveDocument:(id)sender
{
	if ([super validateMenuItem:_menuSaveDocument] == NO)
		[self saveDocumentAs:sender];
	else
		[super saveDocument:sender];
}



#pragma mark -
#pragma mark General accessors


// --------------------------------------------------------------------------
//	discInfo
// --------------------------------------------------------------------------
//	Returns a dictionary describing the disc.
//
- (NSDictionary*)discInfo
{
	return (NSDictionary*)[trackArray objectAtIndex:0];
}


// --------------------------------------------------------------------------
//	trackInfoForTrack
// --------------------------------------------------------------------------
//	Returns the info for a specific track.
//
- (NSDictionary*)trackInfoForTrack:(unsigned)zeroBasedIndex
{
	return (NSDictionary*)[trackArray objectAtIndex:zeroBasedIndex+1];
}

// --------------------------------------------------------------------------
//	trackInfo
// --------------------------------------------------------------------------
//	Returns the entire track info array.
//
- (NSArray*)trackInfo
{
	return trackArray;
}


// --------------------------------------------------------------------------
//	count
// --------------------------------------------------------------------------
//	Returns the number of tracks in the document.
//
- (unsigned)count
{
	unsigned	arraySize = [trackArray count];
	return (arraySize > 0) ? (arraySize - 1):0;
}


// --------------------------------------------------------------------------
//	totalLength
// --------------------------------------------------------------------------
//	Returns the total length of the tracks in the document.
//
- (DRMSF*)totalLength
{
	uint64_t	frames = 0;
	unsigned	i, count;
	
	for (i=0, count=[self count]; i<count; ++i)
	{
		NSDictionary	*trackInfo = [self trackInfoForTrack:i];
		frames += [[trackInfo numberForKey:EABTrackLength] unsignedLongLongValue];
	}
	
	return [DRMSF msfWithFrames:(unsigned long)frames];
}


// --------------------------------------------------------------------------
//	hasValidISRCForTrack:
// --------------------------------------------------------------------------
//	Returns whether the track ISRC is valid.  Empty is considered valid.
//
- (BOOL)hasValidISRCForTrack:(unsigned)index
{
	NSDictionary	*trackInfo = [self trackInfoForTrack:index];
	NSString		*isrc = [trackInfo stringForKey:EABTrackISRC];
	
	// Check for empty ISRC.
	if ([isrc isEqualToString:@""])
		return YES;
	
	// Call through to the other method.
	return [self isValidISRC:isrc incomplete:NO];
}



// --------------------------------------------------------------------------
//	isValidISRC:incomplete:
// --------------------------------------------------------------------------
//	Checks the ISRC string for validity.
//
- (BOOL)isValidISRC:(NSString*)isrc incomplete:(BOOL)incomplete
{
	// Get the string as ASCII, and make sure it's 12 bytes long.
	NSData *data = [isrc dataUsingEncoding:NSASCIIStringEncoding];
	if (data == nil)
		return NO;
	
	// Check the length.
	unsigned length = [data length];
	if (length > 12)
		return NO;
	if (!incomplete && length != 12)
		return NO;
	
	// Make sure the characters are within the right ranges.
	const char *byte = (char*)[data bytes];
	unsigned i;
	for (i=0; i<length; ++i)
	{
		char	c = byte[i];
		BOOL	alpha = (c >= 'A' && c <= 'Z');
		BOOL	num = (c >= '0' && c <= '9');
		if (i<2) {			// first two chars are A-Z
			if (!alpha) return NO;
		} else if (i<5) {	// next three chars are 0-9 A-Z
			if (!alpha && !num) return NO;
		} else	{			// remaining chars are 0-9
			if (!num) return NO;
		}
	}
	
	// Looks valid.
	return YES;
}



#pragma mark -
#pragma mark Disc and Track Properties


// --------------------------------------------------------------------------
//	discPropertyForKey:
// --------------------------------------------------------------------------
//	Returns the specified disc property.
//
- (id)discPropertyForKey:(NSString*)key
{
	// Sanity check that the key is valid.
	if (key == nil)
		return nil;
	
	return [(NSDictionary *)[trackArray objectAtIndex:0] objectForKey:key];
}

// --------------------------------------------------------------------------
//	setDiscProperty:forKey:
// --------------------------------------------------------------------------
//	Modifies the specified disc property.  A nil value removes the specified
//	property.
//
//	All modifications must go through here so that undo is properly supported.
//
- (void)setDiscProperty:(id)property forKey:(NSString*)key
{
	// Remap empty strings and zero numbers to nil.
	if (property != nil &&
		(([property isKindOfClass:[NSString class]] && [(NSString*)property length] == 0) ||
		 ([property isKindOfClass:[NSNumber class]] && [(NSNumber*)property intValue] == 0)))
		property = nil;
	
	// Sanity check that the key is valid.
	if (key == nil)
		return;
	
	// This is a no-op if the property isn't actually changing.
	id	currentValue = [self discPropertyForKey:key];
	if (property == currentValue || [property isEqual:currentValue])
		return;
	
	// Tell the undo manager how to undo this action.
	//NSLog(@"-[AudioCDDocument setDiscProperty:%@ forKey:%@]", property, key);
	[[[self undoManager] prepareWithInvocationTarget:self]
		setDiscProperty:currentValue forKey:key];
	
	// Set the property.
	if (property == nil)
		[(NSMutableDictionary *)[trackArray objectAtIndex:0] removeObjectForKey:key];
	else
		[(NSMutableDictionary *)[trackArray objectAtIndex:0] setObject:property forKey:key];
	
	// A disc property has changed.
	[self discPropertyChanged:key];
}


// --------------------------------------------------------------------------
//	propertyForKey:ofTrack:
// --------------------------------------------------------------------------
//	Returns the specified track property.
//
- (id)propertyForKey:(NSString*)key ofTrack:(unsigned)index
{
	// Sanity check that the key and index are valid.
	if (key == nil || index > [self count])
		return nil;
	
	return [(NSDictionary *)[trackArray objectAtIndex:(index+1)] objectForKey:key];
}


// --------------------------------------------------------------------------
//	setProperty:forKey:ofTrack:
// --------------------------------------------------------------------------
//	Modifies the specified track property.  A nil value removes the specified
//	property.
//
//	All modifications must go through here so that undo is properly supported.
//
- (void)setProperty:(id)property forKey:(NSString*)key ofTrack:(unsigned)index
{
	// Remap empty strings and zero numbers to nil.
	if (property != nil &&
		(([property isKindOfClass:[NSString class]] && [(NSString*)property length] == 0) ||
		 ([property isKindOfClass:[NSNumber class]] && [(NSNumber*)property intValue] == 0)))
		property = nil;
	
	// Sanity check that the key and index are valid.
	if (key == nil || index > [self count])
		return;
	
	// This is a no-op if the property isn't actually changing.
	id	currentValue = [self propertyForKey:key ofTrack:index];
	if (property == currentValue || [property isEqual:currentValue])
		return;
	
	// Tell the undo manager how to undo this action.
	//NSLog(@"-[AudioCDDocument setProperty:%@ forKey:%@ ofTrack:%u]", property, key, index);
	[[[self undoManager] prepareWithInvocationTarget:self]
		setProperty:currentValue forKey:key ofTrack:index];
	
	// Set the property.
	NSMutableDictionary	*dict = [trackArray objectAtIndex:(index+1)];
	if (property == nil)
		[dict removeObjectForKey:key];
	else
		[dict setObject:property forKey:key];
	
	// A track property has changed.
	[self track:index propertyChanged:key];
}


#pragma mark -
#pragma mark Track List



// --------------------------------------------------------------------------
//	addTrackFromFile:
// --------------------------------------------------------------------------
//	Creates a new track from the file, appending it to the end of the list.
//
- (void)addTrackFromFile:(NSString*)pathToFile
{
	[self addTrackFromFile:pathToFile atIndex:[self count]];
}

// --------------------------------------------------------------------------
//	addTrackFromFile:atIndex:
// --------------------------------------------------------------------------
//	Creates a new track from the file, inserting at the specified zero-based
//	track index. 
//
- (void)addTrackFromFile:(NSString*)pathToFile atIndex:(unsigned)index
{
	DRTrack	*				track = nil;
	NSMutableDictionary*	trackInfo = nil;
	uint64_t				trackLength = 0;
	
	// Check that the array index is valid.
	if (index > [self count])
		[NSException raise:NSRangeException format:@"Invalid index %u when adding track - there are currently %u tracks", index, [self count]];
	
	// Double check that we aren't going over the capacity of 99 tracks.
	if ([self count] >= 99)
		[NSException raise:NSRangeException format:@"No more than 99 tracks allowed"];
	
	// Sanity check that the file is producable.  This track is disposable and
	//	we don't remember it; it's just thrown away into the autorelease pool.
	track = [DRTrack trackForAudioFile:pathToFile];
	trackLength = [track estimateLength];
	
	// Allocate and fill in a fresh, default track info dictionary.
	trackInfo = [NSMutableDictionary dictionary];
	[trackInfo setObject:pathToFile forKey:EABTrackFilePath];
	[trackInfo setObject:[NSNumber numberWithUnsignedLongLong:(unsigned long long)trackLength] forKey:EABTrackLength];
	[trackInfo setObject:[NSNumber numberWithBool:NO] forKey:EABTrackPreEmphasisEnabled];
	[trackInfo setObject:[NSNumber numberWithInt:1] forKey:EABTrackPreGap];
	[trackInfo setObject:[NSNumber numberWithBool:NO] forKey:EABTrackIndexPointsEnabled];
	[trackInfo setObject:[NSMutableArray arrayWithCapacity:98] forKey:EABTrackIndexPoints];
	[trackInfo setObject:[[pathToFile lastPathComponent] stringByDeletingPathExtension] forKey:DRCDTextTitleKey];
	
	// Open the file with QT to extract length and CD-Text information.
	Movie	movie = NULL;
	short	movieFile = -1;
	if (CreateMovieFromAudioFile(pathToFile,&movie,&movieFile) == noErr)
	{
		// Update the length if DRTrack couldn't estimate it.
		if (trackLength == 0)
		{
			trackLength = GetLengthOfMovie(movie);
			[trackInfo setObject:[NSNumber numberWithUnsignedLongLong:(unsigned long long)trackLength] forKey:EABTrackLength];
		}
		
		// Add the CD-Text if we find any.
		NSMutableDictionary*	cdtext = ExtractCDTextFromMovie(movie);
		[trackInfo addEntriesFromDictionary:cdtext];
		
		DisposeMovie(movie);
		CloseMovieFile(movieFile);
	}
	
	// Add the new track dictionary to the array.
	[self addTrackDictionary:trackInfo atIndex:index];
}

- (void)addTrackDictionary:(NSMutableDictionary*)track atIndex:(unsigned)index
{
	//NSLog(@"-[AudioCDDocument addTrackDictionary:atIndex:]");
	
	// Tell the undo manager how to undo this action.
	[[[self undoManager] prepareWithInvocationTarget:self]
		removeTrackDictionary:track atIndex:index];
	
	// Insert the dictionary into the array at the specified index.
	[trackArray insertObject:track atIndex:(index+1)];
	
	// The track list has changed.
	[self trackListChanged:(int)index];
}


// --------------------------------------------------------------------------
//	removeTrackAtIndex:
// --------------------------------------------------------------------------
//	Removes the track at the specified zero-based index.  Index 0 is track 1,
//	etc.
//
- (void)removeTrackAtIndex:(unsigned)index
{
	// Pass through to the other removal function so that the content
	//	of the track's dictionary will be preserved when undoing.
	[self removeTrackDictionary:[trackArray objectAtIndex:(index+1)] atIndex:index];
}

- (void)removeTrackDictionary:(NSMutableDictionary*)track atIndex:(unsigned)index
{
	//NSLog(@"-[AudioCDDocument removeTrackDictionary:atIndex:]");
	
	// Tell the undo manager how to undo this action.
	[[[self undoManager] prepareWithInvocationTarget:self]
		addTrackDictionary:track atIndex:index];
	
	// Remove the dictionary from the specified index.
	[trackArray removeObject:track inRange:NSMakeRange(index+1,1)];
	
	// The track list has changed.
	[self trackListChanged:(int)EABTracksRemoved];
}


// --------------------------------------------------------------------------
//	moveTracks:from:to:copying:
// --------------------------------------------------------------------------
//	Reorganizes the track list.  "tracks" is an array of NSNumbers
//	representing zero-based track indices.
//
//	Returns the new index of the first item.
//
- (unsigned)moveTracks:(NSArray*)array to:(unsigned)destination copying:(BOOL)copying
{
	//NSLog(@"-[AudioCDDocument moveTracks:to:copying:]");
	
	NSMutableArray	*movedTracks = [NSMutableArray arrayWithCapacity:[array count]];
	NSEnumerator	*iter = [array reverseObjectEnumerator];
	unsigned		result = destination;
	id				val;
	
	// Accumulate the selected elements from the track array, accumulating
	//	them in movedTracks.  We iterate backwards to avoid changing the
	//	dragged track numbers.
	while ((val = [iter nextObject]) != NULL)
	{
		int			trackToMove = [val intValue];
		NSMutableDictionary	*track = [trackArray objectAtIndex:(trackToMove+1)];
		
		// If we're copying then duplicate the item.
		if (copying)
			track = [track mutableCopy];
		
		// Accumulate this track into the array.
		[movedTracks addObject:track];
		
		// If we're moving then remove the original item.
		//	This may change the destination index.
		if (!copying)
		{
			[self removeTrackAtIndex:trackToMove];
			if (trackToMove <= destination)
				destination -= 1;
		}
	}
	
	// Preserve the new index since we're going to return it.
	result = destination;
	
	// Now add the objects back into the track array at the specified
	//	row.  We iterate movedTracks backwards, but since they were
	//	added backwards we're actually getting them in normal
	//	(lowest-to-highest) order!
	iter = [movedTracks reverseObjectEnumerator];
	while ((val = [iter nextObject]) != NULL)
		[self addTrackDictionary:val atIndex:(destination++)];
	
	return result;
}


- (unsigned)moveTrack:(unsigned)source to:(unsigned)destination copying:(BOOL)copying
{
	// We can pass this call through to the array variant.
	return [self moveTracks:[NSArray arrayWithObject:[NSNumber numberWithUnsignedInt:source]]
		to:destination copying:copying];
}




#pragma mark -
#pragma mark Posting Notifications


// --------------------------------------------------------------------------
//	trackListChanged:
// --------------------------------------------------------------------------
//	Called internally when the track list changes.
//
- (void)trackListChanged:(int)trackIndex
{
	// Post a notification to clients.
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter]
		postNotificationName:EABAudioCDTrackListChanged object:self
		userInfo:[NSDictionary
			dictionaryWithObject:[NSNumber numberWithInt:trackIndex]
			forKey:EABWhichTrackKey]];
}



// --------------------------------------------------------------------------
//	discPropertyChanged:
// --------------------------------------------------------------------------
//	Called internally when a disc property changes.
//
- (void)discPropertyChanged:(NSString*)key
{
	// Post a notification to clients.
	if (key == nil)
	{
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter]
			postNotificationName:EABAudioCDDiscPropertyChanged object:self];
	}
	else
	{
		[(NSNotificationCenter *)[NSNotificationCenter defaultCenter]
			postNotificationName:EABAudioCDDiscPropertyChanged object:self
			userInfo:[NSDictionary dictionaryWithObject:key forKey:EABWhichPropertyKey]];
	}
}



// --------------------------------------------------------------------------
//	track:propertyChanged:
// --------------------------------------------------------------------------
//	Called internally when a track property changes.
//
- (void)track:(unsigned)index propertyChanged:(NSString*)key
{
	[(NSNotificationCenter *)[NSNotificationCenter defaultCenter]
		postNotificationName:EABAudioCDTrackPropertyChanged object:self
		userInfo:[NSDictionary dictionaryWithObjectsAndKeys:
			key, EABWhichPropertyKey,
			[NSNumber numberWithUnsignedInt:index], EABWhichTrackKey, nil]];
}



#pragma mark -
#pragma mark Saving and Loading


// --------------------------------------------------------------------------
//	dataRepresentationOfType:
// --------------------------------------------------------------------------
//	NSDocument - flattens document into an NSData, for saving.
//
- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // All of our data is contained in a single array, so when creating
	//	our data representation we simply flatten that.
	NSString *errorDescription = nil;
	NSData *data = [NSPropertyListSerialization dataFromPropertyList:trackArray format:NSPropertyListXMLFormat_v1_0 errorDescription:&errorDescription];
	
	// Log errors if they occur here, which they shouldn't.
	if (data == nil || errorDescription)
		NSLog(@"Error serializing AudioCDDocument: %@", errorDescription);
    return data;
}

// --------------------------------------------------------------------------
//	loadDataRepresentation:ofType:
// --------------------------------------------------------------------------
//	NSDocument - reconstitutes docuemnt from an NSData, for loading.
//
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Reconstruct the track array we generated above.
	NSString *errorDescription = nil;
	NSPropertyListFormat format = NSPropertyListXMLFormat_v1_0;
	NSArray	*loaded = [NSPropertyListSerialization propertyListFromData:data
								mutabilityOption:NSPropertyListMutableContainers
										  format:&format
								errorDescription:&errorDescription];
	
	if (loaded == nil || errorDescription) {
		NSLog(@"Invalid AudioCDDocument: %@", errorDescription);
		return NO;
	}
	
	// Sanity check that it's really an array.
	if (![loaded isKindOfClass:[NSMutableArray class]]) {
		NSLog(@"Invalid AudioCDDocument - top-level plist is not an array!");
		return NO;
	}
	
	// Sanity check that there's at least 1 and no more than 100 items.
	//	That's 1 entry for the disc and up to 99 tracks.
	unsigned	count = [loaded count];
	if (count == 0 || count > 100) {
		NSLog(@"Invalid AudioCDDocument - %s items in top-level array!", (count == 0) ? "not enough":"too many");
		return NO;
	}
	
	// Iterate the array and make sure every item is a dictionary, and contains
	//	at least the required keys.
	unsigned	i;
	for (i=0; i<count; ++i)
	{
		NSMutableDictionary	*info = [loaded objectAtIndex:i];
		
		// Each element of the array has to be a dictionary.
		if (![info isKindOfClass:[NSMutableDictionary class]]) {
			NSLog(@"Invalid AudioCDDocument - element %d of top-level array isn't a dictionary!", i);
			return NO;
		}
		
		// The track file path is the only key that's absolutely required;
		//	all the rest can be reconstructed in document editing.
		NSString	*requiredKey = (i == 0) ? nil:EABTrackFilePath;
		if (i != 0 && ![[info objectForKey:EABTrackFilePath] isKindOfClass:[NSString class]])
		{
			NSLog(@"Invalid AudioCDDocument - element %d of top-level array is missing required key EABTrackFilePath!", i, requiredKey);
			return NO;
		}
	}
	
	// Okay, it looks valid.  Stash it away in our guts.
	[trackArray release];
	trackArray = [loaded retain];
	
	// Send notifications to any clients that might be listening.
	[self trackListChanged:EABTracksRemoved];
	for (i=0; i<count; ++i)
		[self trackListChanged:i];
	[self discPropertyChanged:nil];
	
    return YES;
}



#pragma mark -
#pragma mark Burning



// -------------------------------------------------------------------------------
//	setBurnProperties:
// -------------------------------------------------------------------------------
//	Fills in the properties for a DRBurn object according to the settings
//	in the document.
//
- (void)setBurnProperties:(DRBurn*)burn
{
	NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
	DRCDTextBlock		*cdtext = nil;
	NSData				*mcn = nil;
	NSMutableDictionary	*burnProperties = [[burn properties] mutableCopy];
	
	// Are we adding an MCN to the disc?
	if ([[self discPropertyForKey:EABDiscMCNEnabled] boolValue] != NO)
	{
		// Get the MCN.
		mcn = [self mcnDataForDisc];
		
		// Add it to the burn properties.
		if (mcn != nil)
			[burnProperties setObject:mcn forKey:DRMediaCatalogNumberKey];
	}
	
	// Are we adding CD-Text to the disc?
	if ([[self discPropertyForKey:EABDiscCDTextEnabled] boolValue] != NO)
	{
		// Allocate an empty CD-Text block.
		cdtext = [DRCDTextBlock cdTextBlockWithLanguage:@""
								encoding:DRCDTextEncodingISOLatin1Modified];
		
		// Go through the document and copy all of the CD-Text
		//	information into the block.
		unsigned	i, count = [trackArray count];
		for (i=0; i<count; ++i)
		{
			NSDictionary *dict = [trackArray objectAtIndex:i];
			NSArray *keys = [dict allKeys];
			NSEnumerator *iter = [keys objectEnumerator];
			NSString *key;
			while ((key = [iter nextObject]) != nil)
			{
				// Copy any CD-Text key into the CD-Text block.  EAB is
				//	the "namespace" we use for our application's private keys,
				//	and everything else is a CD-Text key.
				if (![key hasPrefix:@"EAB"])
					[cdtext setObject:[dict objectForKey:key] forKey:key ofTrack:i];
			}
			
			// If the track has an ISRC specified and it's being added to
			//	the CD-Text, do so.
			if (i>0 && [[dict numberForKey:EABTrackISRCInCDText] boolValue])
			{
				NSData *isrc = [self isrcDataForTrack:i-1];
				if (isrc != nil)
				{
					// Hyphenate the ISRC data to make it look nicer when it's
					//	in CD-Text.
					char	cstr[16];
					char	*ip = (char*)[isrc bytes];
					snprintf(cstr,sizeof(cstr),"%.2s-%.3s-%.2s-%.5s",&ip[0],&ip[2],&ip[5],&ip[7]);
					cstr[15] = 0;
					
					[cdtext setObject:[NSString stringWithUTF8String:cstr] forKey:DRCDTextMCNISRCKey ofTrack:i];
				}
			}
		}
		
		// If we had an MCN above, put it into the CD-Text too.
		if (mcn)
			[cdtext setObject:mcn forKey:DRCDTextMCNISRCKey ofTrack:0];
		
		// Add the CD-Text block to the burn properties.
		[burnProperties setObject:cdtext forKey:DRCDTextKey];
	}
	
	// Set the accumulated burn properties back onto the object.
	[burn setProperties:burnProperties];
	[pool release];
}



// -------------------------------------------------------------------------------
//	createLayoutForBurn
// -------------------------------------------------------------------------------
//	Creates an array of DRTracks from the settings in the document.
//
//	May raise an NSObjectNotAvailableException if a track was not found
//	or could not be imported.
//
- (id)createLayoutForBurn
{
	unsigned i, count = [self count];
	NSMutableArray	*tracks = [NSMutableArray arrayWithCapacity:count];
	
	for (i=0; i<count; ++i)
	{
		NSDictionary	*trackInfo = [self trackInfoForTrack:i];
		NSString		*path = [trackInfo objectForKey:EABTrackFilePath];
		
		// Create the track.
		DRTrack			*track = [DRTrack trackForAudioFile:path];
		if (track == nil)
		{
			NSLog(@"An error occurred at track %d: %@", (i+1), path);
			[NSException raise:NSObjectNotAvailableException
						format:@"Source file for track %u was not found, or could not be imported!  Filename was %@",
						(i+1), path];
		}
		
		// Set track properties from the document.
		unsigned	preGapLengthInFrames = (unsigned)([[trackInfo numberForKey:EABTrackPreGap] floatValue] * 75.0);
		
		NSMutableDictionary	*trackProperties = [[track properties] mutableCopy];
		[trackProperties setObject:[NSNumber numberWithUnsignedInt:preGapLengthInFrames] forKey:DRPreGapLengthKey];
		[trackProperties setObject:[trackInfo numberForKey:EABTrackPreEmphasisEnabled] forKey:DRAudioPreEmphasisKey];
		if ([[trackInfo numberForKey:EABTrackISRCEnabled] boolValue])
		{
			NSData *isrc = [self isrcDataForTrack:i];
			if (isrc)
				[trackProperties setObject:isrc forKey:DRTrackISRCKey];
		}
		if ([[trackInfo numberForKey:EABTrackIndexPointsEnabled] boolValue])
		{
			NSArray	*indexPoints = [trackInfo objectForKey:EABTrackIndexPoints];
			if (indexPoints)
				[trackProperties setObject:indexPoints forKey:DRIndexPointsKey];
		}
		[track setProperties:trackProperties];
		
		// Add this track to the list.
		[tracks addObject:track];
	}
	
	return tracks;
}


// -------------------------------------------------------------------------------
//	mcnDataForDisc
// -------------------------------------------------------------------------------
//	Returns an NSData for the MCN of the specified disc.  Only valid MCNs
//	are returned.
//
- (NSData*)mcnDataForDisc
{
	NSString *mcn = [self discPropertyForKey:EABDiscMCN];
	if (mcn)
	{
		// Convert the MCN into the appropriate format:
		//	an NSData containing 13 bytes.
		NSData *data = [mcn dataUsingEncoding:NSASCIIStringEncoding];
		if ([data length] == 13)
			return data;
	}
	return nil;
}


// -------------------------------------------------------------------------------
//	isrcDataForTrack:
// -------------------------------------------------------------------------------
//	Returns an NSData for the ISRC of the specified track.  Only valid ISRCs
//	are returned.
//
- (NSData*)isrcDataForTrack:(unsigned)index
{
	NSDictionary *trackInfo = [self trackInfoForTrack:index];
	NSString *isrc = [trackInfo objectForKey:EABTrackISRC];
	if (isrc && [self isValidISRC:isrc incomplete:NO])
	{
		// Convert the ISRC into the appropriate format:
		//	an NSData containing 12 bytes.
		NSData *data = [isrc dataUsingEncoding:NSASCIIStringEncoding];
		if ([data length] == 12)
			return data;
	}
	return nil;
}


@end
