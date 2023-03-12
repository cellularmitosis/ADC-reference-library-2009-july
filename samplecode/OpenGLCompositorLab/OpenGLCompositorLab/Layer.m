/*
	Layer.m
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

#import "Layer.h"
#import <OpenGL/glext.h>
#import <OpenGL/CGLCurrent.h>
#import "OpenGLCodecs.h"
#import "mat3x3.h"

@interface Layer(Private)
- (void)generateRemapTexture;
@end

@implementation Layer

+ (void)initialize
{
	if([self class] == [Layer class])
	{
		ICMPixelFormatInfo pixelInfo;
		
		// Register information about our custom pixel format with the ICM.
		memset(&pixelInfo, 0, sizeof(pixelInfo));
		pixelInfo.size	= sizeof(ICMPixelFormatInfo);
		pixelInfo.formatFlags	  = 0;
		pixelInfo.bitsPerPixel[0] = 32;
	
		// Ignore any errors as this could be a duplicate registration
		ICMSetPixelFormatInfo(kOpenGLPixelFormat, &pixelInfo);

		OpenGLRAWCodec_DoRegister();
		OpenGLCodec_DoRegister();
	}
}

- init;
{
	[super init];
	fade = 1.0;
	enableUpdates = YES;

	polyMask[0] = GL_TRUE;
	polyMask[1] = GL_TRUE;
	polyMask[2] = GL_TRUE;
	polyMask[3] = GL_TRUE;

	polyColor[0] = 1.0f;
	polyColor[1] = 1.0f;
	polyColor[2] = 1.0f;
	polyColor[3] = 1.0f;
	
	layerEnable = YES;
	layerPreMult = NO;
	layerBlendEnable = NO;
	layerSrcBlend = GL_ONE;
	layerDstBlend = GL_ZERO;
	
	layerMask[0] = GL_TRUE;
	layerMask[1] = GL_TRUE;
	layerMask[2] = GL_TRUE;
	layerMask[3] = GL_TRUE;
	
	layerColor[0] = 1.0f;
	layerColor[1] = 1.0f;
	layerColor[2] = 1.0f;
	layerColor[3] = 1.0f;

	remapTextureWidthLog2 = 4;
	remapTextureHeightLog2 = 4;
	remapTextureDepthLog2 = 4;
	
        colorMatrixHue = 0.0f;
        colorMatrixSaturation = 1.0f;
        colorMatrixBrightness = 1.0f;
        
	return self;
}

- (id)objectValueForTableColumn:(NSTableColumn *)tableColumn
{
    return name;
}

- (void)setOpenGLContext:(NSOpenGLContext *)ctx;
{
	context = ctx;
}

- (void)updateLayerInfo
{
}

/*
**  Minimum of four doubles.
*/
static inline double fmin4(double a, double b, double c, double d)
{
    if(a > b)   a = b;
    if(a > c)   a = c;
    if(a > d)   a = d;
    return a;
}


/*
**  Maximum of four doubles.
*/
static inline double fmax4(double a, double b, double c, double d)
{
    if(a < b)   a = b;
    if(a < c)   a = c;
    if(a < d)   a = d;
    return a;
}

