/*

	Copyright:	Copyright © 2002-2003 Apple Computer, Inc., All Rights Reserved

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.
	
				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.
	
				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.
	
				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#import "Controller.h"
#import "CompositeGLView.h"
#import "MovieLayer.h"
#import "Quartz2DLayer.h"
#import "DVLayer.h"

static GLenum blendPopUpTags[] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_SRC_ALPHA_SATURATE,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA
};

static void setBlendTags(NSPopUpButton *button)
{
	int i;
	for(i = 0; i < sizeof(blendPopUpTags)/sizeof(GLenum); i++)
		[[button itemAtIndex:i] setTag:blendPopUpTags[i]];
}

@implementation Controller

- (void)awakeFromNib
{
	setBlendTags(_polySrcBlend);
	setBlendTags(_polyDstBlend);
	setBlendTags(_layerSrcBlend);
	setBlendTags(_layerDstBlend);	
        [tableView setVerticalMotionCanBeginDrag:YES];

	[self synchronizeUI];
}

- (id)init
{
	[super init];
	
	layers = [[NSMutableArray alloc] init];
	
	if(!layerTimer)
	{
		layerTimer = [[NSTimer timerWithTimeInterval:(1.0/60.0f)
					target:self
					selector:@selector(heartbeat:)
					userInfo:nil
					repeats:YES] retain];
		// Add timer to all runloop modes so animation continues even when doing
		// model things.
		[[NSRunLoop currentRunLoop] addTimer:layerTimer forMode:NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:layerTimer forMode:NSModalPanelRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:layerTimer forMode:NSEventTrackingRunLoopMode];
	}
	clockedLayers = [[NSMutableArray alloc] init];
        
	return self;
}

// Inform all layers it's time to do whatever animation they want.
- (void)heartbeat:(NSTimer *)timer
{
	[layers makeObjectsPerformSelector:@selector(heartbeat)];
	[glView displayIfNeeded];
}

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
	[window setShowsResizeIndicator:NO];

	[[NSColorPanel sharedColorPanel] setContinuous:YES];
	[[NSColorPanel sharedColorPanel] setShowsAlpha:YES];
}

- (void)open:(id)sender
{
	NSOpenPanel *panel;
	int result;
	NSArray *filenames;
	MovieLayer *movieLayer;
	
	panel = [NSOpenPanel openPanel];
	
        [panel setAllowsMultipleSelection: YES];
	result = [panel runModalForTypes:nil];
	if(result == NSOKButton)
	{
                int i;
		filenames = [panel filenames];
		
                for (i = 0; i < [filenames count]; i++) {
                    movieLayer = [[MovieLayer alloc] initWithFilename:[filenames objectAtIndex:i]];
                    if(movieLayer)
                            [self addFrontMostLayer:movieLayer];
                    [movieLayer release];
                }
	}	
}

- (void)openBackground:(id)sender
{
	NSOpenPanel *panel;
	int result;
	NSArray *filenames;
	MovieLayer *movieLayer;
	
	panel = [NSOpenPanel openPanel];
	
        [panel setAllowsMultipleSelection: NO];
	result = [panel runModalForTypes:nil];
	if(result == NSOKButton)
	{
            int i;
		filenames = [panel filenames];
		
                for (i = 0; i < [filenames count]; i++) {
                    movieLayer = [[MovieLayer alloc] initWithFilename:[filenames objectAtIndex:i]];
                    if(movieLayer)
                            [self addBackMostLayer:movieLayer];
                    [movieLayer release];
                }
                    
		
	}	
}

- (void)openDV:(id)sender
{
	DVLayer *dvLayer;
	
	dvLayer = [[DVLayer alloc] init];
	if(dvLayer)
		[self addBackMostLayer:dvLayer];		
	[self setSelectedLayer:dvLayer];
}

- (void)addBackMostLayer:(Layer *)layer
{
	[layer setOpenGLContext:[glView openGLContext]];
	[layers addObject:layer];
	[self setSelectedLayer:layer];

	[glView setNeedsDisplay:YES];
}

- (void)addFrontMostLayer:(Layer *)layer
{
	[layer setOpenGLContext:[glView openGLContext]];
	[layers insertObject:layer atIndex: 0];
	[self setSelectedLayer:layer];

	[glView setNeedsDisplay:YES];
}

- (void)addQuartzLayer:(id)sender
{
	Layer *layer = [[Quartz2DLayer alloc] initWithFrameRect:NSMakeRect(0.0f,0.0f,512.0f,512.0f)];
	
	[self addFrontMostLayer:layer];
	
	[layer release];
	[glView setNeedsDisplay:YES];
}

- (void)setGarbageMatte: (id)sender
{
    NSImage   *image;
    NSArray   *reps;
    int        i,n;

    image = [sender image];
	reps = [image representations];
	n    = [reps count];

	for(i=0 ; i<n ; i++)
		if([[reps objectAtIndex: i] isKindOfClass: [NSBitmapImageRep class]])
			[selectedLayer setGarbageMatte: [reps objectAtIndex: i]];
}

- (void)attachGarbageMatte: (NSString *)file
{
    NSImage   *image;
    NSArray   *reps;
    int        i,n;

    image = [[NSImage alloc] initWithContentsOfFile: file];
	reps = [image representations];
	n    = [reps count];

	for(i=0 ; i<n ; i++)
		if([[reps objectAtIndex: i] isKindOfClass: [NSBitmapImageRep class]])
			[selectedLayer setGarbageMatte: [reps objectAtIndex: i]];

    [image release];
}

- (NSArray *)layers
{
	return layers;
}

- (void)setSelectedLayer:(Layer *)layer
{
	selectedLayer = layer;
	[self synchronizeUI];
}

- (void)selectLayer:(id)sender
{
	int index;
	
	index = [sender selectedRow];
	
	if(index >= 0)
		[self setSelectedLayer:[layers objectAtIndex:index]];
}

// Table View data source stuff
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [layers count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
	return [[layers objectAtIndex:row] objectValueForTableColumn:tableColumn];
}

// optional - drag and drop support
- (BOOL)tableView:(NSTableView *)tv writeRows:(NSArray*)rows toPasteboard:(NSPasteboard*)pboard
{
    [pboard declareTypes:[NSArray arrayWithObject:@"Layers"] owner:self];
    [pboard setPropertyList:rows forType:@"Layers"];
    return YES;
}

    // This method is called after it has been determined that a drag should begin, but before the drag has been started.  To refuse the drag, return NO.  To start a drag, return YES and place the drag data onto the pasteboard (data, owner, etc...).  The drag image and other drag related information will be set up and provided by the table view once this call returns with YES.  The rows array is the list of row numbers that will be participating in the drag.

- (NSDragOperation)tableView:(NSTableView*)tv validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)op
{
    return NSDragOperationMove;
}

    // This method is used by NSTableView to determine a valid drop target.  Based on the mouse position, the table view will suggest a proposed drop location.  This method must return a value that indicates which dragging operation the data source will perform.  The data source may "re-target" a drop if desired by calling setDropRow:dropOperation: and returning something other than NSDragOperationNone.  One may choose to re-target for various reasons (eg. for better visual feedback when inserting into a sorted position).

- (BOOL)tableView:(NSTableView*)tv acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)op
{
    return YES;
}
    // This method is called when the mouse is released over an outline view that previously decided to allow a drop via the validateDrop method.  The data source should incorporate the data from the dragging pasteboard at this time.

// Drag 'n Drop support

- (void)synchronizeUI
{
	// Table view
	BOOL enable = selectedLayer ? YES : NO;
	BOOL mask[4];
	float red, green, blue, alpha;
	GLenum blend;
	
	if(enable)
	{
		[tableView selectRow:[layers indexOfObject:selectedLayer] byExtendingSelection:NO];
		[tableView scrollRowToVisible:[layers indexOfObject:selectedLayer]];
	}
	else
		[tableView deselectAll:nil];
	
	// Source
	
	// Effects
	
	// Clear
	[_clearEnable setEnabled: enable];
	[_clearEnable setState:   enable ? [selectedLayer isClearEnabled] : NO];
	
	[_clearPreMult setEnabled: enable];
	[_clearPreMult setState:   enable ? [selectedLayer isClearPreMult] : NO];
	
	[[_clearMask cellAtRow:0 column:0] setEnabled:enable];
	[[_clearMask cellAtRow:0 column:1] setEnabled:enable];
	[[_clearMask cellAtRow:0 column:2] setEnabled:enable];
	[[_clearMask cellAtRow:0 column:3] setEnabled:enable];
	
	if(enable)
		[selectedLayer getClearMaskRed:&mask[0] green:&mask[1] blue:&mask[2] alpha:&mask[3]];
	else
		mask[3] = mask[2] = mask[1] = mask[0] = NO;
		
	[[_clearMask cellAtRow:0 column:0] setState:mask[0]];
	[[_clearMask cellAtRow:0 column:1] setState:mask[1]];
	[[_clearMask cellAtRow:0 column:2] setState:mask[2]];
	[[_clearMask cellAtRow:0 column:3] setState:mask[3]];
	
	if(enable)
		[selectedLayer getClearColorRed:&red green:&green blue:&blue alpha:&alpha];
	else
		alpha = blue = green = red = 1.0f;
	
	[_clearColor setEnabled:enable];
	[_clearColor setColor:[NSColor colorWithDeviceRed:red green:green blue:blue alpha:alpha]];

	// Poly
	[_polyEnable setEnabled: enable];
	[_polyEnable setState:   enable ? [selectedLayer isPolyEnabled] : NO];
	
	[_polyPreMult setEnabled: enable];
	[_polyPreMult setState:   enable ? [selectedLayer isPolyPreMult] : NO];

	[_polyEnableBlend setEnabled: enable];
	[_polyEnableBlend setState:   enable ? [selectedLayer isPolyBlendEnabled] : NO];
	
	[[_polyMask cellAtRow:0 column:0] setEnabled:enable];
	[[_polyMask cellAtRow:0 column:1] setEnabled:enable];
	[[_polyMask cellAtRow:0 column:2] setEnabled:enable];
	[[_polyMask cellAtRow:0 column:3] setEnabled:enable];
	
	if(enable)
		[selectedLayer getPolyMaskRed:&mask[0] green:&mask[1] blue:&mask[2] alpha:&mask[3]];
	else
		mask[3] = mask[2] = mask[1] = mask[0] = NO;
		
	[[_polyMask cellAtRow:0 column:0] setState:mask[0]];
	[[_polyMask cellAtRow:0 column:1] setState:mask[1]];
	[[_polyMask cellAtRow:0 column:2] setState:mask[2]];
	[[_polyMask cellAtRow:0 column:3] setState:mask[3]];
	
	if(enable)
		[selectedLayer getPolyColorRed:&red green:&green blue:&blue alpha:&alpha];
	else
		alpha = blue = green = red = 1.0f;
	
	[_polyColor setEnabled:enable];
	[_polyColor setColor:[NSColor colorWithDeviceRed:red green:green blue:blue alpha:alpha]];
	
	if(enable)
		blend = [selectedLayer polySrcBlend];
	else
		blend = GL_ONE;	
	[_polySrcBlend selectItemAtIndex:[_polySrcBlend indexOfItemWithTag:blend]];

	if(enable)
		blend = [selectedLayer polyDstBlend];
	else
		blend = GL_ZERO;	
	[_polyDstBlend selectItemAtIndex:[_polyDstBlend indexOfItemWithTag:blend]];
	
	// Layer
	[_layerEnable setEnabled: enable];
	[_layerEnable setState:   enable ? [selectedLayer isLayerEnabled] : NO];
	
	[_layerPreMult setEnabled: enable];
	[_layerPreMult setState:   enable ? [selectedLayer isLayerPreMult] : NO];

	[_layerEnableBlend setEnabled: enable];
	[_layerEnableBlend setState:   enable ? [selectedLayer isLayerBlendEnabled] : NO];
	
	[[_layerMask cellAtRow:0 column:0] setEnabled:enable];
	[[_layerMask cellAtRow:0 column:1] setEnabled:enable];
	[[_layerMask cellAtRow:0 column:2] setEnabled:enable];
	[[_layerMask cellAtRow:0 column:3] setEnabled:enable];
	
	if(enable)
		[selectedLayer getLayerMaskRed:&mask[0] green:&mask[1] blue:&mask[2] alpha:&mask[3]];
	else
		mask[3] = mask[2] = mask[1] = mask[0] = NO;
		
	[[_layerMask cellAtRow:0 column:0] setState:mask[0]];
	[[_layerMask cellAtRow:0 column:1] setState:mask[1]];
	[[_layerMask cellAtRow:0 column:2] setState:mask[2]];
	[[_layerMask cellAtRow:0 column:3] setState:mask[3]];
	
	if(enable)
		[selectedLayer getLayerColorRed:&red green:&green blue:&blue alpha:&alpha];
	else
		alpha = blue = green = red = 1.0f;
	
	[_layerColor setEnabled:enable];
	[_layerColor setColor:[NSColor colorWithDeviceRed:red green:green blue:blue alpha:alpha]];
	
	if(enable)
		blend = [selectedLayer layerSrcBlend];
	else
		blend = GL_ONE;	
	[_layerSrcBlend selectItemAtIndex:[_layerSrcBlend indexOfItemWithTag:blend]];

	if(enable)
		blend = [selectedLayer layerDstBlend];
	else
		blend = GL_ZERO;	
	[_layerDstBlend selectItemAtIndex:[_layerDstBlend indexOfItemWithTag:blend]];
	
	//
	// Effects Tab
	//
	
	// background removal
	[_backgroundRemoveEnable setEnabled:enable];
	[_backgroundRemoveEnable setState:enable ? [selectedLayer isBackgroundRemovalEnabled] : NO];
	
	if(enable)
		[selectedLayer getBackgroundColorRed:&red green:&green blue:&blue];
	else
		red = green = blue = 1.0f;
	[_backgroundColor setEnabled:enable];
	[_backgroundColor setColor:[NSColor colorWithDeviceRed:red green:green blue:blue alpha:1.0f]];
	
	[_backgroundTolerance setEnabled:enable];
	[_backgroundTolerance setFloatValue:enable ? [selectedLayer backgroundTolerance] : 0.0f];
	
        // Color correction
        [_colorCorrectionEnable setEnabled:enable];
        [_colorCorrectionEnable setState: enable ? [selectedLayer isColorCorrectionEnabled] : NO];
        
        [_colorCorrection setEnabled:enable];
        [_colorCorrection setFloatValue: enable ? [selectedLayer colorCorrection] : 0.0f];

	if(enable)
		[selectedLayer getColorCorrectionSourceRed:&red green:&green blue:&blue];
	else
		red = green = blue = 1.0f;
	[_colorCorrectionSourceColor setEnabled:enable];
	[_colorCorrectionSourceColor setColor:[NSColor colorWithDeviceRed:red green:green blue:blue alpha:1.0f]];

	if(enable)
		[selectedLayer getColorCorrectionDestRed:&red green:&green blue:&blue];
	else
		red = green = blue = 1.0f;
	[_colorCorrectionDestColor setEnabled:enable];
	[_colorCorrectionDestColor setColor:[NSColor colorWithDeviceRed:red green:green blue:blue alpha:1.0f]];
        
	// Color matrix
	[_colorMatrixEnable setEnabled:enable];
	[_colorMatrixEnable setState: enable ? [selectedLayer isColorMatrixEnabled] : NO];
	
	[_colorMatrixHue setEnabled:enable];
	[_colorMatrixHue setFloatValue: enable ? [selectedLayer colorMatrixHue] : 0.0f];
	
	[_colorMatrixSaturation setEnabled:enable];
	[_colorMatrixSaturation setFloatValue: enable ? [selectedLayer colorMatrixSaturation] : 1.0f];
	
	[_colorMatrixBrightness setEnabled:enable];
	[_colorMatrixBrightness setFloatValue: enable ? [selectedLayer colorMatrixBrightness] : 1.0f];
}

- (void)setClearEnable:(id)sender
{
	[selectedLayer setClearEnable:[sender state]];
}

- (void)setClearPreMult:(id)sender
{
	[selectedLayer setClearPreMult:[sender state]];
}

- (void)setClearMask:(id)sender
{
	BOOL red, green, blue, alpha;
	
	red   = [[sender cellAtRow:0 column:0] state];
	green = [[sender cellAtRow:0 column:1] state];
	blue  = [[sender cellAtRow:0 column:2] state];
	alpha = [[sender cellAtRow:0 column:3] state];
	
	[selectedLayer setClearMaskRed:red green:green blue:blue alpha:alpha];
}

- (void)setClearColor:(id)sender
{
	float red, green, blue, alpha;
	NSColor *color;
	
	// Make sure it's in RGBA color space
	color = [[sender color] colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&red green:&green blue:&blue alpha:&alpha];
	[selectedLayer setClearColorRed:red green:green blue:blue alpha:alpha];
}

- (void)setPolyEnable:(id)sender
{
	[selectedLayer setPolyEnable:[sender state]];
}

- (void)setPolyPreMult:(id)sender
{
	[selectedLayer setPolyPreMult:[sender state]]; 
}

- (void)setPolyEnableBlend:(id)sender
{
	[selectedLayer setPolyBlendEnable:[sender state]];
}

- (void)setPolyMask:(id)sender
{
	BOOL red, green, blue, alpha;
	
	red   = [[sender cellAtRow:0 column:0] state];
	green = [[sender cellAtRow:0 column:1] state];
	blue  = [[sender cellAtRow:0 column:2] state];
	alpha = [[sender cellAtRow:0 column:3] state];
	
	[selectedLayer setPolyMaskRed:red green:green blue:blue alpha:alpha];
}

- (void)setPolySrcBlend:(id)sender
{
	[selectedLayer setPolySrcBlend:[[sender selectedItem] tag]];
}

- (void)setPolyDstBlend:(id)sender
{
	[selectedLayer setPolyDstBlend:[[sender selectedItem] tag]];
}

- (void)setPolyColor:(id)sender
{
	float red, green, blue, alpha;
	NSColor *color;
	
	// Make sure it's in RGBA color space
	color = [[sender color] colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&red green:&green blue:&blue alpha:&alpha];
	[selectedLayer setPolyColorRed:red green:green blue:blue alpha:alpha];
}

- (void)setLayerEnable:(id)sender
{
	[selectedLayer setLayerEnable:[sender state]];
}

- (void)setLayerPreMult:(id)sender
{
	[selectedLayer setLayerPreMult:[sender state]]; 
}

- (void)setLayerEnableBlend:(id)sender
{
	[selectedLayer setLayerBlendEnable:[sender state]];
}

- (void)setLayerMask:(id)sender
{
	BOOL red, green, blue, alpha;
	
	red   = [[sender cellAtRow:0 column:0] state];
	green = [[sender cellAtRow:0 column:1] state];
	blue  = [[sender cellAtRow:0 column:2] state];
	alpha = [[sender cellAtRow:0 column:3] state];
	
	[selectedLayer setLayerMaskRed:red green:green blue:blue alpha:alpha];
}

- (void)setLayerSrcBlend:(id)sender
{
	[selectedLayer setLayerSrcBlend:[[sender selectedItem] tag]];
}

- (void)setLayerDstBlend:(id)sender
{
	[selectedLayer setLayerDstBlend:[[sender selectedItem] tag]];
}

- (void)setLayerColor:(id)sender
{
	float red, green, blue, alpha;
	NSColor *color;
	
	// Make sure it's in RGBA color space
	color = [[sender color] colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&red green:&green blue:&blue alpha:&alpha];
	[selectedLayer setLayerColorRed:red green:green blue:blue alpha:alpha];
}

// Effects tab
- (void)setBackgroundRemoveEnable:(id)sender
{
	[selectedLayer setBackgroundRemovalEnable:[sender state]];
}

- (void)setBackgroundColor:(id)sender
{
	float red, green, blue, alpha;
	NSColor *color;
	
	// Make sure it's in RGBA color space
	color = [[sender color] colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&red green:&green blue:&blue alpha:&alpha];
	[selectedLayer setBackgroundColorRed:red green:green blue:blue];
}

- (void)setBackgroundTolerance:(id)sender
{
	[selectedLayer setBackgroundTolerance:[sender floatValue]];
}

- (void)setColorCorrectionEnable:(id)sender
{
        [selectedLayer setColorCorrectionEnable:[sender state]];
}

- (void)setColorCorrection:(id)sender
{
        [selectedLayer setColorCorrection:[sender floatValue]];
}

- (void)setColorCorrectionSourceColor:(id)sender
{
	float red, green, blue, alpha;
	NSColor *color;
	
	// Make sure it's in RGBA color space
	color = [[sender color] colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&red green:&green blue:&blue alpha:&alpha];
	[selectedLayer setColorCorrectionSourceRed:red green:green blue:blue];
}

- (void)setColorCorrectionDestColor:(id)sender
{
	float red, green, blue, alpha;
	NSColor *color;
	
	// Make sure it's in RGBA color space
	color = [[sender color] colorUsingColorSpaceName:NSDeviceRGBColorSpace];
	[color getRed:&red green:&green blue:&blue alpha:&alpha];
	[selectedLayer setColorCorrectionDestRed:red green:green blue:blue];
}

- (void)setColorMatrixEnable:(id)sender
{
	[selectedLayer setColorMatrixEnable:[sender state]];
}

- (void)setColorMatrixHue:(id)sender
{
	[selectedLayer setColorMatrixHue:[sender floatValue]];
}

- (void)setColorMatrixSaturation:(id)sender
{
	[selectedLayer setColorMatrixSaturation:[sender floatValue]];
}

- (void)setColorMatrixBrightness:(id)sender
{
	[selectedLayer setColorMatrixBrightness:[sender floatValue]];
}

@end
