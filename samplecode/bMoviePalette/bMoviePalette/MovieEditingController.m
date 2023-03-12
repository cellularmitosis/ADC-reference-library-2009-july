/**
 *	filename: MovieEditingController.m
 *	created : Thu Apr  5 18:09:53 2001
 *	LastEditDate Was "Wed May 30 14:17:25 2001"
 *
 */

/*
 	Copyright (c) 1997-2001 Apple Computer, Inc.
 	All rights reserved.

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

#import "MovieEditingController.h"

#import <QuickTime/QuickTime.h>

#import "MyMovie.h"
#import "TimeFormatter.h"
#import "SoundFileWell.h"

@implementation MovieEditingController

- (void)_resetTimes
{
    [_times release];
    _times = nil;
}

- (void)_rebuildTableColumns
{
    NSArray       *columns;
    NSTableColumn *column;
    unsigned int i,count;
    TimeFormatter *numberFormatter;

    numberFormatter = [[TimeFormatter alloc] init];

        /* IF the tableView has more columns than movies remove until they are
         * equal
         */
    while([movieClips numberOfColumns] > [_movies count]){
        [movieClips removeTableColumn:[[movieClips tableColumns] lastObject]];
    }

    while([_movies count] > [movieClips numberOfColumns]){
        column = [[NSTableColumn alloc] init];
        [column setWidth:80];
        [movieClips addTableColumn:column];
        [column release];
    }

    columns = [movieClips tableColumns];
    count = [columns count];
    for(i=0;i<count;i++){
        NSImageCell *imageCell;
        MyMovie     *movie;

        imageCell = [[NSImageCell alloc] init];
        [imageCell setImageFrameStyle:NSImageFrameGrayBezel];
        movie = [_movies objectAtIndex:i];
        column = [columns objectAtIndex:i];
        [column setWidth:80];
        [column setIdentifier:movie];
        [column setDataCell:imageCell];
        [imageCell release];
        [[column headerCell] setFormatter:numberFormatter];
        [[column headerCell] setObjectValue:[NSNumber numberWithLong:[movie movieDuration]]];
    }

    column = [[NSTableColumn alloc] init];
    [[column headerCell] setStringValue:@""];
    [column sizeToFit];
    [movieClips addTableColumn:column];
    [column release];
    [movieClips sizeLastColumnToFit];
}

- (void)_insertMovie:(MyMovie *)movie atIndex:(int)index
{
    if (_movies == nil){
        _movies = [[NSMutableArray alloc] init];
    }

    if (index < 0){
        [_movies addObject:movie];
    }else{
        [_movies insertObject:movie atIndex:index];
    }

    [self _rebuildTableColumns];
}

- (void)_idleMovie:(id)sender
{
    MCIdle([movieView movieController]);
}

- (void)_commonInit
{
    _selectedIndex = -1;
    _listenToNotification = YES;
    EnterMovies();
}

- init
{
    [super init];
    [self _commonInit];
    return self;
}

- (void)dealloc
{
    [_soundURL release];
    [_movies release];
    [movieClips setDelegate:nil];
    [movieClips setDataSource:nil];
    ExitMovies();
    [super dealloc];
}

- (void)awakeFromNib
{
        /* Fix the data cell of the first column posterImage to be
         * and NSImageCell
         */
    [movieClips setRowHeight:80];
    [movieClips setDelegate:self];
    [movieClips setDataSource:self];
    [movieClips registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,nil]];
    [[movieClips window] setDelegate:self];
    [self performSelector:@selector(_movieInit:) withObject:nil afterDelay:0];
}

- (void)_movieInit:(id)sender
{
        /* Make an empty movie to put in the movieView */
    MyMovie            *blankMovie;
    NSImage            *blackImage;

    [[movieView window] makeKeyAndOrderFront:nil];

    blankMovie = [MyMovie emptyMovie];
    blackImage = [[NSImage alloc] initWithSize:[movieView frame].size];

    [blackImage lockFocus];
    [[NSColor blackColor] set];
    NSRectFill(NSMakeRect(0,0,[blackImage size].width,[blackImage size].height));

    [blackImage unlockFocus];

    [blankMovie insertImage:blackImage sourceStartTime:0 sourceDurationTime:5.0];
    [movieView setMovie:blankMovie];
    [movieView setNeedsDisplay:YES];
    [movieView start:nil];
    [movieView stop:nil];
    [blackImage release];
}