- (void)computePerspectiveMatrix
{
	float dx1, dx2, Ex;
	float dy1, dy2, Ey;

        // Set up matrix terms for doing projective mapping from a unit square to the
        // output coordinates.  We'll figure out how to prenormalize the matrix later
        // on once we have this working.  Until then we'll normalize the inputs manually
        // before running the inputs through the matrix.    	
        
        tx[0] = tx[3] = 0;
        tx[1] = tx[2] = 1;

        ty[0] = ty[1] = 0;
        ty[2] = ty[3] = 1;
        
	dx1 = sx[1] - sx[2];
	dx2 = sx[3] - sx[2];
	Ex = sx[0] - sx[1] + sx[2] - sx[3];
	
	dy1 = sy[1] - sy[2];
	dy2 = sy[3] - sy[2];
	Ey = sy[0] - sy[1] + sy[2] - sy[3];

#define det4(a,b,c,d) ((a) * (d) - (b) * (c))
	matrix.g = det4(Ex,dx2,Ey,dy2) / det4(dx1,dx2,dy1,dy2);
	matrix.h = det4(dx1,Ex,dy1,Ey) / det4(dx1,dx2,dy1,dy2);

	matrix.a = sx[1] - sx[0] + matrix.g * sx[1];
	matrix.b = sx[3] - sx[0] + matrix.h * sx[3];
	matrix.c = sx[0];
	matrix.d = sy[1] - sy[0] + matrix.g * sy[1];
	matrix.e = sy[3] - sy[0] + matrix.h * sy[3];
	matrix.f = sy[0];
        
        // May as well cache screen coordinates too
	xPos[0] = matrix.a*tx[0] + matrix.b*ty[0] + matrix.c;
	yPos[0] = matrix.d*tx[0] + matrix.e*ty[0] + matrix.f;
	wPos[0] = matrix.g*tx[0] + matrix.h*ty[0] + 1.0f;

	xPos[1] = matrix.a*tx[1] + matrix.b*ty[1] + matrix.c;
	yPos[1] = matrix.d*tx[1] + matrix.e*ty[1] + matrix.f;
	wPos[1] = matrix.g*tx[1] + matrix.h*ty[1] + 1.0f;

	xPos[2] = matrix.a*tx[2] + matrix.b*ty[2] + matrix.c;
	yPos[2] = matrix.d*tx[2] + matrix.e*ty[2] + matrix.f;
	wPos[2] = matrix.g*tx[2] + matrix.h*ty[2] + 1.0f;

	xPos[3] = matrix.a*tx[3] + matrix.b*ty[3] + matrix.c;
	yPos[3] = matrix.d*tx[3] + matrix.e*ty[3] + matrix.f;
	wPos[3] = matrix.g*tx[3] + matrix.h*ty[3] + 1.0f;
        
        minX = fmin4(sx[0], sx[1], sx[2], sx[3]);
        minY = fmin4(sy[0], sy[1], sy[2], sy[3]);
        maxX = fmax4(sx[0], sx[1], sx[2], sx[3]);
        maxY = fmax4(sy[0], sy[1], sy[2], sy[3]);
        
        layerRect.origin.x = minX;
        layerRect.origin.y = minY;
        layerRect.size.width  = maxX-minX;
        layerRect.size.height = maxY-minY;
}

- (void)setRect:(NSRect)rect
{
        sx[3] = sx[0] = rect.origin.x;
        sy[0] = sy[1] = rect.origin.y;
        sx[1] = sx[2] = rect.origin.x + rect.size.width;
        sy[2] = sy[3] = rect.origin.y + rect.size.height;
        [self computePerspectiveMatrix];
        
	[self layerUpdated];
}

- (void)setPosition:(NSPoint)point
{
        NSPoint oldOrigin = layerRect.origin;
        float dx, dy;
        
        dx = point.x - oldOrigin.x;
        dy = point.y - oldOrigin.y;
        
	layerRect.origin = point;
        
        sx[0] += dx;
        sx[1] += dx;
        sx[2] += dx;
        sx[3] += dx;
        
        sy[0] += dy;
        sy[1] += dy;
        sy[2] += dy;
        sy[3] += dy;
        
        [self computePerspectiveMatrix];
        
	[self layerUpdated];
}

- (void)setPosition:(NSPoint)point atIndex:(unsigned)index
{
        sx[index] = point.x;
        sy[index] = point.y;

        [self computePerspectiveMatrix];        
	[self layerUpdated];
}

- (void)setSize:(NSSize)size
{
	layerRect.size = size;
        
        sx[1] = sx[2] = sx[0] + size.width;
        sy[2] = sy[3] = sy[0] + size.height;

        [self computePerspectiveMatrix];
        
	[self layerUpdated];
}

- (NSPoint)position
{
	return layerRect.origin;
}

- (NSSize)size
{
	return layerRect.size;
}

- (NSRect)rect
{
	return layerRect;
}

- (int)pointIndexForPoint:(NSPoint)point
{
    float dist, bestDist = 100000000.0f;
    float dx, dy;
    
    int i, index = -1;
    for(i = 0; i < 4; i++)
    {
        dx = point.x - sx[i];
        dy = point.y - sy[i];
        
        dist = sqrt(dx*dx+dy*dy);
        if(dist < bestDist)
        {
            bestDist = dist;
            index = i;
        }
    }
    return index;
}

- (void)layerUpdated
{
	if(enableUpdates)
		[[NSNotificationCenter defaultCenter] postNotificationName:@"LayerUpdated" object:self];
}

