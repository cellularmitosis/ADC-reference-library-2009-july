/*
	File:		Snapshot.c

	Contains:	This application demonstrates how to quickly and		
				efficiently capture the main device's desktop into			
				a window.  The program basically reads the image 			
				stored in the the main device's pixmap then copies			
				it to a custom pixmap.  The custom pixmap is de-			
				fined at the same depth of the main device and 				
				contains an identical copy of that device's color-			
				table.  This is done to provide the fastest 				
				performance possible when copying from an offscreen			
				to onscreen pixmap.  By making sure the pixel values		
				map to the exact same colors in both colortables,			
				copybits will do a direct transfer of bits without			
				wasting time remapping the colors.  Also the ctSeed			
				field for each colortable should be the same.  Finally,		
				since the main device's bounding rect is different			
				than that of the offscreen's, the copying performance		
				for the device to the offscreen is slightly affected		
				because of the scaling required.  However, the copying		
				performance for the offscreen to the window is the 			
				best possible since the bounding rects for each are			
				identical.										

	Written by: EL	

	Copyright:	Copyright © 1991-1999 by Apple Computer, Inc., All Rights Reserved.

				You may incorporate this Apple sample source code into your program(s) without
				restriction. This Apple sample source code has been provided "AS IS" and the
				responsibility for its operation is yours. You are not permitted to redistribute
				this Apple sample source code as "Apple sample source code" after having made
				changes. If you're going to re-distribute the source, we require that you make
				it clear in the source that the code was descended from Apple sample source
				code, but that you've made changes.

	Change History (most recent first):
				08/2000		JM				Carbonized, non-Carbon code is commented out
											for demonstration purposes.
				11/6/1999	GGS			 	Updated to work with modern (1999) Mac OS.  
											Fixed a PixMap disposing bug.  Updated casts
											and headers.
				7/14/1999	KG				Updated for Metrowerks Codewarror Pro 2.1				

*/

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

/* Constant Declarations */

#define	WWIDTH		((qd.screenBits.bounds.right - qd.screenBits.bounds.left) / 2)
#define	WHEIGHT		((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) / 2)

#define WLEFT		(((qd.screenBits.bounds.right - qd.screenBits.bounds.left) - WWIDTH) / 2)
#define WTOP		(((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) - WHEIGHT) / 2)

#define MENU_BAR_ID			128
#define MENU_BAR_IDX 		129
#define FILE_MENU 			128

enum {
	FILE_NEW = 1,
	FILE_REFRESH,
	FILE_CLOSE,
	FILE_SAVE,
	FILE_QUIT
};

/* Global Variable Definitions */

WindowPtr	gWindow;			/* Main window */
PixMap		gPixMap;			/* Offscreen pixmap */
Rect		gBounds;			/* Offscreen pixmap's bounding rect */
Boolean		gDone = false;		// Application termination global

void initMac();
void setUp();
void handleMenuSelection(long result);
void handleKeyPress(EventRecord *);
void saveToPICTFile();
void createWindow();

void initPixmap();
void createImage();
void drawImage();

pascal OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn);

void doEventLoop();

int main()
{
	initMac();
	
	setUp();
	
	createWindow();				/* Create a window to display the final image. */
	
	initPixmap();				/* Initialize offscreen pixmap. */
	createImage();				/* Copy the main screen's pixmap into the offscreen's. */
	drawImage();				/* Copy the offscreen's pixmap onto the window. */

	doEventLoop();				/* Handle any events. */
	
	DisposeWindow( gWindow );
    
    return 0;
}

void initMac()
{
	/*MaxApplZone();
	
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );*/
	InitCursor();
	FlushEvents( 0, everyEvent );
}

void setUp()
{
	Handle	menuBar;
	OSErr 	anErr = noErr;
	long	aLong;
	long	response;
	
	anErr = Gestalt(gestaltSystemVersion, &response);
	
	// Carbon Porting guidelines say provide alternate menu bar/menu scheme for OS X
	// This is just one way of doing this, which is pretty static
	if (response >= 0x01000) 
		menuBar = GetNewMBar(MENU_BAR_IDX);
	else
		menuBar = GetNewMBar(MENU_BAR_ID);
	
	if ( menuBar == nil || anErr != noErr )
		 ExitToShell();	

	SetMenuBar(menuBar);
	DisposeHandle(menuBar);

	DrawMenuBar();
	
    // Install 'quit' event handler
	if ((Gestalt(gestaltAppleEventsAttr, &aLong) == noErr)) {
		    anErr = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
			         NewAEEventHandlerUPP(AEQuitHandler), 0, false);
		    if (anErr != noErr)  
		    	ExitToShell();
	}
}

