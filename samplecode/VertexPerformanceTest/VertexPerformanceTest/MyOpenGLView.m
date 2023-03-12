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


#import "MyOpenGLView.h"
#include "faultformation.h"
#include "Textures.h"
#include <math.h>
#include <sys/time.h>


#ifndef glSecondaryColor3fv
#define glSecondaryColor3fv glSecondaryColor3fvEXT
#define glSecondaryColorPointer glSecondaryColorPointerEXT
#endif

@implementation MyOpenGLView

- (float) X { return x; }
- (float) Y { return y; }
- (float) Z { return z; }
- (int) detail { return detail; }
- (float) performance { return performance; }

- (int) modelTriangleCount
{
    return detail * detail * 2;
}

- (int) modelVertexCount
{
    return (detail +1) * 2 * detail;
}

- (void) setX:(float) newx 
{ 
    x = newx;
    [self setNeedsDisplay:YES];
}
    
- (void) setY:(float) newy 
{ 
    y = newy;
    [self setNeedsDisplay:YES];
}
    
- (void) setZ:(float) newz 
{ 
    z = newz;
    [self setNeedsDisplay:YES];
}

- (int) textureUnitCount
{
    GLuint  count;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &count);
    return count;
}

- (void) setTexturing:(int) value  unit:(int)unit
{
    if (unit > kTextureUnitCount) return;
	textureEnabled[unit] =  (value ? GL_TRUE : GL_FALSE);
    [self rebuildDisplayList];
    [self setNeedsDisplay:YES];
}

- (void) setTexturing:(int*) value
{
    int i;
    for (i=0; i< kTextureUnitCount; i++)
    {
	textureEnabled[i] =  (value[i] ? GL_TRUE : GL_FALSE);
    }
    [self rebuildDisplayList];
    [self setNeedsDisplay:YES];
}

- (void) setMacros:(int) value
{
	macrosEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self setNeedsDisplay:YES];
}

- (void) setMatrix:(int) value
{
	matrixEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self setNeedsDisplay:YES];
}

- (void) setNormals:(int) value
{
	normalsEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self rebuildDisplayList];
	[self setNeedsDisplay:YES];
}

- (void) setRotate:(int) value
{
	rotationEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self setNeedsDisplay:YES];
}

- (void) setColor:(int) value
{
	colorEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self rebuildDisplayList];
	[self setNeedsDisplay:YES];
}

- (void) setSecColor:(int) value
{
	secColorEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self rebuildDisplayList];
	[self setNeedsDisplay:YES];
}

- (void) setFogFactor:(int) value
{
	fogFactorEnabled =  (value ? GL_TRUE : GL_FALSE);
	[self rebuildDisplayList];
	[self setNeedsDisplay:YES];
}

GLCoord3 calcNormal(GLCoord3 c1, GLCoord3 c2, GLCoord3 c3)
{
    GLCoord3 	v1, v2, n;
    float	len;
    
    v1.x = c1.x - c2.x;
    v1.y = c1.y - c2.y;
    v1.z = c1.z - c2.z;
    v2.x = c2.x - c3.x;
    v2.y = c2.y - c3.y;
    v2.z = c2.z - c3.z;
    n.x  = v1.y * v2.z - v2.y * v1.z;
    n.y  = v1.x * v2.z - v2.x * v1.z;
    n.z  = v1.x * v2.y - v2.x * v1.y;
    len = sqrt ( (n.x * n.x) + (n.y * n.y) + (n.z * n.z));
    n.x = n.x / len;
    n.y = n.y / len;
    n.z = n.z / len;
    
    return n;
} 

