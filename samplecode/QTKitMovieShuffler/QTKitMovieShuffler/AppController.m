/*

File: AppController.m

Abstract: Controller class to manage play buttons, volume slider and
		   movie list items

Version: <1.0>

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


#import "AppController.h"
#import "SimpleTreeNode.h"
#import "ImageAndTextCell.h"
#import "NSArray_Extensions.h"
#import "NSOutlineView_Extensions.h"
#import "QTMovieExtensions.h"

#import "QTUtils.h"


// ================================================================
// Useful Macros
// ================================================================

#define DragDropSimplePboardType 	@"MyCustomOutlineViewPboardType"
#define DragDropMoviePboardType		@"MyMoviePboardType"

#define INITIAL_INFODICT			@"InitInfo"
#define COLUMNID_IS_EXPANDABLE		@"IsExpandableColumn"
#define COLUMNID_NAME				@"NameColumn"
#define COLUMNID_NODE_KIND			@"NodeKindColumn"

// Conveniences for accessing nodes, or the data in the node.
#define NODE(n)						((SimpleTreeNode*)n)
#define NODE_DATA(n)				((SimpleNodeData*)[NODE((n)) nodeData])
#define SAFENODE(n)					((SimpleTreeNode*)((n)?(n):(treeData)))

@implementation AppController

- (id)init {
    NSDictionary *initData = nil;
    
    self = [super init];
    if (self==nil) return nil;
    
	// create an empty dictionary to start with...
	initData = [NSDictionary dictionary];
	
    treeData = [[SimpleTreeNode treeFromDictionary: initData] retain];
	    
    return self; 
}

- (void)dealloc {

    [treeData release];
    [draggedNodes release];
    [iconImages release];
    treeData = nil;
    draggedNodes = nil;
    iconImages = nil;
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];

    [super dealloc];
}

- (void)awakeFromNib {

    NSTableColumn *tableColumn = nil;
    ImageAndTextCell *imageAndTextCell = nil;

	[[NSNotificationCenter defaultCenter] addObserver:self
		selector:@selector(selectionDidChange:)
		name:@"NSOutlineViewSelectionDidChangeNotification" object:outlineView];

    // Insert custom cell types into the table view, the standard one does text only.
    // We want one column to have text and images.
    tableColumn = [outlineView tableColumnWithIdentifier: COLUMNID_NAME];
    imageAndTextCell = [[[ImageAndTextCell alloc] init] autorelease];
    [imageAndTextCell setEditable: YES];
    [tableColumn setDataCell:imageAndTextCell];


    // Register to get our custom type, strings, and filenames.... try dragging each into the view!
	[outlineView registerForDraggedTypes:[NSArray arrayWithObjects:
		DragDropSimplePboardType, 
		DragDropMoviePboardType, 
		NSFilenamesPboardType, 
		NSURLPboardType, 
		 nil]];

	// we want vertical motion treated as a drag, not
	// as a selection change
    [outlineView setVerticalMotionCanBeginDrag: YES];
	
	// initialize the current "active" row
	activeRow = 0;
}

- (NSArray*)draggedNodes   { return draggedNodes; }
- (NSArray *)selectedNodes { return [outlineView allSelectedItems]; }
- (BOOL)allowOnDropOnGroup { return YES; }
- (BOOL)allowOnDropOnLeaf  { return YES; }
- (BOOL)allowBetweenDrop   { return YES; }
- (BOOL)onlyAcceptDropOnRoot { return NO; }

// ================================================================
// Target / action methods. (most wired up in IB)
// ================================================================


- (void)deleteSelections:(id)sender {
    NSArray *selection = [self selectedNodes];
    
    // Tell all of the selected nodes to remove themselves from the model.
    [selection makeObjectsPerformSelector: @selector(removeFromParent)];
    [outlineView deselectAll:nil];
    [outlineView reloadData];
}


- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem {
    if ([menuItem action]==@selector(deleteSelections:)) {
	// The delete selection item should be disabled if nothing is selected.
	return ([[self selectedNodes] count]>0);
    }    
    return YES;
}

//
// given an array of movie file paths, create an
// array of TreeNodes
//
-(NSMutableArray *)treeNodesFromMovies:(NSArray *)moviePaths
{
	NSMutableArray	*treeNodes = nil;
	NSEnumerator	*paths = nil;
	id				thisPath = nil;
	
	paths = [moviePaths reverseObjectEnumerator];
	treeNodes = [[NSMutableArray alloc] init];

	while (thisPath = [paths nextObject])
	{
		MyQTMovie *aQTMovie = [[MyQTMovie alloc] initWithFile:thisPath error:nil];
		if (aQTMovie)
		{
			SimpleTreeNode *newItem = 
				[SimpleTreeNode treeNodeWithData:[SimpleNodeData leafDataWithMovie:aQTMovie]];
			[aQTMovie release];
			
			[treeNodes addObject:newItem];			
		}
	}

	[treeNodes autorelease];
	return treeNodes;
}

// takes an array of movie TreeNodes and
// adds them to the NSOutlineView
//
-(void)addMoviesToOutlineView:(NSArray *)treeNodes atIndex:(int)childIndex
{
	[SAFENODE(nil) insertChildren:treeNodes atIndex:childIndex];
}


@end

// ================================================================
//  NSOutlineView data source methods. (The required ones)
// ================================================================

@implementation AppController (DataSourceMethods)

// Required methods.
- (id)outlineView:(NSOutlineView *)olv child:(int)index ofItem:(id)item {
    return [SAFENODE(item) childAtIndex:index];
}
- (BOOL)outlineView:(NSOutlineView *)olv isItemExpandable:(id)item {
    return [NODE_DATA(item) isGroup];
}
- (int)outlineView:(NSOutlineView *)olv numberOfChildrenOfItem:(id)item {
    return [SAFENODE(item) numberOfChildren];
}
- (id)outlineView:(NSOutlineView *)olv objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    id objectValue = nil;
    
    // The return value from this method is used to configure the state of the items cell via setObjectValue:
    if([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) {
	objectValue = [NODE_DATA(item) name];
    } else if([[tableColumn identifier] isEqualToString: COLUMNID_IS_EXPANDABLE] && [NODE_DATA(item) isGroup]) {
	// Here, object value will be used to set the state of a check box.
	objectValue = [NSNumber numberWithBool: [NODE_DATA(item) isExpandable]];
    } else if([[tableColumn identifier] isEqualToString: COLUMNID_NODE_KIND]) {
	objectValue = ([NODE_DATA(item) isLeaf] ? @"Leaf" : @"Group");
    }
    
    return objectValue;
}

// Optional method: needed to allow editing.
- (void)outlineView:(NSOutlineView *)olv setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item  {
    if([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) {
	[NODE_DATA(item) setName: object];
    } else if ([[tableColumn identifier] isEqualToString: COLUMNID_IS_EXPANDABLE]) {
	[NODE_DATA(item) setIsExpandable: [object boolValue]];
	if (![NODE_DATA(item) isExpandable] && [outlineView isItemExpanded: item]) [outlineView collapseItem: item];
    } else if([[tableColumn identifier] isEqualToString: COLUMNID_NODE_KIND]) {
	// We don't allow editing of this column, so we should never actually get here.
    }

}

// ================================================================
//  NSOutlineView delegate methods.
// ================================================================

- (BOOL)outlineView:(NSOutlineView *)olv shouldExpandItem:(id)item {
    return [NODE_DATA(item) isExpandable];
}

- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item {    
    if ([[tableColumn identifier] isEqualToString: COLUMNID_NAME]) {
	// Make sure the image and text cell has an image.  If not, give it something random.
	if (item && ![NODE_DATA(item) iconRep]) [NODE_DATA(item) setIconRep: [self _setIconImage]];
	// Set the image here since the value returned from outlineView:objectValueForTableColumn:... didn't specify the image part...
	[(ImageAndTextCell*)cell setImage: [NODE_DATA(item) iconRep]];
	// For fun, lets display in upper case!
	[(ImageAndTextCell*)cell setStringValue: [[cell stringValue] uppercaseString]];
    } else if ([[tableColumn identifier] isEqualToString: COLUMNID_IS_EXPANDABLE]) {
	[cell setEnabled: [NODE_DATA(item) isGroup]];
    } else if ([[tableColumn identifier] isEqualToString: COLUMNID_NODE_KIND]) {
	// Don't do anything unusual for the kind column.
    }
}

// ================================================================
//  NSOutlineView data source methods. (dragging related)
// ================================================================

- (BOOL)outlineView:(NSOutlineView *)olv writeItems:(NSArray*)items toPasteboard:(NSPasteboard*)pboard {
    draggedNodes = items; // Don't retain since this is just holding temporaral drag information, and it is only used during a drag!  We could put this in the pboard actually.
    
    // Provide data for our custom type, and simple NSStrings.
    [pboard declareTypes:[NSArray arrayWithObjects: NSTIFFPboardType, DragDropMoviePboardType, nil] owner:self];

    // the actual data doesn't matter since DragDropSimplePboardType drags aren't recognized by anyone but us!.
    [pboard setData:[NSData data] forType:DragDropMoviePboardType]; 
    
    return YES;
}

- (unsigned int)outlineView:(NSOutlineView*)olv validateDrop:(id <NSDraggingInfo>)info proposedItem:(id)item proposedChildIndex:(int)childIndex {
    // This method validates whether or not the proposal is a valid one. Returns NO if the drop should not be allowed.
    SimpleTreeNode *targetNode = item;
    BOOL targetNodeIsValid = YES;

    if ([self onlyAcceptDropOnRoot]) {
	targetNode = nil;
	childIndex = NSOutlineViewDropOnItemIndex;
    } else {
	BOOL isOnDropTypeProposal = childIndex==NSOutlineViewDropOnItemIndex;
	
	// Refuse if: dropping "on" the view itself unless we have no data in the view.
	if (targetNode==nil && childIndex==NSOutlineViewDropOnItemIndex && [treeData numberOfChildren]!=0) 
	    targetNodeIsValid = NO;
    
	if (targetNode==nil && childIndex==NSOutlineViewDropOnItemIndex && [self allowOnDropOnLeaf]==NO)
	    targetNodeIsValid = NO;
	
	// Refuse if: we are trying to do something which is not allowed as specified by the UI check boxes.
	if ((targetNodeIsValid && isOnDropTypeProposal==NO && [self allowBetweenDrop]==NO) ||
	    ([NODE_DATA(targetNode) isGroup] && isOnDropTypeProposal==YES && [self allowOnDropOnGroup]==NO) ||
	    ([NODE_DATA(targetNode) isLeaf ] && isOnDropTypeProposal==YES && [self allowOnDropOnLeaf]==NO))
	    targetNodeIsValid = NO;
	    
	// Check to make sure we don't allow a node to be inserted into one of its descendants!
	if (targetNodeIsValid && ([info draggingSource]==outlineView) && [[info draggingPasteboard] availableTypeFromArray:[NSArray arrayWithObject: DragDropMoviePboardType]] != nil) {
	    NSArray *_draggedNodes = [[[info draggingSource] dataSource] draggedNodes];
	    targetNodeIsValid = ![targetNode isDescendantOfNodeInArray: _draggedNodes];
	}
    }
    
    // Set the item and child index in case we computed a retargeted one.
    [outlineView setDropItem:targetNode dropChildIndex:childIndex];

    return targetNodeIsValid ? NSDragOperationGeneric : NSDragOperationNone;
}


- (void)_performDropOperation:(id <NSDraggingInfo>)info onNode:(TreeNode*)parentNode atIndex:(int)childIndex {
    // Helper method to insert dropped data into the model. 
    NSPasteboard * pboard = [info draggingPasteboard];
    NSMutableArray * itemsToSelect = nil;
	BOOL extendSelection = NO;

    // Do the appropriate thing depending on whether the data is DragDropMoviePboardType or NSFilenamesPboardType.
    if ([pboard availableTypeFromArray:[NSArray arrayWithObjects:DragDropMoviePboardType, nil]] != nil) 
	{
        AppController *dragDataSource = [[info draggingSource] dataSource];
        NSArray *_draggedNodes = [TreeNode minimumNodeCoverFromNodesInArray: [dragDataSource draggedNodes]];
        NSEnumerator *draggedNodesEnum = [_draggedNodes objectEnumerator];
        SimpleTreeNode *_draggedNode = nil, *_draggedNodeParent = nil;

		itemsToSelect = (NSMutableArray *)_draggedNodes;

        while ((_draggedNode = [draggedNodesEnum nextObject])) {
            _draggedNodeParent = (SimpleTreeNode*)[_draggedNode nodeParent];
            if (parentNode==_draggedNodeParent && [parentNode indexOfChild: _draggedNode]<childIndex) childIndex--;
            [_draggedNodeParent removeChild: _draggedNode];
        }
        [parentNode insertChildren: _draggedNodes atIndex: childIndex];
    } 
	else if ([pboard availableTypeFromArray:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]] != nil)
	{
		NSArray *moviePaths = [[info draggingPasteboard] propertyListForType:NSFilenamesPboardType];
		itemsToSelect = [self treeNodesFromMovies:moviePaths];

		[self addMoviesToOutlineView:itemsToSelect atIndex:childIndex];
		extendSelection = NO;

	}

    [outlineView reloadData];
    [outlineView selectItems: itemsToSelect byExtendingSelection: extendSelection];

}


- (BOOL)outlineView:(NSOutlineView*)olv acceptDrop:(id <NSDraggingInfo>)info item:(id)targetItem childIndex:(int)childIndex {
    TreeNode *parentNode = nil;

    // Determine the parent to insert into and the child index to insert at.
    if ([NODE_DATA(targetItem) isLeaf]) {
        parentNode = (SimpleTreeNode*)(childIndex==NSOutlineViewDropOnItemIndex ? [targetItem nodeParent] : targetItem);
        childIndex = (childIndex==NSOutlineViewDropOnItemIndex ? [[targetItem nodeParent] indexOfChild: targetItem]+1 : 0);
        if ([NODE_DATA(parentNode) isLeaf]) [NODE_DATA(parentNode) setIsLeaf:NO];
    } else {            
        parentNode = SAFENODE(targetItem);
	childIndex = (childIndex==NSOutlineViewDropOnItemIndex?0:childIndex);
    }

    [self _performDropOperation:info onNode:parentNode atIndex:childIndex];
        
    return YES;
}

//
// received when selection did change
//
- (void)selectionDidChange:(NSNotification *)notification
{
	[self newMovieSelection];
}

@end

@implementation AppController (PlayButtonCode)

-(void)setPlayButtonToUnpressedState
{
	[playButton setState:NSOffState];
}

-(void)setPlayButtonToPressedState
{
	[playButton setState:NSOnState];
}

-(BOOL)isPlayButtonUnpressed
{
	return([playButton state] == NSOffState);
}

@end

@implementation AppController (SliderCode)


	// initialize the movie timeline slider min/max to appropriate 
	// values for the current movie
-(void)initTimeLineSliderForMovie
{
	MyQTMovie *curMovie = [self activeRowMovie];
	QTTime duration = QTZeroTime;
	duration = [curMovie duration];
	[movieTimeLineSlider setMaxValue: duration.timeValue];
	[movieTimeLineSlider setIntValue: 0];
}

	// set the time line slider to reflect the current movie time
-(void)adjustTimeLineSliderForCurrentMovieTime
{
	MyQTMovie *curMovie = [self activeRowMovie];

	NSNumber *num = [NSNumber numberWithLongLong:[curMovie currentTimeValue]];
	[movieTimeLineSlider setDoubleValue:[num doubleValue]];
}

	// adjust the time line display (text) to show the current movie
	// time
-(void)adjustTimeDisplayForCurrentMovieTime
{
	MyQTMovie *curMovie = [self activeRowMovie];
	[movieTimeTextBox setStringValue:QTStringFromTime([curMovie currentPlayTime])];
}

	// set movie volume to correspond to current slider setting
- (void)adjustMovieVolumeToCurrentSliderSetting
{
    /* set the movie volume to correspond to the
        current value of the slider */
	[[theMovieView movie] setVolume:[movieVolumeSlider floatValue]];
}

	// slider action proc. -- set movie volume to correspond to 
	// current slider setting	