void createWindow()
{
	Rect 	wBounds;
	BitMap	bitMap;
	Rect 	tempRect1;
	int		top, left, width, height;
	
	GetQDGlobalsScreenBits(&bitMap);
	
	width = ((bitMap.bounds.right - bitMap.bounds.left) / 2);
	height = ((bitMap.bounds.bottom - bitMap.bounds.top) / 2);

	left = (((bitMap.bounds.right - bitMap.bounds.left) - width) / 2);
	top = (((bitMap.bounds.bottom - bitMap.bounds.top) - height) / 2);
	
	/* Create a window to display the final offscreen image. */
	
	//SetRect( &wBounds, WLEFT, WTOP, WLEFT + WWIDTH, WTOP + WHEIGHT );
	SetRect( &wBounds, left, top, left + width, top + height );
	
	gWindow = NewCWindow( 0L, &wBounds, "\pSnapshot Test", false, documentProc,
							(WindowPtr)-1L, true, 0L );
							
	//SetRect( &gBounds, 0, 0, gWindow->portRect.right - gWindow->portRect.left,
	//				gWindow->portRect.bottom - gWindow->portRect.top );
	GetPortBounds(GetWindowPort(gWindow), &tempRect1);
	SetRect( &gBounds, 0, 0, tempRect1.right - tempRect1.left, 
	 tempRect1.bottom - tempRect1.top);
							
	//SetPort( gWindow );
	SetPortWindowPort( gWindow );
}

void initPixmap()
{
	Ptr			offBaseAddr;	/* Pointer to the off-screen pixel image */
	short		bytesPerRow;
	GDHandle	mainDevice;
	CTabHandle	cTable;
	short		depth;

	/* Get a handle to the main device. */
	mainDevice = GetMainDevice();

	/* Store its current pixel depth. */
	depth = (**(**mainDevice).gdPMap).pixelSize;

	/* Make an identical copy of its pixmap's colortable. */
	cTable = (**(**mainDevice).gdPMap).pmTable;
	(void) HandToHand( (Handle*)(&cTable) );

	bytesPerRow = ((gBounds.right - gBounds.left) * depth) / 8;
	offBaseAddr = NewPtr((unsigned long)bytesPerRow * (gBounds.bottom - gBounds.top));
	
	gPixMap.baseAddr = offBaseAddr;  			/* Point to image */
	gPixMap.rowBytes = bytesPerRow | 0x8000;	/* MSB set for PixMap */
	gPixMap.bounds = gBounds;     				/* Use given bounds */
	gPixMap.pmVersion = 0;           			/* No special stuff */
	gPixMap.packType = 0;            			/* Default PICT pack */
	gPixMap.packSize = 0;            			/* Always zero in mem */
	gPixMap.hRes = 72;      					/* 72 DPI default res */
	gPixMap.vRes = 72;      					/* 72 DPI default res */
	gPixMap.pixelSize = depth;       			/* Set # bits/pixel */
	//gPixMap.planeBytes = 0;          			/* Not used */
	//gPixMap.pmReserved = 0;          			/* Not used */

	gPixMap.pixelType = 0;       				/* Indicates indexed */
	gPixMap.cmpCount = 1;        				/* Have 1 component */
	gPixMap.cmpSize = depth;     				/* Component size=depth */
	gPixMap.pmTable = cTable; 					/* Handle to CLUT */
}

void createImage()
{
	GDHandle	mainDevice;

	mainDevice = GetMainDevice();

	/* Store the screen's pixmap image in the offscreen pixmap. */

	CopyBits( (BitMap *)*(**mainDevice).gdPMap, (BitMap *)(&gPixMap),
				&(**(**mainDevice).gdPMap).bounds, &gPixMap.bounds, srcCopy, 0l );
}

void drawImage(WindowPtr theWindow)
{
	Rect tempRect1;
	/* Copy the offscreen image back onto the window. */

	//CopyBits( (BitMap *)&gPixMap, &gWindow->portBits, &gPixMap.bounds,
	//			&gWindow->portRect, srcCopy, 0l);
	
	CopyBits( (BitMap *)&gPixMap, GetPortBitMapForCopyBits(GetWindowPort(theWindow)), &gPixMap.bounds,
		GetPortBounds(GetWindowPort(gWindow), &tempRect1), srcCopy, 0L);
				
	ShowWindow( gWindow );
}

