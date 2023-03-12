/*
	File:		GenerateFont.c

	Contains:	Program to generate C source code for the InterruptSafeDebug font.

	Written by:	Quinn "The Eskimo!"

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

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

	Change History (most recent first):

$Log: GenerateFont.c,v $
Revision 1.3  2002/11/08 23:21:15         
Convert nil to NULL.

Revision 1.2  2001/11/07 15:49:55         
Tidy up headers, add CVS logs, update copyright.


         <1>    23/11/98    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////////

#include "MoreSetup.h"

// Mac OS interfaces

#include <MacTypes.h>
#include <QDOffscreen.h>
#include <Fonts.h>

// Standard C interfaces

#include <stdio.h>

/////////////////////////////////////////////////////////////////////

void main(void)
{
	OSStatus err;
	GWorldPtr gWorld;
	Rect bounds;
	UInt8 *base;
	ByteCount rowBytes;
	ItemCount i;
	ItemCount row;
	
	printf("Hello Cruel World!\n");
	printf("QStandard.c\n");
	fflush(stdout);

	// Create an offscreen GWorld to hold all 256 font
	// characters, arranged horizontally.
	
	bounds.top = 0;
	bounds.left = 0;
	bounds.bottom = 10;
	bounds.right = 256 * 8;
	
	err = NewGWorld(&gWorld, 1, &bounds, NULL, NULL, 0);
	if (err == noErr) {
		
		// Setup the GWorld for our use.
		
		LockPixels(gWorld->portPixMap);
		
		SetGWorld(gWorld, NULL);
		TextFont(kFontIDMonaco);
		TextSize(9);		
		EraseRect(&bounds);
		
		// Draw all 256 characters, horizontally, into
		// the offscreen GWorld.
		
		for (i = 0; i < 256; i++) {
			MoveTo(i * 8 + 1, 7);
			DrawChar(i);
		}
		
		// Walk through the pixmap data in the GWorld,
		// printing it to stdout.
		
		base = (UInt8 *) GetPixBaseAddr(gWorld->portPixMap);
		rowBytes = ((**(gWorld->portPixMap)).rowBytes) & 0x3FFF;
		for (row = 0; row < 10; row++) {
			for (i = 0; i < 256; i++) {
				printf("0x%02x, ", *(base + (rowBytes * row) + i));
			}
			printf("\n");
		}
		
		// Clean up.
		
		DisposeGWorld(gWorld);
	}
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n");
	}
	printf("Done.  Press command-Q to Quit.\n");
}