- (void) setDetail:(int) newdetail 
{ 
    int 	i,j;
    Vertex	*p;
    float	*height;
    
    if (newdetail == detail)
        return;
        
    if (detail < 4)
        detail = 4;
    else if (detail > 96)
        detail = 96;
    else
        detail = newdetail;

    //constucting a detail*detail grid from triangle strips
    // requires ((detail+1)*2)*detail verticies
    if (verticies) free (verticies);
    datasize = ( (sizeof(Vertex)+sizeof(AuxVertex)) * ((detail+1)*2) * detail );
    verticies = (Vertex *) valloc(datasize);
    verticies_aux = (AuxVertex *) ((GLubyte *)verticies + (sizeof(Vertex) * ((detail+1)*2) * detail) );
    p = verticies;
    
    //The terrain function needs a square.  Since we need 1 more column of
    // height to make the strips, we need to make the height detail+1 size square
    //
    height = (float *) valloc (sizeof(float) * ((detail+1) * (detail+1) ));
    srand(129);
    MakeTerrainFault(height, detail+1, 32, 1.5, 0, 1, 0.5f );
    
    for ( i = 0; i < detail; i++)
    {
        float *heightVals1 = &height[i * (detail+1)];
        float *heightVals2 = &height[(i+1) * (detail+1)];
        for (j = 0; j <= detail; j++)
        {
            p->texCoord.x = ((1.0f / detail) * i);
            p->vertex.x = p->texCoord.x - 0.5f;
            p->texCoord.y = ((1.0f / detail) * j);
            p->vertex.y = p->texCoord.y - 0.5f;
            p->vertex.z = *heightVals1++ - 0.5f;
            p++; 

            p->texCoord.x = ((1.0f / detail) * (i+1));
            p->vertex.x = p->texCoord.x - 0.5f;
            p->texCoord.y = ((1.0f / detail) * (j));
            p->vertex.y = p->texCoord.y - 0.5f;
            p->vertex.z = *heightVals2++ - 0.5f;
            p++;
        }
    }
    free (height);
    
    {
        GLuint	verticiesInStrip = (detail+1)*2;
        for (i = 0; i < detail; i++)
        {
           for (j = 0; j < verticiesInStrip ; j++)
            {
                GLuint index = (i*verticiesInStrip)+j;
                
                verticies_aux[index].secondaryColor.x = 0;
                verticies_aux[index].secondaryColor.y = (float) i / (float)detail;
                verticies_aux[index].secondaryColor.z = (float) j / (float)verticiesInStrip ;

                verticies_aux[index].color.x = (float) j / (float)verticiesInStrip;
                verticies_aux[index].color.y = 0;
                verticies_aux[index].color.z = (float) i / (float)detail;
            }
        }
    }
    
    p = verticies;
    for ( i = 0; i < detail; i++)
    {
        Vertex *q;
        
        q = verticies + (i * (detail+1) * 2);
        //First 2 normals are identical from 1st triangle
        p[0].normal = calcNormal(q[0].vertex, q[1].vertex, q[2].vertex); p++;
        p[0].normal = calcNormal(q[0].vertex, q[1].vertex, q[2].vertex); p++;
        for (j = 0; j < detail; j++)
        {
            p[0].normal = calcNormal(q[0].vertex, q[1].vertex, q[2].vertex); 
            p++; q++;
            p[0].normal = calcNormal(q[1].vertex, q[0].vertex, q[2].vertex);
            p++; q++;
        }
    }

    if (elementsIndex) free (elementsIndex);
    elementsIndex = (GLuint *) valloc( sizeof(GLuint) * ((detail+1)*2) * detail);

    for (i = 0 ; i < ((detail+1)*2) * detail; i++)
        elementsIndex[i] = i;
        
    [self rebuildDisplayList];
    [self setNeedsDisplay:YES];
}

- (void) setSize:(int) newSize 
{
    size = newSize;
	
    [self reshape];
    [self setNeedsDisplay:YES];
}

- (void) rebuildDisplayList
{
    BOOL	savemacrosEnabled = macrosEnabled;

    macrosEnabled = NO;
    if (listID != 0) glDeleteLists(1, listID);
	
    listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);
    [self drawImmediateModeRep];
    glEndList();
    macrosEnabled = savemacrosEnabled;
}

- (void) spinModel
{
    if (rotationEnabled)
    {
        x = x + 0.25f;
        y = y + 0.5f;
        z = z + 1.5f;
        if (x > 360.0f) x = x - 360.0f;
        if (y > 360.0f) y = y - 360.0f;
        if (z > 360.0f) z = z - 360.0f;
        [self setNeedsDisplay:YES];
    }
}

- (void) setDrawMethod:(int) method 
{ 
    drawMethod = method;
    [self rebuildDisplayList];
    [self setNeedsDisplay:YES];
}

