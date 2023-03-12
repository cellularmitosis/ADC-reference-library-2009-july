///////////////////////////////////////////////////////////////////////////////
/*
	File:		GLMain.cpp

	Project:	Carbon GLSnapshot

	Contains:	Implementation for the main application
	
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
///////////////////////////////////////////////////////////////////////////////

#include "GLMain.h"

//////////////////////////////////////////////////////////////////////////////////
// Main Function								//
//////////////////////////////////////////////////////////////////////////////////
int main(void)
{
    pMainWindow = windowInitialize(&mainWindow);
    InitializeGL(&mainWindow);
    InitializeControls();
    InstallEventHandlers();

    RunApplicationEventLoop();

    windowDestroy(&mainWindow);
    Shutdown();
    ExitToShell();
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// Global Function Definitions							//
//////////////////////////////////////////////////////////////////////////////////
void InitializeGL(GLWindow *glw)
{
    OSStatus err = noErr;
    int i = 0;
 
	
    // LIGHT PROPERTIES
    // Material
    GLfloat mat_spec[] = {0.5, 0.5, 0.5, 1.0};
    GLfloat mat_shine[] = {0.5};
    // Position
    GLfloat light_pos[] = {0.0, 0.0, -500.0, 0.0};
    // Color
    GLfloat light_color[] = {1.0, 1.0, 1.0, 0.5};

    if(glw == NULL)
	return; // FIXME: Should report an error
    
    // Set up our pixel format
    glw->glData.glAttribs[i++] = AGL_RGBA; 
    glw->glData.glAttribs[i++] = AGL_DOUBLEBUFFER;
    glw->glData.glAttribs[i++] = AGL_DEPTH_SIZE;
    glw->glData.glAttribs[i++] = 32;
    //glw->glData.glAttribs[i++] = AGL_PIXEL_SIZE;
    //glw->glData.glAttribs[i++] = 32;	
    glw->glData.glAttribs[i++] = AGL_ACCELERATED;
    glw->glData.glAttribs[i++] = AGL_NONE; 

    glw->glData.pxlfmt = aglChoosePixelFormat(NULL, 0, glw->glData.glAttribs);
    
    glw->glCtx = aglCreateContext(glw->glData.pxlfmt, NULL);
    err = aglSetDrawable(glw->glCtx, GetWindowPort(glw->pWin));
    if(err == GL_FALSE)
	    SysBeep(100);
    err = aglSetCurrentContext(glw->glCtx);
    if(err == GL_FALSE)
	    SysBeep(100);
    
    aglDestroyPixelFormat(glw->glData.pxlfmt);

    // Material specification commands    
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shine);

    // Light specification commands
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_color);
    
    // Light enablers
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);

    // Default shading model
    glShadeModel(GL_SMOOTH);
    //glEnable(GL_NORMALIZE);

    // Depth Buffer Operations
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Alpha Channel Operations
    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc();
    
    // Blending Operations
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation();
    
    // Fog Operations
    //glEnable(GL_FOG);
    
    // Finally, get the camera initialized
    cameraInitialize(&glw->glCam, glw->pWin);
}

MenuHandle InitializeMenu(void)
{
	InitCursor();
	
	// Init Menus
	theMenu = NewMenu (kMenuApple, "\p\024");	// new  apple menu
	InsertMenu (theMenu, 0);			// add menu to end

	theMenu = NewMenu (kMenuFile, "\pFile");	// new menu
	InsertMenu (theMenu, 0);			// add menu to end
	AppendMenu (theMenu, "\pQuit/Q"); 		// add items
	SetMenuItemCommandID(theMenu, 1, 'quit');
	DrawMenuBar();
	return theMenu;
}

void Render(GLWindow *glw)
{
    // This is our main rendering loop
    renderTestCube(glw);
}

void Shutdown(void)
{
    // Remove and dispose of our timers
    RemoveEventLoopTimer(renderTimer);
    if(snapshotTimer)
	RemoveEventLoopTimer(snapshotTimer);    
    DisposeEventLoopTimerUPP(renderTimerUPP);
    if(snapshotTimerUPP)
	DisposeEventLoopTimerUPP(snapshotTimerUPP);

    // Remove our other handlers
    DisposeEventHandlerUPP(appCommandProcessor);
    DisposeEventHandlerUPP(windowEventProcessor);
    DisposeEventHandlerUPP(menuEventProcessor);
    DisposeEventHandlerUPP(controlEventProcessor);
}

void InitializeControls(void)
{
    ControlID ctrlID;
    
    ctrlID.id = kCGLSButtonSnapshotID;
    ctrlID.signature = kCGLSButtonSnapshot;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &buttonSnapshot);

    ctrlID.id = kCGLSButtonSequenceID;
    ctrlID.signature = kCGLSButtonSequence;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &buttonSequence);

    ctrlID.id = kCGLSButtonStartID;
    ctrlID.signature = kCGLSButtonStart;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &buttonStart);
	
    ctrlID.id = kCGLSTextFieldTimeIntervalID;
    ctrlID.signature = kCGLSTextFieldTimeInterval;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &sequenceTimeInterval);

    ctrlID.id = kCGLSTextFieldFrameCount1ID;
    ctrlID.signature = kCGLSTextFieldFrameCount1;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &sequenceFrameCount);
	
    ctrlID.id = kCGLSTextFieldElapsedTimeID;
    ctrlID.signature = kCGLSTextFieldElapsedTime;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &movieTimeLimit);

    ctrlID.id = kCGLSTextFieldFrameCount2ID;
    ctrlID.signature = kCGLSTextFieldFrameCount2;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &movieFrameCount);

    ctrlID.id = KCGLSFramesCapturedCountID;
    ctrlID.signature = KCGLSFramesCapturedCount;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &movieFramesCapturedCounter);

    ctrlID.id = KCGLSProgressBarID;
    ctrlID.signature = kCGLSProgressBar;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &movieProgressBar);

    ctrlID.id = kCGLSSwitchTimeLimitID;
    ctrlID.signature = kCGLSSwitchTimeLimit;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &movieSwitchUseTimeLimit);

    ctrlID.id = kCGLSSwitchFrameLimitID;
    ctrlID.signature = kCGLSSwitchFrameLimit;
    GetControlByID(mainWindow.supportWindow, &ctrlID, &movieSwitchUseFrameLimit);


/*
static void SetControlText (WindowRef window, OSType sig, SInt32 id, char * text)
{
ControlID idControl;
ControlRef control;

idControl.signature = sig;
idControl.id = id;
GetControlByID (window, &idControl, &control);
SetControlData (control, kControlNoPart, kControlStaticTextTextTag, strlen (text), text);
}
*/

}