- (IBAction)adjustMovieVolume:sender
{
    /* set the movie volume to correspond to the
        current value of the slider */
	[self adjustMovieVolumeToCurrentSliderSetting];
}

@end

@implementation AppController (MoviePlayback)

	// button press to begin playing the currently selected movie
- (IBAction)playMovie:sender
{
	switch ([sender state])
	{
		case NSOnState:

			if ([self lastMovieInList] &&
					[self currentMovieTimeIsEndOfMovie])
			{
				[[self activeRowMovie] gotoBeginning];
				[self initTimeLineSliderForMovie];
				
				// play the next available (selected or non-selected) movie
				// in the list, and wraparound to the beginning of the list
				// if necessary
				[self playNextAvailableMovie:YES];
			}
			else
			{
				[self playMovieViewMovie];
			}

		break;

		case NSOffState:
			[[theMovieView movie] setRate:0];
		break;
	}
}

	// play the movie currently set for the movie view
- (void) playMovieViewMovie
{
	// if no movie is currently set to the view, then
	// set it to the first available movie
	if (![theMovieView movie])
	{
		[self setActiveRowToFirstAvailableRow];
	}

	// reset GUI only if a new movie has been chosen
	if ([self currentPlayingMovie] != [self currentMovieViewMovie])
	{
		[self setupMovieViewForNewMovie];
	}

	[self setupControlsForNewMovie];
		
	[theMovieView play:self];
}


	// play the next available (either selected or non-selected) movie
