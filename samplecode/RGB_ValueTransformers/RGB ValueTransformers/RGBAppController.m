
//
//  RGBAppController.m
//  RGB Image
//
//  Created by jcr on Thu Feb 21 2002.
//  Copyright (c) 2002  Apple Computer, Inc. All rights reserved.

static NSString *imageKey = @"image";
static NSString *combinedImageKey = @"combinedImage";
static NSString *showsRedKey = @"showsRed";
static NSString *showsGreenKey = @"showsGreen";
static NSString *showsBlueKey = @"showsBlue";
static NSString *showsAlphaKey = @"showsAlpha";

@implementation RGBAppController

+ (void) initialize
	{
	// ValueTransformers need to be created before the nib loads.
	[NSValueTransformer setValueTransformer:[RGBTransformer transformerWithColor:[NSColor redColor]] forName:@"redTransformer"];
	[NSValueTransformer setValueTransformer:[RGBTransformer transformerWithColor:[NSColor greenColor]] forName:@"greenTransformer"];
	[NSValueTransformer setValueTransformer:[RGBTransformer transformerWithColor:[NSColor blueColor]] forName:@"blueTransformer"];
	[NSValueTransformer setValueTransformer:[RGBTransformer transformerWithColor:[NSColor blackColor] operation:NSCompositeSourceAtop] forName:@"alphaTransformer"];
	// Make sure that anything bound to "combinedImage" updates when the image or the checkboxes are changed.
	[self setKeys:[NSArray arrayWithObjects: imageKey, showsRedKey, showsGreenKey, showsBlueKey, showsAlphaKey, nil] triggerChangeNotificationsForDependentKey:combinedImageKey];
	}

- (void) awakeFromNib // Set all these values with key-value coding, so that the checkboxes notice the change.
	{
	[self setValue:[NSNumber numberWithBool:YES] forKey:showsRedKey];
	[self setValue:[NSNumber numberWithBool:YES] forKey:showsGreenKey];
	[self setValue:[NSNumber numberWithBool:YES] forKey:showsBlueKey];
	[self setValue:[NSNumber numberWithBool:YES] forKey:showsAlphaKey];
	}
	
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
	{
	[self showTheMandrill:nil];  // Just so that we're not looking at a blank when we start.
	}
	
- (IBAction) showTheMandrill:sender  // Load up a test image.
	{
	[self setValue:[NSImage imageNamed:@"mandrill"] forKey:imageKey];
	}

- (IBAction)imageChanged:(id)sender
	{
	[self setValue:[sender image] forKey:imageKey];  // Again, using KVC ensures that the observers notice.
	[[sender cell] setHighlighted:NO];
	}

- (NSImage *) image { return image; }
- (void) setImage:(NSImage *) anImage  // Standard paranoid accessor method.
	{
	if (anImage != image)
	  {
		[anImage retain]; 
		[image release];
		image = anImage;
		}
	}

- (NSImage *) RGBImage  // image consisting of the selected components, not including alpha.
	{
	NSImage 
		*rgbImage = [[[NSImage alloc] initWithSize:[image size]] autorelease];
		
  [rgbImage lockFocus];
			
		if (showsRed) 	
			[[image transformedValueWithTransformer:[NSValueTransformer valueTransformerForName:@"redTransformer"]] 
			compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];
		
		if (showsGreen) 	
			[[image transformedValueWithTransformer:[NSValueTransformer valueTransformerForName:@"greenTransformer"]] 
				compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];

		if (showsBlue) 	
			[[image transformedValueWithTransformer:[NSValueTransformer valueTransformerForName:@"blueTransformer"]] 
			compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];
			
	[rgbImage unlockFocus];
	
	return rgbImage;
	}
	
- (NSImage *) combinedImage  // Reconstitute the image from the selected components
	{
	if (image) // Must have an image to have a combinedImage.
		{
		NSSize 
			imageSize = [image size];
			
		NSImage 
			*newImage = [[NSImage alloc] initWithSize:imageSize];
		
		NSRect 
			imageRect = NSMakeRect(0, 0, imageSize.width, imageSize.height);
			
		[newImage lockFocus];
			
			if (showsAlpha)  
				{
				[[NSColor clearColor] set];
				NSRectFill(imageRect);
				[[[NSValueTransformer valueTransformerForName:@"alphaTransformer"] transformedValue:image] 
					compositeToPoint:NSZeroPoint operation:NSCompositeCopy];
				}
			else  // Not using alpha, so give something to draw the RGB image over.
				{
				[[NSColor whiteColor] set];
				NSRectFill(imageRect);
				}
	
			[[self RGBImage] compositeToPoint:NSZeroPoint operation:NSCompositeSourceAtop];
		[newImage unlockFocus];
		return [newImage autorelease];
		}
	return nil;
	}

- (void) setCombinedImage:(NSImage *) anImage {}  // This is just here so that key-value coding won't complain.
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
