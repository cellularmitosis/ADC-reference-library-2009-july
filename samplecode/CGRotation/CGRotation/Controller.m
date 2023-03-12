/*

File: Controller.m

Abstract: Handles initialization of the ImageView as well as communication
	between other controls and the ImageView's Image

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2007 Apple Inc., All Rights Reserved

*/

#import "Controller.h"
#import "ImageView.h"

@implementation Controller

-(id)init
{
	self = [super init];
	if(self != nil)
	{
		rotation = 0.0f;
		scaleX = 1.0f;
		scaleY = 1.0f;
		translateX = 0.0f;
		translateY = 0.0f;
		preserveAspectRatio = NO;
	}
	return self;
}

- (void)awakeFromNib
{
	openImageIOSupportedTypes = NULL;
	// Ask CFBundle for the location of our demo image
	CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("demo"), CFSTR("png"), NULL);
	if(url != NULL)
	{
		// And if available, load it
		[imageView setImage:IICreateImage(url)];
		CFRelease(url);
	}
	[[imageView window] center];
	[self setPreserveAspectRatio:NO];
	[self resetTransformations];
}

-(void)dealloc
{
	if(openImageIOSupportedTypes != NULL)
	{
		CFRelease(openImageIOSupportedTypes);
	}
	[super dealloc];
}

- (IBAction)changeScaleX:(id)sender
{
	[self setScaleX:scaleX + [sender floatValue]];
	[sender setFloatValue:0.0f];
}

- (IBAction)changeScaleY:(id)sender
{
	[self setScaleY:scaleY + [sender floatValue]];
	[sender setFloatValue:0.0f];
}

- (IBAction)changeTranslateX:(id)sender
{
	[self setTranslateX:translateX + [sender floatValue]];
	[sender setFloatValue:0.0f];
}

- (IBAction)changeTranslateY:(id)sender
{
	[self setTranslateY:translateY + [sender floatValue]];
	[sender setFloatValue:0.0f];
}

- (IBAction)reset:(id)sender
{
	[self resetTransformations];
	[imageView setNeedsDisplay:YES];
}

// Returns an array with the extensions that match the given Uniform Type Identifier (UTI).
-(NSArray*)extensionsForUTI:(CFStringRef)uti
{
	// If anything goes wrong, we'll return nil, otherwise this will be the array of extensions for this image type.
	NSArray * extensions = nil;
	// Only get extensions for UTIs that are images (i.e. conforms to public.image aka kUTTypeImage)
	// This excludes PDF support that ImageIO advertises, but won't actually use.
	if(UTTypeConformsTo(uti, kUTTypeImage))
	{
		// Copy the decleration for the UTI (if it exists)
		CFDictionaryRef decleration = UTTypeCopyDeclaration(uti);
		if(decleration != NULL)
		{
			// Grab the tags for this UTI, which includes extensions, OSTypes and MIME types.
			CFDictionaryRef tags = CFDictionaryGetValue(decleration, kUTTypeTagSpecificationKey);
			if(tags != NULL)
			{
				// We are interested specifically in the extensions that this UTI uses
				CFTypeRef filenameExtensions = CFDictionaryGetValue(tags, kUTTagClassFilenameExtension);
				if(filenameExtensions != NULL)
				{
					// It is valid for a UTI to export either an Array (of Strings) representing multiple tags,
					// or a String representing a single tag.
					CFTypeID type = CFGetTypeID(filenameExtensions);
					if(type == CFStringGetTypeID())
					{
						// If a string was exported, then wrap it up in an array.
						extensions = [NSArray arrayWithObject:(NSString*)filenameExtensions];
					}
					else if(type == CFArrayGetTypeID())
					{
						// If an array was exported, then just return that array.
						extensions = [[(NSArray*)filenameExtensions copy] autorelease];
					}
				}
			}
			CFRelease(decleration);
		}
	}
	return extensions;
}

// On Tiger NSOpenPanel only understands extensions, not UTIs, so we have to obtain a list of extentions
// from the UTIs that Image IO tells us it can handle.
-(void)createOpenTypesArray
{
	if(openImageIOSupportedTypes == NULL)
	{
		CFArrayRef imageIOUTIs = CGImageSourceCopyTypeIdentifiers();
		CFIndex i, count = CFArrayGetCount(imageIOUTIs);
		openImageIOSupportedTypes = [[NSMutableArray alloc] initWithCapacity:count];
		for(i = 0; i < count; ++i)
		{
			[openImageIOSupportedTypes addObjectsFromArray:
				[self extensionsForUTI:CFArrayGetValueAtIndex(imageIOUTIs, i)]];
		}
		CFRelease(imageIOUTIs);
	}
}