-(void) playNextAvailableMovie:(BOOL)wrapFlag
{
	if([self setNextAvailableMovie:wrapFlag])
	{
		[self playMovieViewMovie];
	}
}

-(void)setupMovieViewForNewMovie
{
	[theMovieView setMovie:[self activeRowMovie]];
	[theMovieView setControllerVisible:NO];
	[theMovieView setEditable:NO];
	[theMovieView setNeedsDisplay:YES];
}

-(void)setupControlsForNewMovie
{
	[self initTimeLineSliderForMovie];
	[self setPlayButtonToUnpressedState];
	[self enableMovieCallbacksForCurrentMovie];
	[self adjustMovieVolumeToCurrentSliderSetting];
}

	// button press to set movie time to beginning of the movie
- (IBAction)goToBeginningOfMovie:sender
{
	[theMovieView gotoBeginning:sender];
	[self adjustTimeLineSliderForCurrentMovieTime];
	[self adjustTimeDisplayForCurrentMovieTime];
}

	// button press to set movie time to end of the movie
- (IBAction)goToEndOfMovie:sender
{
	[theMovieView gotoEnd:sender];
	[self adjustTimeLineSliderForCurrentMovieTime];
	[self adjustTimeDisplayForCurrentMovieTime];
}


	// returns the movie currently set to the movie view
