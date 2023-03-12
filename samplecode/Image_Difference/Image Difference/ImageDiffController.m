#import "ImageDiffController.h"
#import "NSImageDifference.h"

@implementation ImageDiffController

	// These methods will get invoked if you drop an image on either NSImageView at the top of the window.
- (IBAction)leftImageChanged:(id)sender { [self updateDiffImage]; }
- (IBAction)rightImageChanged:(id)sender { [self updateDiffImage]; }

- (void) updateDiffImage
	{
	[differenceImageView setImage:[[leftImageView image] differenceFromImage:[rightImageView image]]];
	[[leftImageView cell] setHighlighted:NO];  // For some reason, when you drag an image into an NSImageView, it gets selected.  
	[[rightImageView cell] setHighlighted:NO];
	}

- (IBAction) saveDiffImage:sender
  {
  [[NSSavePanel savePanel]
    beginSheetForDirectory:NSHomeDirectory()  // Seems like a reasonable place to start..
    file:@"DiffImage" 	// default filename
    modalForWindow:window   //Run as a sheet, attached to "window"
    modalDelegate:self 	// The save panel should call me when it's done..
    didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) // ... using this selector
    contextInfo:NULL];
  }

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
  {
  if (returnCode == NSOKButton)  // The user hit "save", so...
    [[[differenceImageView image] TIFFRepresentation]  // Create the contents of a .tiff file
      writeToFile:[[sheet filename] stringByAppendingString: @".tiff"] // save it at the path we get from the panel.
      atomically:YES];  // That's about all there is to it..
  }

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication { return YES; }

@end

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
