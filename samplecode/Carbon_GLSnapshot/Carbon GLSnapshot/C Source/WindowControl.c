//////////////////////////////////////////////////////////////////////////////////
/*
	File:		WindowControl.c

	Project:	CarbonEvent Shell

	Contains:	Implementation of the window control functionality
	
	Author: 	Todd Previte
	
	Copyright:	2001 Apple Computer, Inc., All Rights Reserved

	Copyright:	(c) 2002 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
	("Apple") in consideration of your agreement to the following terms, and your
	use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these terms.  If you do not agree with these terms,
	please do not use, install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and subject
	to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
//////////////////////////////////////////////////////////////////////////////////

#include "WindowControl.h"
#include "CarbonEventHandlers.h"

long windowWidth(GLWindow *glw)				{return glw->winW;}
long windowHeight(GLWindow *glw)			{return glw->winH;}
AGLContext windowOpenGLContext(GLWindow *glw)		{return glw->glCtx;}
void windowSetMouseDelta(GLWindow *glw, Point* delta)	{glw->mouseDelta = *delta;}

// Window Controls
WindowRef windowInitialize(GLWindow *glw)
{
    OSStatus err;
    
    if(glw == NULL)
    {
	err = -1;
	return NULL;
    }

#if __APPLE_CC__
    err = CreateNibReference(CFSTR("main"), &glw->nibRef);
    err = SetMenuBarFromNib(glw->nibRef, CFSTR("MenuBar"));
    err = CreateWindowFromNib(glw->nibRef, CFSTR("MainWindow"), &glw->pWin);
    err = CreateWindowFromNib(glw->nibRef, CFSTR("Window"), &glw->supportWindow); 
#else
    err = CreateNewWindow(kDocumentWindowClass, 
			  kWindowStandardDocumentAttributes | 
			  kWindowStandardHandlerAttribute,
			  &winRect, 
			  &glw->pWin);
    InitializeMenu();
#endif
    ShowWindow(glw->supportWindow);
    ShowWindow(glw->pWin);
    // Set our rectangle
    GetWindowPortBounds(glw->pWin, &glw->winRect);
    return glw->pWin;
}

void windowDestroy(GLWindow *glw)
{
	MenuHandle hMenu;
	aglSetDrawable(glw->glCtx, NULL);
	aglSetCurrentContext(NULL);
	aglDestroyContext(glw->glCtx);
	
	DisposeWindow((WindowPtr)glw->pWin);
	glw->pWin = NULL;
	if(glw->supportWindow)
	    DisposeWindow((WindowPtr)glw->supportWindow);
	hMenu = GetMenuHandle (kMenuFile);
	DeleteMenu (kMenuFile);
	DisposeMenu (hMenu);

	hMenu = GetMenuHandle (kMenuApple);
	DeleteMenu (kMenuApple);
	DisposeMenu (hMenu);

}

void windowRefresh(GLWindow *glw)
{
    windowClear(glw);
}

void windowProcessWindow(GLWindow *glw)
{
    //windowRenderDebugText();
    aglSwapBuffers(glw->glCtx);
}

void windowClear(GLWindow *glw)
{
    if(glw->glCtx)
	aglSetCurrentContext(glw->glCtx);
    else
	return;
    //glCam.Initialize(glw->pWin);
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
    aglSwapBuffers(glw->glCtx);
}

void windowResize(GLWindow *glw, uint32 newH, uint32 newW)
{
    glw->winH = newH;
    glw->winW = newW;
    aglSetCurrentContext(glw->glCtx);
    aglSetDrawable(glw->glCtx, NULL);
    aglSetDrawable(glw->glCtx, GetWindowPort(glw->pWin));
}

void windowRenderDebugText(GLWindow *glw)
{
    Point3D textQuad[4];
    GLdouble qx = 0.25;
    GLdouble qy = 0.25;
    GLdouble qz = 0.50;

    Point3D p0 = {qx, qy, qz};
    Point3D p1 = {-qx, qy, qz};
    Point3D p2 = {-qx, -qy, qz};
    Point3D p3 = {qx, -qy, qz};

    textQuad[0] = p0;
    textQuad[1] = p1;
    textQuad[2] = p2;
    textQuad[3] = p3;

    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW);

    glColor4f(0.0, 1.0, 0.0, 1.0); // 1/2 alpha solid green
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //glTranslatef(0.5, 0.5, 0.0);
#if USE_VERTEX_ARRAY
    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, textQuad);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
#endif
    glPopMatrix();
    glPushMatrix();
    glPopMatrix();
    windowRenderText(glw);
}

void windowRenderText(GLWindow *glw)
{
    // set to per pixel orthographic context for 2D/text drawing
    GLint matrixMode;
    char md[256];
    Point3D cp ;
    glDisable(GL_TEXTURE_2D);
    glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
    glMatrixMode (GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity ();
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity ();
	glScalef(2.0 / 640, -2.0 / 480, 1.0);
	glTranslatef(-640 / 2.0, -480 / 2.0, 0.0);
	// now have pixel scaled ortho mapping
	glColor4d(0.0, 1.0, 0.0, 0.5);
	glRasterPos3d(10, 15, 0); 
	//windowFont.RenderString("OpenGL CarbonEvents w/ Carbon Timer");
	glRasterPos3d(10, 30, 0); 
	//windowFont.RenderFramerate();
	//sprintf(md, "Mouse Delta X: %d  Delta Y: %d", mouseDelta.h, mouseDelta.v);
	glRasterPos3d(10, 45, 0); 
	//windowFont.RenderString(md);
	cp =  glw->glCam.camPosition;
	glRasterPos3d(10, 60, 0); 
	sprintf(md, "Camera Position: X: %f  Y: %f  Z: %f", cp.x, cp.y, cp.z);
	//windowFont.RenderString(md);
	glRasterPos3d(10, 75, 0); 
	sprintf(md, "Camera Angle: %f", glw->glCam.camAngle);
	//windowFont.RenderString(md);
	glRasterPos3d(10, 90, 0); 
	sprintf(md, "Field Of View: %f", glw->glCam.fov);
	//windowFont.RenderString(md);
    glPopMatrix(); // GL_MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(matrixMode);
    glEnable(GL_TEXTURE_2D);
    glFlush();
}

//////////////////////////////////////////////////////////////////////////////////
// windowBuildFromNIB								//
//////////////////////////////////////////////////////////////////////////////////
void windowBuildFromNIB(GLWindow *glw)
{
/*    IBNibRef nibRef;
    OSStatus err = noErr;
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = CreateMenuBarFromNib(nibRef, CFSTR("MainMenu"), &pMainMenu);
    SetMenuBar(pMainMenu);
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &pMainWindow);

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow(pMainWindow);
*/
}

