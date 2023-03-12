/*
 *  stereoTest.cpp
 *  GLUTTest
 *
 *  Created by GGS on Wed Nov 06 2002.
 *  Copyright (c) 2002 Apple. All rights reserved.
 *

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
 
 /* Uses techniques described by Paul Bourke 1999 - 2002 */
 /* including parallel axis asymmetric frustum perspective projection */
 /* see: <http://astronomy.swin.edu.au/~pbourke/opengl/stereogl/> */
 
 /* Tranguloid Trefoil example surface by Roger Bagula see <http://astronomy.swin.edu.au/~pbourke/surfaces/tranguloid/> */

 
#include <iostream>

using std::cout;
using std::endl;

#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h>

#include "trackball.h"

#define CROSSPROD(p1,p2,p3) \
   p3.x = p1.y*p2.z - p1.z*p2.y; \
   p3.y = p1.z*p2.x - p1.x*p2.z; \
   p3.z = p1.x*p2.y - p1.y*p2.x
   
#define EPS             0.00001

#define DTOR            0.0174532925
#define RTOD            57.2957795
#define TWOPI           6.283185307179586476925287
#define PI              3.141592653589793238462643

typedef struct {
   GLdouble x,y,z;
} recVec;

typedef struct {
   GLdouble r,g,b;
} recColor;

typedef struct {
	recVec viewPos; // View position
	recVec viewDir; // View direction vector
	recVec viewUp; // View up direction
	recVec rotPoint; // Point to rotate about
	GLdouble focalLength; // Focal Length along view direction
	GLdouble aperture; // gCamera aperture
	GLdouble eyeSep; // Eye separation
	GLint screenWidth,screenHeight; // current window/screen height and width
} recCamera;

GLuint gColorScheme = 1;
GLuint gSubDivisions = 64;
GLuint gIJRatio = 3;
recVec *gVertexPos = NULL,*gVertexNormal = NULL;
recColor *gVertexColor = NULL;
GLuint gPolyList = 0;
GLuint gLineList = 0;
GLuint gPointList = 0;

char gSurfName[256] = "";
char gSurfCredit[256] = "";
char gSurfX[256] = "";
char gSurfY[256] = "";
char gSurfZ[256] = "";
char gSurfRange[256] = "";

int main_window = NULL;

GLint gDollyPanStartPoint[2] = {0, 0};
GLfloat gTrackBallRotation [4] = {0.0, 0.0, 0.0, 0.0};
GLboolean gDolly = GL_FALSE;
GLboolean gPan = GL_FALSE;
GLboolean gTrackBall = GL_FALSE;
GLfloat gWorldRotation [4] = {100.0, -0.7, 0.6, 0.5};

GLboolean gStereo = GL_FALSE;
GLboolean gPoints = GL_FALSE;
GLboolean gLines = GL_FALSE;
GLboolean gPolygons = GL_TRUE;
GLboolean gShowHelp = GL_TRUE;
GLboolean gShowInfo = GL_TRUE;
GLboolean gShowCredits = GL_TRUE;
GLboolean gLighting = 4;
GLint gSurface = 0;

GLdouble gShapeSize = 11.0;
recCamera gCamera;
recVec gOrigin = {0.0, 0.0, 0.0};

GLboolean gDragStart = false;

int gLastKey = ' ';

int gMainWindow = 0;


#pragma mark ---- gCamera control -----

void gCameraReset(void)
{
   gCamera.aperture = 50;
   gCamera.focalLength = 9;
   gCamera.eyeSep = gCamera.focalLength / 20;
   gCamera.rotPoint = gOrigin;

   gCamera.viewPos.x = 0.0;
   gCamera.viewPos.y = 0.0;
   gCamera.viewPos.z = -gCamera.focalLength;
   gCamera.viewDir.x = -gCamera.viewPos.x; 
   gCamera.viewDir.y = -gCamera.viewPos.y; 
   gCamera.viewDir.z = -gCamera.viewPos.z;

   gCamera.viewUp.x = 0;  
   gCamera.viewUp.y = 1; 
   gCamera.viewUp.z = 0;
}

#pragma mark ---- Utilities -----

void
DrawString(GLfloat x, GLfloat y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
  }
}

void normalise(recVec *p)
{
   double length;

   length = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
   if (length != 0) {
      p->x /= length;
      p->y /= length;
      p->z /= length;
   } else {
      p->x = 0;
      p->y = 0;
      p->z = 0;
   }
}

recVec CalcNormal(recVec p,recVec p1,recVec p2)
{
   recVec n,pa,pb;

   pa.x = p1.x - p.x;
   pa.y = p1.y - p.y;
   pa.z = p1.z - p.z;
   pb.x = p2.x - p.x;
   pb.y = p2.y - p.y;
   pb.z = p2.z - p.z;
   normalise(&pa);
   normalise(&pb);
  
   n.x = pa.y * pb.z - pa.z * pb.y;
   n.y = pa.z * pb.x - pa.x * pb.z;
   n.z = pa.x * pb.y - pa.y * pb.x;
   normalise(&n);

   return(n);
}

/* Based on GetColour by Paul Bourke */

