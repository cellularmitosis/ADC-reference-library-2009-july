/*
	File:		aglString.c

	Contains:	string output handling for aglFont.

	Written by:	Geoff Stahl

	Copyright:	2000 Apple Computer, Inc., All Rights Reserved

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

#ifdef __APPLE_CC__
    #include <Carbon/Carbon.h>
    
    #include <AGL/agl.h>
#else
    #include <DriverServices.h>
    #include <Timer.h>
    #include <Math64.h>

    #include <agl.h>
    #include <gl.h>
#endif

#include <stdio.h>

#include "aglString.h"

//-----------------------------------------------------------------------------------------------------------------------

// Draw frame rate in current color at current raster positon with provided font display list
// This version handles multiple windows, thus the calling code must track the values of frames and time from call to call
// The application needs to maintain a cstring (32 characters should be fine), frame counter and time for each window, 
// but this routine will take care of all updates.

void MultiWinDrawFrameRate (GLuint fontList, char * cString, long * frames, AbsoluteTime * time)
{	
	AbsoluteTime currTime = UpTime ();
	float deltaTime = (float) AbsoluteDeltaToDuration (currTime, *time);
	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;

	(*frames)++;
	
	if (0.5 <= deltaTime)	// has update interval passed
	{
		sprintf (cString, "FPS: %0.1f",  (float) *frames / deltaTime);
		*time = currTime;	// reset for next time interval
		*frames = 0;
	}
	
	DrawCStringGL (cString, fontList);
}

//-----------------------------------------------------------------------------------------------------------------------

// Draw frame rate in curent color at current raster positon with provided font display list

void DrawFrameRate (GLuint fontList)
{	
	static char aChar[256] = "";
	static long frames = 0;
/*
	static UInt64 time = 0;
	UnsignedWide currTime = {0,0};
	double deltaTime;
	Microseconds (&currTime);
	deltaTime = (float) (UnsignedWideToUInt64 (currTime) - time);
	deltaTime /= 1000000.0;
	frames++;
	if (0.5 <= deltaTime)	// has update interval passed
	{
		sprintf (aChar, "FPS: %0.1f",  (float) frames / deltaTime);
		time = UnsignedWideToUInt64 (currTime);	// reset for next time interval
		frames = 0;
	}
*/
	static AbsoluteTime time = {0,0};
	AbsoluteTime currTime = UpTime ();
	double deltaTime = (float) AbsoluteDeltaToDuration (currTime, time);
	if (0 > deltaTime)	// if negative microseconds
		deltaTime /= -1000000.0;
	else				// else milliseconds
		deltaTime /= 1000.0;
	frames++;
	if (0.5 <= deltaTime)	// has update interval passed
	{
		sprintf (aChar, "FPS: %0.1f",  (double) frames / deltaTime);
		time = currTime;	// reset for next time interval
		frames = 0;
	}
	
	DrawCStringGL (aChar, fontList);
}
#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

void DrawPStringGL (Str255 pstrOut, GLuint fontList)
{
	GLint i;
	for (i = 1; i <= pstrOut[0]; i++)
		glCallList (fontList + pstrOut[i]);
}

//-----------------------------------------------------------------------------------------------------------------------

void DrawCStringGL (char * cstrOut, GLuint fontList)
{
	GLint i = 0;
	while (cstrOut [i])
		glCallList (fontList + cstrOut[i++]);
}

//-----------------------------------------------------------------------------------------------------------------------

GLuint BuildFontGL (AGLContext ctx, GLint fontID, Style face, GLint size)
{
	GLuint listBase = glGenLists (256);
	if (aglUseFont (ctx, fontID , face, size, 0, 256, (long) listBase))
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		return listBase;
	}
	else
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glDeleteLists (listBase, 256);
		return 0;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DeleteFontGL (GLuint fontList)
{
	if (fontList)
		glDeleteLists (fontList, 256);
}