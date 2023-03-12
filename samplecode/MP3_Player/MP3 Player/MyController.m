/*
 MyController.m
 MP3 Player

 Author: MCF
 
 Description: This file contains the implementation for MyController, the object that is the data source for the table, managing the array of MP3 paths and handling drag-and-drop of MP3s into MP3 Player.

 Copyright (c) 2001-2002, Apple Computer, Inc., all rights reserved.
 */
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import "MyController.h"

@implementation MyController

// Runs when we unpack from the nib
-(void)awakeFromNib
{
    // The "songs" array holds paths to MP3s in the table
    songs=[[NSMutableArray alloc] init];
    
    // When we double-click a row in the table, we want it to call our own routine
    // (implemented in our custom table class) to play the song.
    // Incidentally, we also have a "Play" button in the nib wired up to -playSong:
    [songTable setDoubleAction:@selector(playSong:)];
    
    // We tell the table that we'll be dragging in filenames
    [songTable registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,nil]];
    
    // When we quit the app, we want the song to stop playing, so we register to have stopPlaying:
    // be called when the proper notification comes in
    [[NSNotificationCenter defaultCenter] addObserver: songTable selector: @selector(stopPlaying:) name: NSApplicationWillTerminateNotification object:NSApp];
}

// Mandatory tableview data source method
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [songs count];
}

// Mandatory tableview data source method
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    // We only have one column, so the row is all we care about
    return [songs objectAtIndex:row];
}

// Since the songs array is stored in this object, we need an accessor method to give us the particular
// song path for a given selected row in the table.  The tableview will call this method when trying
// to play a song.
- (NSString *)songSelected
{    return [songs objectAtIndex:[songTable selectedRow]]; }

// Stop the table's rows from being editable when we double-click on them
- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(int)row
{    return NO; }

// when a drag-and-drop operation comes through, and a filename is being dropped on the table,
// we need to tell the table where to put the new filename (right at the end of the table).
// This controls the visual feedback to the user on where their drop will go.
- (NSDragOperation)tableView:(NSTableView*)tv validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)op
{
    [tv setDropRow:[tv numberOfRows] dropOperation:NSTableViewDropAbove];
    return NSTableViewDropAbove;
}

// This routine does the actual processing for a drag-and-drop operation on a tableview.
// As the tableview's data source, we get this call when it's time to update our backend data.
- (BOOL)tableView:(NSTableView*)tv acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)op
{
    // Get the drag-n-drop pasteboard
    NSPasteboard *myPasteboard=[info draggingPasteboard];
    // What type of data are we going to allow to be dragged?  The pasteboard
    // might contain different formats
    NSArray *typeArray=[NSArray arrayWithObjects:NSFilenamesPboardType,nil];
    NSString *filePath,*availableType;
    NSArray *filesList;
    int i;

    // find the best match of the types we'll accept and what's actually on the pasteboard
    availableType=[myPasteboard availableTypeFromArray:typeArray];
    // In the file format type that we're working with, get all data on the pasteboard
    filesList=[myPasteboard propertyListForType:availableType];
    // Insert the MP3 filenames into our songs array
    for (i=0;i<[filesList count];i++)
    {
        filePath=[filesList objectAtIndex:i];
        [songs insertObject:filePath atIndex:row+i];
    }
    // We've updated our array.  Now reload the table.
    [songTable reloadData];
    // Select the last song.
    [songTable selectRow:row+i-1 byExtendingSelection:NO];
    return YES;
}

@end