-(MyQTMovie *)currentMovieViewMovie
{
	return ((MyQTMovie *)[theMovieView movie]);
}

	// returns the movie from the current active row
-(MyQTMovie *)currentPlayingMovie
{
	return ([self activeRowMovie]);
}

	// assigns a new movie to the movie view
-(void)newMovieSelection
{
		// first stop any currently playing movie
	if ([(MyQTMovie *)[theMovieView movie] isPlaying] == YES)
	{
		[[theMovieView movie] setRate:0];
		[theMovieView gotoBeginning:self];
	}

	[self clearAllMovieCallbacks];

	// if no items are selected, set active
	// row item to first available row item
	if ([outlineView numberOfSelectedRows] == 0)
	{
		[self setActiveRowToFirstAvailableRow];
	}
	else
	{
			// get the next available movie
		if (![self setActiveRowToFirstSelectedRow])
			[self setActiveRowToFirstAvailableRow];
	}

	[[theMovieView movie] gotoBeginning];
	[theMovieView gotoBeginning:self];

	[self setupMovieViewForNewMovie];
	[self setupControlsForNewMovie];
}


	// returns YES if the current movie is the last available
	// movie in the list, NO if not
-(BOOL) lastMovieInList
{
	if ([self isActiveRowSelected])
	{
		return( [self isActiveRowLastSelectedRow]);
	}
	else
	{
		return ([self isActiveRowLastRow]);
	}
}

	// returns YES if the current movie time is the end of the movie,
	// NO if not