#define VTX_ALL(enables, x,y) \
{ \
	if(enables & 0x00000001) glNormal3fv((GLfloat *)&x.normal); \
	if(enables & 0x00000002) glColor3fv((GLfloat *)&y.color); \
	if(enables & 0x00000004) glSecondaryColor3fv((GLfloat *)&y.secondaryColor); \
	if(enables & 0x00000008) glMultiTexCoord2fv(GL_TEXTURE0, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000010) glMultiTexCoord2fv(GL_TEXTURE1, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000020) glMultiTexCoord2fv(GL_TEXTURE2, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000040) glMultiTexCoord2fv(GL_TEXTURE3, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000080) glMultiTexCoord2fv(GL_TEXTURE4, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000100) glMultiTexCoord2fv(GL_TEXTURE5, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000200) glMultiTexCoord2fv(GL_TEXTURE6, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000400) glMultiTexCoord2fv(GL_TEXTURE7, (GLfloat *)&x.texCoord); \
	glVertex3fv((GLfloat *)&x.vertex); \
}

#define VTX_MACRO_ALL(enables, x,y) \
{ \
	if(enables & 0x00000001) cgl_ctx->disp.normal3fv(cgl_ctx->rend, (GLfloat *)&x.normal); \
	if(enables & 0x00000002) cgl_ctx->disp.color3fv(cgl_ctx->rend, (GLfloat *)&y.color); \
	if(enables & 0x00000004) cgl_ctx->disp.secondary_color3fv(cgl_ctx->rend, (GLfloat *)&y.secondaryColor); \
	if(enables & 0x00000008) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE0, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000010) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE1, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000020) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE2, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000040) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE3, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000080) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE4, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000100) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE5, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000200) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE6, (GLfloat *)&x.texCoord); \
	if(enables & 0x00000400) cgl_ctx->disp.multi_tex_coord2fv(cgl_ctx->rend, GL_TEXTURE7, (GLfloat *)&x.texCoord); \
	cgl_ctx->disp.vertex3fv(cgl_ctx->rend, (GLfloat *)&x.vertex); \
}

- (void) drawImmediateModeRep
{
    int i,j;
    int triCount = ((detail+1) * 2);
    Vertex *p = verticies;
    AuxVertex *paux = verticies_aux;
	GLuint enables;
	
	// Setup enable bit field
	enables  = (normalsEnabled    ? 0x00000001 : 0x00000000);
	enables |= (colorEnabled      ? 0x00000002 : 0x00000000);
	enables |= (secColorEnabled   ? 0x00000004 : 0x00000000);
	enables |= (textureEnabled[0] ? 0x00000008 : 0x00000000);
	enables |= (textureEnabled[1] ? 0x00000010 : 0x00000000);
	enables |= (textureEnabled[2] ? 0x00000020 : 0x00000000);
	enables |= (textureEnabled[3] ? 0x00000040 : 0x00000000);
	enables |= (textureEnabled[4] ? 0x00000080 : 0x00000000);
	enables |= (textureEnabled[5] ? 0x00000100 : 0x00000000);
	enables |= (textureEnabled[6] ? 0x00000200 : 0x00000000);
	enables |= (textureEnabled[7] ? 0x00000400 : 0x00000000);

    if(macrosEnabled)
    {
        CGLContextObj cgl_ctx = CGLGetCurrentContext();
        
        for(i = 0; i < detail; i++)
        {
            glBegin(GL_TRIANGLE_STRIP);                
            for(j = 0; j < triCount; j++)
            {
                VTX_MACRO_ALL(enables, p[0], paux[0]);
                p++;
                paux++;
            }
            glEnd();
        }
    }
    else
    {
        for(i = 0; i < detail; i++)
        {
            glBegin(GL_TRIANGLE_STRIP);                
            for(j = 0; j < triCount; j++)
            {
                VTX_ALL(enables, p[0], paux[0]);
                p++;
                paux++;
            }
            glEnd();
        }
    }
}

- (void) drawImmediateMode:(int) reps
{
    int i;
    
    for(i = 0; i < reps; i++)
    {
        if(matrixEnabled) glLoadMatrixf(matrix);
		
        [self drawImmediateModeRep];
    }
}    

- (void) drawDisplayList:(int) reps
{
    int i;
    
    for(i=0; i < reps; i++)
    {
        if(matrixEnabled) glLoadMatrixf(matrix);
        glCallList(listID);
    }
}    

