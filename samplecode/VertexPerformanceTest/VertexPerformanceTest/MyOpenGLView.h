/*
 * 
    Copyright:	Copyright © 2003 Apple Computer, Inc., All Rights Reserved

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
 *
 */


#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <OpenGL/CGLContext.h>
#import <OpenGL/CGLCurrent.h>

enum {
    kVertexCalls = 0,
    kDisplayLists = 1,
    kDrawArrays = 2,
    kCompiledVertexArray = 3,
    kVertexArrayRange = 4
};

typedef struct GLCoord3
{
    GLfloat	x;
    GLfloat	y;
    GLfloat	z;
} GLCoord3;

typedef struct GLCoord2
{
    GLfloat	x;
    GLfloat	y;
} GLCoord2;

typedef struct Vertex {
    GLCoord2	texCoord;
    GLCoord3	normal;
    GLCoord3	vertex;
} Vertex;   

typedef struct AuxVertex {
    GLCoord3	color;
    GLCoord3	secondaryColor;
    GLfloat	fogCoord;
    GLfloat	pad;
}AuxVertex;

#define kTextureUnitCount 8

@interface MyOpenGLView : NSOpenGLView
{
    float	x, y, z;
    float	performance;
    float   	matrix[16];
    int		detail;
    int     	size;
    int		drawMethod;
    GLuint	listID;
    GLuint	textureID[8];
    
    Vertex	*verticies;
    AuxVertex	*verticies_aux;
    GLuint	*elementsIndex;
    GLuint	datasize;
    
    GLboolean	textureEnabled[kTextureUnitCount];
    GLboolean	normalsEnabled;
    GLboolean	rotationEnabled;
    GLboolean   macrosEnabled;
    GLboolean   matrixEnabled;
    GLboolean	drawWithLists;
    GLboolean	colorEnabled;
    GLboolean	secColorEnabled;
    GLboolean	fogFactorEnabled;
    
    BOOL	m_glContextInitialized;
    BOOL	displayListDirty;
    //BOOL	hasVertexArrayRange;
}


- (float) X;
- (float) Y;
- (float) Z;
- (int) detail;
- (float) performance;
- (int) modelTriangleCount;
- (int) modelVertexCount;
- (int) textureUnitCount;
- (void) rebuildDisplayList;

- (void) setX:(float) newx;
- (void) setY:(float) newy;
- (void) setZ:(float) newz;
- (void) setRotate:(int) value;
- (void) setDetail:(int) detail;
- (void) setMacros:(int) value;
- (void) setMatrix:(int) value;
- (void) setColor:(int) value;
- (void) setSecColor:(int) value;
- (void) setSize:(int) size;
- (void) setDrawMethod:(int) method;
- (void) setTexturing:(int) value  unit:(int)unit;
- (void) setTexturing:(int*) value;
- (void) setNormals:(int) value;
- (void) setFogFactor:(int) value;
- (void) spinModel;

- (void) initGL;

- (void) drawImmediateModeRep;
- (void) drawImmediateMode:(int) reps;
- (void) drawDisplayList:(int) reps;
- (void) drawArrays: (BOOL) useElements :(int) reps;

@end