recColor GetColor(double v,double vmin,double vmax,int type)
{
   double dv,vmid;
	recColor c = {1.0,1.0,1.0};
	recColor c1,c2,c3;
	double ratio;

   if (v < vmin)
      v = vmin;
   if (v > vmax)
      v = vmax;
   dv = vmax - vmin;

	switch (type) {
	case 1:
   	if (v < (vmin + 0.25 * dv)) {
      	c.r = 0;
      	c.g = 4 * (v - vmin) / dv;
			c.b = 1;
   	} else if (v < (vmin + 0.5 * dv)) {
      	c.r = 0;
			c.g = 1;
      	c.b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
   	} else if (v < (vmin + 0.75 * dv)) {
      	c.r = 4 * (v - vmin - 0.5 * dv) / dv;
			c.g = 1;
      	c.b = 0;
   	} else {
			c.r = 1;
      	c.g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
      	c.b = 0;
   	}
		break;
	case 2:
		c.r = (v - vmin) / dv;
		c.g = 0;
		c.b = (vmax - v) / dv;
		break;
	case 3:
		c.r = (v - vmin) / dv;
		c.b = c.r;
		c.g = c.r;
		break;
	case 4:
      if (v < (vmin + dv / 6.0)) {
         c.r = 1; 
         c.g = 6 * (v - vmin) / dv;
         c.b = 0;
      } else if (v < (vmin + 2.0 * dv / 6.0)) {
         c.r = 1 + 6 * (vmin + dv / 6.0 - v) / dv;
         c.g = 1;
         c.b = 0;
      } else if (v < (vmin + 3.0 * dv / 6.0)) {
         c.r = 0;
         c.g = 1;
         c.b = 6 * (v - vmin - 2.0 * dv / 6.0) / dv;
      } else if (v < (vmin + 4.0 * dv / 6.0)) {
         c.r = 0;
         c.g = 1 + 6 * (vmin + 3.0 * dv / 6.0 - v) / dv;
         c.b = 1;
      } else if (v < (vmin + 5.0 * dv / 6.0)) {
         c.r = 6 * (v - vmin - 4.0 * dv / 6.0) / dv;
         c.g = 0;
         c.b = 1;
      } else {
         c.r = 1;
         c.g = 0;
         c.b = 1 + 6 * (vmin + 5.0 * dv / 6.0 - v) / dv;
      }
		break;
   case 5:
      c.r = (v - vmin) / (vmax - vmin);
      c.g = 1;
      c.b = 0;
		break;
   case 6:
      c.r = (v - vmin) / (vmax - vmin);
      c.g = (vmax - v) / (vmax - vmin);
      c.b = c.r;
		break;
   case 7:
      if (v < (vmin + 0.25 * dv)) {
         c.r = 0;
         c.g = 4 * (v - vmin) / dv;
         c.b = 1 - c.g;
      } else if (v < (vmin + 0.5 * dv)) {
			c.r = 4 * (v - vmin - 0.25 * dv) / dv;
         c.g = 1 - c.r;
         c.b = 0;
      } else if (v < (vmin + 0.75 * dv)) {
         c.g = 4 * (v - vmin - 0.5 * dv) / dv;
			c.r = 1 - c.g;
         c.b = 0;
      } else {
         c.r = 0;
         c.b = 4 * (v - vmin - 0.75 * dv) / dv;
			c.g = 1 - c.b;
      }
      break;
   case 8:
      if (v < (vmin + 0.5 * dv)) {
         c.r = 2 * (v - vmin) / dv;
         c.g = c.r;
         c.b = c.r;
      } else {
         c.r = 1 - 2 * (v - vmin - 0.5 * dv) / dv;
			c.g = c.r;
         c.b = c.r;
      }
      break;
   case 9:
      if (v < (vmin + dv / 3)) {
         c.b = 3 * (v - vmin) / dv;
			c.g = 0;
         c.r = 1 - c.b;
      } else if (v < (vmin + 2 * dv / 3)) {
         c.r = 0;
         c.g = 3 * (v - vmin - dv / 3) / dv;
         c.b = 1;
      } else {
         c.r = 3 * (v - vmin - 2 * dv / 3) / dv;
         c.g = 1 - c.r;
			c.b = 1;
      }
      break;
   case 10:
      if (v < (vmin + 0.2 * dv)) {
         c.r = 0;
         c.g = 5 * (v - vmin) / dv;
         c.b = 1;
      } else if (v < (vmin + 0.4 * dv)) {
         c.r = 0;
         c.g = 1;
         c.b = 1 + 5 * (vmin + 0.2 * dv - v) / dv;
      } else if (v < (vmin + 0.6 * dv)) {
         c.r = 5 * (v - vmin - 0.4 * dv) / dv;
         c.g = 1;
         c.b = 0;
      } else if (v < (vmin + 0.8 * dv)) {
         c.r = 1;
         c.g = 1 - 5 * (v - vmin - 0.6 * dv) / dv;
         c.b = 0;
      } else {
         c.r = 1;
         c.g = 5 * (v - vmin - 0.8 * dv) / dv;
         c.b = 5 * (v - vmin - 0.8 * dv) / dv;
      }
      break;
   case 11:
		c1.r = 200 / 255.0; c1.g =  60 / 255.0; c1.b =   0 / 255.0;
		c2.r = 250 / 255.0; c2.g = 160 / 255.0; c2.b = 110 / 255.0;
      c.r = (c2.r - c1.r) * (v - vmin) / dv + c1.r;
      c.g = (c2.g - c1.g) * (v - vmin) / dv + c1.g;
      c.b = (c2.b - c1.b) * (v - vmin) / dv + c1.b;
      break;
	case 12:
		c1.r =  55 / 255.0; c1.g =  55 / 255.0; c1.b =  45 / 255.0;
//      c2.r = 200 / 255.0; c2.g =  60 / 255.0; c2.b =   0 / 255.0; 
		c2.r = 235 / 255.0; c2.g =  90 / 255.0; c2.b =  30 / 255.0;
		c3.r = 250 / 255.0; c3.g = 160 / 255.0; c3.b = 110 / 255.0;
		ratio = 0.4;
		vmid = vmin + ratio * dv;
		if (v < vmid) {
      	c.r = (c2.r - c1.r) * (v - vmin) / (ratio*dv) + c1.r;
      	c.g = (c2.g - c1.g) * (v - vmin) / (ratio*dv) + c1.g;
      	c.b = (c2.b - c1.b) * (v - vmin) / (ratio*dv) + c1.b;
		} else {
         c.r = (c3.r - c2.r) * (v - vmid) / ((1-ratio)*dv) + c2.r;
         c.g = (c3.g - c2.g) * (v - vmid) / ((1-ratio)*dv) + c2.g;
         c.b = (c3.b - c2.b) * (v - vmid) / ((1-ratio)*dv) + c2.b;
		}
		break;
	case 13:
      c1.r =   0 / 255.0; c1.g = 255 / 255.0; c1.b =   0 / 255.0;
      c2.r = 255 / 255.0; c2.g = 150 / 255.0; c2.b =   0 / 255.0;
      c3.r = 255 / 255.0; c3.g = 250 / 255.0; c3.b = 240 / 255.0;
      ratio = 0.3;
      vmid = vmin + ratio * dv;
      if (v < vmid) {
         c.r = (c2.r - c1.r) * (v - vmin) / (ratio*dv) + c1.r;
         c.g = (c2.g - c1.g) * (v - vmin) / (ratio*dv) + c1.g;
         c.b = (c2.b - c1.b) * (v - vmin) / (ratio*dv) + c1.b;
      } else {
         c.r = (c3.r - c2.r) * (v - vmid) / ((1-ratio)*dv) + c2.r;
         c.g = (c3.g - c2.g) * (v - vmid) / ((1-ratio)*dv) + c2.g;
         c.b = (c3.b - c2.b) * (v - vmid) / ((1-ratio)*dv) + c2.b;
      }
		break;
   case 14:
      c.r = 1;
      c.g = 1 - (v - vmin) / dv;
      c.b = 0;
      break;
   case 15:
      if (v < (vmin + 0.25 * dv)) {
         c.r = 0;
         c.g = 4 * (v - vmin) / dv;
         c.b = 1;
      } else if (v < (vmin + 0.5 * dv)) {
         c.r = 0;
         c.g = 1;
         c.b = 1 - 4 * (v - vmin - 0.25 * dv) / dv;
      } else if (v < (vmin + 0.75 * dv)) {
			c.r = 4 * (v - vmin - 0.5 * dv) / dv;
         c.g = 1;
         c.b = 0;
      } else {
         c.r = 1;
			c.g = 1;
         c.b = 4 * (v - vmin - 0.75 * dv) / dv;
      }
      break;
   case 16:
      if (v < (vmin + 0.5 * dv)) {
         c.r = 0.0;
         c.g = 2 * (v - vmin) / dv;
         c.b = 1 - 2 * (v - vmin) / dv;
      } else {
         c.r = 2 * (v - vmin - 0.5 * dv) / dv;
         c.g = 1 - 2 * (v - vmin - 0.5 * dv) / dv;
         c.b = 0.0;
      }
      break;
   case 17:
      if (v < (vmin + 0.5 * dv)) {
         c.r = 1.0;
         c.g = 1 - 2 * (v - vmin) / dv;
         c.b = 2 * (v - vmin) / dv;
      } else {
         c.r = 1 - 2 * (v - vmin - 0.5 * dv) / dv;
         c.g = 2 * (v - vmin - 0.5 * dv) / dv;
         c.b = 1.0;
      }
      break;
   case 18:
      c.r = 0;
      c.g = (v - vmin) / (vmax - vmin);
      c.b = 1;
      break;
   case 19:
      c.r = (v - vmin) / (vmax - vmin);
      c.g = c.r;
      c.b = 1;
      break;
   case 20:
      c1.r =   0 / 255.0; c1.g = 160 / 255.0; c1.b =   0 / 255.0;
      c2.r = 180 / 255.0; c2.g = 220 / 255.0; c2.b =   0 / 255.0;
      c3.r = 250 / 255.0; c3.g = 220 / 255.0; c3.b = 170 / 255.0;
      ratio = 0.3;
      vmid = vmin + ratio * dv;
      if (v < vmid) {
         c.r = (c2.r - c1.r) * (v - vmin) / (ratio*dv) + c1.r;
         c.g = (c2.g - c1.g) * (v - vmin) / (ratio*dv) + c1.g;
         c.b = (c2.b - c1.b) * (v - vmin) / (ratio*dv) + c1.b;
      } else {
         c.r = (c3.r - c2.r) * (v - vmid) / ((1-ratio)*dv) + c2.r;
         c.g = (c3.g - c2.g) * (v - vmid) / ((1-ratio)*dv) + c2.g;
         c.b = (c3.b - c2.b) * (v - vmid) / ((1-ratio)*dv) + c2.b;
      }
      break;
	}
	return(c);
}