- (IBAction)playClip:(id)sender
{
    MyMovie *movie;

    _selectedIndex = [movieClips selectedColumn];
    if (_selectedIndex >= 0){
        movie = [_movies objectAtIndex:_selectedIndex];

        if ([movieView movie] != movie){
            int volume;

            [movieView setMovie:movie];
                /* Force the movie to be displaied on screen, starting and stopping the movie does the trick
                 */
            volume = [movieView volume];
            [movieView setVolume:0];
            [movieView start:nil];
            [movieView stop:nil];
            [movieView setVolume:volume];
        }
        [self _resetTimes];
    }
}

- (IBAction)playMovie:(id)sender
{
    BOOL          err;
    unsigned int  count,i;
    MyMovie      *movie;
    MyMovie      *dstMovie;
    int           volume;
    Track         origTrack;
    Track         newTrack;
    MyMovie      *soundMovie;

    _selectedIndex = 0;
    soundMovie = nil;

    if (_soundURL){
        soundMovie = [[MyMovie alloc] initWithURL:_soundURL byReference:YES];
    }

        /* Clear the old times and make new ones */
    [self _resetTimes];
    _times = [[NSMutableArray alloc] init];

    dstMovie = [MyMovie emptyMovie];
    count = [_movies count];

        /* Build up array of interesting times and sort them */
    for(i=0;i<count;i++){
        TimeValue duration;

        movie = [_movies objectAtIndex:i];
        err = [dstMovie appendMovie:movie];
        duration = GetTrackDuration([dstMovie firstVideoTrack]);
        [_times addObject:[NSNumber numberWithLong:duration]];
    }
    [_times sortUsingSelector:@selector(compare:)];

    _listenToNotification = NO;
    [movieClips selectRow:_selectedIndex byExtendingSelection:NO];
    _listenToNotification = YES;

    if (_soundURL){
        origTrack = [dstMovie firstSoundTrack];
        newTrack = [soundMovie firstSoundTrack];

        [dstMovie replaceTrack:origTrack withTrack:newTrack];
    }

    volume = [movieView volume];
    [movieView setMovie:dstMovie];

    [movieView showController:YES adjustingSize:YES];
    [movieView setVolume:volume];
    [movieView start:nil];
    [dstMovie setDelegate:self];
    if (_soundURL){
        [soundMovie release];
    }
}

- (IBAction)saveMovie:(id)sender
{
    NSWindow    *window;
    NSSavePanel *savePanel;

    window    = [movieClips window];
    savePanel = [NSSavePanel savePanel];

    [savePanel setRequiredFileType:@"mov"];
    [self performSelector:@selector(_idleMovie:) withObject:nil afterDelay:0];
    [savePanel beginSheetForDirectory:nil
                                 file:nil
                       modalForWindow:window
                        modalDelegate:self
                       didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:)
                          contextInfo:nil];
}

- (IBAction)setSoundTrack:(SoundFileWell *)sender
{
    [_soundURL release];
    _soundURL = [[sender url] copy];
}

- (IBAction)deleteClip:(id)sender
{
    [_movies removeObjectAtIndex:_selectedIndex];
    [self _rebuildTableColumns];
    [movieClips reloadData];
    [self _resetTimes];
    [self playClip:nil];
}

- (IBAction)splitClip:(id)sender
{
    if (_selectedIndex >= 0){
        long int splitPoint;
        MyMovie *selectedMovie;
        NSArray *movies;

        selectedMovie = [[_movies objectAtIndex:_selectedIndex] retain];
        splitPoint = GetMovieTime([selectedMovie QTMovie],NULL);

        if (splitPoint > 0){
            [_movies removeObjectAtIndex:_selectedIndex];
            movies = [selectedMovie splitMovieAtTime:splitPoint];
            [self _insertMovie:[movies objectAtIndex:0] atIndex:_selectedIndex];
            if (_selectedIndex+1 < [_movies count]){
                [self _insertMovie:[movies objectAtIndex:1] atIndex:_selectedIndex+1];
            }else{
                [self _insertMovie:[movies objectAtIndex:1] atIndex:[_movies count]];
            }

            [selectedMovie release];
            [movieClips reloadData];

            _selectedIndex = [movieClips selectedColumn];
            [movieView setMovie:[_movies objectAtIndex:_selectedIndex]];
        }else{
            NSBeep();
        }
    }
    [self _resetTimes];
}

