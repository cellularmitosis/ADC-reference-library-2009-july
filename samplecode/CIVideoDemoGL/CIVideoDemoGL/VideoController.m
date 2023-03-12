/*

File: VideoController.m

Abstract:   Application controller object.

Version: 1.0

© Copyright 2005-2007 Apple Inc., All rights reserved.

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

#import <QuartzCore/QuartzCore.h>
#import "VideoController.h"
#import "VideoView.h"

@interface VideoController (private)

- (void)loadMovie:(NSString *)path;

@end

@implementation VideoController

//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
    // only needed if you rely on Image Units
    [CIPlugIn loadAllPlugIns];
}

//--------------------------------------------------------------------------------------------------

- (void)applicationWillFinishLaunching:(NSNotification *)note
{
    NSString *moviePath;

    /* See if we have a known pathname */
    moviePath = [[NSUserDefaults standardUserDefaults] stringForKey:@"MoviePath"];
    if(moviePath)
    {
        [self loadMovie:moviePath];
    }
    
    [filterDrawer open];
}

//--------------------------------------------------------------------------------------------------

- (void)openMovie:(id)sender
{
    NSOpenPanel *openPanel;
    int rv;
    
    openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setResolvesAliases:YES];
    [openPanel setCanChooseFiles:YES];
    
    rv = [openPanel runModalForTypes:nil];
    if(rv == NSFileHandlingPanelOKButton)
        [self loadMovie:[openPanel filename]];
}

//--------------------------------------------------------------------------------------------------

- (void)loadMovie:(NSString *)path
{
    QTMovie	*qtMovie = nil;
    NSError	*error;
        
    qtMovie = [QTMovie movieWithFile:path error:&error];
    if(qtMovie)
    {
        [[NSUserDefaults standardUserDefaults] setObject:path forKey:@"MoviePath"];
        [[NSUserDefaults standardUserDefaults] synchronize];
        [videoView setQTMovie:qtMovie];
        [[videoView window] setTitleWithRepresentedFilename:path];
        [nextButton setEnabled:YES];
        [prevButton setEnabled:YES];
        [playButton setEnabled:YES];
        [videoScrubber setEnabled:YES];
    }
}

- (IBAction)toggledTrickProfile:(id)sender
{
   [videoView updateColorProfile:[videoView	viewDisplayID]];
}

//--------------------------------------------------------------------------------------------------

-(NSWindow *)progressSheet
{
    return progressSheet;
}

-(VideoView *)videoView
{
    return videoView;
}

-(int)useTrickProfile
{
    return [trickProfile state];
}

- (NSProgressIndicator*)progressIndicator
{
    return progressIndicator;
}

//--------------------------------------------------------------------------------------------------
// DELEGATE methods
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

- (BOOL)windowShouldClose:(id)sender	//close box quits the app
{
    [NSApp terminate:self];
    return YES;
}

//--------------------------------------------------------------------------------------------------

- (void)movieTimeChanged:(VideoView*)sender
{
    QTTime  currentTime = [sender currentTime];
    QTTime  duration	= [sender movieDuration];
    NSString *timeString = QTStringFromTime(currentTime);
    
    if  (timeString) [timecodeField setStringValue:timeString];
    
    [videoScrubber setDoubleValue:(double)currentTime.timeValue / (double)duration.timeValue];
        
}

//--------------------------------------------------------------------------------------------------

@end