#pragma mark ---- Drawing ----

void SetLighting(void)
{
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {90.0};

	GLfloat position[4] = {7.0,-7.0,12.0,0.0};
	GLfloat ambient[4]  = {0.2,0.2,0.2,1.0};
	GLfloat diffuse[4]  = {1.0,1.0,1.0,1.0};
	GLfloat specular[4] = {1.0,1.0,1.0,1.0};
	
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

	switch (gLighting) {
		case 0:
			break;
		case 1:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 2:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
		case 3:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 4:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
	}
	
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glEnable(GL_LIGHT0);
}


/* Code based on work by Paul Bourke */

#define kSurfaces 5

// expects u & v (-PI to PI)
recVec Eval(double u, double v, int type)
{
	recVec p;
	double temp;
   
	switch (type) {
		case 0:
			sprintf (gSurfName, "Tranguloid Trefoil");
			sprintf (gSurfCredit, "by Roger Bagula");
			sprintf (gSurfX, "x = 2 sin(3 u) / (2 + cos(v)) )");
			sprintf (gSurfY, "y = 2 (sin(u) + 2 sin(2 u)) / (2 + cos(v + 2 pi / 3)) ");
			sprintf (gSurfZ, "z = (cos(u) - 2 cos(2 u)) (2 + cos(v)) (2 + cos(v + 2 pi / 3)) / 4");
			sprintf (gSurfRange, "-pi <= u <= pi, -pi <= v <= pi ");
			p.x = sin(3*u) * 2 / (2 + cos(v));
			p.y = (sin(u) + 2 * sin(2*u)) * 2 / (2 + cos(v + TWOPI / 3));
			p.z = (cos(u) - 2 * cos(2*u)) * (2 + cos(v)) * (2 + cos(v + TWOPI/3))/4;
		break;
		case 1:
			sprintf (gSurfName, "Triaxial Tritorus");
			sprintf (gSurfCredit, "by Roger Bagula");
			sprintf (gSurfX, "x = sin(u) (1 + cos(v))");
			sprintf (gSurfY, "y = sin(u + 2pi / 3) (1 + cos(v + 2pi / 3)) ");
			sprintf (gSurfZ, "z = sin(u + 4pi / 3) (1 + cos(v + 4pi / 3))");
			sprintf (gSurfRange, "0 <= u <= 2 pi, 0 <= v <= 2 pi");
			p.x = sin (u) * (1 + cos (v));
			p.y = sin (u + 2 * PI / 3) * (1 + cos (v + 2 * PI / 3));
			p.z = sin (u + 4 * PI / 3) * (1 + cos (v + 4 * PI / 3));
		break;
		case 2:
			sprintf (gSurfName, "Stiletto Surface");
			sprintf (gSurfCredit, "by Roger Bagula");
			sprintf (gSurfX, "x =  (2 + cos(u)) cos(v)^3 sin(v)");
			sprintf (gSurfY, "y =  (2 + cos(u + 2pi /3)) cos(v + 2pi / 3)^2 sin(v + 2pi / 3)^2");
			sprintf (gSurfZ, "z = -(2 + cos(u - 2pi / 3)) cos(v + 2pi / 3)^2 sin(v + 2pi / 3)^2");
			sprintf (gSurfRange, "0 <= u <= 2 pi, 0 <= v <= 2 pi");
			// reverse u and v for better distribution or points
			temp = u;
			u = v + PI; v = temp + PI; // convert to: 0 <= u <= 2 pi, 0 <= v <= 2 pi 
			p.x =  (2 + cos(u)) * pow(cos(v), 3) * sin(v);
			p.y =  (2 + cos(u+TWOPI/3)) * pow (cos(v+TWOPI/3), 2) * pow (sin(v+TWOPI/3), 2);
			p.z = -(2 + cos(u-TWOPI/3)) * pow (cos(v+TWOPI/3), 2) * pow (sin(v+TWOPI/3), 2);
 		break;
		case 3:
			sprintf (gSurfName, "Slippers Surface");
			sprintf (gSurfCredit, "by Roger Bagula");
			sprintf (gSurfX, "x =  (2 + cos(u)) cos(v)^3 sin(v)");
			sprintf (gSurfY, "y =  (2 + cos(u + 2pi / 3)) cos(2pi / 3 + v)^2 sin(2pi / 3 + v)^2");
			sprintf (gSurfZ, "z = -(2 + cos(u - 2pi / 3)) cos(2pi / 3 - v)^2 sin(2pi / 3 - v)^3");
			sprintf (gSurfRange, "0 <= u <= 2 pi, 0 <= v <= 2 pi");
			temp = u;
			u = v + PI * 2; v = temp + PI; // convert to: 0 <= u <= 4 pi, 0 <= v <= 2 pi 
			p.x =  (2 + cos (u)) * pow (cos (v), 3) * sin(v);
			p.y =  (2 + cos (u + TWOPI / 3)) * pow (cos (TWOPI / 3 + v), 2) * pow (sin (TWOPI / 3 + v), 2);
			p.z = -(2 + cos (u - TWOPI / 3)) * pow (cos (TWOPI / 3 - v), 2) * pow (sin (TWOPI / 3 - v), 3);
		break;
		case 4:
			sprintf (gSurfName, "Maeder's Owl");
			sprintf (gSurfCredit, "by R. Maeder");
			sprintf (gSurfX, "x = v cos(u) - 0.5 v^2 cos(2 u)");
			sprintf (gSurfY, "y = - v sin(u) - 0.5 v^2 sin(2 u) ");
			sprintf (gSurfZ, "z = 4 v^1.5 cos(3 u / 2) / 3 ");
			sprintf (gSurfRange, "0 <= u <= 4 pi, 0 <= v <= 1");
			u = (u + PI) * 2; v = (v + PI) / TWOPI; // convert to: 0 <= u <= 4 pi, 0 <= v <= 1 
			p.x = v * cos(u) - 0.5 * v * v * cos(2 * u);
			p.y = -v * sin(u) - 0.5 * v * v * sin(2 * u);
			p.z = 4 * pow(v,1.5) * cos(1.5 * u) / 3;
		break;
		
		
		

         p.x = v * cos(u) - 0.5 * v * v * cos(2 * u);
         p.y = - v * sin(u) - 0.5 * v * v * sin(2 * u);
         p.z = 4 * pow(v,1.5) * cos(1.5 * u) / 3;
 	}


   return(p);
}

