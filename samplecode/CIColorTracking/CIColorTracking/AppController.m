/*
Project: CIColorTracking

File: AppController.m

Abstract: 
This is the implementation file for AppController, a class that handles the interaction between the
GUI, the video, and the tracking filters, allowing you to select a source movie, change
tracking options, view the tracking video, see a preview of the mask used for the video, 
and overlay the mask image onto the tracking video.

Version 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

#import "AppController.h"
#import "VideoCIView.h"
#import "VideoHandler.h"

@interface AppController (private)

- (void)_loadMovie:(NSString *)path;
- (void)_renderFilterChain:(BOOL)rechain;

@end

@implementation AppController

//--------------------------------------------------------------------------------------------------

- (void)awakeFromNib
{
	lock = [[NSRecursiveLock alloc] init];
	
    // Load all Image Units that are installed in /Library/Graphics/Image Units
    [CIPlugIn loadAllPlugIns];
    // Get a list of all filters.
    NSArray	    *filterList = [CIFilter filterNamesInCategories:nil];	
    // If the filter need for color tracking is not in the list, then
    // load the filter from the  PlugIns directory of the application.
    if(![filterList containsObject:@"GPUGemsAreaMean"])
    {
		NSString    *path = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"TrackingImageUnit.plugin"];
		NSURL	    *pluginURL = [NSURL fileURLWithPath:path];
		[CIPlugIn loadPlugIn:pluginURL allowNonExecutable:NO];
    }
    
    // Update the filters list. 
    filterList = [CIFilter filterNamesInCategories:nil];
    // Check to make sure that the filter needed for color tracking is on the list.
    if(![filterList containsObject:@"GPUGemsAreaMean"])
    {
		NSRunCriticalAlertPanel(@"Required filters are missing", @"The Tracking Image Unit is not installed", @"Quit", nil, nil, nil);
		[NSApp terminate:self];
    }
	
	// Load the filters that you need for color tracking. Some of these are built-in to Core Image,
	// Others are provided by the Tracking Image Unit.
	cropFilter = [[CIFilter filterWithName:@"CICrop"] retain];
	stage1_Filter_maskFromColor = [[CIFilter filterWithName:@"MaskFromColor"] retain];
	stage2_Filter_multiplyCoordinates = [[CIFilter filterWithName:@"CoordinateMaskWithColor"] retain];
	stage2_Filter_areaMean = [[CIFilter filterWithName:@"GPUGemsAreaMean"] retain];
	stage2_Filter_coordinateNormalize = [[CIFilter filterWithName:@"Centroid"] retain];
	stage3_Filter_compositeResult = [[CIFilter filterWithName:@"CompositeFilter"] retain];
	postProcessing_Filter_colorMatrix = [[CIFilter filterWithName:@"CIColorMatrix"] retain];
	postProcessing_Filter_composite = [[CIFilter filterWithName:@"CISourceOverCompositing"] retain];
	
	// Set initial values for filter.
	[cropFilter setValue:[CIVector vectorWithX:70.0 Y:0.0 Z:640.0 W:480.0] forKey:@"inputRectangle"];
	[stage1_Filter_maskFromColor setDefaults];
	[stage2_Filter_areaMean setDefaults];
	[stage3_Filter_compositeResult setDefaults];
	[postProcessing_Filter_colorMatrix setDefaults];
	CIVector	*alphaVector = [CIVector vectorWithX:0.0 Y:0.0 Z:0.0 W:[maskOpacitySlider floatValue]];
	[postProcessing_Filter_colorMatrix setValue:alphaVector forKey:@"inputAVector"];
	
	// Read the overlay image from the resources.
	NSString	*imagePath = [[NSBundle bundleForClass:[self class]] pathForResource:@"Duck" ofType:@"tiff"];
	[stage3_Filter_compositeResult setValue:[CIImage imageWithContentsOfURL:[NSURL fileURLWithPath:imagePath]] forKey:@"inputOverlayImage"];
}

//--------------------------------------------------------------------------------------------------

- (void)applicationWillFinishLaunching:(NSNotification *)note
{
    NSString *moviePath;
	CIColor  *cicolor;

    // Check whether the the MoviePath key is set to a pathname.
    moviePath = [[NSUserDefaults standardUserDefaults] stringForKey:@"MoviePath"];
    // If so, load the movie located at that path.
    if(moviePath)
    {
        [self _loadMovie:moviePath];
    }
	
	// Get the default tracking color.
	NSData *theData = [[[NSUserDefaults standardUserDefaults] stringForKey:@"TrackingColor"] retain];
	if (theData != nil)
		trackingColor = (NSColor *)[NSUnarchiver unarchiveObjectWithData:theData]; 
	// Set the default tracking color as the value of the input color parameters for the mask from color filter.
	if(!trackingColor)
        trackingColor =  [[NSColor colorWithCIColor: [stage1_Filter_maskFromColor valueForKey:@"inputColor"]] retain];

	[colorTrackingBox setColor:trackingColor];
	
	// Set the defatult tracking tolerance value.
	NSNumber *trackingToleranceValue = (NSNumber *)[[NSUserDefaults standardUserDefaults] stringForKey:@"TrackingTolerance"];
	if(!trackingToleranceValue)
		trackingToleranceValue = [stage1_Filter_maskFromColor valueForKey:@"inputThreshold"];
	trackingTolerance = [trackingToleranceValue floatValue];
	[toleranceSlider setFloatValue:trackingTolerance];
	[toleranceField setFloatValue:trackingTolerance];
	
	// Allocate a CIColor object and initialize it with the tracking color.
	cicolor = [[CIColor alloc] initWithColor: trackingColor];
    
    // Set the color input value and threshold value for the mask from color filter.
	[stage1_Filter_maskFromColor setValue:[cicolor autorelease] forKey:@"inputColor"];
	[stage1_Filter_maskFromColor setValue:[NSNumber numberWithFloat:trackingTolerance] forKey:@"inputThreshold"];
	[self _renderFilterChain:YES];
}

//--------------------------------------------------------------------------------------------------

- (void)dealloc
{
	[trackingColor release];
	[videoHandler release];
	[lock release];
	[stage1_Filter_maskFromColor release];
	[stage2_Filter_multiplyCoordinates release];
	[stage2_Filter_areaMean release];
	[stage2_Filter_coordinateNormalize release];
	[stage3_Filter_compositeResult release];
	[postProcessing_Filter_colorMatrix release];
	[postProcessing_Filter_composite release];
	[super dealloc];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)movieScrubAction:(id)sender
{
	[videoHandler scrubMovie:[sender doubleValue]];
	[playButton setImage:[videoHandler isPlaying] ? [NSImage imageNamed:@"FS_Pause_Normal.tif"] : [NSImage imageNamed:@"FS_Play_Normal.tif"]];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)selectMovie:(id)sender
{
    NSOpenPanel		*openPanel;
    int				rv;
    
    openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setResolvesAliases:YES];
    [openPanel setCanChooseFiles:YES];
    
    rv = [openPanel runModalForTypes:nil];
    if(rv == NSFileHandlingPanelOKButton)
        [self _loadMovie:[openPanel filename]];
}

//--------------------------------------------------------------------------------------------------

- (void)_loadMovie:(NSString *)path
{
	if(!videoHandler)
		videoHandler = [[VideoHandler alloc] initWithView:videoView delegate:self];

    if([videoHandler loadMovieFromPath:path])
    {
        [[NSUserDefaults standardUserDefaults] setObject:path forKey:@"MoviePath"];
        [[NSUserDefaults standardUserDefaults] synchronize];
        		
		[maskOpacitySlider setEnabled:YES];
		[showCompositeCheckBox setEnabled:YES];
		[playButton setEnabled:YES];
		[videoScrubber setEnabled:YES];
    }
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setCompositeVisible:(id)sender
{
	[lock lock];
	[self _renderFilterChain:NO];
	[lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setMaskOverlayOpacity:(id)sender
{
	[lock lock];
	CIVector	*alphaVector = [CIVector vectorWithX:0.0 Y:0.0 Z:0.0 W:[sender floatValue]];
	[postProcessing_Filter_colorMatrix setValue:alphaVector forKey:@"inputAVector"];
	[self _renderFilterChain:YES];
	[lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setTrackingColor:(id)sender
{
	[lock lock];
	[trackingColor release];
	trackingColor = [[colorTrackingBox color] retain];
	[[NSUserDefaults standardUserDefaults] setObject:[NSArchiver archivedDataWithRootObject:trackingColor] forKey:@"TrackingColor"];
	[[NSUserDefaults standardUserDefaults] synchronize];	
	[stage1_Filter_maskFromColor setValue:[CIColor colorWithRed:[trackingColor redComponent] green:[trackingColor greenComponent] blue:[trackingColor blueComponent]] forKey:@"inputColor"];
	[self _renderFilterChain:YES];
	[lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setTrackingTolerance:(id)sender
{
	[lock lock];
	trackingTolerance = [sender floatValue];
	[[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithFloat:trackingTolerance] forKey:@"TrackingTolerance"];
	[[NSUserDefaults standardUserDefaults] synchronize];	
	[toleranceSlider setFloatValue:trackingTolerance];
	[toleranceField setFloatValue:trackingTolerance];
	[stage1_Filter_maskFromColor setValue:[NSNumber numberWithFloat:trackingTolerance] forKey:@"inputThreshold"];
	[self _renderFilterChain:YES];
	[lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)togglePlayback:(id)sender
{
	[videoHandler togglePlayback];
	[playButton setImage:[videoHandler isPlaying] ? [NSImage imageNamed:@"FS_Pause_Normal.tif"] : [NSImage imageNamed:@"FS_Play_Normal.tif"]];
}


//--------------------------------------------------------------------------------------------------

- (void)renderFrame:(CVImageBufferRef)inFrame progress:(double)inProgress
{
    CIImage  *image;

    [lock lock];
	[cropFilter setValue:[CIImage imageWithCVImageBuffer:inFrame] forKey:@"inputImage"];
    image = [cropFilter valueForKey:@"outputImage"];

    [stage1_Filter_maskFromColor setValue: image forKey:@"inputImage"];
    [stage3_Filter_compositeResult setValue: image forKey:@"inputImage"];
    [self _renderFilterChain:YES];
    [videoScrubber setDoubleValue:inProgress];
    [lock unlock];
}

//--------------------------------------------------------------------------------------------------

- (void)_renderFilterChain:(BOOL)rechain
{
	if([stage1_Filter_maskFromColor valueForKey:@"inputImage"])
	{
        if(rechain)
		{
			CGRect  extent;

			[stage2_Filter_multiplyCoordinates setValue:[stage1_Filter_maskFromColor valueForKey:@"outputImage"] forKey:@"inputImage"];

			extent = [[stage2_Filter_multiplyCoordinates valueForKey:@"outputImage"] extent];
					[stage2_Filter_areaMean setValue: [CIVector vectorWithX: CGRectGetMinX(extent)  Y: CGRectGetMinY(extent)
						Z: CGRectGetWidth(extent)  W: CGRectGetHeight(extent)]  forKey:@"inputExtent"];
			[stage2_Filter_areaMean setValue:[stage2_Filter_multiplyCoordinates valueForKey:@"outputImage"] forKey:@"inputImage"];
			[stage2_Filter_coordinateNormalize setValue:[stage2_Filter_areaMean valueForKey:@"outputImage"] forKey:@"inputImage"];

			[stage3_Filter_compositeResult setValue:[stage2_Filter_coordinateNormalize valueForKey:@"outputImage"] forKey:@"inputLocation"];

			[postProcessing_Filter_colorMatrix setValue:[stage1_Filter_maskFromColor valueForKey:@"outputImage"] forKey:@"inputImage"];
			if([showCompositeCheckBox state] == NSOnState)
				[postProcessing_Filter_composite setValue:[stage3_Filter_compositeResult valueForKey:@"outputImage"] forKey:@"inputBackgroundImage"];
			else
				[postProcessing_Filter_composite setValue:[stage1_Filter_maskFromColor valueForKey:@"inputImage"] forKey:@"inputBackgroundImage"];
			[postProcessing_Filter_composite setValue:[postProcessing_Filter_colorMatrix valueForKey:@"outputImage"] forKey:@"inputImage"];
		}

		[maskView setImage:[stage1_Filter_maskFromColor valueForKey:@"outputImage"]];
		[videoView setImage:[postProcessing_Filter_composite valueForKey:@"outputImage"]];
	}
}

@end