- (void) drawArrays: (BOOL) useElements :(int) reps
{
    int i, j;
    int triCount = ((detail+1) * 2);
    
    if(!m_glContextInitialized) 
    	return;
    
    if(useElements) 
    {
        for(i = 0; i < reps; i++)
        {
            if(matrixEnabled) glLoadMatrixf(matrix);
			
            for (j = 0; j<detail; j++)
                glDrawElements(GL_TRIANGLE_STRIP, triCount, GL_UNSIGNED_INT, &elementsIndex[j*triCount]);
        }
    }    
    else
    {
        for(i = 0; i < reps; i++)
        {
            if(matrixEnabled) glLoadMatrixf(matrix);
			
            for(j = 0; j<detail; j++)
                glDrawArrays(GL_TRIANGLE_STRIP, j * triCount, triCount);
        }
    }	
}    

- (void) initGL
{
    int i;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFinish();
    
    [self setDetail:4];

    glGenTextures(1, &textureID[0]);
    {
        NSImage * image = [NSImage imageNamed:@"OpenGLLogo"];
        GLuint	w, h;
        TextureFromNSImage(image, &textureID[0], &w, &h);
    }

    StringToTexture("1  1  1\n  1  1  1\n 1  1  1", "Helvetica", 36, &textureID[1], 128, 128, [NSColor redColor]);
    StringToTexture(" 2 2 2 2 2 2 2 2 2 2 2 2 2 2", "Helvetica", 36, &textureID[2], 128, 128, [NSColor blueColor]);
    StringToTexture("3 3 3 3\n 3 3 3 3\n3 3 3 3 3 3 3 3 3 3 3 3", "Helvetica", 36, &textureID[3], 128, 128, [NSColor greenColor]);
    
    StringToTexture("  4  4  4\n  4  4  4\n 4  4  4", "Helvetica", 36, &textureID[4], 128, 128, [NSColor redColor]);
    StringToTexture(" 5 5 5 5 5 5 5 5 5 5 5 5 5 5", "Helvetica", 36, &textureID[5], 128, 128, [NSColor blueColor]);
    StringToTexture("6 6 6 6\n 6 6 6 6\n6 6 6 6 6 6 6 6 6 6 6 6", "Helvetica", 36, &textureID[6], 128, 128, [NSColor greenColor]);
    StringToTexture("7  7  7\n  7  7  7\n 7  7  7", "Helvetica", 36, &textureID[6], 128, 128, [NSColor redColor]);

    glEnable(GL_DEPTH_TEST);
    for (i = 0; i < kTextureUnitCount; i++) textureEnabled[i] = GL_FALSE;
    normalsEnabled = GL_FALSE;
    macrosEnabled  = GL_FALSE;
    matrixEnabled  = GL_FALSE;
    colorEnabled     = GL_FALSE;
    secColorEnabled  = GL_FALSE;
    fogFactorEnabled = GL_FALSE;
    size = 0;
    //{
    //	const GLubyte *extensions = NULL;
    //	extensions = glGetString(GL_EXTENSIONS);
    //	hasVertexArrayRange = gluCheckExtension("GL_APPLE_vertex_array_range", extensions);
    //	if (!hasVertexArrayRange) hasVertexArrayRange = gluCheckExtension("GL_NV_vertex_array_range", extensions);
    //}
    m_glContextInitialized = YES;
    [self reshape];
}

- (void)reshape	// scrolled, moved or resized
{
    NSRect rect;
    [super reshape];
    
    rect = [self bounds];
    glViewport(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 45.0f - (float) size, (GLfloat) rect.size.width / (GLfloat) rect.size.height, 0.1f, 150.0f );
    gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glMatrixMode(GL_MODELVIEW);
    [self setNeedsDisplay:YES];
}    

