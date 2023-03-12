/*
	File:		OpenGL Stereo.c

	Contains:	An example of the use of full screen stereo APIs with OpenGL.

	Written by:	GGS

	Copyright:	2002 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):


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

// #define kDebug 1

// system includes ----------------------------------------------------------

#include "Carbon Include.h"
#include <Carbon/Carbon.h>

#include <AGL/agl.h>
#include <AGL/aglRenderers.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
 
// project includes ---------------------------------------------------------

#include "aglString.h"

// statics/globals (internal only) ------------------------------------------

// Menu defs
enum 
{
	kMenuApple = 128,
	kMenuFile = 129,
	
	kAppleAbout = 1,
	kFileQuit = 1
};

enum 
{
	kForegroundSleep = 10,
	kBackgroundSleep = 10000
};
EventLoopTimerRef gTimer = NULL;

short gBitsPerPixel = 32;
short gContextWidth = 640;
short gContextHeight = 480;

const RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };

AGLContext gaglContext = 0;
GLuint gFontList;
char gcstrMode [256] = "";

Rect gRectPort = {0, 0, 0, 0};

UInt32 gSleepTime = kForegroundSleep;
Boolean gDone = false, gfFrontProcess = true;

GLfloat gfScale = 1.0;

GDHandle ghTargetDevice = NULL;

#pragma mark -

//-----------------------------------------------------------------------------------------------------------------------

// Copy C string to Pascal string

void CToPStr (StringPtr outString, const char *inString)
{	
	unsigned char x = 0;
	do {
		*(((char*)outString) + x + 1) = *(inString + x);
		x++;
	} while ((*(inString + x) != 0)  && (x < 256));
	*((char*)outString) = (char) x;									
}

// --------------------------------------------------------------------------

void ReportError (char * strError)
{
	char errMsgCStr [256];
	Str255 strErr;

	sprintf (errMsgCStr, "%s", strError);

	// out as debug string
	CToPStr (strErr, errMsgCStr);
	DebugStr (strErr); 
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

GLenum aglDebugStr (void)
{
	GLenum err = aglGetError();
	if (AGL_NO_ERROR != err)
		ReportError ((char *)aglErrorString(err));
	return err;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

GLenum glDebugStr (void)
{
	GLenum err = glGetError();
	if (GL_NO_ERROR != err)
		ReportError ((char *)gluErrorString(err));
	return err;
}

#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Setup

AGLContext SetupAGLFullScreen (GDHandle display, short * pDepth, short * pWidth, short * pHeight)
{
	GLint			attrib[64];
	GLint val = 0;
	AGLPixelFormat 	fmt;
	AGLContext     	ctx;
	
	short i = 0;
	attrib [i++] = AGL_RGBA; // red green blue and alpha
	attrib [i++] = AGL_DOUBLEBUFFER; // double buffered
	attrib [i++] = AGL_ACCELERATED; // HWA pixel format only
	attrib [i++] = AGL_STEREO; // stereo
	attrib [i++] = AGL_FULLSCREEN;
	attrib [i++] = AGL_PIXEL_SIZE; // screen deoth we are looking for
	attrib [i++] = *pDepth;
	attrib [i++] = AGL_NONE;

	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat)  {// check for existance of OpenGL
		ReportError ("OpenGL not installed");
		return NULL;
	}

	fmt = aglChoosePixelFormat(&display, 1, attrib); // this may fail if looking for acclerated across multiple monitors
	if (NULL == fmt) {
		ReportError("Could not find valid pixel format");
		aglDebugStr ();
		return NULL;
	}
	
	aglDescribePixelFormat (fmt, AGL_PIXEL_SIZE, &val);

	ctx = aglCreateContext (fmt, NULL); // Create an AGL context
	if (NULL == ctx) {
		ReportError ("Could not create context");
		aglDebugStr ();
		return NULL;
	}
	
#ifdef kDebug
	aglEnable (ctx, AGL_FS_CAPTURE_SINGLE);
#endif

	if (!aglSetFullScreen (ctx, *pWidth, *pHeight, 0, 0)) { // AGL should now try to find the closest freq
		ReportError ("SetFullScreen failed");
		aglDebugStr ();
		return NULL;
	}

	if (!aglSetCurrentContext (ctx)) { // make the context the current context
		ReportError ("SetCurrentContext failed");
		aglDebugStr ();
		aglSetDrawable (ctx, NULL); // turn off full screen
		return NULL;
	}

	aglDestroyPixelFormat(fmt); // pixel format is no longer needed

	return ctx;
}

// --------------------------------------------------------------------------
// routines to draw stereo syncing lines

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
		glColor3d(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES); // Draw a line of the correct length
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			if(buffer == GL_BACK_LEFT)
				glVertex3f(window_width * 0.5f, window_height - 0.5f, 0.0f);
			else
				glVertex3f(window_width, window_height - 0.5f, 0.0f);
		glEnd();
	
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(matrixMode);
		
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}	
	glPopAttrib();
}

// --------------------------------------------------------------------------

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
		glColor3d(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES); // Draw a line of the correct length
			glVertex3f(0.0f, window_height - 0.5f, 0.0f);
			if(buffer == GL_BACK_LEFT)
				glVertex3f(window_width * 0.5f, window_height - 0.5f, 0.0f);
			else
				glVertex3f(window_width, window_height - 0.5f, 0.0f);
		glEnd();
	
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(matrixMode);
		
		glViewport(vp[0], vp[1], vp[2], vp[3]);
	}
}

// --------------------------------------------------------------------------

// OpenGL Drawing

void DrawGL (Rect * pRectPort)
{
	static float rotation = 0.0;
	unsigned long buffer;
	GLint width = pRectPort->right - pRectPort->left;
	GLint height = pRectPort->bottom - pRectPort->top;
	
	if (gaglContext == 0)
		return;
	aglSetCurrentContext (gaglContext); // ensure our context is current prior to drawing
	aglDebugStr ();	
	aglUpdateContext (gaglContext); // test not normally needed
	aglDebugStr ();
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f); // Clear color buffer to dark grey
	glDebugStr ();
	
	// scale drawing, just for fun
    glViewport ((width - (width * gfScale)) / 2, (height - (height * gfScale)) / 2,
				width * gfScale, height * gfScale);
    
    rotation += 0.5;
	
	glMatrixMode (GL_MODELVIEW);
	glDebugStr ();
	glLoadIdentity ();
	glDebugStr ();
	glRotated (rotation, 0.0, 0.0, 1.0);
	glDebugStr ();
	
	for (buffer = GL_BACK_LEFT; buffer <= GL_BACK_RIGHT; buffer++) {
		glDrawBuffer (buffer);
		if (buffer == GL_BACK_LEFT) {
			glClearColor  (0.40f, 0.00f, 0.00f, 1.0f);
			glColor3d (1.0, 1.0, 0.0);
		} else {
			glColor3d (0.0, 1.0, 1.0);
			glClearColor  (0.00f, 0.00f, 0.40f, 1.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glDebugStr ();
		glBegin(GL_QUADS); // Draw a smooth shaded polygon
			glVertex3d(0.7, 0.7, 0.0);
			glVertex3d(-0.7, 0.7, 0.0);
			glVertex3d(-0.7, -0.7, 0.0);
			glVertex3d(0.7, -0.7, 0.0);
		glEnd();
		glDebugStr ();
		
		// draw info
		{
			GLint matrixMode;
			glViewport (0, 0, width, height);
			glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
			glMatrixMode (GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity ();
				glMatrixMode (GL_MODELVIEW);
				glPushMatrix();
					glLoadIdentity ();
					glScalef (2.0 / width, -2.0 /  height, 1.0);
					glTranslatef (-width / 2.0, -height / 2.0, 0.0);
					glColor3f (1.0, 1.0, 1.0);
					glRasterPos2i (10, 12); 
					DrawFrameRate (gFontList); // in aglString Sample (developer.apple.com/samplecode)
					glRasterPos2i (10, height - 27); 
					DrawCStringGL (gcstrMode, gFontList); // in aglString Sample (developer.apple.com/samplecode)
					glRasterPos2i (10, height - 15); 
					DrawCStringGL ((char*) glGetString (GL_VENDOR), gFontList); // in aglString Sample (developer.apple.com/samplecode)
					glRasterPos2i (10, height - 3); 
					DrawCStringGL ((char*) glGetString (GL_RENDERER), gFontList); // in aglString Sample (developer.apple.com/samplecode)
				glPopMatrix(); // GL_MODELVIEW
				glMatrixMode (GL_PROJECTION);
			glPopMatrix();
			glMatrixMode (matrixMode);
		}
		glDebugStr ();
	}
	// draw sync lines
	DrawBlueLine (width, height);
	// or... (depending on how much state you want saved and how long the routine will take to execute
//	DrawBlueLine_Simple ();

	aglSwapBuffers(gaglContext); // send swap command
}

// --------------------------------------------------------------------------

void DoUpdate (void)
{
	DrawGL (&gRectPort);
}

// --------------------------------------------------------------------------

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData)
{
	#pragma unused (inTimer, userData)
	DoUpdate ();
}

// --------------------------------------------------------------------------

EventLoopTimerUPP GetTimerUPP (void)
{
	static EventLoopTimerUPP	sTimerUPP = NULL;
	
	if (sTimerUPP == NULL)
		sTimerUPP = NewEventLoopTimerUPP (IdleTimer);
	
	return sTimerUPP;
}

//--------------------------------------------------------------------------

// OpenGL Cleanup

void CleanupAGL(AGLContext ctx)
{
	aglSetDrawable (ctx, NULL);
	aglSetCurrentContext (NULL);
	aglDestroyContext (ctx);
}

// --------------------------------------------------------------------------

void SetupGL (void) // ensure global rez, display and bit depth set prior to calling
{
	short fNum = 0;

	ghTargetDevice = GetMainDevice (); // default to main device							
	gaglContext = SetupAGLFullScreen (ghTargetDevice, &gBitsPerPixel, &gContextWidth, &gContextHeight); // Setup the OpenGL context
	SetRect (&gRectPort, 0, 0, gContextWidth, gContextHeight); // l, t, r, b
	sprintf (gcstrMode, "Stereo: %d x %d x %d", gContextWidth, gContextHeight, (**(**ghTargetDevice).gdPMap).pixelSize);
	if (gaglContext) {
		GetFNum("\pMonaco", &fNum);									// build font
		gFontList = BuildFontGL (gaglContext, fNum, normal, 9); // in aglString Sample (developer.apple.com/samplecode)
		InstallEventLoopTimer (GetCurrentEventLoop(), 0, 0.0005, GetTimerUPP (), 0, &gTimer);
	}
}

// --------------------------------------------------------------------------

void CleanUpGL (void)
{
	DeleteFontGL (gFontList);
	if (gaglContext)
		CleanupAGL (gaglContext); // Cleanup the OpenGL context
	gaglContext = 0;
}

#pragma mark -
// --------------------------------------------------------------------------

static pascal OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, SInt32 refcon )
{
	#pragma unused (appleEvt, reply, refcon)
	gDone =  true;
	return false;
}

// --------------------------------------------------------------------------

void InitToolbox(void)
{
	OSErr err;
	long response;
	MenuHandle menu;
	
	InitCursor();
	
	// Init Menus
	menu = NewMenu (kMenuApple, "\p\024");			// new  apple menu
	InsertMenu (menu, 0);							// add menu to end

	menu = NewMenu (kMenuFile, "\pFile");			// new menu
	InsertMenu (menu, 0);							// add menu to end

	// insert application menus here
	
	// add quit if not under Mac OS X
	err = Gestalt (gestaltMenuMgrAttr, &response);
	if ((err == noErr) && !(response & gestaltMenuMgrAquaLayoutMask))
			AppendMenu (menu, "\pQuit/Q"); 					// add quit

	DrawMenuBar();
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
	if (err != noErr)
		ExitToShell();
}

// --------------------------------------------------------------------------

Boolean SetUp (void)
{
	InitToolbox ();
	gaglContext = 0;
	SetupGL ();
	if (gaglContext)
		return true;
	else
		return false;
}

// --------------------------------------------------------------------------

void DoMenu (SInt32 menuResult)
{
	SInt16 theMenu;
	SInt16 theItem;
	MenuRef theMenuHandle;
		
	theMenu = HiWord(menuResult);
	theItem = LoWord(menuResult);
	theMenuHandle = GetMenuHandle(theMenu);

	switch (theMenu) {
		case kMenuApple:
			switch (theItem) {
				case kAppleAbout:
					break;
				default:
					break;
			}
			break;
		case kMenuFile:
			switch (theItem) {
				case kFileQuit:
					gDone = true;
					break;
			}
			break;
	}
	HiliteMenu(0);
	DrawMenuBar();
}

// --------------------------------------------------------------------------

void DoKey (SInt8 theKey, SInt8 theCode)
{
	#pragma unused (theCode, theKey)
    if ((theKey == '=') || (theKey == '+'))
        gfScale *= 1.1;
    else if (theKey == '-')
        gfScale /= 1.1;
}

// --------------------------------------------------------------------------

Boolean WaitNextEventWrapper (EventMask eventMask, EventRecord *theEvent, unsigned long sleep,RgnHandle mouseRgn)
{
	return WaitNextEvent (eventMask, theEvent, sleep, mouseRgn);
}

// --------------------------------------------------------------------------

void UpdateWrapper (EventRecord *theEvent)
{
	WindowRef whichWindow;
	GrafPtr pGrafSave;
	
	whichWindow = (WindowRef) theEvent->message;
	GetPort (&pGrafSave);
	SetPort((GrafPtr) GetWindowPort(whichWindow));
	BeginUpdate(whichWindow);
	DoUpdate();
	SetPort((GrafPtr) GetWindowPort(whichWindow));
	EndUpdate(whichWindow);
	SetPort (pGrafSave);
}

// --------------------------------------------------------------------------

void DoEvent (void)
{
	EventRecord theEvent;
	WindowRef whichWindow;
	SInt16 whatPart;
	if (WaitNextEventWrapper (everyEvent, &theEvent, gSleepTime, NULL))
	{
		switch (theEvent.what) {
			case mouseDown:
				whatPart = FindWindow (theEvent.where, &whichWindow);
				switch (whatPart) {
					case inContent:
						break;
					case inMenuBar: {
							SInt32 menuResult = MenuSelect (theEvent.where);
							if (HiWord (menuResult) != 0)
								DoMenu (menuResult);
						}
						break;
					case inDrag: {
							// full screen no drag
						}
						break;
					case inGrow: {
							// full screen no grow
						}
						break;
					case inGoAway: {
							// full screen no go away
						}
						break;
					case inZoomIn:
					case inZoomOut: {
							// full screen no zoom
						}
						break;
					case inSysWindow:
						break;
				}
				break;
			case keyDown:
			case autoKey: {
				SInt8 theKey;
				SInt8 theCode;
				theKey = theEvent.message & charCodeMask;
				theCode = (theEvent.message & keyCodeMask) >> 8;
				if ((theEvent.modifiers & cmdKey) != 0) {
					SInt32 menuResult = MenuKey (theKey);
					if (HiWord(menuResult) != 0)
						DoMenu (menuResult);
				} else
					DoKey (theKey, theCode);
			}
				break;
			case updateEvt: {
				UpdateWrapper (&theEvent);
			}
				break;
			case diskEvt:
				break;
			case osEvt:
				if (theEvent.message & 0x01000000) {	//	Suspend/resume event
					if (theEvent.message & 0x00000001) { //	Resume
						gSleepTime = kForegroundSleep;	
						gfFrontProcess = true;
					} else {
						gSleepTime = kBackgroundSleep;	//	Suspend
						gfFrontProcess = false;
					}
				}
				break;

			case kHighLevelEvent:
				AEProcessAppleEvent (&theEvent);
				break;
		}
	}
}

// --------------------------------------------------------------------------

void CleanUp (void)
{
	MenuHandle hMenu;
	
	RemoveEventLoopTimer(gTimer);
	gTimer = NULL;

	CleanUpGL ();
	
	hMenu = GetMenuHandle (kMenuFile);
	DeleteMenu (kMenuFile);
	DisposeMenu (hMenu);

	hMenu = GetMenuHandle (kMenuApple);
	DeleteMenu (kMenuApple);
	DisposeMenu (hMenu);
}

// --------------------------------------------------------------------------

int main (void)
{
	if (SetUp ())	
		while (!gDone) 
			DoEvent ();
	CleanUp ();
	return 0;
}