char* windowSnapshot(GLWindow* glw)
{
    // This is where we'll snap a picture of our OpenGL context
    // and return a pointer to the raw byte data
    char   			*dataBuffer = NULL;
    GLRect 			glRegion;
    long 			imgSize;
    QDErr			error;
    GLenum glerr;

    RectToGLRect(&glRegion, &glw->winRect);
    // Alloc enough memory for this data
    imgSize = glRegion.width * glRegion.height * 4;
    dataBuffer = MemAlloc(imgSize);
    // Read the back buffer (glReadBuffer() is set to GL_BACK by default in a double buffered context
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,640,480, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV , dataBuffer);
    glerr = glGetError();
    if(glerr != 0)
	printf("Error: glReadPixels() failed : %li\n", glerr);

    InvertGLImage(dataBuffer, imgSize, 640 * 4);

    // Allocate memory for the buffer if its null
    if(gwBuffer == NULL)
	gwBuffer = malloc(sizeof(GWorldPtr));
	
    // Add this new image to our GWorld
    error = NewGWorldFromPtr(&gwBuffer[imageCount], k32ARGBPixelFormat, &glw->winRect, 0, 0, 0, dataBuffer, 640*4);

    // Increment the image count;
    imageCount++;
    
    // Check to see if we're in a sequence or a movie
    if(snapshotTimer || snapshotTimerUPP)
    {
	// Just return nothing
	return NULL;
    }
    else
    {
	void *noData = NULL;
	// only 1 image, so process it
	//ProcessImageArray(imageCount, gwBuffer);
	MPProcessImageAsync(noData);
	// Return the data buffer
	return dataBuffer;
    }
}

