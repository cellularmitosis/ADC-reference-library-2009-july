#import "TransformedImageController.h"
#import "TransformedImageView.h"
#import "NSImageGradients.h"
#import "NSAffineTransform_Shearing.h"

@implementation TransformedImageController

- (void) awakeFromNib
  {
  [self resetTransformations:nil];
  }

- (IBAction) resetTransformations: sender
  {
  [imageView setTransform:[NSAffineTransform transform]];
  scale = 1.0;
  rotation = xShear = yShear = 0;
  }
  
  // User/Filesystem interactions.
- (IBAction) openDocument: sender
  {
  [[NSOpenPanel openPanel]  // Get the shared open panel
    beginSheetForDirectory:NSHomeDirectory()	// Point it at the user's home
    file:nil
    modalForWindow:window  // This makes it show up as a sheet, attached to window
    modalDelegate:self		// Tell me when you're done.
    didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)  // Call this method when you're done..
    contextInfo:NULL];  
  }

- (void) openPanelDidEnd:(NSOpenPanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
  {
  [imageView setImage:[[[NSImage alloc] initWithContentsOfFile:[sheet filename]]autorelease]];
  [self resetTransformations:nil];
  }
  
- (IBAction)saveDocument:(id)sender
  {
  [[NSSavePanel savePanel]
    beginSheetForDirectory:NSHomeDirectory()  // seems like a reasonable place to start..
    file:@"Transformed Image" 	// default filename
    modalForWindow:window   //Run as a sheet, attached to "window"
    modalDelegate:self 	// The save panel should call me when it's done..
    didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) // ... using this selector
    contextInfo:NULL];
  }

- (void) savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
  {
  if (returnCode == NSOKButton)  // The user hit "save", so...
    [[[imageView transformedImage] TIFFRepresentation]  // Create the contents of a .tiff file
      writeToFile:[NSString stringWithFormat:@"%@.%@", [sheet filename], @"tiff"] // save it at the path we get from the panel.
      atomically:YES];  // That's about all there is to it..
  }

- (void) fixMatrix
  {
  NSAffineTransform 
    *transform = [NSAffineTransform transform];  // This comes to us set to the identity transform.
    
  [transform rotateByDegrees:rotation];  // Bend it to our will!
  [transform scaleBy:scale];
  [transform shearXBy:xShear yBy:yShear];
  
  [imageView setTransform: transform];  // And apply it to the image view.
  }

  // The actions..
- (IBAction) takeRotationFrom: sender  { rotation = [sender floatValue]; [self fixMatrix];}
- (IBAction) takeScaleFrom: sender { scale = [sender floatValue]; [self fixMatrix]; }
- (IBAction) takeXShearFrom: sender { xShear = [sender floatValue]; [self fixMatrix]; }
- (IBAction) takeYShearFrom: sender { yShear = [sender floatValue];[self fixMatrix]; }
- (IBAction) resetShear: sender { xShear = yShear = 0.0; [self fixMatrix]; }

- (IBAction) showTheGradient: sender { [imageView setImage:[NSImage prettyGradientImageForWWDCDemo]];}

 - (IBAction) imageViewRecievedImage: sender
  {
  [self resetTransformations:nil];
  [[imageView cell] setHighlighted:NO];
  }
 
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