- (BOOL) currentMovieTimeIsEndOfMovie
{	
	return[[self activeRowMovie] currentTimeEqualsDuration];
}

	// set current movie playing cell to next available
	// (selected or non-selected) cell
-(BOOL) setNextAvailableMovie:(BOOL)wrapFlag
{
	if ([self isActiveRowSelected])
	{
		return([self setActiveRowToNextSelectedRow:wrapFlag]);
	}
	else
	{
		return ([self setActiveRowToNextRow:wrapFlag]);
	}
}

	// returns the number of movies in the list
-(int)numberOfMoviesInList
{
	return( [SAFENODE(nil) numberOfChildren]);
}

-(MyQTMovie *) movieAtRow:(int)index
{
	int numChildren = [self numberOfMoviesInList];
	if (index < numChildren)
	{
		SimpleTreeNode *root = SAFENODE(nil);
		TreeNode *tNode = [root childAtIndex:index];
		SimpleNodeData *nodeData = (SimpleNodeData *)[tNode nodeData];
		return([nodeData movie]);
	}
	
	return nil;
}

	// create a single flattened, self-contained movie for 
	// all the available movie clips
- (IBAction)createMovieFileWithMovieClips:(id)sender 
{
	QTMovie *flattenedMovie = nil;
	int i, numClips = 0;
	
	// first allocate an array to store all our movie clips
	NSMutableArray *movieClips = [[NSMutableArray alloc] init];
	if (!movieClips) goto bail;

	numClips = [self numberOfMoviesInList];
	for (i=0 ; i<numClips ; ++i)
	{
		[movieClips addObject:[self movieAtRow:i]];
	}

	// now create a single movie file which contains all the
	// movie clips
	NSString *destFilePath = [self putFileString];
	if (nil != destFilePath)
	{
		flattenedMovie = [QTUtils appendMovies:movieClips destFilePath:destFilePath];
	}

	// clean up - release our array
	[movieClips release];

bail:
	return;
}