- (void)drawLayer
{	
	float lclminX, lclminY, lclmaxX, lclmaxY;

	lclminX = NSMinX(layerRect);
	lclminY = NSMinY(layerRect);
	lclmaxX = NSMaxX(layerRect);
	lclmaxY = NSMaxY(layerRect);
	
	if(!colorTextureName || (hasAlpha && !alphaTextureName))
		[self createTextures];
	
	if(clearEnable)
	{
		if(clearPreMult)
			glClearColor(clearColor[0]*clearColor[3], 
				     clearColor[1]*clearColor[3], 
				     clearColor[2]*clearColor[3], 
				     clearColor[3]);
		else
			glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
		glColorMask(clearMask[0],  clearMask[1], clearMask[2], clearMask[3]);
			
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	if(polyEnable)
	{
		if(polyBlendEnable)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		
		glBlendFunc(polySrcBlend, polyDstBlend);
		glColorMask(polyMask[0],  polyMask[1], polyMask[2], polyMask[3]);
		
		if(polyPreMult)			
			glColor4f(polyColor[0]*polyColor[3], 
				  polyColor[1]*polyColor[3],
				  polyColor[2]*polyColor[3],
				  polyColor[3]);
		else
			glColor4f(polyColor[0], 
				  polyColor[1],
				  polyColor[2],
				  polyColor[3]);

		glPushMatrix();
		glTranslatef(minX+maxX/2,minY+maxY/2,0.0f);
		
		// We should move this into another method that can be overridden
		// by different kinds of layers.
		glBegin(GL_QUADS);
		glVertex2f(-(maxX-minX)/2, 0.0f);
		glVertex2f(0.0f, -(maxY-minY)/2);
		glVertex2f((maxX-minX)/2, 0.0f);
		glVertex2f(0.0f,(maxY-minY)/2);
		glEnd();
		angle += 2.0f;
		glPopMatrix();
	}

	if(layerEnable)
	{
                GLenum alphaTextureUnit;
                
		// Always modulate texture by primary vertex color.
                glActiveTexture(GL_TEXTURE0_ARB);

		// This code is where the magic happens for doing color adjustments or
		// background removal.  You can think of using a 3D dependent texture
		// read for computing r',g',b',a' = f(r,g,b).
                if(colorMatrixEnable || backgroundRemoveEnable)
                {
                        if(remapDataDirty)
                                [self generateRemapTexture];
                                
                        glActiveTextureARB(GL_TEXTURE2_ARB);		
                        alphaTextureUnit = GL_TEXTURE2_ARB;
                        if(alphaTextureData && alphaTextureName)
                        {		
                                glBindTexture(GL_TEXTURE_RECTANGLE_EXT, alphaTextureName);
                                glEnable(GL_TEXTURE_RECTANGLE_EXT);
                                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                                glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_RECTANGLE_EXT);
                                glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE1_ARB);
                        }
                        
                        glActiveTextureARB(GL_TEXTURE1_ARB);
                        glBindTexture(GL_TEXTURE_3D, remapTextureName);
                        glEnable(GL_TEXTURE_3D);
                        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DEPENDENT_RGB_TEXTURE_3D_NV);
                        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB);
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                        
                        glActiveTextureARB(GL_TEXTURE0_ARB);
                        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_RECTANGLE_EXT);
                        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0_ARB);
                        
                        glEnable(GL_TEXTURE_SHADER_NV);
                }
                else
                {
                        glDisable(GL_TEXTURE_SHADER_NV);
                        
                        glActiveTextureARB(GL_TEXTURE1_ARB);
                        alphaTextureUnit = GL_TEXTURE1_ARB;
                        if(alphaTextureData && alphaTextureName)
                        {		
                                glBindTexture(GL_TEXTURE_RECTANGLE_EXT, alphaTextureName);
                                glEnable(GL_TEXTURE_RECTANGLE_EXT);
                                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                        }
                        glActiveTextureARB(GL_TEXTURE0_ARB);
                }
                
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		
		if(layerBlendEnable)
		{
			glEnable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_ALWAYS, 0.0f);
		}
		else
			glDisable(GL_BLEND);
                
		if(colorTextureData && colorTextureName)
		{
			glBlendFunc(layerSrcBlend, layerDstBlend);
			glColorMask(layerMask[0], layerMask[1], layerMask[2], layerMask[3]);
			glBindTexture(GL_TEXTURE_RECTANGLE_EXT, colorTextureName);
                        
			if(layerPreMult)			
				glColor4f(layerColor[0]*layerColor[3], 
					  layerColor[1]*layerColor[3],
					  layerColor[2]*layerColor[3],
					  layerColor[3]);
			else
				glColor4f(layerColor[0], 
					  layerColor[1],
					  layerColor[2],
					  layerColor[3]);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
                        glMultiTexCoord2f(alphaTextureUnit, 0.0f, 0.0f);
			glVertex4f(xPos[0], yPos[0], 0.0f, wPos[0]);
			glTexCoord2f(colorTextureWidth, 0.0f);
                        glMultiTexCoord2f(alphaTextureUnit, alphaTextureWidth, 0.0f);
			glVertex4f(xPos[1], yPos[1], 0.0f, wPos[1]);
			glTexCoord2f(colorTextureWidth, colorTextureHeight);
                        glMultiTexCoord2f(alphaTextureUnit, alphaTextureWidth, alphaTextureHeight);
			glVertex4f(xPos[2], yPos[2], 0.0f, wPos[2]);
			glTexCoord2f(0.0f, colorTextureHeight);
                        glMultiTexCoord2f(alphaTextureUnit, 0.0f, alphaTextureHeight);
			glVertex4f(xPos[3], yPos[3], 0.0f, wPos[3]);
			glEnd();
		}
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_RECTANGLE_EXT);
	}
        
        // Disable all texturing and shaders.
        glDisable(GL_TEXTURE_SHADER_NV);
        glActiveTextureARB(GL_TEXTURE2_ARB);
        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        glDisable(GL_TEXTURE_3D);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
        
}

