
//
//  RGBAppController.m
//  RGB Image
//
//  Created by jcr on Thu Feb 21 2002.
//  Copyright (c) 2002  Apple Computer, Inc. All rights reserved.

#import "RGBAppController.h"
#import "NSImageColorSeparation.h"
 
@implementation RGBAppController

- (void) awakeFromNib
	{
	showRed = showGreen = showBlue = showAlpha = YES;
	[self imageChanged:nil];
	}

- (IBAction) showTheMandrill:sender
	{
	[originalImageView setImage:[NSImage imageNamed:@"mandrill"]];
	[self imageChanged:nil];
	}
	
- (IBAction)imageChanged:(id)sender
	{
	NSImage 
		*originalImage = sender ? [sender image] : [originalImageView image];
		
	[[sender cell] setHighlighted:NO];
	if (sender)
		[originalImageView setImage:originalImage];
	[redImageView setImage: [[originalImageView image] redImage]];	
	[greenImageView setImage: [[originalImageView image] greenImage]];	
	[blueImageView setImage: [[originalImageView image] blueImage]];	
	[alphaImageView setImage: [originalImage alphaMaskWithColor:[NSColor blackColor]]];
	[combinedImageView setImage:[self combinedImage]];
	}

- (NSImage *) RGBImage  // image consisting of the selected components, not including alpha.
	{
	NSImage 
		*rgbImage = [[[NSImage alloc] initWithSize:[[originalImageView image] size]] autorelease];
  [rgbImage lockFocus];
	if (showRed) [[redImageView image] compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];
	if (showGreen) [[greenImageView image] compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];
	if (showBlue) [[blueImageView image] compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];
	[rgbImage unlockFocus];
	return rgbImage;
	}

- (NSImage *) combinedImage
	{
	NSImage 
		*original = [originalImageView image];
		
	NSSize 
		imageSize = [original size];
		
	NSImage 
		*newImage = [[[NSImage alloc] initWithSize:imageSize] autorelease];
	
	[newImage lockFocus];
	
	[[NSColor clearColor] set];
	NSRectFill(NSMakeRect(0, 0, imageSize.width, imageSize.height));

	if (showAlpha)  [[original alphaMaskWithColor:[NSColor blackColor]] 
		compositeToPoint:NSZeroPoint operation:NSCompositeCopy];
	[[self RGBImage] compositeToPoint:NSZeroPoint operation:NSCompositeSourceAtop];
	[newImage unlockFocus];
	return newImage;
	}
	
 // some one-liners.
- (IBAction)setUsesAlpha:(id)sender 	{ showAlpha = [sender intValue]; [self imageChanged:nil]; }
- (IBAction)setShowRed:(id)sender { showRed = [sender intValue]; [self imageChanged:nil]; }
- (IBAction)setShowGreen:(id)sender { showGreen = [sender intValue]; [self imageChanged:nil]; }
- (IBAction)setShowBlue:(id)sender { showBlue = [sender intValue]; [self imageChanged:nil]; }
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