@end

@implementation AppController (MovieCallbacks)

	// remove callbacks (movie playback, movie end, movie stopped)
	// for all movies
-(void)clearAllMovieCallbacks
{
	int i, numClips = 0;

	numClips = [self numberOfMoviesInList];
	for (i=0 ; i<numClips ; ++i)
	{
		[[self movieAtRow:i] removeAllMovieCallbacks];
	}

}

	// enable callbacks (movie playback, movie end, movie stopped)
	// for current movie
-(void)enableMovieCallbacksForCurrentMovie
{
	MyQTMovie *aMovie = [self activeRowMovie];

	[aMovie setMovieDidEndNotificationCallback:@selector(movieDidEndCallBack:) withObject:self];
	[aMovie setMovieRateChangeNotificationCallback:@selector(movieStoppedCallBack:) withObject:self];
	[aMovie setMoviePlaybackNotificationCallback:@selector(moviePlayingCallBack:) withObject:self];
}

	// routine to be called during movie playback
-(void)moviePlayingCallBack:(id)sender
{
	if (![movieTimeLineSlider isUserDraggingSlider])
	{
		[self adjustTimeLineSliderForCurrentMovieTime];
		[self adjustTimeDisplayForCurrentMovieTime];
	}

	if ([sender isPlaying])
	{
		if ([self isPlayButtonUnpressed])
			[self setPlayButtonToPressedState];
	}
}

	// routine to be called when movie playback stops
-(void)movieStoppedCallBack:(id)sender
{
	[self adjustTimeLineSliderForCurrentMovieTime];
	[self adjustTimeDisplayForCurrentMovieTime];

	if (![movieTimeLineSlider isUserDraggingSlider])
	{
		[self setPlayButtonToUnpressedState];
	}
}

	// routine to be called when movie playback ends (end of movie)