- (void)drawRect:(NSRect)rect
{
    int i;

    if (!m_glContextInitialized) 
        [self initGL];
    
    if (NULL == verticies) {printf("NULL == verticies\n"); return; }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(x, 1.0f, 0.0f, 0.0f);
    glRotatef(y, 0.0f, 1.0f, 0.0f);
    glRotatef(z, 0.0f, 0.0f, 1.0f);
	
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /*
        texture setup
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (i = 0; i < kTextureUnitCount; i++)
    {    
        if (textureEnabled[i]) 
        {
            glColor4f(1,1,1,1);
            glActiveTextureARB(GL_TEXTURE0 + i);
            glClientActiveTextureARB(GL_TEXTURE0 + i);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, textureID[i]);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (i==0) ? GL_MODULATE: GL_DECAL);
            glEnable(GL_TEXTURE_2D);
            glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &verticies[0].texCoord);
        }
    }
    glActiveTextureARB (GL_TEXTURE0);
    glClientActiveTextureARB (GL_TEXTURE0);
    
    /*
        normal setup
    */
    if (normalsEnabled)
    {
        GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat mat_shininess[]  = { 50.0f };
        GLfloat light_position[] = { 0.0f, 3.0f, 10.0f, 0.0f };
                
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
		
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, sizeof(Vertex), &verticies[0].normal);
    }
    
    /*
        Color Setup
    */
    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
    if ((colorEnabled) || (normalsEnabled)) 
        glColor4f(1,1,1,1);
    else if (secColorEnabled)
    {
        Boolean texEnable = FALSE;
        for (i = 0; i < kTextureUnitCount; i++)
            if ( textureEnabled[i] )
                texEnable = TRUE;
        if ( !texEnable) glColor4f(0,0,0, 1.0f) ;
    }
    
    if (colorEnabled)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, sizeof(AuxVertex), &verticies_aux[0].color);
    }
    
    if (secColorEnabled)
    {
        glEnable(GL_COLOR_SUM);
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        glSecondaryColorPointer(3, GL_FLOAT, sizeof(AuxVertex), &verticies_aux[0].secondaryColor);
    }
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &verticies[0].vertex);
	
    if (drawMethod == kCompiledVertexArray)
    {
        glLockArraysEXT(0, ((detail+1)*2) * detail);
    }
#if GL_APPLE_vertex_array_range
    else if (drawMethod == kVertexArrayRange)
    {
        glVertexArrayRangeAPPLE( datasize, (GLvoid *)verticies);
        glEnableClientState( GL_VERTEX_ARRAY_RANGE_APPLE );
        glFlushVertexArrayRangeAPPLE( datasize, (GLvoid *)verticies);
    }
#endif

    {
        struct	timeval t1, t2;
        float	t=0;
        int	reps = 0;
        int	loop_rep;
        int	tri_count = [self modelTriangleCount];
        
        loop_rep = 20000.0f / (float) tri_count;
        if(loop_rep < 10)        loop_rep = 10;
        else if(loop_rep > 2000) loop_rep = 2000;

        gettimeofday(&t1, NULL);
        
        do
        {
            switch (drawMethod)
            {
                case kVertexCalls:
                    [self drawImmediateMode:loop_rep];
                    break;
                    
                case kDisplayLists:
                    [self drawDisplayList:loop_rep];
                    break;
                    
                case kDrawArrays:
                    [self drawArrays:NO :loop_rep];
                    break;
    
                case kCompiledVertexArray:
                    [self drawArrays:YES :loop_rep];
                    break;

                 case kVertexArrayRange:
                    [self drawArrays:NO :loop_rep];
                    break;
            }
    
            glFlush();
            gettimeofday(&t2, NULL);
            t = 1000000.0f * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec);
            reps+=loop_rep;
        }
        while (t < 100000.0f); //.1 second
        glFinish();
        gettimeofday(&t2, NULL);
        t = 1000000.0f * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec);
        performance = (float)([self modelTriangleCount]*reps) /  t;
    }
	
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);

    if (secColorEnabled)
    {
        glDisable(GL_COLOR_SUM);
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    }
    
    for (i = 0; i < kTextureUnitCount; i++)
    {
        if (textureEnabled[i]) 
        {
            glActiveTextureARB (GL_TEXTURE0+i);
            glClientActiveTextureARB (GL_TEXTURE0+i);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_TEXTURE_2D);
        }
    }
    glActiveTextureARB (GL_TEXTURE0);
    glClientActiveTextureARB (GL_TEXTURE0);

    if (normalsEnabled)
    {
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
    
    if (colorEnabled)
         glDisableClientState(GL_COLOR_ARRAY);
 
    if (drawMethod == kCompiledVertexArray)
    {
        glUnlockArraysEXT();
    }
#if GL_APPLE_vertex_array_range
    else if (drawMethod == kVertexArrayRange)
    {
        glDisableClientState(GL_VERTEX_ARRAY_RANGE_APPLE);
        glVertexArrayRangeAPPLE(0,0);
    }
#endif
    
    if (rotationEnabled)
        [self setNeedsDisplay:YES];
}


- (void) awakeFromNib
{
    m_glContextInitialized = NO;
    
}

//- (BOOL) hasVertexArrayRange
//{
//    return hasVertexArrayRange;
//}



@end