- (void)setColorFormat:(OSType)format colorWidth:(unsigned)width colorHeight:(unsigned)height colorData:(unsigned char *)data colorRowBytes:(unsigned)rowBytes;
{	
	switch(format)
	{
		case 'raw ':
			colorTextureFormat = GL_BGRA;
			colorTextureType = GL_UNSIGNED_INT_8_8_8_8_REV;
			colorTextureBytesPerPixel = 4;
			colorTextureInternalFormat = GL_RGBA8;
			break;
			
		case '2vuy':
			colorTextureFormat = GL_YCBCR_422_APPLE;
			colorTextureType = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
			colorTextureBytesPerPixel = 2;
			colorTextureInternalFormat = GL_RGB8;
			break;

		case 'yuvs':
			colorTextureFormat = GL_YCBCR_422_APPLE;
			colorTextureType = GL_UNSIGNED_SHORT_8_8_APPLE;
			colorTextureBytesPerPixel = 2;
			colorTextureInternalFormat = GL_RGB8;
			break;
	}
	colorTextureWidth = width;
	colorTextureHeight = height;
	colorTextureData = data;
	colorTextureBytesPerRow = rowBytes;
	colorTextureRowLength = rowBytes / colorTextureBytesPerPixel;
	
	[context makeCurrentContext];
	[self createTextures];
}

- (void)lockBits
{
	[context makeCurrentContext];
	[self dirtyColorTexture];
}

- (void)unlockBits
{
	[self layerUpdated];
}

- (void)createTextures
{
	if(colorTextureData)
	{
		float priority = 0.0f;
		if(colorTextureName)
			glDeleteTextures(1, &colorTextureName);
		colorTextureName = 0;	
		glGenTextures(1, &colorTextureName);
		
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, colorTextureRowLength);

		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, colorTextureName);
		glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, colorTextureInternalFormat,
			colorTextureWidth, colorTextureHeight, 0,
			colorTextureFormat, colorTextureType, colorTextureData);
                
		glPrioritizeTextures(1, &colorTextureName, &priority);		 
                glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);					 
	}
			
	if(alphaTextureData)
	{
		if(alphaTextureName)
			glDeleteTextures(1, &alphaTextureName);
		alphaTextureName = 0;
		glGenTextures(1, &alphaTextureName);
		
		glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, alphaTextureBytesPerRow);
		
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, alphaTextureName);
		glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_INTENSITY,
			alphaTextureWidth, alphaTextureHeight, 0,
			GL_LUMINANCE, GL_UNSIGNED_BYTE, alphaTextureData);
			
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);					 			
	}
}

- (void)dirtyColorTexture
{
	if(colorTextureName)
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, colorTextureRowLength);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, colorTextureName);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0,
			colorTextureWidth, colorTextureHeight,
			colorTextureFormat, colorTextureType, colorTextureData);			
	}
}

- (void)dirtyAlphaTexture
{
	if(alphaTextureName)
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, alphaTextureBytesPerRow);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, alphaTextureName);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0,
			alphaTextureWidth, alphaTextureHeight,
			GL_LUMINANCE, GL_UNSIGNED_BYTE, alphaTextureData);
	}
}

