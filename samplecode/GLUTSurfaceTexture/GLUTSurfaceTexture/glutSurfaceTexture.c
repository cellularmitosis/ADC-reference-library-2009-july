/*
 *  glutSurfaceTexture.cpp
 *  GLUTTest
 *
 *  Created by GGS on Wed Feb 12 2003.
 *  Copyright (c) 2003-2004 Apple. All rights reserved.
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
 
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
 
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "trackball.h"
#include "stanfordbunny.h"

#define DTOR 0.0174532925
#define CROSSPROD(p1,p2,p3) \
   p3.x = p1.y*p2.z - p1.z*p2.y; \
   p3.y = p1.z*p2.x - p1.x*p2.z; \
   p3.z = p1.x*p2.y - p1.y*p2.x

enum {
	kTextureSize = 256
};

typedef struct {
   GLdouble x,y,z;
} recVec;

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

GLfloat gShapeSize = 7.0f;

GLint gDollyPanStartPoint[2] = {0, 0};
GLfloat gTrackBallRotation [4] = {0.0, 0.0, 0.0, 0.0};
GLboolean gDolly = GL_FALSE;
GLboolean gPan = GL_FALSE;
GLboolean gTrackBall = GL_FALSE;
GLfloat gWorldRotation [4] = {100.0, -0.7, 0.6, 0.5};

GLboolean gRectTexture = GL_FALSE;
GLboolean gSurfaceTexture = GL_TRUE;
GLint gTextureWidth = kTextureSize;
GLint gTextureHeight = kTextureSize;

GLboolean gLines = GL_FALSE;
GLboolean gPolygons = GL_TRUE;

GLboolean gShowHelp = GL_TRUE;
GLboolean gShowInfo = GL_TRUE;

recCamera gCamera;
recVec gOrigin = {0.0, 0.0, 0.0};

int gLastKey = ' ';

int gMainWindow = 0, gTextureWindow = 0;
GLint gSurfaceTexName = 0;
GLint gTextureValid = 0;

GLuint gBunnyWireList = NULL;
GLuint gBunnySolidList = NULL;
GLboolean gBunnyLines = GL_FALSE;


// simple cube data
GLint cube_num_vertices = 8;
GLfloat cube_vertices [8][3] = {
{1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0},
{1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0} };

GLint num_faces = 6;
short cube_faces [6][4] = {
{3, 2, 1, 0}, {2, 3, 7, 6}, {0, 1, 5, 4}, {3, 0, 4, 7}, {1, 2, 6, 5}, {4, 5, 6, 7} };
GLdouble cube_texCoords [2][4] = {
{0.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 0.0} };

#pragma mark ---- gCamera control ----

void gCameraReset(void)
{
   gCamera.aperture = 40;
   gCamera.focalLength = 10;
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

#pragma mark ---- Utilities ----

void
drawGLString(GLfloat x, GLfloat y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
  }
}


#pragma mark ---- Drawing ----


void SetLighting(void)
{
	GLfloat mat_ambient[] = {0.329412, 0.223529, 0.027451, 1.0};
	GLfloat mat_diffuse[] = {0.780392, 0.568627, 0.113725, 1.0};
	GLfloat mat_specular[] = {0.992157, 0.941176, 0.807843, 1.0};
	GLfloat mat_shininess[] = {27.8974};

	GLfloat position[4] = {7.0,-7.0,12.0,0.0};
	GLfloat ambient[4]  = {0.2,0.2,0.2,1.0};
	GLfloat diffuse[4]  = {0.9,0.9,0.9,1.0};
	GLfloat specular[4] = {1.0,1.0,1.0,1.0};
	
	glMaterialfv (GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv (GL_FRONT, GL_SHININESS, mat_shininess);
	
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glEnable(GL_LIGHT0);
}

void drawGLText (GLint window_width, GLint window_height)
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
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Trackball Rotation: (%0.1f, %0.2f, %0.2f, %0.2f)", gTrackBallRotation[0], gTrackBallRotation[1], gTrackBallRotation[2], gTrackBallRotation[3]);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "World Rotation: (%0.1f, %0.2f, %0.2f, %0.2f)", gWorldRotation[0], gWorldRotation[1], gWorldRotation[2], gWorldRotation[3]);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Aperture: %0.1f", gCamera.aperture);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf (outString, "Focus Distance: %0.1f", gCamera.focalLength);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
	}
	
	if (gShowHelp) {
		line = 1;
		sprintf (outString, "Controls:\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "left button drag: rotate camera\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "right (or crtl-left) button drag: dolly (zoom) camera\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "arrows: aperture & focal length\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "S: toggle surface texture\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "H: toggle help\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
		sprintf (outString, "I: toggle info\n");
		drawGLString (10, (lineSpacing * line++) + startOffest, outString);
	}
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(matrixMode);
	
	glViewport(vp[0], vp[1], vp[2], vp[3]);
}

static void DrawCube (GLfloat fSize, GLint texWidth, GLint texHeight)
{
	long f, i;
 	glEnable (GL_POLYGON_OFFSET_FILL);
	if (gPolygons)
	{
		glColor3f (1.0, 1.0, 1.0);
		glBegin (GL_QUADS);
		for (f = 0; f < num_faces; f++)
			for (i = 0; i < 4; i++)
			{
				glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
				glTexCoord2d (cube_texCoords [0][i] * (gRectTexture ? texWidth : 1.0f), 
							  cube_texCoords [1][i] * (gRectTexture ? texHeight : 1.0f));
			}
		glEnd ();
	}
	if (gLines)
	{
		glDisable(gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D);
		glColor3f (0.0, 1.0, 1.0);
		for (f = 0; f < num_faces; f++)
		{
			glBegin (GL_LINE_LOOP);
				for (i = 0; i < 4; i++)
					glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
			glEnd ();
		}
	}
	glDisable (GL_POLYGON_OFFSET_FILL);
}

void initMain (void)
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);    
	glFrontFace(GL_CCW);
	glPolygonOffset (1.0, 1.0);
	glEnable (GL_ALPHA_TEST); 
	glAlphaFunc (GL_GREATER, 0.5);

	glPolygonOffset (1.0, 1.0);
		
	gRectTexture = gluCheckExtension ((GLubyte *)"GL_EXT_texture_rectangle", glGetString (GL_EXTENSIONS)); // can we use texture rectangle
	
	glClearColor (0.2f, 0.2f, 0.4f, 1.0f);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gLines = GL_FALSE; //  do not draw edges
	
	gCameraReset ();
}

void initTexture (void)
{
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);    
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glColor3f(1.0,1.0,1.0);
	gBunnySolidList = GenStanfordBunnySolidList ();
	gBunnyWireList = GenStanfordBunnyWireList ();

	glClearColor(0.0,0.0,0.0,0.0);

	SetLighting();
	glEnable(GL_LIGHTING);
	glDisable(gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D); // disable texturing
}

#pragma mark ---- GLUT callbacks ----


void reshape (int w, int h)
{
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	gCamera.screenWidth = w;
	gCamera.screenHeight = h;
	glutPostRedisplay();
}

void texturereshape (int w, int h)
{
	if (!gRectTexture) { // if we require power of two, do not resize
		glutReshapeWindow (kTextureSize, kTextureSize);
		return;
	}
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	gTextureWidth = w;
	gTextureHeight = h;
	if (0 != gSurfaceTexName) { // force texture update
		glutSetWindow (gMainWindow);
		glDeleteTextures (1, &gSurfaceTexName);
		glFlush();
		gSurfaceTexName = 0;
		glutSetWindow (gTextureWindow);
	}
	glutPostRedisplay();
	glutPostWindowRedisplay(gMainWindow);
}

void texturedisplay(void)
{
	static GLfloat fRot [3] = { 0.0, 0.0, 0.0 };
	static GLfloat fVel [3] = { 0.3, 0.1, 0.2 };
	static GLfloat fAccel [3] = { 0.003, -0.005, 0.004 };
	GLfloat fVMax = 2.0;
	short i;

	// do spin velocities
	for (i = 0; i < 3; i++) {
		fVel[i] += fAccel[i];
		if (fVel[i] > fVMax) {
			fAccel[i] *= -1.0;
			fVel[i] = fVMax;
		} else if (fVel[i] < -fVMax) {
			fAccel[i] *= -1.0;
			fVel[i] = -fVMax;
		}
		fRot[i] += fVel[i];
		while (fRot[i] > 360.0)
			fRot[i] -= 360.0;
		while (fRot[i] < -360.0)
			fRot[i] += 360.0;
	}

    // Viewing transformation
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (45.0, 1.0, 1.0, 5.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -2.0);

	glRotatef (fRot[0], 1.0, 0.0, 0.0);
	glRotatef (fRot[1], 0.0, 1.0, 0.0);
	glRotatef (fRot[2], 0.0, 0.0, 1.0);

	// fill surface texture, this could be any GL content...
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (gBunnyLines) {
		glDisable (GL_LIGHTING);
		glColor3f (1.0, 1.0, 1.0); // no coloring
		glCallList (gBunnyWireList);
	} else {
		glEnable(GL_LIGHTING);
		glCallList (gBunnySolidList);
	}
	glutSwapBuffers();
	gTextureValid = 1;
}

void maindisplay(void)
{
	GLdouble ratio, radians, wd2;
	GLdouble left, right, top, bottom, near, far;

	if (!gTextureValid) { // ensure we have a valid texture to draw with
		glutSetWindow (gTextureWindow);
		texturedisplay();
		glutSetWindow (gMainWindow);
	}	
		
	near = -gCamera.viewPos.z - gShapeSize * 0.5;
	if (near < 0.1) near = 0.1;
	far = -gCamera.viewPos.z + gShapeSize * 0.5;

	// Misc stuff
	ratio  = gCamera.screenWidth / (double)gCamera.screenHeight;
	radians = DTOR * gCamera.aperture / 2;
	wd2     = near * tan(radians);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	left  = - ratio * wd2;
	right =   ratio * wd2;
	top    =   wd2;
	bottom = - wd2;
	glFrustum (left, right, bottom, top, near, far);
	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	gluLookAt (gCamera.viewPos.x, gCamera.viewPos.y, gCamera.viewPos.z,
			gCamera.viewPos.x + gCamera.viewDir.x,
			gCamera.viewPos.y + gCamera.viewDir.y,
			gCamera.viewPos.z + gCamera.viewDir.z,
			gCamera.viewUp.x, gCamera.viewUp.y ,gCamera.viewUp.z);
			
	// track ball rotation
	glRotatef (gTrackBallRotation[0], gTrackBallRotation[1], gTrackBallRotation[2], gTrackBallRotation[3]);
	glRotatef (gWorldRotation[0], gWorldRotation[1], gWorldRotation[2], gWorldRotation[3]);
	
	if (gSurfaceTexture) { // if surface texturing is on
		glEnable(gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D); // enable texturing 
#pragma mark === SurfaceTexture ===
		if (0 == gSurfaceTexName) {
			glGenTextures (1, &gSurfaceTexName);
			glBindTexture (gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D, gSurfaceTexName);
			// the following is superfluous for texture rectangle but required for texture 2D without mipmaps
			glTexParameterf (gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glutSurfaceTexture (gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D, GL_RGBA8, gTextureWindow);
		} else {
			glBindTexture (gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D, gSurfaceTexName);
		}
		glColor3f (1.0, 1.0, 1.0); // no coloring
	} else { // disable if no surface texture context
		glDisable(gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D);
		glColor3f (0.8f, 0.1f, 0.1f); // red cube
	}
	DrawCube(2.0, gTextureWidth, gTextureHeight); // draw a cube with the faces textured with surface texture
	glDisable(gRectTexture ? GL_TEXTURE_RECTANGLE_EXT : GL_TEXTURE_2D); // enable texturing 

	drawGLText (gCamera.screenWidth, gCamera.screenHeight);
	// this could be used for testing/exprimentation by forcing the texture to be deleted every frame
//	if (0 != gSurfaceTexName) { // force texture deletion
//		glDeleteTextures (1, &gSurfaceTexName);
//		gSurfaceTexName = 0;
//	}
	glutSwapBuffers();
}

void animate(void)
{
	// ensure we are current window (since are calling display directly)
	glutSetWindow (gTextureWindow);
	texturedisplay();
	glutPostWindowRedisplay(gMainWindow);
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
	case GLUT_KEY_LEFT: // arrow left, smaller aperture
		gCamera.aperture -= 0.5f;
		if (gCamera.aperture < 0.0f)
			gCamera.aperture = 0.0f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT: // arrow right, larger aperture
		gCamera.aperture += 0.5f;
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
	case 'h': // help
	case 'H':
		gShowHelp =  1 - gShowHelp;
		glutPostWindowRedisplay(gMainWindow);
		break;
	case 'i': // info
	case 'I':
		gShowInfo =  1 - gShowInfo;
		glutPostWindowRedisplay(gMainWindow);
		break;
	case 's': // control surface texturing
	case 'S':
		gSurfaceTexture = 1 - gSurfaceTexture;
		break;
	case 'w': // wire frame
	case 'W':
		gBunnyLines = 1 - gBunnyLines;
		break;
  }
}

void mainclose (void)
{
	if (0 != gSurfaceTexName) { // force texture update
		glutSetWindow (gMainWindow);
		glDeleteTextures (1, &gSurfaceTexName);
		glFlush();
		gSurfaceTexName = 0;
	}
}

#pragma mark ---- main ----

int main (int argc, const char * argv[])
{
    glutInit(&argc, (char **)argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // non-stereo for main window
	glutInitWindowPosition (300, 50);
	glutInitWindowSize (800, 600);
	gMainWindow = glutCreateWindow("GLUTSurfaceTexture");

    initMain(); // GL init

    glutReshapeFunc (reshape);
    glutDisplayFunc (maindisplay);
	glutKeyboardFunc (key);
	glutSpecialFunc (special);
	glutMouseFunc (mouse);
	glutWMCloseFunc (mainclose);


	glutInitWindowSize (kTextureSize, kTextureSize);
	glutInitWindowPosition (40, 50);
	gTextureWindow = glutCreateWindow("Surface Texture");

    initTexture(); // GL init

	glutKeyboardFunc (key);
    glutDisplayFunc (texturedisplay);
    glutReshapeFunc (texturereshape);
    glutIdleFunc (animate);
	glutWMCloseFunc (mainclose);

    glutMainLoop();
    return 0;
}