void windowSnapshotSequence(GLWindow *glw)
{
    // Get the values from the controls (time and number of frames)
    // I think we need to pass a count along to the snapshot function
    // in order to figure out when to remove the damned thing.
    // There's no other way to do it without creating my own API.

    char buffer[256];
    Size dataSize;
    
    // If our UPP is valid, we've already got a timer going
    if(snapshotTimerUPP)
	return;

    GetControlData(sequenceTimeInterval, kControlEditTextPart, kControlStaticTextTextTag, 256, buffer, &dataSize);
    seqTimeInt = atoi(buffer);
    GetControlData(sequenceFrameCount, kControlEditTextPart, kControlStaticTextTextTag, 256, buffer, &dataSize);
    seqFrameCount = atoi(buffer);
    
    if(seqTimeInt <= 0)
	seqTimeInt = 1;
    if(seqFrameCount <= 0)
	seqFrameCount = 1;

    // Now allocate memory for our GWorld buffer
    if(gwBuffer)
    {
	free(gwBuffer);
	gwBuffer = NULL;
    }
    gwBuffer = malloc(seqFrameCount * sizeof(GWorldPtr));
    // Remember, we need to use NewGWorldFromPtr() on EACH of these elements as we create them
    
    // Now we kick off a new timer to create our snapshots
    snapshotTimerUPP = NewEventLoopTimerUPP(SnapshotTimedEventProcessor);
    InstallEventLoopTimer(GetCurrentEventLoop(),
			  0,
			  kEventDurationMillisecond * seqTimeInt,
			  snapshotTimerUPP,
			  NULL, // User Data
			  &snapshotTimer);
    

}

void windowCaptureFrames(GLWindow *glw)
{
    // This routine is what we'll use to capture frames for building a movie
    // We need an array of GWorld to hold the image data we're reading out of OpenGL
    // Then we'll process that buffer with another thread

    char fBuffer[256], tBuffer[256], frBuffer[256];
    Size dataSize;
    
    // bail if we're already in the middle of a snapshot sequence
    if(snapshotTimerUPP)
	return;
    
    // Get the desired frame rate
    GetControlData(movieFrameRate, kControlEditTextPart, kControlStaticTextTextTag, 256, frBuffer, &dataSize);
    fps = atoi(frBuffer);
    if(fps <= 0)
	fps = 32;
	
    if(movieUseTimeLimit)
    {
	GetControlData(movieTimeLimit, kControlEditTextPart, kControlStaticTextTextTag, 256, fBuffer, &dataSize);
	seqTimeInt = atoi(fBuffer);
    }
    
    if(movieUseFrameLimit)
    {
	GetControlData(movieFrameCount, kControlEditTextPart, kControlStaticTextTextTag, 256, tBuffer, &dataSize);
	seqFrameCount = atoi(tBuffer);
    }

    if(movTimeLimit <= 0)
	movTimeLimit = 10;
    if(movFrameCount <= 0)
	movFrameCount = 300;

    // now kick off our sequence timer    
    snapshotTimerUPP = NewEventLoopTimerUPP(MovieCaptureTimedEventProcessor);
    InstallEventLoopTimer(GetCurrentEventLoop(),
			  0,
			  kEventDurationSecond / fps,
			  snapshotTimerUPP,
			  NULL, // User Data
			  &snapshotTimer);    
}

void windowAddFrameToMovie(GLWindow *glw, GWorldPtr *buffer)
{
    // FIXME: Obviously this isn't working yet.
}

void renderTestCube(GLWindow *glw)
{
    int i, j;
    float x, y;
    i = j = 0;
    x = y = 1.0;
    glClearColor(0.1,0.2,0.3,0.4);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glColor4f(0.0, 1.0, 0.0, 1.0); // 1/2 alpha solid green
    glBegin(GL_LINES);

    for(i = 0; i < 10; i++)
    {
	//x = (float)i/10;
	glColor4f(0.0, 0.0, 1.0, 0.0);
	glVertex3f(5.0, -0.2, x);
	glVertex3f(-5.0, -0.2, x);
	x -= 1.0;
    }
    for(j = 0; j < 10; j++)
    {
	glColor4f(1.0, 0.0, 0.0, 0.0);
	glVertex3f(y, -0.2, 0.0);
	glVertex3f(y, -0.2, -10.0);
	y -= 1.0;
    }
    glEnd();
    glFlush();
    renderCubeAtOrigin(glw);
    aglSwapBuffers(glw->glCtx);
}

void renderCubeAtOrigin(GLWindow *glw)
{
    glBegin(GL_QUADS);
	glColor4f(0.0, 1.0, 0.0, 1.0);
	glVertex3f(0.1, 0.1, 0.0);
	glVertex3f(0.1, -0.1, 0.0);
	glVertex3f(-0.1, -0.1, 0.0);
	glVertex3f(-0.1, 0.1, 0.0);
    glEnd();
}