/* NSTableView datasource methods */
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    return 1;
}

- (id)tableView:(NSTableView *)tv objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    MyMovie *identifier;

    identifier = [tableColumn identifier];

    if (identifier){
        return [identifier valueForKey:@"posterImage"];
    }
    return nil;
}

- (NSDragOperation)tableView:(NSTableView*)tv
                validateDrop:(id <NSDraggingInfo>)info
                 proposedRow:(int)row
       proposedDropOperation:(NSTableViewDropOperation)op
{
    [tv setDropRow: -1 dropOperation:NSTableViewDropOn];
    return NSDragOperationMove;
}


- (BOOL)tableView:(NSTableView*)tv
       acceptDrop:(id <NSDraggingInfo>)info
              row:(int)index
    dropOperation:(NSTableViewDropOperation)op
{
    NSPasteboard *pasteboard;

    pasteboard = [info draggingPasteboard];

    if ([[pasteboard types] containsObject:NSFilenamesPboardType]){
        NSArray  *types;
        NSURL    *url;
        MyMovie  *movie;
        unsigned int i,count;

        types = [pasteboard propertyListForType:NSFilenamesPboardType];
        count = [types count];

        for (i=0;i<count;i++){
            url = [[NSURL alloc] initFileURLWithPath:[types objectAtIndex:i]];
            movie = [[MyMovie alloc] initWithURL:url byReference:YES];
            [self _insertMovie:movie atIndex:-1];
            [movie release];
            [url release];
        }
        [movieClips reloadData];
        return YES;
    }
    return NO;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    if (_listenToNotification){
        _selectedIndex = [movieClips selectedColumn];
        [self playClip:nil];
    }
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectTableColumn:(NSTableColumn *)tableColumn
{
    if ([tableColumn identifier] == nil){
        return NO;
    }
    return YES;
}


- (void)tableView:(NSTableView*)tableView didDragTableColumn:(NSTableColumn *)tableColumn
{
    MyMovie *oldMovie;
    NSArray *columns;
    unsigned int i,count;

    _listenToNotification = NO;
    if ([tableColumn identifier] != nil){
        oldMovie = [tableColumn identifier];
        columns = [tableView tableColumns];
        count = [columns count];
        [_movies release];
        _movies = [[NSMutableArray alloc] init];
        for(i=0;i<count;i++){
            MyMovie *movie;
            movie = [[columns objectAtIndex:i] identifier];
            if (movie){
                [_movies addObject:movie];
            }
        }
        _selectedIndex = [movieClips columnWithIdentifier:oldMovie];
    }else{
        _selectedIndex = 0;
    }
    [self playClip:nil];
    _listenToNotification = YES;
}

/* Save panel delegate method */
- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [sheet orderOut:nil];
    [sheet setDelegate:nil];

    if (returnCode == NSOKButton){
        MyMovie      *movie;

        movie = (MyMovie *)[movieView movie];

        SetMoviePosterTime([movie QTMovie],0);
        [movie writeToFile:[sheet filename] atomically:YES];
    }
}

/* Archiving support not really used here just need a place to call
 * EnterMovies
 */
- (id)initWithCoder:(NSCoder*)coder
{
    [self _commonInit];
    _movies = [[coder decodeObject] retain];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_movies];
}

/* MyMovie Delegate method, called when something happens in the movie */
- (void)movieDrawingComplete:(MyMovie *)movie
{
    Movie        mov;
    long         currentMovieTime;
    int          i;
    long         value;
    BOOL         found;

    found = NO;
    mov = [movie QTMovie];
    currentMovieTime = GetMovieTime(mov,NULL);

    i = [_times count];
    while(i--){
        value = [[_times objectAtIndex:i] longValue];
        if (currentMovieTime >= value){
            found = YES;
            i++;
            break;
        }
    }

    _listenToNotification = NO;

    if (found == YES){
        if (i < [_movies count]){
            _selectedIndex = i;
        }
    }else{
        _selectedIndex = 0;
    }

    [movieClips selectColumn:_selectedIndex byExtendingSelection:NO];
    [movieClips scrollColumnToVisible:_selectedIndex];
    _listenToNotification = YES;
}

/* NSWindow Delegate methods */
- (void)windowDidBecomeMain:(NSNotification *)notification
{
    [self _idleMovie:nil];
}

- (void)windowDidResignMain:(NSNotification *)notification
{
    [self _idleMovie:nil];
}

@end