- (void)updateColorMatrix
{
	identmat3(colorMatrix);  
	huerotatemat3(colorMatrix, colorMatrixHue);
	saturatemat3(colorMatrix, colorMatrixSaturation);
	cscalemat3(colorMatrix, colorMatrixBrightness, colorMatrixBrightness, colorMatrixBrightness);
}

static inline void RGB_YCC(float r,float g,float b, float *y,float *cr,float *cb)
{
	float     u0,u1,u2;
	float     v0,v1,v2;
	float     w0,w1,w2;
    
	u0  =   77*r;        u1  =  131*r;        u2  =  -44*r;
	v0  =  150*g;        v1  = -110*g;        v2  =  -87*g;
	w0  =   29*b;        w1  =  -21*b;        w2  =  131*b;
    
	*y  = (u0 + v0 + w0) * (1.0/256.0);
	*cr = (u1 + v1 + w1) * (1.0/256.0);
	*cb = (u2 + v2 + w2) * (1.0/256.0);
}

static inline void matteRemove(float *r,float *g,float *b,float *a,
    float cosP,float sinP,float tanA,float sat, float kr,float kg,float kb)
{
	float  rv,gv,bv,av;
	float  x,y,z, k;
	float  cr,cb;
    
	rv = *r;
	gv = *g;
	bv = *b;
	av = 1;
    
	RGB_YCC(rv,gv,bv, &y,&cr,&cb);
    
	if(y > 0.05  &&  y < 0.90)
	{
	    z  = cr * cosP  -  cb * sinP;
	    x  = cr * sinP  +  cb * cosP;
    
	    if(z < 0)
		z = -z;
    
	    k  = x  -  z*tanA;
	    if(k > 0)
	    {
		k  = k*sat;
		if(k > 1)  k = 1;
    
		rv = rv - k*kr;
		gv = gv - k*kg;
		bv = bv - k*kb;
		av = 1-k;
    
		if(rv<0)  rv = 0;
		if(gv<0)  gv = 0;
		if(bv<0)  bv = 0;
		if(av<0)  av = 0;
	    }
	}
    
	*r = rv;
	*g = gv;
	*b = bv;
	*a = av;
}

