/*
	File:		PixMap2PixPat2ppat.c

	Contains:	This snippet shows how to convert a 'icl8' image to a	
				PixMap image and then to a PixPat and then finally to		
				a 'ppat' resource.	In this example, the 'ppat'				
				resource is saved-off into a resource file.	

	Written by: EL	

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
        6/2003		MK				Updated for ProjectBuilder
        
        08/2000		JM				Carbonized, non-Carbon code is commented out
                                                        for demonstration purposes.
        5/9/2000	DH				Updated to use GetPixRowBytes() and not leak PixMap.
        
        7/13/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"



/* Constant Declarations */

#define	WWIDTH		320
#define	WHEIGHT		160

/* Global Variable Definitions */

WindowPtr	gWindow;

void initMac();
void createWindow();
void doEventLoop();
void drawWindow();

void createPixMap();

void PixMap2PixPat();
void PixPat2ppat();
void ppat2file(Handle, Str255);

int main()
{
	PixMapHandle	pixmap;
	PixPatHandle	pixpat;
	
	initMac();
	
	pixmap = NewPixMap();
	pixpat = NewPixPat();
	
	createWindow();
	
	createPixMap( pixmap );
	PixMap2PixPat( pixmap, pixpat );
	PixPat2ppat( pixpat );
	
	doEventLoop( pixpat );
        
        return 0;
}

void initMac()
{
	InitCursor();
	FlushEvents( 0, everyEvent );
}

void createWindow()
{
	Rect rect;
	BitMap	bitMap;
	int	top, left;
	
	GetQDGlobalsScreenBits(&bitMap);
	
	left = (((bitMap.bounds.right - bitMap.bounds.left) - WWIDTH) / 2);
	top = (((bitMap.bounds.bottom - bitMap.bounds.top) - WHEIGHT) / 2);
	
	SetRect( &rect, left, top, left + WWIDTH, top + WHEIGHT );
	
	gWindow = NewCWindow( 0L, &rect, "\pPixMap2PixPat2ppat", true, noGrowDocProc,
							(WindowPtr)-1L, true, 0L );
                            
	SetPortWindowPort( gWindow );
}

void createPixMap( pixmap )
PixMapHandle	pixmap;
{
	Handle		iclHandle;
	char		depth;
	Rect		rect;
		
	SetRect( &rect, 0, 0, 32, 32 );
	depth = 8;
	
	/* Create a pixmap from the icl8 pixel image. */
	
	iclHandle = GetResource( 'icl8', 129 );
	
	HLock( iclHandle );
	HNoPurge( iclHandle );
		
	(**pixmap).baseAddr = *iclHandle;
	(**pixmap).rowBytes = (((rect.right - rect.left) * depth) / 8) | 0x8000;
	(**pixmap).bounds = rect;
	(**pixmap).pmVersion = 0;
	(**pixmap).packType = 0;
	(**pixmap).packSize = 0;
	(**pixmap).hRes = 0x00480000;
	(**pixmap).vRes = 0x00480000;
	(**pixmap).pixelSize = depth;
	(**pixmap).pixelType = 0;
	(**pixmap).cmpCount = 1;
	(**pixmap).cmpSize = depth;
	(**pixmap).pmTable = GetCTable( depth );
}

void PixMap2PixPat( pixmap, pixpat )
PixMapHandle	pixmap;
PixPatHandle	pixpat;
{
	Handle      image;
	long		imageSize;
	
	//	Dispose of the old patMap first
	DisposePixMap( (**pixpat).patMap );
	
	(**pixpat).patMap = pixmap;
	
	//	Compute the size of the image using GetPixRowBytes(), not rowBytes!
	imageSize = GetPixRowBytes((**pixpat).patMap) *
				((**(**pixpat).patMap).bounds.bottom -
				(**(**pixpat).patMap).bounds.top);
				
	PtrToHand( (**pixmap).baseAddr, &image, imageSize );
	(**pixpat).patData = image;
}

