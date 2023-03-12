//
//  Controller.m
//  TintedImage
//
//  Created by jcr on Thu Feb 21 2002.
//  Copyright (c) 2002  Apple Computer, Inc. All rights reserved.

#import "Controller.h"
#import "TintedImage.h"

@implementation Controller

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
  {
  [NSColor setIgnoresAlpha:NO];  // This turns on the alpha slider in the color panel.
  }
  
- (void) awakeFromNib  // Make sure the UI is coherent after the nib is loaded.
  {
  [self takeColorFrom: colorWell];
  [self takeOperationFrom: opsMenu];
  }
  
- (IBAction)takeNewImageFrom:(id)sender  // This message will get sent when an image is dropped on either NSImageView
  {
  // A bit of explanation here:  when an image is dropped on either of the image wells, I want the original to appear on the left, and the tinted version to appear on the right.  I'm only testing for the case where the sender is the right-side image view.  If the sender is the left-side image view, then it will already have the correct image showing.
  if (sender == tintedImageView)  
    [inputImageView setImage:[sender image]];
  [self updateViews];
  }

- (IBAction)takeOperationFrom:(id)sender  //Action for the pop-up list that selects the compositing mode.
  {
  // in the Interface Builder, you can set the tag values of cells, such as those in the pop up list.  I've set them up to correspond to the values defined  in the NSCompositingOperation enumerated type.
  op = [[sender selectedItem] tag];
  [self updateViews];
  }  
   
- (IBAction)takeColorFrom:(id)sender
  {
  [self setColor:[sender color]];
  [self updateViews];
  }

- (void) setColor:(NSColor *) aColor  // This is your basic paranoid accessor method...
  {
  if (aColor)
    {
    [aColor retain];
    if (color) 
      [color release];
    color = aColor;
    }
  }

- (void) updateViews // Method to keep the UI in sync with any user actions like changing the color, dropping an image, etc.
  {
  [tintedImageView setImage:[[inputImageView image] tintedImageWithColor:color operation:op]];
  [[tintedImageView cell] setHighlighted:NO];  // Whithout this call, the image wells will have a light grey background
  [[inputImageView cell] setHighlighted:NO];
  }

- (IBAction)saveTintedImage:(id)sender
  {
  [[NSSavePanel savePanel]
    beginSheetForDirectory:NSHomeDirectory()  // seems like a reasonable place to start..
    file:@"Tinted Image" 	// default filename
    modalForWindow:window   //Run as a sheet, attached to "window"
    modalDelegate:self 	// The save panel should call me when it's done..
    didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) // ... using this selector
    contextInfo:NULL];
  }

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void  *)contextInfo
  {
  if (returnCode == NSOKButton)  // The user hit "save", so...
    [[[tintedImageView image] TIFFRepresentation]  // Create the contents of a .tiff file
      writeToFile:[NSString stringWithFormat:@"%@.%@", [sheet filename], @"tiff"] // save it at the path we get from the panel.
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