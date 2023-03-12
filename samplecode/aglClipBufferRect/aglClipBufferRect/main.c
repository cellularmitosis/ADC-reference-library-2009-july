/*
 *  main.c
 *  aglClipBufferRectTest
 *
 *  Created by ggs on Thurs May 24 2002.

	Copyright:	Copyright © 2002 Apple Computer, Inc., All Rights Reserved

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

#ifdef __MACH__
	#include <Carbon/Carbon.h>
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <CarbonEvents.h>
	#include <Controls.h>
	#include <string.h>
#endif

// ---------------------------------

GLint cube_num_vertices = 8;
GLfloat cube_vertices [8][3] = {
{1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0},
{1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0} };

GLint num_faces = 6;
short cube_faces [6][4] = {
{3, 2, 1, 0}, {2, 3, 7, 6}, {0, 1, 5, 4}, {3, 0, 4, 7}, {1, 2, 6, 5}, {4, 5, 6, 7} };
short cube_texCoords [2][4] = {
{0.0, 0.0, 1.0, 1.0}, {0.0, 1.0, 1.0, 0.0} };

// ---------------------------------

enum {
	kBufferRectInset = 75
};

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData);
EventLoopTimerUPP GetTimerUPP (void);

EventHandlerUPP gMainEvtHandler = NULL;  	// Command-process event handler
EventHandlerUPP gModalEvtHandler = NULL;	// modal window event handler
EventHandlerUPP gWindowEvtHandler = NULL;	// window event handler

EventLoopTimerRef gTimer = NULL; // timer

Boolean gBufferRect = false;
Boolean gClip = false;
Boolean gAnimate = false;

WindowRef gWindow = NULL;
WindowRef gModalWindow = NULL;

// ---------------------------------


// sets static text control of id and sig to text

static void SetControlText (WindowRef window, SInt32 id, OSType sig, char * text)
{
    ControlID idControl;
    ControlRef control;
    
    idControl.signature = sig;
    idControl.id = id; 
    GetControlByID (window, &idControl, &control);
    SetControlData (control, kControlNoPart, kControlStaticTextTextTag, strlen (text), text);
}

// ---------------------------------

static void SetClipRegion (WindowRef win)
{
	RgnHandle       clipRgn = NewRgn();
	RgnHandle       maskRgn = NewRgn();
	Rect            rectPort, bounds = { -32767, -32767, 32767, 32767 };
	ControlID idControl;
	ControlRef control;
	
	GetWindowPortBounds(win, &rectPort);
	
	// some random region
	SetEmptyRgn (clipRgn);
	OpenRgn ();
	MoveTo (rectPort.left + 10, rectPort.top + 30);
	LineTo (rectPort.right - 20, rectPort.top + 50);
	LineTo (rectPort.right - 50, rectPort.bottom - 25);
	LineTo (rectPort.left + 10, rectPort.top + 50);
	CloseRgn (clipRgn);
	
	// control cutouts
	idControl.signature = 'cbrt';
	for (idControl.id = 0; idControl.id < 4; idControl.id++) {
		GetControlByID (win, &idControl, &control);
		GetControlBounds (control, &bounds);
		RectRgn(maskRgn, &bounds);
		DiffRgn(clipRgn, maskRgn, clipRgn);
	}
	// set clip region for agl
	aglSetInteger ((AGLContext)GetWRefCon (win), AGL_CLIP_REGION, (const GLint *)clipRgn);
	DisposeRgn(clipRgn);
	DisposeRgn(maskRgn);
}

// ---------------------------------

static void cleanupAGL(AGLContext ctx)
{
	aglSetCurrentContext(NULL);
	aglSetDrawable(ctx, NULL);
	aglDestroyContext(ctx);
}

// ---------------------------------

static void resizeGL (WindowRef win)
{
	// handle window size updates for GL
	GLfloat fAspect;
	short w, h;
	Rect rectPort;
	GLint bufferRect[4];
	char strOut[128];
	
	GetWindowPortBounds(win, &rectPort);
	w = rectPort.right - rectPort.left;
	h = rectPort.bottom - rectPort.top;
	
	// set control text
	sprintf (strOut, "Window size: %d x %d", w, h);
	SetControlText (win, 2, 'cbrt', strOut);
	DrawControls (win);
	
	if (gClip) {
		SetClipRegion (win);
		aglEnable ((AGLContext)GetWRefCon (win), AGL_CLIP_REGION);
	} else
		aglDisable ((AGLContext)GetWRefCon (win), AGL_CLIP_REGION);
	if (gBufferRect) {
		w -= kBufferRectInset * 2;
		h -= kBufferRectInset * 2;
		bufferRect[0] = kBufferRectInset;
		bufferRect[1] = kBufferRectInset;
		bufferRect[2] = w;
		bufferRect[3] = h;
		aglSetInteger ((AGLContext)GetWRefCon (win), AGL_BUFFER_RECT, bufferRect);
		aglEnable ((AGLContext)GetWRefCon (win), AGL_BUFFER_RECT);
	} else
		aglDisable ((AGLContext)GetWRefCon (win), AGL_BUFFER_RECT);
	// Prevent a divide by zero
	if(h == 0)
		h = 1;
	fAspect = (GLfloat) w / (GLfloat) h;
	
	aglSetCurrentContext ((AGLContext)GetWRefCon (win));
	// not really needed because enable/disable above actually does this
//	aglUpdateContext ((AGLContext)GetWRefCon (win));

	// Set Viewport to window/buffer rect dimensions
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	glMatrixMode (GL_PROJECTION);

	// Reset coordinate system
	glLoadIdentity ();

	// Setup perspective for viewing
	gluPerspective (55.0, fAspect, 3, 20);
}

// ---------------------------------

static void drawCubeGL (GLfloat fSize)
{
	long f, i;
	glBegin (GL_QUADS);
	glColor3f (0.8f, 0.2f, 0.6f);
	for (f = 0; f < num_faces; f++)
		for (i = 0; i < 4; i++)
			glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
	glEnd ();
	glColor3f (0.0, 1.0, 1.0);
	for (f = 0; f < num_faces; f++)
	{
		glBegin (GL_LINE_LOOP);
			for (i = 0; i < 4; i++)
				glVertex3f(cube_vertices[cube_faces[f][i]][0] * fSize, cube_vertices[cube_faces[f][i]][1] * fSize, cube_vertices[cube_faces[f][i]][2] * fSize);
		glEnd ();
	}
}

// ---------------------------------

static void drawGL (AGLContext aglContext)
{
	static GLfloat fRot [3] = { 0.0, 0.0, 0.0 };
	static GLfloat fVel [3] = { 0.3, 0.1, 0.2 };
	static GLfloat fAccel [3] = { 0.003, -0.005, 0.004 };
	GLfloat fVMax = 2.0;
	short i;
	// do velocities
	if (gAnimate == true) {
		for (i = 0; i < 3; i++)
		{
			fVel[i] += fAccel[i];
			if (fVel[i] > fVMax)
			{
				fAccel[i] *= -1.0;
				fVel[i] = fVMax;
			}
			else if (fVel[i] < -fVMax)
			{
				fAccel[i] *= -1.0;
				fVel[i] = -fVMax;
			}
			fRot[i] += fVel[i];
			while (fRot[i] > 360.0)
				fRot[i] -= 360.0;
			while (fRot[i] < -360.0)
				fRot[i] += 360.0;
		}
	}
	aglSetCurrentContext (aglContext);

    // Viewing transformation
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -8.0);

	glRotatef (fRot[0], 1.0, 0.0, 0.0);
	glRotatef (fRot[1], 0.0, 1.0, 0.0);
	glRotatef (fRot[2], 0.0, 0.0, 1.0);

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawCubeGL(2.0);
}

// ---------------------------------

static AGLContext setupAGL(AGLDrawable win)
{
	GLint          attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_NONE };
	AGLPixelFormat fmt;
	AGLContext     ctx;
	GLboolean      ok;

	/* Choose an rgb pixel format */
	fmt = aglChoosePixelFormat(NULL, 0, attrib);
	if(fmt == NULL) return NULL;

	/* Create an AGL context */
	ctx = aglCreateContext(fmt, NULL);
	if(ctx == NULL) return NULL;

	/* Attach the window to the context */
	ok = aglSetDrawable(ctx, win);
	if(!ok) return NULL;
	
	/* Make the context the current context */
	ok = aglSetCurrentContext(ctx);
	if(!ok) return NULL;

	/* Pixel format is no longer needed */
	aglDestroyPixelFormat(fmt);
	
	glShadeModel (GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);	// Hidden surface removal
    glEnable(GL_CULL_FACE);	// Do not draw inside of cube
    glFrontFace(GL_CCW);	// Counter clock-wise polygons face out
	
	glClearColor (0.2f, 0.2f, 0.4f, 1.0f);	
	return ctx;
}

