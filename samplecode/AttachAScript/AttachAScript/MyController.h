/*

File: MyController.h

Abstract: Main controller's header file for the AttachAScript sample.

Version: 1.0

(c) Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#import <Cocoa/Cocoa.h>
#import "TAttachedScripts.h"

/* the following are indexes into the imagesTable stored in our controller object */
enum {
	kShuffleOffImage=0,
	kShuffleOnImage=1,
	kPlayImage=2,
	kPauseImage=3,
	kRepeatOffImage=4,
	kRepeatAllImage=5,
	kRepeatOneImage=6,
	kButtonImagesCount=7
};

	/* intervals */
enum {
	kMaximumUpdateInterval=21, /* we will wait no longer than this before updating our UI */
	kNotPlayingUpdateInterval=7 /* when no song is playing, we will update our ui this often */
};

@interface MyController : NSObject
{
    IBOutlet id playpausebutton;
    IBOutlet id statusline;
    IBOutlet id playlists;
    IBOutlet id tracklist;
    IBOutlet id volumesetting;
    IBOutlet id gongbutton;
    IBOutlet id repeatbutton;
    IBOutlet id shufflebutton;
	TAttachedScripts* myScript;
	NSArray *playlistItems;
	NSArray *tracklistItems; /* array of track names */
	NSArray *imagesTable; /* a table of images we swap in to our buttons */
	NSSound* gongSound; /* the sound we play when the gong button is clicked */
}
- (IBAction)adjustvolume:(id)sender;
- (IBAction)goback:(id)sender;
- (IBAction)goforward:(id)sender;
- (IBAction)playpause:(id)sender;
- (IBAction)gong:(id)sender;
- (IBAction)repeatsetting:(id)sender;
- (IBAction)shufflesetting:(id)sender;

- (TAttachedScripts *)myScript;
- (void)setMyScript:(TAttachedScripts *)value;

- (NSArray *)playlistItems;
- (void)setPlaylistItems:(NSArray *)newPlaylistItems;

- (NSArray *)tracklistItems;
- (void)setTracklistItems:(NSArray *)newTracklistItems;

- (NSArray *)imagesTable;
- (void)setImagesTable:(NSArray *)newImagesTable;

- (NSSound *)gongSound;
- (void)setGongSound:(NSSound *)newGongSound;

- (void)updatePlayerStatus;

- (int)numberOfRowsInTableView: (NSTableView *) tableView;
- (id)tableView: (NSTableView *) tableView 
		objectValueForTableColumn: (NSTableColumn *)
		tableColumn row: (int) row;

@end


