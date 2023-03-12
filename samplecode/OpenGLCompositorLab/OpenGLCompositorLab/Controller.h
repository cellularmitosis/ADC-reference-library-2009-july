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
#import <Cocoa/Cocoa.h>
#import <QuickTime/QuickTime.h>

@class Layer;

@interface Controller : NSObject
{
    IBOutlet id glView;
    IBOutlet id window;
    IBOutlet id drawer;
    
    // Layer storage
    Layer		*selectedLayer;
    NSMutableArray	*layers;	
    NSTimer		*layerTimer;
    NSMutableArray      *clockedLayers;
    
    // Drawer UI stuff
    
    // Layers
    IBOutlet id tableView;
    
    // Source
    IBOutlet id layerName;
    IBOutlet id layerType;
    
    // Effect
    IBOutlet NSButton *_clearEnable;
    IBOutlet NSButton *_clearPreMult;
    IBOutlet NSMatrix *_clearMask;
    IBOutlet NSColorWell *_clearColor;
    
    IBOutlet NSButton *_polyEnable;
    IBOutlet NSButton *_polyPreMult;
    IBOutlet NSButton *_polyEnableBlend;
    IBOutlet NSMatrix *_polyMask;
    IBOutlet NSPopUpButton *_polySrcBlend;
    IBOutlet NSPopUpButton *_polyDstBlend;
    IBOutlet NSColorWell *_polyColor;
    
    IBOutlet NSButton *_layerEnable;
    IBOutlet NSButton *_layerPreMult;
    IBOutlet NSButton *_layerEnableBlend;
    IBOutlet NSMatrix *_layerMask;
    IBOutlet NSPopUpButton *_layerSrcBlend;
    IBOutlet NSPopUpButton *_layerDstBlend;
    IBOutlet NSColorWell *_layerColor;
    
    IBOutlet NSButton *_backgroundRemoveEnable;
    IBOutlet NSColorWell *_backgroundColor;
    IBOutlet NSSlider *_backgroundTolerance;
    
    IBOutlet NSButton *_colorCorrectionEnable;
    IBOutlet NSSlider *_colorCorrection;
    IBOutlet NSColorWell *_colorCorrectionSourceColor;
    IBOutlet NSColorWell *_colorCorrectionDestColor;
    
    IBOutlet NSButton *_colorMatrixEnable;
    IBOutlet NSSlider *_colorMatrixHue;
    IBOutlet NSSlider *_colorMatrixSaturation;
    IBOutlet NSSlider *_colorMatrixBrightness;
}

// Menu actions (and eventually ToolBar)
- (void)open:(id)sender;
- (void)openBackground:(id)sender;
- (void)openDV:(id)sender;
- (void)addQuartzLayer:(id)sender;

// Misc
- (void)attachGarbageMatte: (NSString *)file;

// Layer manipulation
- (NSArray *)layers;
- (void)setSelectedLayer:(Layer *)layer;
- (void)addFrontMostLayer:(Layer *)layer;
- (void)addBackMostLayer:(Layer *)layer;

// UI Actions
- (void)selectLayer:(id)sender;
- (void)synchronizeUI;

// Compositing Tab
- (void)setClearEnable:(id)sender;
- (void)setClearPreMult:(id)sender;
- (void)setClearMask:(id)sender;
- (void)setClearColor:(id)sender;

- (void)setPolyEnable:(id)sender;
- (void)setPolyPreMult:(id)sender;
- (void)setPolyEnableBlend:(id)sender;
- (void)setPolyMask:(id)sender;
- (void)setPolySrcBlend:(id)sender;
- (void)setPolyDstBlend:(id)sender;
- (void)setPolyColor:(id)sender;

- (void)setLayerEnable:(id)sender;
- (void)setLayerPreMult:(id)sender;
- (void)setLayerEnableBlend:(id)sender;
- (void)setLayerMask:(id)sender;
- (void)setLayerSrcBlend:(id)sender;
- (void)setLayerDstBlend:(id)sender;
- (void)setLayerColor:(id)sender;

// Effects Tab
- (void)setBackgroundRemoveEnable:(id)sender;
- (void)setBackgroundColor:(id)sender;
- (void)setBackgroundTolerance:(id)sender;

- (void)setColorCorrectionEnable:(id)sender;
- (void)setColorCorrection:(id)sender;
- (void)setColorCorrectionSourceColor:(id)sender;
- (void)setColorCorrectionDestColor:(id)sender;

- (void)setColorMatrixEnable:(id)sender;
- (void)setColorMatrixHue:(id)sender;
- (void)setColorMatrixSaturation:(id)sender;
- (void)setColorMatrixBrightness:(id)sender;

@end
