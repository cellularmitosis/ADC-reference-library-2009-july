/*
 *   BasicApplicationController.m
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

#import "BasicApplicationController.h"

@implementation BasicApplicationController

- (void) awakeFromNib
{
    // setup the timer
    m_timer = [NSTimer scheduledTimerWithTimeInterval: 0.05 target:self 
                selector:@selector(timerEventHandler) userInfo:0 repeats:YES];
                
                
    // Init the rotation to zero
    m_rotx = m_roty = m_rotz = 0;

    m_NURBSUVSurface		= NULL;
    m_BezierUVSurface		= NULL;
    m_ControlPts		= NULL;
    m_ControlPointVertices	= NULL;
}

float CreateBSpliineBasis(float u, int i, int p, const float *knots)
{
    GLfloat	Q, D, N=0;
    
    if (0 == p)
    {
        N = ((knots[i] <= u) && (u < knots[i+1])) ? 1 : 0;
        
        return N;
    }
    
    if (D = (knots[i+p] - knots[i]))
    {
        if (Q = (u - knots[i])/D)
        {
            N = Q/D * CreateBSpliineBasis(u, i, p-1, knots);
        }
    }
    
    if (D = (knots[i+p+1] - knots[i+1]))
    {
        if (Q = (knots[i+p+1] - u))
        {
            N += Q/D * CreateBSpliineBasis(u, i+1, p-1, knots);
        }
    }
    
    return N;
}

- (VariableFormatVertex	*) CreateNURBSUVSurface: (GLint) LOD
{
    VariableFormatVertex	*NURBSUVSurface = [VariableFormatVertex new];
    GLint			nVerticesRequired;
    GLint			vertexIndex;
    GLint			i, j;
    GLfloat			u, v, ustep, vstep;
    GLint			rows, cols;
    const GLfloat		UKnots[] = {0,0,0,0,1,1,1,1};
    const GLfloat		VKnots[] = {0,0,0,0,1,1,1,1};

    rows = LOD;
    cols = LOD;

    ustep = 1.0 / (GLfloat)rows;
    vstep = 1.0 / (GLfloat)cols;
    
    rows++;
    cols++;
    
    if(NURBSUVSurface)
    {
        GLuint	vertexAttributes = kAttributeColor | kAttributeTexture0 | kAttributeSecondaryColor;
        
        nVerticesRequired = rows * cols;
        
        [NURBSUVSurface initWithVertexType: vertexAttributes vertexArrayOfCount: nVerticesRequired];

        [NURBSUVSurface setMeshRows: rows Cols: cols];

        // Fill in the vertices with basis functions for a 4x4 NURB
        for(vertexIndex=0, j=0, v=0;v<=1.0; v+=vstep, j++)
        {
            float vBasis[4];
            
            vBasis[0] = CreateBSpliineBasis(v, 0, 3, VKnots);
            vBasis[1] = CreateBSpliineBasis(v, 1, 3, VKnots);                
            vBasis[2] = CreateBSpliineBasis(v, 2, 3, VKnots);                
            vBasis[3] = CreateBSpliineBasis(v, 3, 3, VKnots);

            for(i=0, u=0;u<=1.0; u+=ustep, i++, vertexIndex++)
            {
                GLfloat	*pVertex = [NURBSUVSurface atRow: j atCol: i];
				GLfloat	*pColor, *pST;
				
                // Put the uTerms in the Position
                pVertex[0] = CreateBSpliineBasis(u, 0, 3, UKnots);
                pVertex[1] = CreateBSpliineBasis(u, 1, 3, UKnots);                
                pVertex[2] = CreateBSpliineBasis(u, 2, 3, UKnots);                
                pVertex[3] = CreateBSpliineBasis(u, 3, 3, UKnots);

                // Put the vTerms in the Color
                pColor = [NURBSUVSurface Color: pVertex];
                pColor[0] = vBasis[0];
                pColor[1] = vBasis[1];                
                pColor[2] = vBasis[2];                
                pColor[3] = vBasis[3];

                pST = [NURBSUVSurface Texture0: pVertex];
                pST[0] = u;
                pST[1] = v;                
            }
        }
		
		[NURBSUVSurface attachToVertexArrayRange];
    }

    return NURBSUVSurface;
}

- (VariableFormatVertex	*) CreateBezierUVSurface
{
    VariableFormatVertex	*BezierUVSurface = [VariableFormatVertex new];
    GLint			nVerticesRequired;
    GLint			vertexIndex;
    GLint			i, j;
    GLfloat			u, v, ustep, vstep;
    GLint			rows, cols;

    rows = 40;
    cols = 40;

    ustep = 1.0 / (GLfloat)rows;
    vstep = 1.0 / (GLfloat)cols;
    
    rows++;
    cols++;
    
    if(BezierUVSurface)
    {
        GLuint	vertexAttributes = kAttributeColor | kAttributeTexture0 | kAttributeTexture1;
        
        nVerticesRequired = rows * cols;
        
        [BezierUVSurface initWithVertexType: vertexAttributes vertexArrayOfCount: nVerticesRequired];

        [BezierUVSurface setMeshRows: rows Cols: cols];

        // Fill in the vertices with basis functions for a 4x4 NURB
        for(vertexIndex=0, j=0, v=0;v<=1.0; v+=vstep, j++)
        {
            float vBasis[4];

            // V Basis functions
            vBasis[0] = (1-v)*(1-v)*(1-v);
            vBasis[1] = 3*v*(1-v)*(1-v);                
            vBasis[2] = 3*v*v*(1-v);
            vBasis[3] = v*v*v;
            
            for(i=0, u=0;u<=1.0; u+=ustep, i++, vertexIndex++)
            {
                GLfloat	*pVertex = [BezierUVSurface atRow: j atCol: i];
                GLfloat	*pBasis;

                // Put the uTerms in the Position
                pBasis = pVertex;
                pBasis[0] = (1-u)*(1-u)*(1-u);
                pBasis[1] = 3*u*(1-u)*(1-u);                
                pBasis[2] = 3*u*u*(1-u);
                pBasis[3] = u*u*u;

                // Put the vTerms in the Color
                pBasis = [BezierUVSurface Color: pVertex];
                pBasis[0] = vBasis[0];
                pBasis[1] = vBasis[1];                
                pBasis[2] = vBasis[2];                
                pBasis[3] = vBasis[3];
            }
        }
    }

    return BezierUVSurface;
}

- (VariableFormatVertex *) CreateDrawableControlPoints: (GLfloat *)pFloats ofRows: (GLint) rows ofCols: (GLint) cols
{
    VariableFormatVertex	*ControlPointVertices = [VariableFormatVertex new];
    GLint			i;
    GLint			count = rows * cols;
    
    if(ControlPointVertices)
    {
        [ControlPointVertices initWithVertexType: kAttributeColor vertexArrayOfCount: count];

        [ControlPointVertices setMeshRows: rows Cols: cols];
        
        // Copy the points over
        for(i=0;i<count; i++)
        {
            GLfloat *pVertex = [ControlPointVertices atVertex: i];
            
            pVertex[0] = *pFloats++;
            pVertex[1] = *pFloats++;
            pVertex[2] = *pFloats++;
            pVertex[3] = *pFloats++;
            
            pVertex = [ControlPointVertices Color: pVertex];

            pVertex[0] = 0.6;
            pVertex[1] = 0.4;
            pVertex[2] = 0.8;
            pVertex[3] = 1.0;
        }
    }
    
    return ControlPointVertices;
}

GLfloat *CreateControlPoints()
{
    GLfloat	*pControlPts = (GLfloat *)malloc(4 * 4 * sizeof(GLfloat) * 4);
    int		u, v;
    
    for(v=0; v<4; v++)
    {
        for(u=0; u<4; u++)
        {
            GLfloat	*pFloat = &pControlPts[v * 16 + u * 4];
            
            pFloat[0] = 2.0 * ((GLfloat)u - 1.5);
            pFloat[1] = 2.0 * ((GLfloat)v - 1.5);
            
            if ((u == 1 || u == 2) && (v == 1 || v == 2))
            {
                if((u == 1) && (v == 1))
                    pFloat[2] = 5.0;
                else if((u == 1) || (v == 1))
                    pFloat[2] = 5.0;
                else
                    pFloat[2] = -5.0;                    
            }
            else
            {
                pFloat[2] = 0.0;
            }
            
            pFloat[3] = 1.0;
        }
    }
    
    return pControlPts;
}

- (int) getPickedControlPoint: (VariableFormatVertex *) controlPoints
{
    GLint		i, nVertices;
    GLfloat		screenPoint[4];
    GLfloat		mouseXY[2];
    
    nVertices = [controlPoints vertexCount];
    
    mouseXY[0] = [m_BasicOpenGLView mouseX];
    mouseXY[1] = [m_BasicOpenGLView mouseY];
    
    for(i=0; i<nVertices; i++)
    {
        GLfloat *pVertex = [controlPoints atVertex: i];
        GLfloat	dist;
        
        [m_BasicOpenGLView projectPoint: pVertex ToScreenCoordinates: screenPoint];
        
        dist  = (mouseXY[0] - screenPoint[0]) * (mouseXY[0] - screenPoint[0]); 
        dist += (mouseXY[1] - screenPoint[1]) * (mouseXY[1] - screenPoint[1]);
        
        if(dist < 50)
            return i;
    }
    
    return -1;
}

- (IBAction) sliderSetControlPointWeight: (id) sender
{
    NSSlider	*slider;
    GLfloat	weight;
    
    slider = sender;
    
    weight = [sender floatValue];
    
    if (-1 != m_lastPickedControlPoint)
    {
        GLfloat 	*pVertex = [m_ControlPointVertices atVertex: m_lastPickedControlPoint];
    
        pVertex[3] = weight;
    }
}

- (void) timerEventHandler
{
    if (NULL == m_NURBSUVSurface)
    {
        m_NURBSUVSurface = [self CreateNURBSUVSurface: 100];
    }

    if (NULL == m_BezierUVSurface)
    {
        m_BezierUVSurface = [self CreateBezierUVSurface];
    }

    if (NULL == m_ControlPts)
    {
        m_ControlPts = CreateControlPoints();

        if (NULL == m_ControlPointVertices)
        {
            m_ControlPointVertices = [self CreateDrawableControlPoints: m_ControlPts ofRows: 4 ofCols: 4];
        }
    }
    
    if (m_BasicOpenGLView)
    {
        if ([m_BasicOpenGLView pickingMode])
        {
            GLint	pickedControlPoint;
        
            if (-1 == m_lastPickedControlPoint)
            {
                pickedControlPoint = [self getPickedControlPoint: m_ControlPointVertices];

                m_lastPickedControlPoint = pickedControlPoint;
            }
            else
            {
                pickedControlPoint = m_lastPickedControlPoint;
            }
            
            if (-1 != pickedControlPoint)
            {
                GLfloat 	*pVertex = [m_ControlPointVertices atVertex: pickedControlPoint];
                GLfloat		screenPoint[4];
                GLfloat		mouseLoc[4];
                GLfloat		dot;
                GLfloat		tempPoint[4];
                
                // Project the control point onto the screen
                [m_BasicOpenGLView projectPoint: pVertex ToScreenCoordinates: screenPoint];

                // Develop a vector from the current point to the mouse location
                mouseLoc[0] = [m_BasicOpenGLView mouseX] - screenPoint[0];
                mouseLoc[1] = [m_BasicOpenGLView mouseY] - screenPoint[1];

                // Create a new screen point to unproject
                screenPoint[0] += mouseLoc[0];
                screenPoint[1] += mouseLoc[1];
                
                // Try to unproject the point to a temporary point
                if ([m_BasicOpenGLView unprojectPoint: screenPoint ToWorldCoordinates: tempPoint])
                {
                    // unproject the tempPoint to the screen to see if it is valid
                    [m_BasicOpenGLView projectPoint: tempPoint ToScreenCoordinates: mouseLoc];
                
                    mouseLoc[0] = mouseLoc[0] - screenPoint[0];
                    mouseLoc[1] = mouseLoc[1] - screenPoint[1];
    
                    // make sure the distance is within 5 pixels before setting new point
                    dot = (mouseLoc[0] * mouseLoc[0]) + (mouseLoc[1] * mouseLoc[1]);
                    if (dot < 100)
                    {
                        pVertex[0] = tempPoint[0];
                        pVertex[1] = tempPoint[1];
                        pVertex[2] = tempPoint[2];
                    }
                }
            }
        }
        else
        {
            m_lastPickedControlPoint = -1;
        }
        
        [m_BasicOpenGLView frameBegin];

		// Draw UI
		if ([m_BasicOpenGLView pickingMode])
			[m_BasicOpenGLView drawVertexArray: m_ControlPointVertices ofType: GL_LINE_STRIP];
		else
		{
			[m_BasicOpenGLView pointSize: 4];

			[m_BasicOpenGLView drawVertexArray: m_ControlPointVertices ofType: GL_POINTS];

			[m_BasicOpenGLView pointSize: 1];
		}

		// Draw scene before UI, UI may pop you out of tcl
        [m_BasicOpenGLView enableVertexPrograms];
		
        [m_BasicOpenGLView loadControlPoints: m_ControlPointVertices];

        if ([m_BasicOpenGLView wireframeMode])
            [m_BasicOpenGLView drawVertexArray: m_NURBSUVSurface ofType: GL_LINE_STRIP];
        else
            [m_BasicOpenGLView drawVertexArray: m_NURBSUVSurface ofType: GL_QUAD_STRIP];

        [m_BasicOpenGLView disableVertexPrograms];

        [m_BasicOpenGLView drawAxis];

        [m_BasicOpenGLView frameEnd];
    }
}

@end
