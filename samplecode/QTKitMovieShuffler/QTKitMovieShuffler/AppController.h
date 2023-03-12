/*

File: AppController.h

Abstract: Interface file for AppController.m

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

Copyright � 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

#import "MySlider.h"
#import "MyQTMovie.h"
#import "QTUtils.h"

@class MySlider;
@class SimpleTreeNode;
@class TreeNode;

@interface AppController : NSObject {
@private    
    SimpleTreeNode			*treeData;
    NSArray					*draggedNodes;
    NSArray					*iconImages;

	int	activeRow;		// the "active" selected row item -- the 
						// row in the OutlineView whose associated
						// movie is currently set in the movie
						// view

    IBOutlet NSOutlineView 	*outlineView;
    IBOutlet NSButton 		*autoSortCheck;

	IBOutlet id				movieTimeTextBox;
	IBOutlet id				playButton;
	IBOutlet MySlider		*movieTimeLineSlider;
	IBOutlet NSSlider		*movieVolumeSlider;
	IBOutlet QTMovieView	*theMovieView;

}

- (id)init ;
- (void)dealloc ;
- (void)awakeFromNib;

- (NSArray*)draggedNodes;
- (NSArray *)selectedNodes;
- (BOOL)allowOnDropOnGroup;
- (BOOL)allowOnDropOnLeaf;
- (BOOL)allowBetweenDrop;
- (BOOL)onlyAcceptDropOnRoot;
- (void)deleteSelections:(id)sender ;
- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem ;

- (NSMutableArray *)treeNodesFromMovies:(NSArray *)moviePaths;
- (void)addMoviesToOutlineView:(NSArray *)treeNodes atIndex:(int)childIndex;

@end


@interface AppController (DataSourceMethods) 

- (id)outlineView:(NSOutlineView *)olv child:(int)index ofItem:(id)item ;
- (BOOL)outlineView:(NSOutlineView *)olv isItemExpandable:(id)item ;
- (int)outlineView:(NSOutlineView *)olv numberOfChildrenOfItem:(id)item ;
- (id)outlineView:(NSOutlineView *)olv objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item ;
- (void)outlineView:(NSOutlineView *)olv setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item;
- (BOOL)outlineView:(NSOutlineView *)olv shouldExpandItem:(id)item;
- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item;
- (BOOL)outlineView:(NSOutlineView *)olv writeItems:(NSArray*)items toPasteboard:(NSPasteboard*)pboard;
- (unsigned int)outlineView:(NSOutlineView*)olv validateDrop:(id <NSDraggingInfo>)info proposedItem:(id)item proposedChildIndex:(int)childIndex ;
- (void)_performDropOperation:(id <NSDraggingInfo>)info onNode:(TreeNode*)parentNode atIndex:(int)childIndex ;
- (BOOL)outlineView:(NSOutlineView*)olv acceptDrop:(id <NSDraggingInfo>)info item:(id)targetItem childIndex:(int)childIndex ;
- (void)selectionDidChange:(NSNotification *)notification;

@end

@interface AppController (PlayButtonCode)

-(void)setPlayButtonToUnpressedState;
-(void)setPlayButtonToPressedState;
-(BOOL)isPlayButtonUnpressed;

@end

@interface AppController (SliderCode)

-(void)initTimeLineSliderForMovie;
-(void)adjustTimeLineSliderForCurrentMovieTime;
-(void)adjustTimeDisplayForCurrentMovieTime;
-(void)adjustMovieVolumeToCurrentSliderSetting;
-(IBAction)adjustMovieVolume:sender;

@end


@interface AppController (MoviePlayback)

-(IBAction)playMovie:sender;
-(void) playMovieViewMovie;
-(void) playNextAvailableMovie:(BOOL)wrapFlag;
-(void)setupMovieViewForNewMovie;
-(void)setupControlsForNewMovie;
-(IBAction)goToBeginningOfMovie:sender;
-(IBAction)goToEndOfMovie:sender;
-(MyQTMovie *)currentMovieViewMovie;
-(MyQTMovie *)currentPlayingMovie;
-(void)newMovieSelection;
-(BOOL) lastMovieInList;
-(BOOL) currentMovieTimeIsEndOfMovie;
-(IBAction)createMovieFileWithMovieClips:(id)sender ;
-(BOOL) setNextAvailableMovie:(BOOL)wrapFlag;
-(int)numberOfMoviesInList;
-(MyQTMovie *) movieAtRow:(int)index;

@end

@interface AppController (MovieCallbacks)

-(void)clearAllMovieCallbacks;
-(void)enableMovieCallbacksForCurrentMovie;
-(void)moviePlayingCallBack:(id)sender;
-(void)movieStoppedCallBack:(id)sender;
-(void)movieDidEndCallBack:(id)sender;

@end

@interface AppController (ActiveRow)

-(MyQTMovie *) activeRowMovie;
-(void) setActiveRow:(int)aRowItem;
-(int) activeRow;
-(BOOL) isActiveRowSelected;
-(BOOL) isActiveRowLastRow;
-(BOOL) isActiveRowLastSelectedRow;
-(BOOL) setActiveRowToNextSelectedRow:(BOOL)wrapFlag;
-(BOOL) setActiveRowToNextRow:(BOOL)wrapFlag;
-(BOOL) setActiveRowToFirstSelectedRow;
-(BOOL) setActiveRowToFirstAvailableRow;

@end

@interface AppController (FileUtils)

-(IBAction) openFile: sender;
-(void) openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo;
-(NSString *)putFileString;

@end

@interface AppController (Private)

- (NSImage *)_setIconImage;

@end