- (void)generateRemapTexture
{
	float r, g, b;
	float r1, g1, b1;
	GLuint x, y, z;
	unsigned char *dst;
	float redScale, blueScale, greenScale;
	
        float     ssin,scos,stan,ssat;
        float     sinP,cosP,tanA,sat, phi;
        float     ky,kcr,kcb, scy,scr,scb;
		float     a,f,c;
        float	  ka0, ka1;
        float	  acc = 2;

        float	sa0  = 40;
        float	sa1  = 20;
        
        ka0 = (M_PI / 180.0) * backgroundTolerance * acc;
        ka1 = (M_PI / 180.0) * backgroundTolerance;
    
        RGB_YCC(backgroundColor[0],backgroundColor[1],backgroundColor[2], &ky,&kcr,&kcb);
        phi    = atan2(kcr, kcb);
        cosP   = cos(phi);
        sinP   = sin(phi);
        tanA   = 1.0 / tan(0.5 * ka0);
        sat    = (ka0 / (ka0-ka1)) / sqrt(kcr*kcr + kcb*kcb);

        RGB_YCC(colorCorrectionSource[0],colorCorrectionSource[1],colorCorrectionSource[2], &scy,&scr,&scb);
        phi    = atan2(scr, scb);
        scos   = cos(phi);
        ssin   = sin(phi);
        stan   = 1.0 / tan(0.5 * sa0);
        ssat   = (sa0 / (sa0-sa1)) / sqrt(scr*scr + scb*scb);
    
	if(remapTextureData)
	{
		free(remapTextureData);
		remapTextureData = NULL;
	}
        
        [self updateColorMatrix];
        
	remapTextureWidth = (1L << remapTextureWidthLog2);
	remapTextureHeight = (1L << remapTextureHeightLog2);
	remapTextureDepth = (1L << remapTextureDepthLog2);

	remapTextureData = malloc(remapTextureWidth * remapTextureHeight * remapTextureDepth * 4 * sizeof(unsigned char));
	
	// Any dimension should be at least 2, otherwise the math falls apart.
	redScale = 1.0f / (remapTextureWidth - 1);
	greenScale = 1.0f / (remapTextureHeight - 1);
	blueScale = 1.0f / (remapTextureDepth - 1);
	
	for(z = 0; z < remapTextureDepth; z++)
	{
		for(y = 0; y < remapTextureHeight; y++)
		{
			dst = remapTextureData + (z*remapTextureDepth + y)*remapTextureHeight * 4 * sizeof(unsigned char);
			
			for(x = 0; x < remapTextureWidth; x++)
			{

				r = x * redScale;
				g = y * greenScale;
				b = z * blueScale;
                                matteRemove(&r,&g,&b,&f, scos,ssin,stan,ssat, 
                                    colorCorrectionSource[0],
                                    colorCorrectionSource[1],
                                    colorCorrectionSource[2]);
                        
				r = x * redScale;
				g = y * greenScale;
				b = z * blueScale;
                                a = 1.0f;
                                
                                if(backgroundRemoveEnable)
                                {
                                    matteRemove(&r,&g,&b,&a, cosP,sinP,tanA,sat,
                                        backgroundColor[0],
                                        backgroundColor[1],
                                        backgroundColor[2]);
                                }
                
                                if(colorCorrectionEnable)
                                    f = 1 - (1-f)*(colorCorrection/100.0);
                                else
                                    f = 1.0f;
                                    
                                c = 0.7*f + 0.3;
                
                                r = c*(r*f + colorCorrectionDest[0]*(1-f)) + 0.0;
                                g = c*(g*f + colorCorrectionDest[1]*(1-f)) + 0.0;
                                b = c*(b*f + colorCorrectionDest[2]*(1-f)) + 0.0;

                                if(colorMatrixEnable)
                                    xformpnt3(colorMatrix,r,g,b,&r1,&g1,&b1);
                                else
                                {
                                    r1 = r;
                                    g1 = g;
                                    b1 = b;
                                }
                                
				// Clamp values.
				if(r1 < 0.0f) r1 = 0.0f;
				else if(r1 > 1.0f) r1 = 1.0f;
				if(g1 < 0.0f) g1 = 0.0f;
				else if(g1 > 1.0f) g1 = 1.0f;
				if(b1 < 0.0f) b1 = 0.0f;
				else if(b1 > 1.0f) b1 = 1.0f;
				
				dst[0] = r1*255.95;
				dst[1] = g1*255.95;
				dst[2] = b1*255.95;
				dst[3] = 255.95*c*(a) + 0.0;
				dst += 4;				
			}
		}
	}
	if(!remapTextureName)
		glGenTextures(1, &remapTextureName);
		
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	
	glBindTexture(GL_TEXTURE_3D, remapTextureName);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, remapTextureWidth, remapTextureHeight, remapTextureDepth, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, remapTextureData);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);			
        
        remapDataDirty = NO;
}

- (float) alphaForPoint: (NSPoint) point
{    
	if (!NSPointInRect(point, layerRect)) return 0.0;   // nothing there.
	
	return 1.0;    
}

- (BOOL)hasBackingStore
{
	return colorTextureName ? YES : NO;
}

- (void) setFade: (float) _fade
{	
	fade = _fade;
	[self updateLayerInfo];
}

- (float) fade;
{
	return fade;
}

- (BOOL)layerNeedsHeartbeat
{
	return NO;
}

- (void)heartBeat
{
}

- (void)enableUpdates:(BOOL)flag
{
	enableUpdates = flag;
}

- (void)centerInRect: (NSRect)frame
{
    double   dx,dy;

    dx = 0.5*(NSWidth(frame)  - NSWidth(layerRect));
    dy = 0.5*(NSHeight(frame) - NSHeight(layerRect));

    [self setPosition: NSMakePoint(NSMinX(frame)+dx, NSMinY(frame)+dy)];
    [self layerUpdated];
}

- (double)aspectRatio
{
    return (double)colorTextureWidth / (double)colorTextureHeight;
}

- (void)setGarbageMatte: (NSBitmapImageRep *)m
{
        int   w,h;
    
        w = [m pixelsWide];
        h = [m pixelsHigh];
    
        if([m bitsPerPixel] != 8  ||  [m samplesPerPixel] != 1)   return;
    
        [m retain];
        [matte release];
        matte = m;

	alphaTextureData        = [matte bitmapData];
	alphaTextureBytesPerRow = [matte bytesPerRow];
	alphaTextureWidth       = [matte pixelsWide];
	alphaTextureHeight      = [matte pixelsHigh];
        hasAlpha = YES;
}