// ---------------------------------

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData)
{
    #pragma unused (inTimer, userData)
	if (gAnimate == true) {
		drawGL ((AGLContext) GetWRefCon(gWindow));
		aglSwapBuffers ((AGLContext) GetWRefCon(gWindow));
		drawGL ((AGLContext) GetWRefCon(gModalWindow));
		aglSwapBuffers ((AGLContext) GetWRefCon(gModalWindow));
	}
}

// ---------------------------------

// make timer UPP

EventLoopTimerUPP GetTimerUPP (void)
{
    static EventLoopTimerUPP	sTimerUPP = NULL;
    
    if (sTimerUPP == NULL)
	sTimerUPP = NewEventLoopTimerUPP (IdleTimer);
    
    return sTimerUPP;
}

// ---------------------------------

// handle events for game window

static pascal OSStatus WindowEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
	#pragma unused (myHandler, userData)
    OSStatus result = eventNotHandledErr;

	if ((GetEventKind(event) == kEventWindowShown) || (GetEventKind(event) == kEventWindowBoundsChanged)) {
		resizeGL ((WindowRef) userData);
		drawGL ((AGLContext) GetWRefCon((WindowRef) userData));
		glFlush ();
	} else if (GetEventKind(event) == kEventWindowDrawContent) {
		drawGL ((AGLContext) GetWRefCon((WindowRef) userData));
		aglSwapBuffers((AGLContext) GetWRefCon((WindowRef) userData));// send swap command
    } else if (GetEventKind(event) == kEventProcessCommand) {
		HICommand command;
		GetEventParameter (event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof(command), NULL, &command);
		if (command.commandID == kHICommandOK)
		{
			if (gTimer)
			RemoveEventLoopTimer(gTimer);
			gTimer = NULL;
			cleanupAGL ((AGLContext)GetWRefCon ((WindowRef)userData));
			DisposeWindow ((WindowRef)userData);
			gWindow = NULL;
			result = noErr;
		}
    }
    return result;
}

