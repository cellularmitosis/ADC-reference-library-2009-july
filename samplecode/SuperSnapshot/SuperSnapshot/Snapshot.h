/*
	File:		Snapshot.h

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
				
				(Updated Description)
				While "carbonizing" this particular sample, I decided 
				to add a bunch of "bells and whistles" to the sample 
				which I had seen in other samples.  In addition, I decided 
				to do a near "full" carbonization of the sample.  I've 
				added some "conditional" logic so that the menus will 
				appear correctly for OS 9 and X (no File->Quit menu 
				under X). I also added support for the "Quit" AppleEvent, 
				so that under OS X selecting "Application Menu"->Quit would 
				exit the program.  In addition, dynamic resizing of windows, 
				setting the size of the windows, saving the screenshot to 
				a pict file, refreshing the snapshot, as well as multiple 
				windows were all added to the program.  Lastly, on OS 9, 
				a menu items will set the desktop picture using Apple 
				Events. While on OS X, a full screen mode display of a 
				snapshot will be displayed (since the method to set the 
				desktop picture on OS X was not available at the time 
				of this writing).									

	Written by: JM	

	Copyright:	Copyright © 1991-2000 by Apple Computer, Inc., All Rights Reserved.

				You may incorporate this Apple sample source code into your program(s) without
				restriction. This Apple sample source code has been provided "AS IS" and the
				responsibility for its operation is yours. You are not permitted to redistribute
				this Apple sample source code as "Apple sample source code" after having made
				changes. If you're going to re-distribute the source, we require that you make
				it clear in the source that the code was descended from Apple sample source
				code, but that you've made changes.
*/

#ifndef __SNAPSHOT__
#define __SNAPSHOT__

//#includes...

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#include <QuickTime/Movies.h>
#else
#include <Carbon.h>
#include <Movies.h>
#endif

/* Constant Declarations */

#define MENU_BAR_ID			128
#define MENU_BAR_IDX 		129

#define ABOUT				1
#define ABOUTDLG			128

enum {
	ABOUT_MENU = 128,
	FILE_MENU,
	FILE_MENUX,
	SIZE_MENU,
	SPECIAL_MENU,
	SPECIAL_MENUX
};

//Note this constant does not include the "Quit" item
#define NUMBER_OF_FILE_MENU_ITEMS 4
enum {
	FILE_NEW = 1,
	FILE_REFRESH,
	FILE_CLOSE,
	FILE_SAVE,
	FILE_QUIT
};

#define NUMBER_OF_SIZES 3
enum {
	SIZE_QUARTER_SCALE = 1,
	SIZE_HALF_SCALE,
	SIZE_FULL_SCALE
};



#define NUMBER_OF_SPECIALS 1
enum {
	SPECIAL_CONFUSING = 1
};

// Prototypes
void initMac();
void destroyAllWindows();
void setUp();
Boolean onOSX();
void handleMenuSelection(long result);
void handleKeyPress(EventRecord *);
void saveToPICTFile();
WindowPtr createWindow();
void resizeWindow(WindowPtr theWindow);
void doDynamicResizing(WindowPtr theWindow);
void doConfusion();
void disposeWindow(WindowPtr);
void writePictToFile(FSSpec *fspec, PicHandle picHandle);
OSErr SetDesktopPict(AEDesc* pAEDesc,SInt32 pIndex);
OSErr MakePictureProperty(SInt32 pIndex, AEDesc* containerObjPtr, AEDesc* propertyObjPtr);
void calculateSystemBounds();
PixMapHandle createScreenPixMap();
void drawImage(WindowPtr);
void doNewSnapshot();
void adjustMenus();
pascal OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn);
void doEventLoop();
void doTrickEventLoop();
void processEvent(EventRecord *anEvent);

OSErr LaunchProcessBySignature(const OSType pTargetType,const OSType pTargetCreator,ProcessSerialNumberPtr psnPtr);
OSErr Find_DTDB_APPL(const OSType pTargetCreator,FSSpec* pFSSpecPtr);
OSErr Search_Volumes(const OSType pTargetType,const OSType pTargetCreator,FSSpec* pFSSpecPtr);
OSErr Search_Volume(const SInt16 pVRefNum,const OSType pTargetType,const OSType pTargetCreator,FSSpec* pFSSpecPtr);

pascal	OSErr	AEHMakeEventSignatureTarget( const OSType targetType,
												  const OSType targetCreator,
												  const AEEventClass eventClass,
												  const AEEventID eventID,
														AppleEvent *theEventPtr );
pascal	OSErr	AEHSendEventNoReturnValue( const AEIdleUPP idleProcUPP,
										   const AppleEvent *theEvent );
pascal OSErr	AEHMakeEventProcessTarget( const ProcessSerialNumberPtr psnPtr,
										   const AEEventClass eventClass,
										   const AEEventID eventID,
												 AppleEvent *theEventPtr );
pascal	OSErr	AEHGetHandlerError( const AppleEvent *reply );
pascal	OSErr	FindProcessBySignature( const OSType targetType,
										const OSType targetCreator,
											  ProcessSerialNumberPtr psnPtr );
pascal	OSErr	OHMakeAliasDescFromFSSpec( const FSSpecPtr fssPtr,
												  AEDesc *aliasDescPtr );
pascal	OSErr	OHMakeAliasDesc( const AliasHandle aliasHandle,
										AEDesc *aliasDescPtr );
										
#endif