- (NSBitmapImageRep *)garbageMatte
{
    return matte;
}

- (void)setClearEnable:(BOOL)flag
{
	clearEnable = flag;
	[self layerUpdated];
}

- (void)setClearMaskRed:(BOOL)red green:(BOOL)green blue:(BOOL)blue alpha:(BOOL)alpha
{
	clearMask[0] = red;
	clearMask[1] = green;
	clearMask[2] = blue;
	clearMask[3] = alpha;
	[self layerUpdated];
}

- (void)setClearPreMult:(BOOL)flag
{
	clearPreMult = flag;
	[self layerUpdated];
}

- (void)setClearColorRed:(GLfloat)red green:(GLfloat)green blue:(GLfloat)blue alpha:(GLfloat)alpha
{
	clearColor[0] = red;
	clearColor[1] = red;
	clearColor[2] = red;
	clearColor[3] = red;
	[self layerUpdated];
}

- (BOOL)isClearEnabled
{
	return clearEnable;
}

- (void)getClearMaskRed:(BOOL *)red green:(BOOL *)green blue:(BOOL *)blue alpha:(BOOL *)alpha
{
	*red   = clearMask[0];
	*green = clearMask[1];
	*blue  = clearMask[2];
	*alpha = clearMask[3];
}

- (BOOL)isClearPreMult
{
	return clearPreMult;
}

- (void)getClearColorRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha
{
	*red   = clearColor[0];
	*green = clearColor[0];
	*blue  = clearColor[0];
	*alpha = clearColor[0];
}

- (void)setPolyEnable:(BOOL)flag
{
	polyEnable = flag;
	[self layerUpdated];
}

- (void)setPolyBlendEnable:(BOOL)flag
{
	polyBlendEnable = flag;
	[self layerUpdated];
}

- (void)setPolyMaskRed:(BOOL)red green:(BOOL)green blue:(BOOL)blue alpha:(BOOL)alpha
{
	polyMask[0] = red;
	polyMask[1] = green;
	polyMask[2] = blue;
	polyMask[3] = alpha;
	[self layerUpdated];
}

- (void)setPolyPreMult:(BOOL)flag
{
	polyPreMult = flag;
	[self layerUpdated];
}

- (void)setPolyColorRed:(GLfloat)red green:(GLfloat)green blue:(GLfloat)blue alpha:(GLfloat)alpha
{
	polyColor[0] = red;
	polyColor[1] = green;
	polyColor[2] = blue;
	polyColor[3] = alpha;
	[self layerUpdated];
}

- (void)setPolySrcBlend:(GLenum)srcBlend
{
	polySrcBlend = srcBlend;
	[self layerUpdated];
}

- (void)setPolyDstBlend:(GLenum)dstBlend
{
	polyDstBlend = dstBlend;
	[self layerUpdated];
}

- (BOOL)isPolyEnabled
{
	return polyEnable;
}

- (BOOL)isPolyBlendEnabled
{
	return polyBlendEnable;
}

- (void)getPolyMaskRed:(BOOL *)red green:(BOOL *)green blue:(BOOL *)blue alpha:(BOOL *)alpha
{
	*red   = polyMask[0];
	*green = polyMask[1];
	*blue  = polyMask[2];
	*alpha = polyMask[3];
}

- (BOOL)isPolyPreMult
{
	return polyPreMult;
}

- (void)getPolyColorRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha
{
	*red   = polyColor[0];
	*green = polyColor[1];
	*blue  = polyColor[2];
	*alpha = polyColor[3];
}

- (GLenum)polySrcBlend
{
	return polySrcBlend;
}

- (GLenum)polyDstBlend
{
	return polyDstBlend;
}

// Layer
- (void)setLayerEnable:(BOOL)flag
{
	layerEnable = flag;
	[self layerUpdated];
}

- (void)setLayerBlendEnable:(BOOL)flag
{
	layerBlendEnable = flag;
	[self layerUpdated];
}

- (void)setLayerMaskRed:(BOOL)red green:(BOOL)green blue:(BOOL)blue alpha:(BOOL)alpha
{
	layerMask[0] = red;
	layerMask[1] = green;
	layerMask[2] = blue;
	layerMask[3] = alpha;
	[self layerUpdated];
}

- (void)setLayerPreMult:(BOOL)flag
{
	layerPreMult = flag;
	[self layerUpdated];
}

