/*
 *  VariableFormatVertex.m
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


#import "VariableFormatVertex.h"


@implementation VariableFormatVertex

- (VariableFormatVertex *) init
{
    self = [super init];
    
    m_vertexAttributes	= 0;
    
    m_stride		= 0;
    m_vertexCount	= 0;
    m_vertices 		= NULL;
    m_isMesh 		= 0;
    m_meshRows 		= 0;
    m_meshCols 		= 0;
    m_useVAR		= 0;
    m_useCVA		= 0;
    m_useVAO		= 0;
    m_useDoubles	= 0;
    m_use8BitComponents		= 0;

    return self;
}

- (void) setFloatSize: (int) size
{
    if (32 == size)
    {
        m_useDoubles = 0;
    }
    else
    {
        m_useDoubles = 1;
    }
}

- (void) setComponentSize: (int) size
{
    if (8 == size)
    {
        m_use8BitComponents = 1;
    }
    else
    {
        m_use8BitComponents = 0;
    }
}

- (VariableFormatVertex *) initWithVertexType: (unsigned int)type vertexArrayOfCount: (int)size
{
    int i;
    
    // Set the number of vertices
    m_vertexCount	= size;
    m_vertexAttributes	= type;
    
    // always have xyzw
    m_stride = 4;
    
    // set texture 0-7 attributes
    for(i=0; i<8; i++)
    {        
        if (type & (1 << i))
        {
            // track the offsets
            m_attributeOffsets[i] = m_stride;
            // s,t
            m_stride += 2;
        }
    }
    
    // set normal, color, secondary color and fog attributes
    for(i=8; i<12; i++)
    {
        if (type & (1 << i))
        {
            // track the offsets
            m_attributeOffsets[i] = m_stride;

            // xyzw, rgba
            m_stride += 4;
        }
    }
    
    // alloc an array of vertices
    i = sizeof(GLfloat) * m_stride * size;
    m_vertices = (GLfloat *)malloc(i);
    
    return self;
}

- (void) dealloc
{
    if (m_vertices)
    {
        glVertexArrayRangeAPPLE(0, m_vertices);
        
        free(m_vertices);
    }
    
    [super dealloc];
}

// VAR support (not gl context safe!!!!)
- (GLboolean) useVAR
{
    return m_useVAR;
}

- (void) attachToVertexArrayRange
{   
    m_useVAR = 1;

    glEnableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
    glVertexArrayRangeAPPLE(m_vertexCount * m_stride * sizeof(GLfloat), m_vertices);
}

- (void) detachFromVertexArrayRange
{
    m_useVAR = 0;

    glVertexArrayRangeAPPLE(0, 0);
    glDisableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
}

- (void) flushVertexArrayRange
{
    if (1 == m_useVAR)
    {
        glFlushVertexArrayRangeAPPLE(m_vertexCount * m_stride * sizeof(GLfloat), m_vertices);
    }
}

// CVA support
- (GLboolean) useCVA
{
    return m_useCVA;
}

- (void) setCVA: (GLboolean) flag
{
    m_useCVA = flag;
}

// VAO support
- (GLboolean) useVAO
{
    return m_useVAO;
}

- (void) setVAO: (GLboolean) flag toVAO: (GLuint) vao
{
    m_useVAO = flag;
    m_VAO = vao;
}

- (GLuint) VAO
{
    return m_VAO;
}

- (void) BindVAO
{
    if (m_useVAO && glIsVertexArrayAPPLE(m_VAO))
    {
        glBindVertexArrayAPPLE(m_VAO);
    }
}

// vertex info routines
- (GLint) vertexCount
{
    return m_vertexCount;
}

- (unsigned int) vertexAttributes
{
    return m_vertexAttributes;
}

- (GLint) stride
{
    return m_stride;
}

- (GLint) byteStride
{
    return m_stride * sizeof(GLfloat);
}

// Vertex indexing routines
- (GLfloat *) atVertex: (int)index
{
    GLfloat *ptr;
    
    ptr = &m_vertices[index * m_stride];
    
    return ptr;
}


// Vertex copy routines
- (void) atVertex: (int) index copyVertex: (GLfloat *) pSrcFloats
{
    int		i;
    GLfloat	*pDstFloats;
    
    pDstFloats = &m_vertices[index * m_stride];
    
    switch(m_stride)
    {
        case 2:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        break;

        case 3:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        break;
        
        case 4:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        break;
        
        case 6:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        pDstFloats[4] = pSrcFloats[4];
        pDstFloats[5] = pSrcFloats[5];
        break;
        
        case 8:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        pDstFloats[4] = pSrcFloats[4];
        pDstFloats[5] = pSrcFloats[5];
        pDstFloats[6] = pSrcFloats[6];
        pDstFloats[7] = pSrcFloats[7];
        break;
        
        case 10:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        pDstFloats[4] = pSrcFloats[4];
        pDstFloats[5] = pSrcFloats[5];
        pDstFloats[6] = pSrcFloats[6];
        pDstFloats[7] = pSrcFloats[7];
        pDstFloats[8] = pSrcFloats[8];
        pDstFloats[9] = pSrcFloats[9];
        break;
        
        default:            
        for(i=0; i<m_stride; i++)
        {
            *pDstFloats++ = *pSrcFloats++;
        }
        break;
    }
}

#pragma separator

// Mesh support routines
- (void) setMeshRows: (int) rows Cols: (int) cols
{
    m_isMesh 	= true;
    m_meshRows 	= rows;
    m_meshCols 	= cols;
}

- (GLboolean) isMesh
{
    return m_isMesh;
}

- (GLint) rows
{
    return m_meshRows;
}

- (GLint) cols
{
    return m_meshCols;
}

- (GLfloat *) atRow: (int) row atCol: (int)col
{
    GLfloat *ptr;
    
    ptr = &m_vertices[(row * m_meshCols + col) * m_stride];
    
    return ptr;
}

- (void)atRow: (int) row atCol: (int) col copyVertex: (GLfloat *) pSrcFloats;
{
    int		i;
    GLfloat	*pDstFloats;
    
    pDstFloats = &m_vertices[(row * m_meshCols + col) * m_stride];
    
    switch(m_stride)
    {
        case 2:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        break;

        case 3:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        break;
        
        case 4:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        break;
        
        case 6:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        pDstFloats[4] = pSrcFloats[4];
        pDstFloats[5] = pSrcFloats[5];
        break;
        
        case 8:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        pDstFloats[4] = pSrcFloats[4];
        pDstFloats[5] = pSrcFloats[5];
        pDstFloats[6] = pSrcFloats[6];
        pDstFloats[7] = pSrcFloats[7];
        break;
        
        case 10:
        pDstFloats[0] = pSrcFloats[0];
        pDstFloats[1] = pSrcFloats[1];
        pDstFloats[2] = pSrcFloats[2];
        pDstFloats[3] = pSrcFloats[3];
        pDstFloats[4] = pSrcFloats[4];
        pDstFloats[5] = pSrcFloats[5];
        pDstFloats[6] = pSrcFloats[6];
        pDstFloats[7] = pSrcFloats[7];
        pDstFloats[8] = pSrcFloats[8];
        pDstFloats[9] = pSrcFloats[9];
        break;
        
        default:            
        for(i=0; i<m_stride; i++)
        {
            *pDstFloats++ = *pSrcFloats++;
        }
        break;
    }
}

// Attribute support routines
- (GLint) usesTexture0
{
    return m_vertexAttributes & kAttributeTexture0;
}
- (GLint) usesTexture1
{
    return m_vertexAttributes & kAttributeTexture1;
}
- (GLint) usesTexture2
{
    return m_vertexAttributes & kAttributeTexture2;
}
- (GLint) usesTexture3
{
    return m_vertexAttributes & kAttributeTexture3;
}
- (GLint) usesTexture4
{
    return m_vertexAttributes & kAttributeTexture4;
}
- (GLint) usesTexture5
{
    return m_vertexAttributes & kAttributeTexture5;
}
- (GLint) usesTexture6
{
    return m_vertexAttributes & kAttributeTexture6;
}
- (GLint) usesTexture7
{
    return m_vertexAttributes & kAttributeTexture7;
}
- (GLint) usesNormal
{
    return m_vertexAttributes & kAttributeNormal;
}
- (GLint) usesColor
{
    return m_vertexAttributes & kAttributeColor;
}
- (GLint) usesSecondaryColor
{
    return m_vertexAttributes & kAttributeSecondaryColor;
}
- (GLint) usesFog
{
    return m_vertexAttributes & kAttributeFog;
}



- (GLfloat *) Texture0: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[0]];
}

- (GLfloat *) Texture1: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[1]];
}

- (GLfloat *) Texture2: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[2]];
}

- (GLfloat *) Texture3: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[3]];
}

- (GLfloat *) Texture4: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[4]];
}

- (GLfloat *) Texture5: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[5]];
}

- (GLfloat *) Texture6: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[6]];
}

- (GLfloat *) Texture7: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[7]];
}

- (GLfloat *) Normal: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[8]];
}

- (GLfloat *) Color: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[9]];
}

- (GLfloat *) SecondaryColor: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[10]];
}

- (GLfloat *) Fog: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[11]];
}

- (GLfloat *) Texture: (GLint) unit atVertex: (GLfloat *) pVertex
{
    return &pVertex[m_attributeOffsets[unit & 0x7]];
}

- (void) atVertex: (GLint) index setPos_X: (GLfloat) x Y: (GLfloat) y  Z: (GLfloat) z
{
    GLfloat	*pData = [self atVertex: index];

    *pData++ = x;
    *pData++ = y;
    *pData++ = z;
    *pData++ = 1.0;
}

- (void) atVertex: (GLint) index setTexture0_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture0: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture1_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture1: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture2_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture2: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture3_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture3: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture4_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture4: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture5_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture5: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture6_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture6: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setTexture7_S: (GLfloat) s T: (GLfloat) t
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Texture7: pData];
    
    *pData++ = s;
    *pData++ = t;
}


- (void) atVertex: (GLint) index setNormal_X: (GLfloat) x Y: (GLfloat) y  Z: (GLfloat) z
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Normal: pData];

    *pData++ = x;
    *pData++ = y;
    *pData++ = z;
}


- (void) atVertex: (GLint) index setColor_R: (GLfloat) r G: (GLfloat) g B: (GLfloat) b
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Color: pData];

    *pData++ = r;
    *pData++ = g;
    *pData++ = b;
}


- (void) atVertex: (GLint) index setSecondaryColor_R: (GLfloat) r G: (GLfloat) g B: (GLfloat) b
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self SecondaryColor: pData];

    *pData++ = r;
    *pData++ = g;
    *pData++ = b;
}


- (void) atVertex: (GLint) index setFog_R: (GLfloat) r G: (GLfloat) g B: (GLfloat) b
{
    GLfloat	*pData = [self atVertex: index];

    pData = [self Fog: pData];

    *pData++ = r;
    *pData++ = g;
    *pData++ = b;
}



@end