void BuildGeometry(void)
{
	long i,j, index;
	long maxI = gSubDivisions * gIJRatio, maxJ = gSubDivisions;
	double u, v, delta=0.001;
	recVec p1,p2;

	if (gVertexPos)
		free (gVertexPos);
	gVertexPos = (recVec*) malloc ((maxI) * (maxJ) * sizeof (recVec));
	if (gVertexNormal)
		free (gVertexNormal);
	gVertexNormal = (recVec*) malloc ((maxI) * (maxJ) * sizeof (recVec));
	if (gVertexColor)
		free (gVertexColor);
	gVertexColor = (recColor*) malloc ((maxI) * (maxJ) * sizeof (recColor));

	for (i = 0; i < maxI; i++) {
		for (j = 0; j < maxJ; j++) {
			index = i * maxJ + j;
			u  = -PI + (i % maxI) * TWOPI / maxI;
			v  = -PI + (j % maxJ) * TWOPI / maxJ;
			gVertexPos[index] = Eval(u,v, gSurface);
			p1 = Eval(u + delta, v, gSurface);
			p2 = Eval(u, v + delta, gSurface);
			gVertexNormal[index] = CalcNormal(gVertexPos[index],p1,p2);
			gVertexColor[index] = GetColor(u, -PI, PI, gColorScheme);
		}
	}
	if (gPolyList)
		glDeleteLists (gPolyList, 1);
	gPolyList = glGenLists (1);
	glNewList(gPolyList, GL_COMPILE);
		for (i=0; i< maxI; i++) {
			glBegin(GL_TRIANGLE_STRIP);
			for (j = 0; j <= maxJ; j++) {
				index = (i % maxI) * maxJ + (j % maxJ);
				glColor3f (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
				glNormal3f (gVertexNormal[index].x, gVertexNormal[index].y, gVertexNormal[index].z);
				glVertex3f (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
	
				index = ((i + 1) % maxI) * maxJ + (j % maxJ);
				glColor3f (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
				glNormal3f (gVertexNormal[index].x, gVertexNormal[index].y, gVertexNormal[index].z);
				glVertex3f (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
			}
			glEnd ();
		}
	glEndList ();

	if (gLineList)
		glDeleteLists (gLineList, 1);
	gLineList = glGenLists (1);
	glNewList(gLineList, GL_COMPILE);
		for (i=0; i< maxI; i++) {
			glBegin(GL_LINE_STRIP);
			for (j = 0; j < maxJ; j++) {
				index = i * maxJ + j;
				glColor3d (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
				glVertex3d (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
			}
			index = i * maxJ + 0;
			glColor3d (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
			glVertex3d (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
			glEnd ();
		}
		for (j=0; j< maxJ; j++) {
			glBegin(GL_LINE_STRIP);
			for (i = 0; i < maxI; i++) {
				index = i * maxJ + j;
				glColor3d (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
				glVertex3d (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
			}
			index = 0 + j;
			glColor3d (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
			glVertex3d (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
			glEnd ();
		}
	glEndList ();

	if (gPointList)
		glDeleteLists (gPointList, 1);
	gPointList = glGenLists (1);
	glNewList(gPointList, GL_COMPILE);
		glBegin(GL_POINTS);
		for (i=0; i< maxI; i++) {
			for (j = 0; j < maxJ; j++) {
				index = i * maxJ + j;
				glColor3d (gVertexColor[index].r, gVertexColor[index].g, gVertexColor[index].b);
				glVertex3d (gVertexPos[index].x, gVertexPos[index].y, gVertexPos[index].z);
			}
		}
		glEnd ();
	glEndList ();
}

void DrawText (GLint window_width, GLint window_height)
{
	char outString [256] = "";
	GLint matrixMode;
	GLint vp[4];
	GLint lineSpacing = 13;
	GLint line = 0;
	GLint startOffest = 7;
	
	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(0, 0, window_width, window_height);
	
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(2.0f / window_width, -2.0f / window_height, 1.0f);
	glTranslatef(-window_width / 2.0f, -window_height / 2.0f, 0.0f);
	
	// draw 
	glColor3f (1.0, 1.0, 0.0);
	if (gShowInfo) {
	
		sprintf (outString, "Camera Position: (%0.1f, %0.1f, %0.1f)", gCamera.viewPos.x, gCamera.viewPos.y, gCamera.viewPos.z);
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Trackball Rotation: (%0.1f, %0.2f, %0.2f, %0.2f)", gTrackBallRotation[0], gTrackBallRotation[1], gTrackBallRotation[2], gTrackBallRotation[3]);
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "World Rotation: (%0.1f, %0.2f, %0.2f, %0.2f)", gWorldRotation[0], gWorldRotation[1], gWorldRotation[2], gWorldRotation[3]);
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Aperture: %0.1f", gCamera.aperture);
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		if (gStereo)
			sprintf (outString, "Eye Separation: %0.2f", gCamera.eyeSep);
		else
			sprintf (outString, "Eye Separation: 0.0");
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Focus Distance: %0.1f", gCamera.focalLength);
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Vertices: %ld, Color Scheme: %ld", gSubDivisions * gIJRatio * gSubDivisions, gColorScheme);
		DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		{
			GLboolean lighting, twoSidedLighting, localViewer;
			lighting = glIsEnabled (GL_LIGHTING);
			glGetBooleanv (GL_LIGHT_MODEL_LOCAL_VIEWER, &localViewer);
			glGetBooleanv (GL_LIGHT_MODEL_TWO_SIDE, &twoSidedLighting);
			if (!gLighting) {
				sprintf (outString, "-- Lighting off");
			} else {
				if (!twoSidedLighting)
					sprintf (outString, "-- Single Sided Lighting");
				else
					sprintf (outString, "-- Two Sided Lighting");
				if (localViewer)
					sprintf (outString, "%s: Local Viewer", outString);
			}	
			DrawString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		}
	}
	
	if (gShowHelp) {
		line = 1;
		sprintf (outString, "Controls:\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "left button drag: rotate camera\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "right (or crtl-left) button drag: dolly (zoom) camera\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "arrows: eye separation & focal length\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "-/+: aperture\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "[/]: cycle color scheme\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, ";/': decrease/increase subdivisions\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "\\: cycle surface type\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "P: toggle points\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "W: toggle wireframe\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "F: toggle fill\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "L: toggle lighting\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "S: toggle stereo\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "H: toggle help\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "I: toggle info\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "C: toggle surface credits\n");
		DrawString (10, (lineSpacing * line++) + startOffest, outString);
	}
	
	if (gShowCredits) {
		#define khStart 350
		line = 1;
		glColor3f (1.0, 1.0, 1.0);
		DrawString (window_width - khStart, (lineSpacing * line++) + startOffest, gSurfName);	
		DrawString (window_width - khStart, (lineSpacing * line++) + startOffest, gSurfCredit);	
		glColor3f (0.7, 0.7, 0.7);
		DrawString (window_width - khStart, (lineSpacing * line++) + startOffest, gSurfX);	
		DrawString (window_width - khStart, (lineSpacing * line++) + startOffest, gSurfY);	
		DrawString (window_width - khStart, (lineSpacing * line++) + startOffest, gSurfZ);	
		DrawString (window_width - khStart, (lineSpacing * line++) + startOffest, gSurfRange);	
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(matrixMode);
	
	glViewport(vp[0], vp[1], vp[2], vp[3]);
}

#pragma mark ---- Blue line syncing code ----

void DrawBlueLine(GLint window_width, GLint window_height)
{
	GLint i;
	unsigned long buffer;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	for(i = 0; i < 6; i++) glDisable(GL_CLIP_PLANE0 + i);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_SHARED_TEXTURE_PALETTE_EXT);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_TEXTURE_RECTANGLE_EXT);
	glDisable(GL_VERTEX_PROGRAM_ARB);
		
	for(buffer = GL_BACK_LEFT; buffer <= GL_BACK_RIGHT; buffer++) {
		GLint matrixMode;
		GLint vp[4];
		
		glDrawBuffer(buffer);
		
		glGetIntegerv(GL_VIEWPORT, vp);
		glViewport(0, 0, window_width, window_height);
		
		glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glScalef(2.0f / window_width, -2.0f / window_height, 1.0f);
		glTranslatef(-window_width / 2.0f, -window_height / 2.0f, 0.0f);
	
		// draw sync lines
		glColor3d(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES); // Draw a background line
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			glVertex3f(window_width, window_height - 0.5f, 0.0f);
		glEnd();
		glColor3d(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES); // Draw a line of the correct length (the cross over is about 40% across the screen from the left
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			if(buffer == GL_BACK_LEFT)
				glVertex3f(window_width * 0.30f, window_height - 0.5f, 0.0f);
			else
				glVertex3f(window_width * 0.80f, window_height - 0.5f, 0.0f);
		glEnd();
	
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(matrixMode);
		
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}	
	glPopAttrib();
}

void DrawBlueLine_Simple(GLint window_width, GLint window_height)
{
	unsigned long buffer;
	
	for(buffer = GL_BACK_LEFT; buffer <= GL_BACK_RIGHT; buffer++) {
		GLint matrixMode;
		GLint vp[4];
		
		glDrawBuffer(buffer);
		
		glGetIntegerv(GL_VIEWPORT, vp);
		glViewport(0, 0, window_width, window_height);
		
		glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glScalef(2.0f / window_width, -2.0f / window_height, 1.0f);
		glTranslatef(-window_width / 2.0f, -window_height / 2.0f, 0.0f);
	
		// draw sync lines
		glColor3d(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES); // Draw a background line
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			glVertex3f(window_width, window_height - 0.5f, 0.0f);
		glEnd();
		glColor3d(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES); // Draw a line of the correct length (the cross over is about 40% across the screen from the left
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			if(buffer == GL_BACK_LEFT)
				glVertex3f(window_width * 0.30f, window_height - 0.5f, 0.0f);
			else
				glVertex3f(window_width * 0.80f, window_height - 0.5f, 0.0f);
			glVertex3f(window_width, window_height - 0.5f, 0.0f);
		glEnd();
	
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(matrixMode);
		
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}
}


#pragma mark ---- GLUT callbacks ----

void init (void)
{
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);    
	glDisable(GL_DITHER);
	glDisable(GL_CULL_FACE);
	
	glLineWidth(1.0);
	glPointSize(1.0);
	
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glFrontFace(GL_CCW);
	glClearColor(0.0,0.0,0.0,0.0);         /* Background recColor */
	gCameraReset ();
	
	SetLighting ();
	BuildGeometry ();
}

void reshape (int w, int h)
{
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	gCamera.screenWidth = w;
	gCamera.screenHeight = h;
	glutPostRedisplay();
}

/* uses parallel axis asymmetric frustum perspective projection for correct stereo */

void display(void)
{
	recVec r;
	GLdouble ratio, radians, wd2, ndfl;
	GLdouble left, right, top, bottom, near, far;

	if (gStereo && (glutGetWindow () == gMainWindow)) // do not draw window while in stereo mode
		return;
	near = -gCamera.viewPos.z - gShapeSize * 0.5;
	if (near < 0.1) near = 0.1;
	far = -gCamera.viewPos.z + gShapeSize * 0.5;

	// Misc stuff
	ratio  = gCamera.screenWidth / (double)gCamera.screenHeight;
	radians = DTOR * gCamera.aperture / 2;
	wd2     = near * tan(radians);
	ndfl    = near / gCamera.focalLength;

	// Derive the two eye positions
	CROSSPROD (gCamera.viewDir, gCamera.viewUp, r);
	normalise (&r);
	if (gStereo) {
		r.x *= gCamera.eyeSep / 3;
		r.y *= gCamera.eyeSep / 3;
		r.z *= gCamera.eyeSep / 3;
	} else {
		r.x = 0.0;
		r.y = 0.0;
		r.z = 0.0;
	}

	// Left:		
	glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	if (gStereo) {
		left  = - ratio * wd2 + 0.5 * gCamera.eyeSep * ndfl;
		right =   ratio * wd2 + 0.5 * gCamera.eyeSep * ndfl;
	} else {
		left  = - ratio * wd2;
		right =   ratio * wd2;
	}
	top    =   wd2;
	bottom = - wd2;
	glFrustum (left, right, bottom, top, near, far);
	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	gluLookAt (gCamera.viewPos.x - r.x, gCamera.viewPos.y - r.y, gCamera.viewPos.z - r.z,
			gCamera.viewPos.x - r.x + gCamera.viewDir.x,
			gCamera.viewPos.y - r.y + gCamera.viewDir.y,
			gCamera.viewPos.z - r.z + gCamera.viewDir.z,
			gCamera.viewUp.x, gCamera.viewUp.y ,gCamera.viewUp.z);
			
	// track ball rotation
	glRotatef (gTrackBallRotation[0], gTrackBallRotation[1], gTrackBallRotation[2], gTrackBallRotation[3]);
	glRotatef (gWorldRotation[0], gWorldRotation[1], gWorldRotation[2], gWorldRotation[3]);
	
	if (gPolygons) {
		if (gLighting)
			glEnable(GL_LIGHTING);
		else 
			glDisable(GL_LIGHTING);
		glCallList (gPolyList);
		glDisable(GL_LIGHTING);
	} else if (gLines) {
		glDisable(GL_LIGHTING);
		glCallList (gLineList);
	} else if (gPoints) {
		glDisable(GL_LIGHTING);
		glCallList (gPointList);
	}
	DrawText (gCamera.screenWidth, gCamera.screenHeight);

	// Right:
	glDrawBuffer(GL_BACK_RIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (gStereo) {
		left  = - ratio * wd2 - 0.5 * gCamera.eyeSep * ndfl;
		right =   ratio * wd2 - 0.5 * gCamera.eyeSep * ndfl;
	} else {
		left  = - ratio * wd2;
		right =   ratio * wd2;
	}
	top    =   wd2;
	bottom = - wd2;
	glFrustum (left, right, bottom, top, near, far);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(gCamera.viewPos.x + r.x, gCamera.viewPos.y + r.y, gCamera.viewPos.z + r.z,
			gCamera.viewPos.x + r.x + gCamera.viewDir.x,
			gCamera.viewPos.y + r.y + gCamera.viewDir.y,
			gCamera.viewPos.z + r.z + gCamera.viewDir.z,
			gCamera.viewUp.x, gCamera.viewUp.y, gCamera.viewUp.z);
			
	// track ball rotation
	glRotatef (gTrackBallRotation[0], gTrackBallRotation[1], gTrackBallRotation[2], gTrackBallRotation[3]);
	glRotatef (gWorldRotation[0], gWorldRotation[1], gWorldRotation[2], gWorldRotation[3]);
	
	if (gPolygons) {
		if (gLighting)
			glEnable(GL_LIGHTING);
		else 
			glDisable(GL_LIGHTING);
		glCallList (gPolyList);
		glDisable(GL_LIGHTING);
	} else if (gLines) {
		glDisable(GL_LIGHTING);
		glCallList (gLineList);
	} else if (gPoints) {
		glDisable(GL_LIGHTING);
		glCallList (gPointList);
	}
	DrawText (gCamera.screenWidth, gCamera.screenHeight);
			
	// Draw blue synch line:
	if (gStereo)
		DrawBlueLine (gCamera.screenWidth, gCamera.screenHeight);

	glutSwapBuffers();
}

void animate(void)
{
//		glutPostRedisplay();
}

void special(int key, int px, int py)
{
  gLastKey = key;
  switch (key) {
	case GLUT_KEY_UP: // arrow forward, close in on world
		gCamera.focalLength -= 0.5f;
		if (gCamera.focalLength < 0.0f)
			gCamera.focalLength = 0.0f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN: // arrow back, back away from world
		gCamera.focalLength += 0.5f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT: // arrow left, close eyes together
		gCamera.eyeSep -= 0.05f;
		if (gCamera.eyeSep < 0.0)
			gCamera.eyeSep = 0.0;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT: // arrow right, spread eyes apart
		gCamera.eyeSep += 0.05f;
		glutPostRedisplay();
		break;
  }
}

void mouseDolly (int x, int y)
{
	if (gDolly) {
		GLfloat dolly = (gDollyPanStartPoint[1] - y) * -gCamera.viewPos.z / 200.0f;
		GLfloat eyeRelative = gCamera.eyeSep / gCamera.focalLength;
		gCamera.focalLength += gCamera.focalLength / gCamera.viewPos.z * dolly; 
		if (gCamera.focalLength < 1.0)
			gCamera.focalLength = 1.0;
		gCamera.eyeSep = gCamera.focalLength * eyeRelative;
		gCamera.viewPos.z += dolly;
		if (gCamera.viewPos.z == 0.0) // do not let z = 0.0
			gCamera.viewPos.z = 0.0001;
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutPostRedisplay();
	}
}

void mousePan (int x, int y)
{
	if (gPan) {
		GLfloat panX = (gDollyPanStartPoint[0] - x) / (900.0f / -gCamera.viewPos.z);
		GLfloat panY = (gDollyPanStartPoint[1] - y) / (900.0f / -gCamera.viewPos.z);
		gCamera.viewPos.x -= panX;
		gCamera.viewPos.y -= panY;
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutPostRedisplay();
	}
}

void mouseTrackball (int x, int y)
{
	if (gTrackBall) {
		rollToTrackball (x, y, gTrackBallRotation);
		glutPostRedisplay();
	}
}

void mouse (int button, int state, int x, int y)
{
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		if (gDolly) { // if we are currently dollying, end dolly
			mouseDolly (x, y);
			gDolly = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		} else if (gPan) {
			mousePan (x, y);
			gPan = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		}
		startTrackball (x, y, 0, 0, gCamera.screenWidth, gCamera.screenHeight);
		glutMotionFunc (mouseTrackball);
		gTrackBall = GL_TRUE;
	} else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		gTrackBall = GL_FALSE;
		glutMotionFunc (NULL);
		rollToTrackball (x, y, gTrackBallRotation);
		if (gTrackBallRotation[0] != 0.0)
			addToRotationTrackball (gTrackBallRotation, gWorldRotation);
		gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		if (gTrackBall) {// if we are currently trackballing, end trackball
			gTrackBall = GL_FALSE;
			glutMotionFunc (NULL);
			rollToTrackball (x, y, gTrackBallRotation);
			if (gTrackBallRotation[0] != 0.0)
				addToRotationTrackball (gTrackBallRotation, gWorldRotation);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		} else if (gPan) {
			mousePan (x, y);
			gPan = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		}
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutMotionFunc (mouseDolly);
		gDolly = GL_TRUE;
	} else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		mouseDolly (x, y);
		gDolly = GL_FALSE;
		glutMotionFunc (NULL);
		gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		glutMotionFunc (NULL);
	}
	else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
		if (gTrackBall) {// if we are currently trackballing, end trackball
			gTrackBall = GL_FALSE;
			glutMotionFunc (NULL);
			rollToTrackball (x, y, gTrackBallRotation);
			if (gTrackBallRotation[0] != 0.0)
				addToRotationTrackball (gTrackBallRotation, gWorldRotation);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		} else if (gDolly) {
			mouseDolly (x, y);
			gDolly = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		}
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutMotionFunc (mousePan);
		gPan = GL_TRUE;
	} else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
		mousePan (x, y);
		gPan = GL_FALSE;
		glutMotionFunc (NULL);
		gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		glutMotionFunc (NULL);
	}
}

void key(unsigned char inkey, int px, int py)
{
  gLastKey = inkey;
  switch (inkey) {
	case 27:
		exit(0);
		break;
	case 'h': // point
	case 'H':
		gShowHelp =  1 - gShowHelp;
		glutPostRedisplay();
		break;
	case 'i': // point
	case 'I':
		gShowInfo =  1 - gShowInfo;
		glutPostRedisplay();
		break;
	case 'c': // point
	case 'C':
		gShowCredits =  1 - gShowCredits;
		glutPostRedisplay();
		break;
	case 'f': // fill
	case 'F':
		gPolygons =  1;
		gLines = 0;
		gPoints = 0;
		glutPostRedisplay();
		break;
	case 'w': // lines
	case 'W':
		gPolygons =  0;
		gLines = 1;
		gPoints = 0;
		glutPostRedisplay();
		break;
	case 'p': // point
	case 'P':
		gPolygons =  0;
		gLines = 0;
		gPoints = 1;
		glutPostRedisplay();
		break;
	case 's': // stereo
	case 'S':
		gStereo =  1 - gStereo;
		if (gStereo) { // switch to full screen
			glutSetWindow (gMainWindow);
			glutHideWindow ();
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO); // stereo display mode for glut
			glutGameModeString("1024x768:32@120"); // must now use full screen game mode
			glutEnterGameMode(); // enter gamemode to get stereo context (may get invalid drawable warnings in console, this is normal and will be fixed in 
			init(); // ensure gl is setup since this is a new window
			glutReshapeFunc (reshape);
			glutDisplayFunc (display);
			glutKeyboardFunc (key);
			glutSpecialFunc (special);
			glutMouseFunc (mouse);
		} else {
			glutLeaveGameMode ();
			glutSetWindow (gMainWindow);
			glutShowWindow ();
			gCamera.screenWidth = glutGet(GLUT_WINDOW_WIDTH);
			gCamera.screenHeight = glutGet(GLUT_WINDOW_HEIGHT);
		}
		break;
	case 'l': // stereo
	case 'L':
		{ // loop though off, one sided, one sided local, two sided, two sided local
			gLighting++;
			if (gLighting > 4)
				gLighting = 0;
			SetLighting ();
			glutPostRedisplay();
		}
		break;
	case '[': // next lower color scheme
	case '{':
		gColorScheme -= 1;
		if (gColorScheme < 1)
			gColorScheme = 20;
		BuildGeometry ();
		glutPostRedisplay();
		break;
	case ']': // next higher color scheme
	case '}':
		gColorScheme += 1;
		if (gColorScheme > 20)
			gColorScheme = 1;
		BuildGeometry ();
		glutPostRedisplay();
		break;
	case ';': // next lower subdivision setting
	case ':':
		gSubDivisions = (GLuint) (gSubDivisions / 1.2);
		if (gSubDivisions < 8)
			gSubDivisions = 8;
		BuildGeometry ();
		glutPostRedisplay();
		break;
	case '\'': // next higher subdivision setting
	case '"':
		gSubDivisions = (GLuint) (gSubDivisions * 1.2);
		BuildGeometry ();
		glutPostRedisplay();
		break;
	case '\\': // next higher subdivision setting
		gSurface += 1;
		gSurface %= kSurfaces;
		BuildGeometry ();
		glutPostRedisplay();
		break;
	case '=': // increase camera aperture
	case '+':
		gCamera.aperture += 0.5f;
		glutPostRedisplay();
		break;
	case '-': // decrease camera aperture
	case '_':
		gCamera.aperture -= 0.5f;
		if (gCamera.aperture < 0.0f)
			gCamera.aperture = 0.0f;
		glutPostRedisplay();
		break;
  }
}

#pragma mark ---- main ----

int main (int argc, const char * argv[])
{
    glutInit(&argc, (char **)argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // non-stereo for main window
	glutInitWindowSize (800, 600);
	gMainWindow = glutCreateWindow("GLUTStereo");

    init();

    glutReshapeFunc (reshape);
    glutDisplayFunc (display);
//    glutIdleFunc (animate);
	glutKeyboardFunc (key);
	glutSpecialFunc (special);
	glutMouseFunc (mouse);

    glutMainLoop();
    return 0;
}