- (void)setLayerColorRed:(GLfloat)red green:(GLfloat)green blue:(GLfloat)blue alpha:(GLfloat)alpha
{
	layerColor[0] = red;
	layerColor[1] = green;
	layerColor[2] = blue;
	layerColor[3] = alpha;
	[self layerUpdated];
}

- (void)setLayerSrcBlend:(GLenum)srcBlend
{
	layerSrcBlend = srcBlend;
	[self layerUpdated];
}

- (void)setLayerDstBlend:(GLenum)dstBlend
{
	layerDstBlend = dstBlend;
	[self layerUpdated];
}

- (BOOL)isLayerEnabled
{
	return layerEnable;
}

- (BOOL)isLayerBlendEnabled
{
	return layerBlendEnable;
}

- (void)getLayerMaskRed:(BOOL *)red green:(BOOL *)green blue:(BOOL *)blue alpha:(BOOL *)alpha
{
	*red   = layerMask[0];
	*green = layerMask[1];
	*blue  = layerMask[2];
	*alpha = layerMask[3];
}

- (BOOL)isLayerPreMult
{
	return layerPreMult;
}

- (void)getLayerColorRed:(float *)red green:(float *)green blue:(float *)blue alpha:(float *)alpha
{
	*red   = layerColor[0];
	*green = layerColor[1];
	*blue  = layerColor[2];
	*alpha = layerColor[3];
}

- (GLenum)layerSrcBlend
{
	return layerSrcBlend;
}

- (GLenum)layerDstBlend
{
	return layerDstBlend;
}

// Effect stuff
- (void)setBackgroundRemovalEnable:(BOOL)flag
{
	backgroundRemoveEnable = flag;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (BOOL)isBackgroundRemovalEnabled
{
	return backgroundRemoveEnable;
}

- (void)setBackgroundColorRed:(float)red green:(float)green blue:(float)blue
{
	backgroundColor[0] = red;
	backgroundColor[1] = green;
	backgroundColor[2] = blue;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (void)getBackgroundColorRed:(float *)red green:(float *)green blue:(float *)blue
{
	*red = backgroundColor[0];
	*green = backgroundColor[1];
	*blue = backgroundColor[2];
	
}

- (void)setBackgroundTolerance:(float)tolerance
{
	backgroundTolerance = tolerance;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (float)backgroundTolerance
{
	return backgroundTolerance;
}

- (void)setColorCorrectionEnable:(BOOL)flag
{
        colorCorrectionEnable = flag;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (BOOL)isColorCorrectionEnabled
{
        return colorCorrectionEnable;
}

- (void)setColorCorrection:(float)correction
{
        colorCorrection = correction;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (float)colorCorrection
{
        return colorCorrection;
}

- (void)setColorCorrectionSourceRed:(float)red green:(float)green blue:(float)blue
{
        colorCorrectionSource[0] = red;
        colorCorrectionSource[1] = green;
        colorCorrectionSource[2] = blue;
        remapDataDirty = YES;
	[self layerUpdated];    
}

- (void)getColorCorrectionSourceRed:(float *)red green:(float *)green blue:(float *)blue
{
        *red = colorCorrectionSource[0];
        *green = colorCorrectionSource[1];
        *blue = colorCorrectionSource[2];
}

- (void)setColorCorrectionDestRed:(float)red green:(float)green blue:(float)blue
{
        colorCorrectionDest[0] = red;
        colorCorrectionDest[1] = green;
        colorCorrectionDest[2] = blue;
        remapDataDirty = YES;
	[self layerUpdated];    
}

- (void)getColorCorrectionDestRed:(float *)red green:(float *)green blue:(float *)blue
{
        *red = colorCorrectionDest[0];
        *green = colorCorrectionDest[1];
        *blue = colorCorrectionDest[2];
}

- (void)setColorMatrixEnable:(BOOL)flag
{
	colorMatrixEnable = flag;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (BOOL)isColorMatrixEnabled
{
	return colorMatrixEnable;
}

- (void)setColorMatrixHue:(float)hue
{
	colorMatrixHue = hue;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (void)setColorMatrixSaturation:(float)saturation
{
	colorMatrixSaturation = saturation;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (void)setColorMatrixBrightness:(float)brightness
{
	colorMatrixBrightness = brightness;
        remapDataDirty = YES;
	[self layerUpdated];
}

- (float)colorMatrixHue
{
	return colorMatrixHue;
}

- (float)colorMatrixSaturation
{
	return colorMatrixSaturation;
}

- (float)colorMatrixBrightness
{
	return colorMatrixBrightness;
}

@end