// ---------------------------------

// handle events for configuration window

static pascal OSStatus ModalEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
	#pragma unused (myHandler)
    OSStatus result = eventNotHandledErr;
    
    if ((GetEventKind(event) == kEventWindowShown) || (GetEventKind(event) == kEventWindowBoundsChanged)) {
		resizeGL ((WindowRef) userData);
		drawGL ((AGLContext) GetWRefCon((WindowRef) userData));
		glFlush ();
    } else if (GetEventKind(event) == kEventWindowDrawContent) {
		drawGL ((AGLContext) GetWRefCon((WindowRef) userData));
		aglSwapBuffers((AGLContext) GetWRefCon((WindowRef) userData));// send swap command
    } else if (GetEventKind(event) == kEventProcessCommand) {
        HICommand command;
        GetEventParameter (event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof(command), NULL, &command);
		if (command.commandID == kHICommandOK) {
			cleanupAGL ((AGLContext)GetWRefCon ((WindowRef)userData));
			DisposeWindow ((WindowRef)userData);
			gModalWindow = NULL;
			result = noErr;
		}
	}
	return result;
}

// ---------------------------------

// main application event handler

static pascal OSStatus MainEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
	#pragma unused (myHandler, userData)
	Rect rectPort;
	CGSize min;
    OSStatus err = eventNotHandledErr;
	IBNibRef 		nibRef;
	EventHandlerRef	ref;
	EventTypeSpec	list[] = { { kEventClassWindow, kEventWindowShown},
				{ kEventClassWindow, kEventWindowDrawContent },
				{ kEventClassCommand, kEventProcessCommand },
				{ kEventClassWindow, kEventWindowBoundsChanged} };

	if (GetEventKind(event) == kEventProcessCommand) {
        HICommand command;
        GetEventParameter (event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof(command), NULL, &command);
        if (command.commandID == 'newm') {
			if (NULL == gModalWindow) {
				err = CreateNibReference(CFSTR("main"), &nibRef);
				err = CreateWindowFromNib(nibRef, CFSTR("ModalWindow"), &gModalWindow);
				GetWindowPortBounds(gModalWindow, &rectPort);
				min.width = rectPort.right - rectPort.left;
				min.height = rectPort.bottom - rectPort.top;
				SetWindowResizeLimits (gModalWindow, &min, NULL);
				if (!gModalEvtHandler)
					gModalEvtHandler = NewEventHandlerUPP (ModalEvtHndlr);
				InstallWindowEventHandler (gModalWindow, gModalEvtHandler, GetEventTypeCount (list), list, (void *)gModalWindow, &ref);
				DisposeNibReference(nibRef);
				SetWRefCon (gModalWindow, (long) setupAGL(GetWindowPort (gModalWindow)));
				ShowWindow( gModalWindow );
			}
			err = noErr;
		} else if (command.commandID == 'neww') {
			if (NULL == gWindow) {
				err = CreateNibReference(CFSTR("main"), &nibRef);
				err = CreateWindowFromNib (nibRef, CFSTR("Window"), &gWindow);
				GetWindowPortBounds(gWindow, &rectPort);
				min.width = rectPort.right - rectPort.left;
				min.height = rectPort.bottom - rectPort.top;
				SetWindowResizeLimits (gWindow, &min, NULL);
				if (!gWindowEvtHandler)
					gWindowEvtHandler = NewEventHandlerUPP (WindowEvtHndlr);
				InstallWindowEventHandler (gWindow, gWindowEvtHandler, GetEventTypeCount (list), list, (void *) gWindow, &ref);
				DisposeNibReference(nibRef);
				// install timer
				if (NULL != gTimer)
					RemoveEventLoopTimer(gTimer);
				InstallEventLoopTimer (GetCurrentEventLoop (), 0, 0.01, GetTimerUPP (), (void *) gWindow, &gTimer);
				SetWRefCon (gWindow, (long) setupAGL(GetWindowPort (gWindow)));
				ShowWindow( gWindow );
			}
			err = noErr;
		} else if (command.commandID == 'bufr') {
			gBufferRect = 1 - gBufferRect;
			if (gWindow) {
				resizeGL (gWindow); // handle chnage  in settings
				drawGL ((AGLContext) GetWRefCon(gWindow)); // redraw
				aglSwapBuffers((AGLContext) GetWRefCon(gWindow));// send swap command
			}
			if (gModalWindow) {
				resizeGL (gModalWindow); // handle chnage  in settings
				drawGL ((AGLContext) GetWRefCon(gModalWindow)); // redraw
				aglSwapBuffers((AGLContext) GetWRefCon(gModalWindow));// send swap command
			}
			err = noErr;
		} else if (command.commandID == 'clip') {
			gClip = 1 - gClip;
			if (gWindow) {
				resizeGL (gWindow); // handle chnage  in settings
				drawGL ((AGLContext) GetWRefCon(gWindow)); // redraw
				aglSwapBuffers((AGLContext) GetWRefCon(gWindow));// send swap command
			}
			if (gModalWindow) {
				resizeGL (gModalWindow);// handle chnage  in settings
				drawGL ((AGLContext) GetWRefCon(gModalWindow)); // redraw
				aglSwapBuffers((AGLContext) GetWRefCon(gModalWindow));// send swap command
			}
			err = noErr;
		} else if (command.commandID == 'anim') {
			gAnimate = 1 - gAnimate;
			err = noErr;
		}
    }
    return err;
}

// ---------------------------------

int main(int argc, char* argv[])
{
#pragma unused (argc, argv)

    IBNibRef 		nibRef;
    OSStatus		err;
    EventHandlerRef	ref;
    EventTypeSpec	list[] = { { kEventClassCommand,  kEventProcessCommand } };
     
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MainMenu"));
    require_noerr( err, CantSetMenuBar );

    gMainEvtHandler = NewEventHandlerUPP (MainEvtHndlr);
    InstallApplicationEventHandler (gMainEvtHandler, GetEventTypeCount (list), list, 0, &ref);

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // Call the event loop
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:

	return err;
}

