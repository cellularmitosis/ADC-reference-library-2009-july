/*
	File:		main.c
	
	Contains:	Code to generate a QuickTime movie with a QuickTime video effect in it.
	
	Written by:	Scott Kuechle
				(based heavily on QuickTime SDK QTShowEffect sample code by Tim Monroe)

	Copyright:	© 1999 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/25/99		srk		first file
		<2>		10/19/99	srk		changed cross fade effect to explode effect


*/


#ifndef __FONTS__
#include <Fonts.h>
#endif

#ifndef __IMAGECODEC__
#include <ImageCodec.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __MOVIESFORMAT__
#include <MoviesFormat.h>
#endif

#ifndef __MEDIAHANDLERS__
#include <MediaHandlers.h>
#endif

#ifndef __NUMBERFORMATTING__
#include <NumberFormatting.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __GESTALT__
#include <Gestalt.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif

#ifndef __PRINTING__
#include <Printing.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __TRAPS__
#include <Traps.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __FILETYPESANDCREATORS__
#include <FileTypesAndCreators.h>
#endif

#if TARGET_OS_WIN32
		// Windows headers
	#define	STRICT
	#include <windows.h>
		// QTML stuff
	#include "QTML.h"
#endif


#include "Main.h"
#include "QTEffects.h"


// miscellaneous constants
#define kMsgDialogRsrcID				129
#define kMsgItemID						3	


#if TARGET_OS_WIN32
	short  gAppResID;
#endif

#if TARGET_OS_MAC


////////////////////
//
// Main (Mac)
//
////////////////////

void main( void )
{
	Movie movie;


	InitMacToolbox ();
	QuickTimeInit();
		/* Create our effects movie */
	movie = QTEffects_CreateEffectsMovie ();
}

#else if TARGET_OS_WIN32


////////////////////
//
// Main (Windows)
//
////////////////////

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCmdLine, int nCmdShow)

{
	Movie movie;


	QuickTimeInit();

		/* Create our effects movie */
	movie = QTEffects_CreateEffectsMovie ();

	#if TARGET_OS_WIN32
		if (gAppResID != -1)
			CloseResFile(gAppResID);
	#endif

	return 0;
}

#endif


#if TARGET_OS_MAC

////////////////////
//
// InitMacToolbox
// Standard Macintosh Toolbox Initialization
//
////////////////////

static void InitMacToolbox (void)
{
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
}

#endif

////////////////////
//
// QuickTimeInit
// Initialize QuickTime here
//
////////////////////

static void QuickTimeInit (void)
{
	OSErr err;

	#if TARGET_OS_WIN32
	char   szAppPathName[256];
	FSSpec resFSSpec;

	err = InitializeQTML(0L);
	CheckError (err, "InitializeQTML error" );
	
	GetModuleFileName(0, szAppPathName, 256);
	NativePathNameToFSSpec(szAppPathName, &resFSSpec, kFullNativePath);
	gAppResID = FSpOpenResFile(&resFSSpec, fsRdWrPerm);

	#endif

	err = EnterMovies ();

	CheckError (err, "EnterMovies error" );
}

////////////////////
//
// Utils_Macintosh_DisplayMsg
// Displays error messages for Macintosh sample code.
//
////////////////////

#if TARGET_OS_MAC
static void Macintosh_DisplayMsg(char *msg)
{
	DialogPtr theDlog;
	Handle item = NULL;
	Rect box;

		theDlog = GetNewDialog(kMsgDialogRsrcID, NULL, (WindowPtr)-1);
		if (theDlog != NULL)
		{
			short itemType;
			
				GetDialogItem(theDlog, kMsgItemID, &itemType, &item, &box);
				if (item != NULL)
				{
					short itemHit;
					
						SetDialogItemText(item, c2pstr(msg));
						ModalDialog(NULL, &itemHit);
						DisposeDialog(theDlog);
						p2cstr((StringPtr)msg);	/* restore C-string */
				}
		}

}
#endif

////////////////////
//
// Utils_Win32_DisplayMsg
// Displays error messages for Win95/NT sample code.
//
////////////////////

#if TARGET_OS_WIN32
static void Win32_DisplayMsg(char *msg)
{

	MessageBox(NULL, msg, "", MB_OK);
}
#endif


////////////////////
//
// CheckError
// Displays error message if an error occurred
//
////////////////////

void CheckError(OSErr error, char *msg)
{
	if (error == noErr)
	{
		return;
	}
	if (strlen(msg) > 0)
	{
		#if TARGET_OS_MAC
			Macintosh_DisplayMsg(msg);
		#else if TARGET_OS_WIN32
			Win32_DisplayMsg(msg);
		#endif
		

		ExitToShell();

	}
}
