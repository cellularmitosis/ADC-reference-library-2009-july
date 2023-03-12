/*

	Layer.h
	WWDC Open Compositor Lab
	Created by kdyke on Mon Oct 22 2001.

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
#import <OpenGL/gl.h>

enum {
	kOpenGLPixelFormat = FOUR_CHAR_CODE('OGLX')
};

@interface Layer : NSObject 
{
	NSOpenGLContext *context;
	NSRect			layerRect; // presentation rect in composite view
	NSString		*name;
	float 			fade;
	int 			hasAlpha;
	BOOL			enableUpdates;
	
	struct {
		GLfloat			a, b, c;
		GLfloat			d, e, f;
		GLfloat			g, h;
	} matrix;
	
	GLfloat			sx[4], sy[4];
	GLfloat			tx[4], ty[4];

	GLfloat			minX, minY, maxX, maxY;
	
	GLfloat			xPos[4];
	GLfloat			yPos[4];
	GLfloat			wPos[4];
        
	GLuint			colorTextureWidth;
	GLuint			colorTextureHeight;	
	void			*colorTextureData;
	GLuint			colorTextureName;
	GLuint			colorTextureFormat;
	GLuint			colorTextureType;
	GLuint			colorTextureInternalFormat;
	GLuint			colorTextureBytesPerRow;
	GLuint			colorTextureBytesPerPixel;
	GLuint			colorTextureRowLength;
	
	GLuint			alphaTextureWidth;
	GLuint			alphaTextureHeight;
	GLuint			alphaTextureName;
	void			*alphaTextureData;
	GLuint			alphaTextureBytesPerRow;
	
	GLboolean		clearEnable;
	GLboolean		clearMask[4];
	GLboolean		clearPreMult;
	GLfloat			clearColor[4];
	
	GLboolean		polyEnable;
	GLboolean		polyPreMult;
	GLboolean		polyBlendEnable;
	GLboolean		polyMask[4];
	GLenum			polySrcBlend;
	GLenum			polyDstBlend;
	GLfloat			polyColor[4];
	
	GLboolean		layerEnable;
	GLboolean		layerPreMult;
	GLboolean		layerBlendEnable;
	GLboolean		layerMask[4];
	GLenum			layerSrcBlend;
	GLenum			layerDstBlend;
	GLfloat			layerColor[4];				
	
	GLboolean		backgroundRemoveEnable;
	GLfloat			backgroundColor[3];
	GLfloat			backgroundTolerance;
	
	GLboolean		colorCorrectionEnable;
	GLfloat			colorCorrection;
	GLfloat			colorCorrectionSource[3];
	GLfloat			colorCorrectionDest[3];
        
	GLboolean		colorMatrixEnable;
	GLfloat			colorMatrixHue;
	GLfloat			colorMatrixSaturation;
	GLfloat			colorMatrixBrightness;
	float			colorMatrix[3][3];

	GLuint			remapTextureDimensions[3];
	unsigned char   *remapTextureData;
	GLuint			remapTextureName;

	GLuint			remapTextureWidthLog2;
	GLuint			remapTextureHeightLog2;
	GLuint			remapTextureDepthLog2;
	
	GLuint			remapTextureWidth;
	GLuint			remapTextureHeight;
	GLuint			remapTextureDepth;
	
	float 			angle;
        
	BOOL			remapDataDirty;
	NSBitmapImageRep  	*matte;
        
	BOOL			layerDirty;
}

- (id)init;
- (id)objectValueForTableColumn:(NSTableColumn *)tableColumn;
- (void)setOpenGLContext:(NSOpenGLContext *)ctx;
- (void)updateLayerInfo;
- (void)setPosition:(NSPoint)point;
- (void)setPosition:(NSPoint)point atIndex:(unsigned)index;
- (NSPoint)position;
- (void)setSize:(NSSize)size;
- (NSSize)size;
- (NSRect)rect;
- (void)setRect:(NSRect)rect;
- (int)pointIndexForPoint:(NSPoint)point;
- (void)layerUpdated;
- (void)dirtyColorTexture;
- (void)dirtyAlphaTexture;
- (void)createTextures;
- (void) setFade: (float) fade;
- (float) fade;
- (float) alphaForPoint: (NSPoint) point;
- (void)heartBeat;
- (void)enableUpdates:(BOOL)flag;
- (void)drawLayer;
- (void)setColorFormat:(OSType)format colorWidth:(unsigned)width colorHeight:(unsigned)height colorData:(unsigned char *)data colorRowBytes:(unsigned)rowBytes;
- (void)lockBits;
- (void)unlockBits;

- (void)centerInRect: (NSRect)frame;
- (double)aspectRatio;

- (void)setGarbageMatte: (NSBitmapImageRep *)matte;
- (NSBitmapImageRep *)garbageMatte;

- (void)setClearEnable:(BOOL)flag;
- (void)setClearMaskRed:(BOOL)red green:(BOOL)green blue:(BOOL)blue alpha:(BOOL)alpha;
- (void)setClearPreMult:(BOOL)flag;
- (void)setClearColorRed:(GLfloat)red green:(GLfloat)green blue:(GLfloat)blue alpha:(GLfloat)alpha;
- (BOOL)isClearEnabled;
- (void)getClearMaskRed:(BOOL *)red green:(BOOL *)green blue:(BOOL *)blue alpha:(BOOL *)alpha;
- (BOOL)isClearPreMult;
- (void)getClearColorRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha;

- (void)setPolyEnable:(BOOL)flag;
- (void)setPolyBlendEnable:(BOOL)flag;
- (void)setPolyMaskRed:(BOOL)red green:(BOOL)green blue:(BOOL)blue alpha:(BOOL)alpha;
- (void)setPolyPreMult:(BOOL)flag;
- (void)setPolyColorRed:(GLfloat)red green:(GLfloat)green blue:(GLfloat)blue alpha:(GLfloat)alpha;
- (void)setPolySrcBlend:(GLenum)srcBlend;
- (void)setPolyDstBlend:(GLenum)dstBlend;

- (BOOL)isPolyEnabled;
- (BOOL)isPolyBlendEnabled;
- (void)getPolyMaskRed:(BOOL *)red green:(BOOL *)green blue:(BOOL *)blue alpha:(BOOL *)alpha;
- (BOOL)isPolyPreMult;
- (void)getPolyColorRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha;
- (GLenum)polySrcBlend;
- (GLenum)polyDstBlend;

- (void)setLayerEnable:(BOOL)flag;
- (void)setLayerBlendEnable:(BOOL)flag;
- (void)setLayerMaskRed:(BOOL)red green:(BOOL)green blue:(BOOL)blue alpha:(BOOL)alpha;
- (void)setLayerPreMult:(BOOL)flag;
- (void)setLayerColorRed:(GLfloat)red green:(GLfloat)green blue:(GLfloat)blue alpha:(GLfloat)alpha;
- (void)setLayerSrcBlend:(GLenum)srcBlend;
- (void)setLayerDstBlend:(GLenum)dstBlend;

- (BOOL)isLayerEnabled;
- (BOOL)isLayerBlendEnabled;
- (void)getLayerMaskRed:(BOOL *)red green:(BOOL *)green blue:(BOOL *)blue alpha:(BOOL *)alpha;
- (BOOL)isLayerPreMult;
- (void)getLayerColorRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha;
- (GLenum)layerSrcBlend;
- (GLenum)layerDstBlend;

- (void)setBackgroundRemovalEnable:(BOOL)flag;
- (BOOL)isBackgroundRemovalEnabled;
- (void)setBackgroundColorRed:(float)red green:(float)green blue:(float)blue;
- (void)getBackgroundColorRed:(float *)red green:(float *)green blue:(float *)blue;
- (void)setBackgroundTolerance:(float)tolerance;
- (float)backgroundTolerance;

- (void)setColorCorrectionEnable:(BOOL)flag;
- (BOOL)isColorCorrectionEnabled;
- (void)setColorCorrection:(float)correction;
- (float)colorCorrection;
- (void)setColorCorrectionSourceRed:(float)red green:(float)gleen blue:(float)blue;
- (void)getColorCorrectionSourceRed:(float *)red green:(float *)gleen blue:(float *)blue;
- (void)setColorCorrectionDestRed:(float)red green:(float)gleen blue:(float)blue;
- (void)getColorCorrectionDestRed:(float *)red green:(float *)gleen blue:(float *)blue;

- (void)setColorMatrixEnable:(BOOL)flag;
- (BOOL)isColorMatrixEnabled;
- (void)setColorMatrixHue:(float)hue;
- (void)setColorMatrixSaturation:(float)saturation;
- (void)setColorMatrixBrightness:(float)brightness;
- (float)colorMatrixHue;
- (float)colorMatrixSaturation;
- (float)colorMatrixBrightness;

@end