void PixPat2ppat( pixpat )
PixPatHandle	pixpat;
{
	Handle			ppat;
	PixPatHandle	pixpatCopy;
	long			pixmapSize, pixpatSize, imageSize, ctableSize;
	
	pixpatCopy = NewPixPat();
	
	HLock((Handle)pixpatCopy);
	HLock((Handle)pixpat);
	
	CopyPixPat( pixpat, pixpatCopy );
	
	pixmapSize	= sizeof( PixMap );
	pixpatSize	= sizeof( PixPat );
	imageSize	= GetPixRowBytes((**pixpatCopy).patMap) *
				((**(**pixpatCopy).patMap).bounds.bottom -
				(**(**pixpatCopy).patMap).bounds.top);
				
	ctableSize	= sizeof( ColorTable ) +
				(**(**(**pixpatCopy).patMap).pmTable).ctSize *
				sizeof( ColorSpec );
	
	/********************************************/
	/* Allocate memory for the 'ppat' resource. */
	/********************************************/
	
	ppat = NewHandleClear( pixpatSize + pixmapSize + imageSize + ctableSize );
	HLock((Handle)ppat);
	
	/**********************/
	/* Fill the resource. */
	/**********************/
	
	BlockMove( *(**(**pixpatCopy).patMap).pmTable,
				*ppat + pixpatSize + pixmapSize + imageSize, ctableSize );
	(**(**pixpatCopy).patMap).pmTable = (CTabHandle)(pixpatSize + pixmapSize + imageSize);
	
	BlockMove( *(**pixpatCopy).patData, *ppat + sizeof( PixPat ) + sizeof( PixMap), imageSize );
	(**pixpatCopy).patData = (Handle)(pixpatSize + pixmapSize);
	
	BlockMove( *(**pixpatCopy).patMap, *ppat + sizeof( PixPat ), pixmapSize );
	(**pixpatCopy).patMap = (PixMapHandle)pixpatSize;
	
	BlockMove( *pixpatCopy, *ppat, pixpatSize );
	
	/************************************************/
	/* Finally, save the 'ppat' resource to a file. */
	/************************************************/

	ppat2file( ppat, "\pppat file.rsrc" );
	
	HUnlock((Handle)ppat);
	HUnlock((Handle)pixpatCopy);
	HUnlock((Handle)pixpat);
}

void ppat2file( Handle ppat, Str255 fname )
{
	// Old routine deleted, replaced by all carbon routines using FSSpec
	// Only light error checking, more robust apps should have more extensive
	// error checking.
	OSErr	anErr;
	FSSpec	fspec;
	int		refNum;
	
	anErr = FSMakeFSSpec(0, 0, fname, &fspec);
	
	FSpCreateResFile(&fspec, 'RSED', 'rsrc', smSystemScript);
		
	if (ResError() == dupFNErr) {
		anErr = FSpDelete(&fspec);
		FSpCreateResFile(&fspec, 'RSED', 'rsrc', smSystemScript);
	}
	
	if (ResError() != noErr || (anErr != noErr && anErr != fnfErr))
		return;
	
	refNum = FSpOpenResFile(&fspec, fsRdWrPerm);
	
	anErr = ResError();
	
	if (anErr == noErr) {
		AddResource( ppat, 'ppat', UniqueID( 'ppat' ), "\pMy ppat" );
		anErr = ResError();
		CloseResFile(refNum);
		anErr = ResError();
	}
	
}

void drawWindow( pixpat )
PixPatHandle	pixpat;
{
	Rect	tempRect1;
	
	//FillCRect( &(*gWindow).portRect, pixpat );
	FillCRect(GetPortBounds(GetWindowPort(gWindow), &tempRect1), pixpat);
	
	TextFont( kFontIDTimes );
	TextSize( 72 );
	TextMode( srcOr );
	
	ForeColor( blackColor );
	MoveTo( 80, 100 );
	DrawString( "\pDone." );
	
	ForeColor( yellowColor );
	MoveTo( 75, 95 );
	DrawString( "\pDone." );
}

void doEventLoop( pixpat )
PixPatHandle	pixpat;
{
	EventRecord event;
	WindowPtr   window;
	short       clickArea;
	Rect        screenRect;

	for (;;)
	{
		if (WaitNextEvent( everyEvent, &event, 0, nil ))
		{
			if (event.what == mouseDown)
			{
				clickArea = FindWindow( event.where, &window );
				
				if (clickArea == inDrag)
				{
					//screenRect = (**GetGrayRgn()).rgnBBox;
					GetRegionBounds(GetGrayRgn(), &screenRect);
					DragWindow( window, event.where, &screenRect );
				}
				else if (clickArea == inContent)
				{
					if (window != FrontWindow())
						SelectWindow( window );
				}
				else if (clickArea == inGoAway)
					if (TrackGoAway( window, event.where ))
						return;
			}
			else if (event.what == updateEvt)
			{
				window = (WindowPtr)event.message;	
				//SetPort( window );
				SetPortWindowPort( window );
				
				BeginUpdate( window );
				drawWindow( pixpat );
				EndUpdate( window );
			}
		}
	}
}