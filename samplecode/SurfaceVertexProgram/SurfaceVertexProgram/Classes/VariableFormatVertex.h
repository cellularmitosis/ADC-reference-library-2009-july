/*
 *  VariableFormatVertex.h
 *
 *  Created by Michael Larson on Tue Mar 11 2003.
 *  Copyright (c) 2003 Apple Computer. All rights reserved.
 *
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
   ("Apple") in consideration of your agreement to the following terms, and your
   use, installation, modification or redistribution of this Apple software
   constitutes acceptance of these terms.  If you do not agree with these terms,
   please do not use, install, modify or redistribute this Apple software.

   In consideration of your agreement to abide by the following terms, and subject
   to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#import <Foundation/Foundation.h>

#define kAttributeTexture(a)		(1 << a)
#define kAttributeTexture0		(1 << 0)
#define kAttributeTexture1		(1 << 1)
#define kAttributeTexture2		(1 << 2)
#define kAttributeTexture3		(1 << 3)
#define kAttributeTexture4		(1 << 4)
#define kAttributeTexture5		(1 << 5)
#define kAttributeTexture6		(1 << 6)
#define kAttributeTexture7		(1 << 7)
#define kAttributeNormal		(1 << 8)
#define kAttributeColor			(1 << 9)
#define kAttributeSecondaryColor	(1 << 10)
#define kAttributeFog			(1 << 11)

enum {
    kglTexture0	= 0,
    kglTexture1,
    kglTexture2,
    kglTexture3,
    kglTexture4,
    kglTexture5,
    kglTexture6,
    kglTexture7,
    kglNormals,
    kglColor,
    kglSecondaryColor,
    kglFog,
    kUnknownVertexAttributeTag
};


@interface VariableFormatVertex : NSObject {
    GLint		m_vertexCount;
    unsigned int	m_vertexAttributes;

    int			m_stride;
    GLfloat		*m_vertices;

    GLint		m_isMesh;
    GLint		m_meshCols, m_meshRows;

    GLint		m_attributeOffsets[12];
    
    GLboolean		m_useVAR;
    GLboolean		m_useCVA;
    GLboolean		m_useVAO;
    GLuint		m_VAO;

@private
    GLint		m_useDoubles;
    GLint		m_use8BitComponents;
}

- (void) setFloatSize: (int) size;
- (void) setComponentSize: (int) size;

// creation
- (VariableFormatVertex *) init;
- (VariableFormatVertex *) initWithVertexType: (unsigned int)type vertexArrayOfCount: (int)size;
- (void) dealloc;

// VAR support
- (GLboolean) useVAR;
- (void) attachToVertexArrayRange;
- (void) detachFromVertexArrayRange;
- (void) flushVertexArrayRange;

// CVA support
- (GLboolean) useCVA;
- (void) setCVA: (GLboolean) flag;

// VAO support
- (GLboolean) useVAO;
- (void) setVAO: (GLboolean) flag toVAO: (GLuint) vao;
- (GLuint) VAO;
- (void) BindVAO;

// vertex copying
- (void)atVertex: (int) index copyVertex: (GLfloat *) pSrcFloats;

// Vertex count and atributes
- (GLint) vertexCount;
- (unsigned int) vertexAttributes;
- (GLint) stride;
- (GLint) byteStride;

// Vertex indexing
- (GLfloat *) atVertex: (int)index;

// mesh support
- (void) setMeshRows: (int) rows Cols: (int) cols;
- (GLboolean) isMesh;
- (GLint) rows;
- (GLint) cols;
- (GLfloat *) atRow: (int) row atCol: (int)col;
- (void)atRow: (int) row atCol: (int) col copyVertex: (GLfloat *) pSrcFloats;

// attribute usage
- (GLint) usesTexture0;
- (GLint) usesTexture1;
- (GLint) usesTexture2;
- (GLint) usesTexture3;
- (GLint) usesTexture4;
- (GLint) usesTexture5;
- (GLint) usesTexture6;
- (GLint) usesTexture7;
- (GLint) usesNormal;
- (GLint) usesColor;
- (GLint) usesSecondaryColor;
- (GLint) usesFog;

// attribute indexing
- (GLfloat *) Texture0: (GLfloat *) pVertex;
- (GLfloat *) Texture1: (GLfloat *) pVertex;
- (GLfloat *) Texture2: (GLfloat *) pVertex;
- (GLfloat *) Texture3: (GLfloat *) pVertex;
- (GLfloat *) Texture4: (GLfloat *) pVertex;
- (GLfloat *) Texture5: (GLfloat *) pVertex;
- (GLfloat *) Texture6: (GLfloat *) pVertex;
- (GLfloat *) Texture7: (GLfloat *) pVertex;
- (GLfloat *) Texture: (GLint) unit atVertex: (GLfloat *) pVertex;
- (GLfloat *) Normal: (GLfloat *) pVertex;
- (GLfloat *) Color: (GLfloat *) pVertex;
- (GLfloat *) SecondaryColor: (GLfloat *) pVertex;
- (GLfloat *) Fog: (GLfloat *) pVertex;

// attribute data
- (void) atVertex: (GLint) index setPos_X: (GLfloat) x Y: (GLfloat) y  Z: (GLfloat) z;
- (void) atVertex: (GLint) index setTexture0_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture1_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture2_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture3_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture4_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture5_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture6_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setTexture7_S: (GLfloat) s T: (GLfloat) t;
- (void) atVertex: (GLint) index setNormal_X: (GLfloat) x Y: (GLfloat) y  Z: (GLfloat) z;
- (void) atVertex: (GLint) index setColor_R: (GLfloat) r G: (GLfloat) g B: (GLfloat) b;
- (void) atVertex: (GLint) index setSecondaryColor_R: (GLfloat) r G: (GLfloat) g B: (GLfloat) b;
- (void) atVertex: (GLint) index setFog_R: (GLfloat) r G: (GLfloat) g B: (GLfloat) b;

@end