void saveToPICTFile()
{

/*
Saving a PixMap as a PICT file isn't too hard.

1.  Open a Picture with the port set to the destination of #2.
2.  CopyBits the PixMap onto itself or another port.  (Because CopyBits is
recorded in Pictures.
3.  Close the picture.
4.  Open the data fork for the file.
5.  Write out 512 bytes of zeros followed by the contents of the Picture
handle.
6.  Close the file.
*/

	PicHandle			picHandle;
	OSErr				anErr = noErr;
	OSType              fileTypeToSave = 'PICT';
    OSType              creatorType = 'ogle';
    NavReplyRecord      reply;
    NavDialogOptions    dialogOptions;
    FSSpec      		documentFSSpec;
    long				inOutCount;
    short				refNum, count;
    AEKeyword   		theKeyword;
    DescType    		actualType;
	unsigned char 		header[512];
	Size        		actualSize;
	Rect				tempRect1;
	
	CopyBits(GetPortBitMapForCopyBits(GetWindowPort(FrontWindow())), (BitMap*) &gPixMap, 
	 GetPortBounds(GetWindowPort(gWindow), &tempRect1), &gPixMap.bounds, srcCopy, 0L);
	
	SetPortWindowPort(gWindow);
	
	picHandle = OpenPicture(&gPixMap.bounds);
	
	CopyBits((BitMap*) &gPixMap, GetPortBitMapForCopyBits(GetWindowPort(FrontWindow())), &gPixMap.bounds, 
	 GetPortBounds(GetWindowPort(gWindow), &tempRect1), srcCopy, 0L);
	 
	ClosePicture();

    for (count = 0; count < 512; count++)
		header[count] = 0x00;

    anErr = NavGetDefaultDialogOptions(&dialogOptions); 
    dialogOptions.dialogOptionFlags |= kNavSelectDefaultLocation;
    
    anErr = NavPutFile( nil, 
    					&reply, 
    					&dialogOptions, 
    					nil,
                        fileTypeToSave, 
                        creatorType, 
                        nil );

	if (anErr == noErr && reply.validRecord) {
		anErr = AEGetNthPtr(&(reply.selection), 1, typeFSS,
                                &theKeyword, &actualType,
                                &documentFSSpec, sizeof(documentFSSpec),
                                &actualSize );
    if (anErr == noErr) {
  	  
  	  		anErr = FSpCreate(&documentFSSpec, creatorType, fileTypeToSave, smSystemScript);
			if (anErr == dupFNErr) {
				anErr = FSpDelete(&documentFSSpec);
				anErr = FSpCreate(&documentFSSpec, creatorType, fileTypeToSave, smSystemScript);
			}		// this is quick 'n' dirty or there'd be more robust handling here
			
    		// write the file
    		FSpOpenDF(&documentFSSpec, fsRdWrPerm, &refNum );
    		inOutCount = 512;
   			anErr = FSWrite(refNum, &inOutCount, header);		// write the header
    		if (anErr == noErr) {
    			inOutCount = GetHandleSize((Handle)picHandle);
				anErr = FSWrite(refNum,&inOutCount,*picHandle);
    		}
    		FSClose( refNum );
  	  }
  	  reply.translationNeeded = false;
  	  anErr = NavCompleteSave(&reply, kNavTranslateInPlace);
    
 	  NavDisposeReply(&reply);
    }
	
	KillPicture(picHandle);
}

void handleMenuSelection(long result)
{
	int menuID, menuItem;
	RgnHandle rgnHandle = NewRgn();
	
	menuID = HiWord(result);
	menuItem = LoWord(result);
	
	if (menuID == FILE_MENU) {
		if (menuItem == FILE_SAVE)
			saveToPICTFile();
		else if (menuItem == FILE_QUIT)
			gDone = true;
		else if (menuItem == FILE_CLOSE) {
			DisposeWindow(FrontWindow());
			if (FrontWindow() == NULL)
				gDone = true;
		}
		else if (menuItem == FILE_NEW) {
		
		}
		else if (menuItem == FILE_REFRESH) {
			createImage();
			drawImage(FrontWindow());
			QDFlushPortBuffer(GetWindowPort(FrontWindow()), GetPortVisibleRegion(GetWindowPort(FrontWindow()), rgnHandle));
		}
	}
	HiliteMenu(0);
	DisposeRgn(rgnHandle);
}

void handleKeyPress(EventRecord *event)
{
	char	key;

	key = event->message & charCodeMask;
	
	// just check to see if we want to quit...
	
	if ( event->modifiers & cmdKey )		/* Command key down? */
		handleMenuSelection(MenuKey(key));
}

// Apple Event - "Quit"
pascal OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	#pragma unused (messagein,refIn,reply)
	
    gDone = true;
    return noErr;
}

void doEventLoop()
{
	EventRecord anEvent;
	WindowPtr   evtWind;
	short       clickArea;
	Rect        screenRect;

	while (!gDone)
	{
		if (WaitNextEvent( everyEvent, &anEvent, 0, nil ))
		{
			if (anEvent.what == mouseDown)
			{
				clickArea = FindWindow( anEvent.where, &evtWind );
				
				if (clickArea == inMenuBar)
					handleMenuSelection(MenuSelect(anEvent.where));
				else if (clickArea == inDrag)
				{
					//screenRect = (**GetGrayRgn ()).rgnBBox;
					GetRegionBounds(GetGrayRgn(), &screenRect);
					DragWindow( evtWind, anEvent.where, &screenRect );
				}
				else if (clickArea == inContent)
				{
					if (evtWind != FrontWindow())
						SelectWindow( evtWind );
				}
				else if (clickArea == inGoAway)
					if (TrackGoAway( evtWind, anEvent.where ))
						gDone = true;
			}
			else if (anEvent.what == updateEvt)
			{
				evtWind = (WindowPtr)anEvent.message;	
				//SetPort( evtWind );
				SetPortWindowPort( evtWind );
				
				BeginUpdate( evtWind );
				drawImage( evtWind );
				EndUpdate (evtWind);
			}
			else if (anEvent.what == autoKey || anEvent.what == keyDown)
				handleKeyPress(&anEvent);
		}
	}
}