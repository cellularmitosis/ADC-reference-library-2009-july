/*
 *   BasicOpenGLView.h
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

#import <Cocoa/Cocoa.h>

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <OpenGL/glext.h>

#import "VariableFormatVertex.h"
#import "Textures.h"

@interface BasicOpenGLView : NSOpenGLView
{
    GLint	m_glContextInited;
    GLint	m_updateDisplay;
    GLint	m_frameNumber;
    GLint	m_maxVertexAttribs;
    
    // Trackball stuff
    GLint	m_mouseDown;
    GLint	m_mouseDragged;
    GLint	m_pickingMode;
    GLfloat 	m_sphi;
    GLfloat	m_xtrans;
    GLfloat 	m_stheta;
    GLfloat 	m_sdepth;
    GLint 	m_downX;
    GLint 	m_downY;
    
    GLint	m_wireframeMode;
    
    NSString	*m_vertexProgramString;
}

- (void) initGL;
- (void) drawRect: (NSRect) rect;

- (void) identity;
- (void) projection;
- (void) modelview;

- (void) rotate_X: (GLfloat) x Y: (GLfloat) y Z: (GLfloat) z;
- (void) scale_X: (GLfloat) x Y: (GLfloat) y Z: (GLfloat) z;
- (void) translate_X: (GLfloat) x Y: (GLfloat) y Z: (GLfloat) z;

- (void) projectPoint: (GLfloat *)srcPoint ToScreenCoordinates: (GLfloat *)dstPoint;
- (int) unprojectPoint: (GLfloat *)srcPoint ToWorldCoordinates: (GLfloat *)dstPoint;

- (void) frameBegin: (int) frameNumber;
- (void) frameBegin;
- (void) frameEnd;

- (void) drawAxis;

- (void) setGLVertexArrayAttributes: (VariableFormatVertex *) vertices atVertex: (GLfloat *) pVertex;
- (void) drawVertexArray: (VariableFormatVertex *) vertices ofType: (GLint) type;

- (void) pointSize: (GLfloat) size;

- (void) enableTextures;
- (void) disableTextures;

- (void) loadVertexProgram: (NSString *) vertexProgramFilename;
- (void) unloadVertexProgram;
- (void) enableVertexPrograms;
- (void) disableVertexPrograms;

- (void) loadControlPoints: (VariableFormatVertex *)controlPts;

- (int) wireframeMode;

- (int) pickingMode;
- (void) setPickingMode: (GLint) mode;
- (GLint) mouseDown;
- (GLfloat) mouseX;
- (GLfloat) mouseY;

@end