-(void)movieDidEndCallBack:(id)sender
{
	if (![movieTimeLineSlider isUserDraggingSlider])
		[(MyQTMovie *)sender removeAllMovieCallbacks];

	// play the next available (selected or non-selected) movie
	// in the list, but don't wraparound to the beginning of 
	// the list

	if (![movieTimeLineSlider isUserDraggingSlider])
	{
		[self playNextAvailableMovie:NO];
	}
}

@end

@implementation AppController (ActiveRow)

- (MyQTMovie *) activeRowMovie
{
	return([self movieAtRow:[self activeRow]]);
}

-(void) setActiveRow:(int)aRowItem
{
	activeRow = aRowItem;
}

- (int) activeRow
{
	return activeRow;
}

-(BOOL) isActiveRowSelected
{
	return ([outlineView isRowSelected:activeRow]);
}

-(BOOL) isActiveRowLastRow
{
	return (activeRow == ([outlineView numberOfRows] - 1));
}

-(BOOL) isActiveRowLastSelectedRow
{
	return(activeRow == [outlineView lastSelectedRow]);
}

-(BOOL) setActiveRowToNextSelectedRow:(BOOL)wrapFlag
{
	int newRow = [outlineView nextSelectedRow:activeRow wrapOK:wrapFlag];
	if (NSNotFound != newRow)
	{
		[self setActiveRow:newRow];
		return YES;
	}
	
	return NO;
}

-(BOOL) setActiveRowToNextRow:(BOOL)wrapFlag
{
	int newRow = [outlineView nextRow:activeRow wrapOK:wrapFlag];
	if (NSNotFound != newRow)
	{
		[self setActiveRow:newRow];
		return YES;
	}
	
	return NO;
}

-(BOOL) setActiveRowToFirstSelectedRow
{
	int newRow = [outlineView nextSelectedRow:-1 wrapOK:NO];
	if (NSNotFound != newRow)
	{
		[self setActiveRow:newRow];
		return YES;
	}
	
	return NO;
}

-(BOOL) setActiveRowToFirstAvailableRow
{
	int newRow = [outlineView nextRow:-1 wrapOK:YES];
	if (NSNotFound != newRow)
	{
		[self setActiveRow:newRow];
		return YES;
	}
	
	return NO;
}

@end

@implementation AppController (FileUtils)


// Open panel delegate methods
- (IBAction) openFile: sender
  {
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	[panel setCanChooseDirectories:YES];
	[panel setAllowsMultipleSelection:YES];
	[panel  // Get the shared open panel
		beginSheetForDirectory:NSHomeDirectory()	// Point it at the user's home
		file:nil
			types:[QTMovie movieUnfilteredFileTypes]
		modalForWindow:[outlineView window]  // This makes it show up as a sheet, attached to window
		modalDelegate:self		
		didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)  // Call this method when you're done..
		contextInfo:NULL];  
  }


- (void) openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
  {
	if (returnCode == 1)
	{
		NSArray *treeNodes = [self treeNodesFromMovies:[sheet filenames]];
		[self addMoviesToOutlineView:treeNodes atIndex:[self activeRow]];

		[outlineView reloadData];
		[outlineView selectItems: treeNodes byExtendingSelection: NO];
	}
  }

	// put up the save panel dialog to allow the user to specify
	// a save file, then return this filename
-(NSString *)putFileString
{
    NSSavePanel *savePanel = nil;

    savePanel = [NSSavePanel savePanel];
    if (!savePanel) goto bail;
    
    if ([savePanel runModalForDirectory:nil file:@"newMovie.mov"] == NSOKButton)
    {
        return [savePanel filename];            
    }

bail:
	return nil;
}

@end

@implementation AppController (Private)


- (NSImage *)_setIconImage {
    NSImage *theImage = nil;

	NSString *imageName = [NSString stringWithFormat:@"moov icon.tiff"];
	theImage = [NSImage imageNamed:imageName];
    
    return theImage;
}

@end

