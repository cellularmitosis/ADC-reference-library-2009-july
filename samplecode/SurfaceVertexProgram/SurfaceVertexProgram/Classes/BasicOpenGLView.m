/*
 *  BasicOpenGLView.m
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

#import "BasicOpenGLView.h"

@implementation BasicOpenGLView

// This loads the default values for this object
- (void) awakeFromNib
{
    m_glContextInited		= false;
    m_updateDisplay		= 0;
    m_frameNumber		= 0;
    m_vertexProgramString	= NULL;
    
    m_wireframeMode		= 0;
	m_uvMeshMode		= 0;
}

// This overrides the pixel depth
- (id) initWithFrame: (NSRect) frame
{
    GLuint attribs[] = 
    {
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAWindow,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 32,
        NSOpenGLPFAAccumSize, 0,
        0
    };
    
    NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) attribs]; 
    
    if (!fmt)
    {
        NSLog(@"No OpenGL pixel format");
    }
    
    return self = [super initWithFrame:frame pixelFormat: [fmt autorelease]];
}

// Init GL for this context
- (void) initGL
{
    NSRect	rect = [self frame];
    NSImage 	*image = [NSImage imageNamed: @"no_copyright_score"];
    GLuint	w, h;
    GLuint 	texID = 0;
    long 	params[] = { 1};

    // grab the gl context
    [[self openGLContext] makeCurrentContext];
	
    glGenTextures(1, &texID);

    TextureFromNSImage(image, &texID, &w, &h);

    glColor4f(1,1,1,1);

    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glEnable(GL_TEXTURE_2D);                

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    // Set the viewport
    glViewport(0, 0, (GLsizei) rect.size.width, (GLsizei) rect.size.height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);

    // Enable Depth tests
    glEnable(GL_DEPTH_TEST);
    
    // Setup camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 60,(GLfloat) rect.size.width /(GLfloat) rect.size.height, 0.1f, 1000.0f );
    gluLookAt(0,  3, 2,
              0,  0, 0,
              0, -1, 0);
    
    // Enter modeling mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    CGLSetParameter(CGLGetCurrentContext(),  kCGLCPSwapInterval, params);

    [self loadVertexProgram: @"SurfaceVertexProgram.vsh"];
}

// Object rectangle initializer
- (void) drawRect: (NSRect) rect
{
    if (false == m_glContextInited)
    {
        [self initGL];
        
        m_glContextInited = true;
    }

    if (1 == m_updateDisplay)
    {
        [[self openGLContext] makeCurrentContext];

        [[self openGLContext] flushBuffer];
        
        m_updateDisplay = 0;
    }
}

// Matrix control
- (void) identity
{
    glLoadIdentity();
}

- (void) projection
{
    glMatrixMode(GL_PROJECTION);
}

- (void) modelview
{
    glMatrixMode(GL_MODELVIEW);
}

- (void) rotate_X: (GLfloat) x Y: (GLfloat) y Z: (GLfloat) z
{
    if (x)	glRotatef(x, 1, 0, 0);
    if (y)	glRotatef(y, 0, 1, 0);
    if (z)	glRotatef(z, 0, 0, 1);
}

- (void) scale_X: (GLfloat) x Y: (GLfloat) y Z: (GLfloat) z
{
    glScalef(x, y, z);
}

- (void) translate_X: (GLfloat) x Y: (GLfloat) y Z: (GLfloat) z
{
    glTranslatef(x, y, z);
}

// Frame control
- (int) frameNumber
{
    return m_frameNumber;
}

- (void) frameBegin: (int) frameNumber
{
    [[self openGLContext] makeCurrentContext];
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);

    m_updateDisplay = 0;

    // Enter modeling mode
    glMatrixMode(GL_MODELVIEW);
    
    m_frameNumber = frameNumber;
}

- (void) frameBegin
{
    [self frameBegin: m_frameNumber + 1];
}

- (void) frameEnd
{
    [[self openGLContext] makeCurrentContext];

    m_updateDisplay = 1;
    
    [self setNeedsDisplay: true];

    [[self openGLContext] flushBuffer];
}

// Debug
- (void) drawAxis
{
    glBegin(GL_LINES);
    
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);

    
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    
    glEnd();
}

// Vertex array attributes
- (void) setGLVertexArrayAttributes: (VariableFormatVertex *) vertices atVertex: (GLfloat *) pVertex
{
    unsigned int	vertexAttributes = [vertices vertexAttributes];
    int			byteStride = [vertices byteStride];	
    int			i;

    // Always enable XYZ
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(4, GL_FLOAT, byteStride, pVertex);

    // Enable / Disable all texture vertex arrays
    for(i=0; i<8; i++)
    {
        // Select texture unit
        glClientActiveTexture(GL_TEXTURE0 + i);

        // enable / disable unit
        if (vertexAttributes & (0x1 << i))
        {
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, byteStride, [vertices Texture: i atVertex: pVertex]);
        }
        else
        {
			if (glIsEnabled(GL_VERTEX_PROGRAM_ARB))
			{
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
			}
			
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    // Normal
    if (vertexAttributes & (0x1 << 8))
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, byteStride, [vertices Normal: pVertex]);
    }
    else
    {
        glDisableClientState(GL_NORMAL_ARRAY);    
    }
    
    // Color
    if (vertexAttributes & (0x1 << 9))
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(3, GL_FLOAT, byteStride, [vertices Color: pVertex]);
    }
    else
    {
        glColor4f(0,0,0,1);
        glDisableClientState(GL_COLOR_ARRAY);    
    }

    // SecondaryColor
    if (vertexAttributes & (0x1 << 10))
    {
        glColor4f(0,0,0,1);
        glEnable(GL_COLOR_SUM);
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        glSecondaryColorPointer(3, GL_FLOAT, byteStride, [vertices SecondaryColor: pVertex]);
    }
    else
    {
        glColor4f(1,1,1,1);
        glDisable(GL_COLOR_SUM);
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);    
    }

    // Fog
    if (vertexAttributes & (0x1 << 11))
    {
        glEnableClientState(GL_FOG_COORDINATE_ARRAY);
        glFogCoordPointer(GL_FLOAT, byteStride, [vertices Fog: pVertex]);
    }
    else
    {
        glDisableClientState(GL_FOG_COORDINATE_ARRAY);    
    }

    if ([vertices useVAR])
    {
        [vertices flushVertexArrayRange];
    }    
}

// Vertex Array Primitive draw routines
- (void) drawVertexArray: (VariableFormatVertex *) vertices ofType: (GLint) type
{
    GLfloat	*pVertex = [vertices atVertex: 0];
    int		nVertices = [vertices vertexCount];

    [self setGLVertexArrayAttributes: vertices atVertex: pVertex];

    switch(type)
    {
        case GL_POINTS:
            glDrawArrays(type, 0, nVertices-1);
        break;

        case GL_LINE_STRIP:
        {
            if ([vertices isMesh])
            {
                GLint	i, j, rows, cols;
                GLint	*indices;
                
                rows = [vertices rows];
                cols = [vertices cols];
                
                indices = (GLint *)malloc(sizeof(GLint) * (cols + 1));
                
                for(i=0; i<rows; i++)
                {
                    for(j=0; j<cols; j++)
                    {
                        indices[j] = (i * cols) + j;
                    }

                    glDrawElements(type, cols, GL_UNSIGNED_INT, indices);
                }
                
                for(j=0; j<cols; j++)
                {
                    for(i=0; i<rows; i++)
                    {
                        indices[i] = (i * cols) + j;
                    }

                    glDrawElements(type, rows, GL_UNSIGNED_INT, indices);
                }
                
                free(indices);
            }
        }
        break;
        
        case GL_QUAD_STRIP:
        {
            if ([vertices isMesh])
            {
                GLint	i, j, index, rows, cols;
                GLint	*indices;
                
                rows = [vertices rows];
                cols = [vertices cols];
                
                indices = (GLint *)malloc(2 * sizeof(GLint) * (cols + 1));
                
                for(i=0; i<rows; i++)
                {
                    for(index=0, j=0; j<cols; j++)
                    {
                        indices[index++] =       i * cols + j;
                        indices[index++] = (i + 1) * cols + j;
                    }

                    glDrawElements(type, 2*cols, GL_UNSIGNED_INT, indices);
                }

                free(indices);
            }
        }
        break;
       
        case GL_LINES:
        case GL_LINE_LOOP:
        case GL_TRIANGLES:
        case GL_QUADS:
        case GL_POLYGON:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        break;
        
        default:
        break;        
    }
}


// Vertex program routines
- (void) loadVertexProgram: (NSString *) vertexProgramFilename
{
    const char 	*pCstring;
    NSString	*temp_vertexProgramString;
    
    NSBundle	*pBundle = [NSBundle mainBundle];
    NSString	*pBundlePath = NULL;
    char 	fullPath[1024];
    
    if (pBundle)
    {
        pBundlePath = [NSString stringWithString: [pBundle resourcePath]];
    }
    
    if (pBundlePath)
    {
        sprintf(fullPath, "%s/%s", [pBundlePath cString], [vertexProgramFilename cString]);
        
        m_vertexProgramString = [NSString stringWithContentsOfFile: [NSString stringWithCString: fullPath]];
    }

    temp_vertexProgramString = [NSString stringWithContentsOfFile: vertexProgramFilename];

    if (m_vertexProgramString)
    {
        pCstring = [m_vertexProgramString cString];
        
        glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                            strlen(pCstring), pCstring);

        if (GL_NO_ERROR != glGetError())
        {
                fprintf(stderr, "Loading Program returned %s\n", (unsigned char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
                
                exit(-1);
        }
    }
}

- (void) unloadVertexProgram
{
    [m_vertexProgramString dealloc];
    
    m_vertexProgramString = NULL;
}

- (void) enableVertexPrograms
{
    glEnable(GL_VERTEX_PROGRAM_ARB);
}

- (void) disableVertexPrograms
{
    glDisable(GL_VERTEX_PROGRAM_ARB);
}

- (void) loadEnvParameter: (GLfloat *) parameter atIndex: (GLint) index
{
    glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, parameter);
}

- (void) loadLocalParameter: (GLfloat *) parameter atIndex: (GLint) index
{
    glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, parameter);
}

- (int) wireframeMode
{
    return m_wireframeMode;
}

- (int) uvMeshMode
{
    return m_uvMeshMode;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint pt;
    
    pt = [theEvent locationInWindow];
    
    [[self openGLContext] makeCurrentContext];

    m_sphi   += (GLfloat) (pt.x - m_downX) / 2.0f;
    m_stheta -= (GLfloat) (m_downY - pt.y) / 2.0f;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    
    glTranslatef(0.0, 0.0, -m_sdepth);
    glRotatef(m_stheta, 1.0, 0.0, 0.0);
    glRotatef(m_sphi, 0.0, 0.0, 1.0);

    m_downX = pt.x;
    m_downY = pt.y;

	[super mouseDragged: theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pt;
    
    pt = [theEvent locationInWindow];
    
    m_downX = pt.x;
    m_downY = pt.y;
	
	[super mouseDown: theEvent];
}

- (void)keyDown:(NSEvent *)theEvent
{
    unichar		c = [theEvent keyCode];
    
    if ([theEvent isARepeat])
    {
        return;
    }
    
    switch (c)
    {
		case 0x2e:	// m key
			m_uvMeshMode = m_uvMeshMode ? 0 : 1;
			break;
			
        case 0x31:
            m_wireframeMode = m_wireframeMode ? 0 : 1;
        break;

        default:
        [super keyDown: theEvent];
    }
}

- (BOOL)acceptsFirstResponder
{
   return YES;
}

- (BOOL)becomeFirstResponder
{
   return  YES;
}

- (BOOL)resignFirstResponder
{
   return YES;
}

@end