-(IBAction)openDocument:(id)sender
{
	NSOpenPanel * panel = [NSOpenPanel openPanel];
	[panel setAllowsMultipleSelection:NO];
	[panel setResolvesAliases:YES];
	[panel setTreatsFilePackagesAsDirectories:YES];
	[panel setMessage:@"Please choose an image file."];
	
	[self createOpenTypesArray];
	
	[panel
		beginSheetForDirectory:nil
		file:nil
		types:openImageIOSupportedTypes
		modalForWindow:[imageView window]
		modalDelegate:self
		didEndSelector:@selector(openImageDidEnd:returnCode:contextInfo:)
		contextInfo:nil];
		
}

-(void)openImageDidEnd:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	if(returnCode == NSOKButton)
	{
		if([[panel filenames] count] > 0)
		{
			ImageInfo * image = IICreateImage((CFURLRef)[NSURL fileURLWithPath:[[panel filenames] objectAtIndex:0]]);
			if(image != NULL)
			{
				// Ownership is transferred to the ImageView.
				[imageView setImage:image];
				[self reset:nil];
			}
		}
	}
}

-(IBAction)saveDocumentAs:(id)sender
{
	NSSavePanel * panel = [NSSavePanel savePanel];
	[panel setCanSelectHiddenExtension:YES];
	[panel setRequiredFileType:@"jpeg"];
	[panel setAllowsOtherFileTypes:NO];
	[panel setTreatsFilePackagesAsDirectories:YES];
	
	[panel
		beginSheetForDirectory:nil
		file:@"untitled image"
		modalForWindow:[imageView window]
		modalDelegate:self
		didEndSelector:@selector(saveImageDidEnd:returnCode:contextInfo:)
		contextInfo:nil];
}

-(void)saveImageDidEnd:(NSSavePanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo
{
	if(returnCode == NSOKButton)
	{
		NSRect frame = [imageView frame];
		IISaveImage([imageView image], (CFURLRef)[panel URL], ceilf(frame.size.width), ceilf(frame.size.height));
	}
}

-(void)setRotation:(float)r
{
	if(r >= 360.0f)
	{
		while(r >= 360.0f)
		{
			r -= 360.0f;
		}
	}
	else if(r < 0.0f)
	{
		while(r < 0.0f)
		{
			r += 360.0f;
		}
	}
	rotation = r;
	[imageView image]->fRotation = 360.0f - r;
	[imageView setNeedsDisplay:YES];
}

-(void)setScaleX:(float)x
{
	[imageView image]->fScaleX = scaleX = x;
	if(preserveAspectRatio)
	{
		[imageView image]->fScaleY = scaleX;
	}
	[imageView setNeedsDisplay:YES];
}

-(void)setScaleY:(float)y
{
	scaleY = y;
	if(!preserveAspectRatio)
	{
		[imageView image]->fScaleY = scaleY;
		[imageView setNeedsDisplay:YES];
	}
}

-(void)setScaleX:(float)x andY:(float)y
{
	[imageView image]->fScaleX = scaleX = x;
	scaleY = y;
	if(preserveAspectRatio)
	{
		[imageView image]->fScaleY = scaleX;
	}
	else
	{
		[imageView image]->fScaleY = scaleY;
	}
	[imageView setNeedsDisplay:YES];
}

-(void)setPreserveAspectRatio:(BOOL)preserve
{
	preserveAspectRatio = preserve;
	[imageView image]->fScaleX = scaleX;
	if(preserveAspectRatio)
	{
		[imageView image]->fScaleY = scaleX;
	}
	else
	{
		[imageView image]->fScaleY = scaleY;
	}
	[scaleYView setHidden:preserveAspectRatio];
	[textScaleYView setHidden:preserveAspectRatio];
	[textLabelXView setHidden:preserveAspectRatio];
	[textLabelYView setHidden:preserveAspectRatio];
	[imageView setNeedsDisplay:YES];
}

-(void)setTranslateX:(float)x
{
	[imageView image]->fTranslateX = translateX = x;
	[imageView setNeedsDisplay:YES];
}

-(void)setTranslateY:(float)y
{
	[imageView image]->fTranslateY = translateY = y;
	[imageView setNeedsDisplay:YES];
}

-(void)setTranslateX:(float)x andY:(float)y
{
	[imageView image]->fTranslateX = translateX = x;
	[imageView image]->fTranslateY = translateY = y;
	[imageView setNeedsDisplay:YES];
}

-(void)resetTransformations
{
	[self setRotation:0.0f];
	[self setScaleX:1.0f andY:1.0f];
	[self setTranslateX:0.0f andY:0.0f];
}

-(float)rotation
{
	return rotation;
}

-(float)scaleX
{
	return scaleX;
}

-(float)scaleY
{
	return scaleY;
}

-(BOOL)preserveAspectRatio
{
	return preserveAspectRatio;
}

-(float)translateX
{
	return translateX;
}

-(float)translateY
{
	return translateY;
}

@end
