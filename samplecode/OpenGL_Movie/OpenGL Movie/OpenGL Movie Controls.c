/*
	File:		OpenGLMovieControls.c

	Contains:	OpenGL Movie demo

	Copyright:	2000 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):
        <7+>     12/6/00    ggs     
         <7>     12/5/00    ggs     Added Mac OS X check
         <6>     12/5/00    ???     removed extranious headers
         <5>     12/5/00    ggs     change comment
         <4>     12/4/00    ggs     Fixed some Carbon and mac OS X things
         <3>    11/25/00    ggs     fixed non-Carbon parts
         <2>    11/25/00    ggs     fix header
         <1>    11/25/00    ggs     Intial Add
        <1+>    11/25/00    ggs     fix header
        
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
    #include "Carbon Include.h"
    #include <Carbon/Carbon.h>
#else
    #include <ControlDefinitions.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "OpenGLMovie.h"

//-----------------------------------------------------------------------------------------------------------------------

static Boolean IsMacOSXRuntime (void);
void PStrCopy (Str255 s1, const Str255 s2 );	// copy s2 -> s1
static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[]);
void SetPrefs (struct structMovieSettings * pSettings);
void GetPrefs (struct structMovieSettings * pSettings);
pascal Boolean OptionsFilter(DialogPtr dlg, EventRecord *event, short *itemHit);
void GetDialogValues (DialogPtr pDialog, struct structMovieSettings * pSettings);
void SetDialogValues (DialogPtr pDialog, struct structMovieSettings * pSettings);

//-----------------------------------------------------------------------------------------------------------------------

// IsMacOSXRuntime

// Runtime check to see if we are running on Mac OS X

static Boolean IsMacOSXRuntime (void)
{
    UInt32 response;
    return (Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000);
}

//-----------------------------------------------------------------------------------------------------------------------

void PStrCopy (Str255 s1, const Str255 s2 )	// copy s2 -> s1
{
	register int length = *s2 + 1;
	while (length--) 
		*s1++ = *s2++;
}

//-----------------------------------------------------------------------------------------------------------------------

static Handle NewOpenHandle(OSType applicationSignature, short numTypes, OSType typeList[])
{
	Handle hdl = NULL;
	
	if ( numTypes > 0 )
	{
	
		hdl = NewHandle(sizeof(NavTypeList) + numTypes * sizeof(OSType));
	
		if ( hdl != NULL )
		{
			NavTypeListHandle open		= (NavTypeListHandle)hdl;
			
			(*open)->componentSignature = applicationSignature;
			(*open)->osTypeCount		= numTypes;
			BlockMoveData(typeList, (*open)->osType, numTypes * sizeof(OSType));
		}
	}
	
	return hdl;
}

// --------------------------------------------------------------------------

void SetPrefs (struct structMovieSettings * pSettings)
{
	FILE * filePrefs;
	filePrefs = fopen ("Prefs File", "wb");
	if  (!filePrefs)						// could not open
		return;
	fwrite (pSettings, sizeof (struct structMovieSettings), 1, filePrefs);
	fclose (filePrefs);
}

// --------------------------------------------------------------------------

void GetPrefs (struct structMovieSettings * pSettings)
{
	FILE * filePrefs;
	filePrefs = fopen ("Prefs File", "rb");
	if  (!filePrefs)						// could not open
		return;
	fread (pSettings, sizeof (struct structMovieSettings), 1, filePrefs);
	fclose (filePrefs);
}

//-----------------------------------------------------------------------------------------------------------------------

pascal Boolean OptionsFilter (DialogPtr theDialog, EventRecord *theEvent, short *itemHit) 
{
	Boolean result = false;
	OSErr err = noErr;
	ModalFilterUPP standardProc;
	if ((theEvent->what == updateEvt) && (DialogPtr)theEvent->message != theDialog) 
	{
	//	err = DispatchWindowUpdate ((WindowPtr)theEvent->message);
	} 
	else if ((theEvent->what == activateEvt) && (DialogPtr)theEvent->message != theDialog) 
	{
	//	DoActivate (theEvent, true);
	} 
	else 
	{
		err = GetStdFilterProc (&standardProc);
		if (err == noErr) 
		{
			result = InvokeModalFilterUPP (theDialog, theEvent, itemHit, standardProc);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------------------------------------------------

void GetDialogValues (DialogPtr pDialog, struct structMovieSettings * pSettings)
{
	short		itemType; 
	Rect		itemRect;
	Handle		itemHandle;
	short		theValue;
	
	// grab control values
	// Display Size
	GetDialogItem (pDialog, 2, &itemType, &itemHandle, &itemRect);
	theValue = GetControlValue ((ControlHandle) itemHandle);
	switch (theValue)
	{
		case 1:
			pSettings->wWindowWidth = 400;
			pSettings->wWindowHeight = 300;
			break;
		case 2:
			pSettings->wWindowWidth = 512;
			pSettings->wWindowHeight = 384;
			break;
		case 3:
			pSettings->wWindowWidth = 640;
			pSettings->wWindowHeight = 480;
			break;
		case 4:
			pSettings->wWindowWidth = 800;
			pSettings->wWindowHeight = 600;
			break;
		case 5:
			pSettings->wWindowWidth = 1024;
			pSettings->wWindowHeight = 768;
			break;
		case 6:
			pSettings->wWindowWidth = 1280;
			pSettings->wWindowHeight = 1024;
			break;
		case 7:
			pSettings->wWindowWidth = 1600;
			pSettings->wWindowHeight = 1200;
			break;
	}
	GetDialogItem (pDialog, 3, &itemType, &itemHandle, &itemRect);
	theValue = GetControlValue ((ControlHandle) itemHandle);
	switch (theValue)
	{
		case 1:
			pSettings->wTextureSize = 32;
			break;
		case 2:
			pSettings->wTextureSize = 64;
			break;
		case 3:
			pSettings->wTextureSize = 128;
			break;
		case 4:
			pSettings->wTextureSize = 256;
			break;
		case 5:
			pSettings->wTextureSize = 512;
			break;
		case 6:
			pSettings->wTextureSize = 1024;
			break;
		case 7:
			pSettings->wTextureSize = 2048;
			break;
	}
	GetDialogItem (pDialog, 4, &itemType, &itemHandle, &itemRect);
	theValue = GetControlValue ((ControlHandle) itemHandle);
	switch (theValue)
	{
		case 1:
			pSettings->wOffScreenDepth = 16;
			break;
		case 2:
			pSettings->wOffScreenDepth = 32;
			break;
	}
	GetDialogItem (pDialog, 5, &itemType, &itemHandle, &itemRect);
	pSettings->fFullScreen = GetControlValue ((ControlHandle) itemHandle);
	GetDialogItem (pDialog, 6, &itemType, &itemHandle, &itemRect);
	pSettings->fTryPackedPixel = GetControlValue ((ControlHandle) itemHandle);
	GetDialogItem (pDialog, 7, &itemType, &itemHandle, &itemRect);
	pSettings->fDirectTexturing = GetControlValue ((ControlHandle) itemHandle);
	GetDialogItem (pDialog, 8, &itemType, &itemHandle, &itemRect);
	pSettings->fUseFog = GetControlValue ((ControlHandle) itemHandle);
	GetDialogItem (pDialog, 9, &itemType, &itemHandle, &itemRect);
	pSettings->fVBLSync = GetControlValue ((ControlHandle) itemHandle);
	GetDialogItem (pDialog, 10, &itemType, &itemHandle, &itemRect);
	pSettings->fLoopMovie = GetControlValue ((ControlHandle) itemHandle);
	GetDialogItem (pDialog, 11, &itemType, &itemHandle, &itemRect);
	pSettings->fUseWaitNextEvent = GetControlValue ((ControlHandle) itemHandle);

}

//-----------------------------------------------------------------------------------------------------------------------

void SetDialogValues (DialogPtr pDialog, struct structMovieSettings * pSettings)
{
	short		itemType; 
	Rect		itemRect;
	Handle		itemHandle;
	
	GetDialogItem (pDialog, 2, &itemType, &itemHandle, &itemRect);
	switch (pSettings->wWindowWidth)
	{
		case 400:
			SetControlValue ((ControlHandle) itemHandle, 1);
			break;
		case 512:
			SetControlValue ((ControlHandle) itemHandle, 2);
			break;
		case 640:
			SetControlValue ((ControlHandle) itemHandle, 3);
			break;
		case 800:
			SetControlValue ((ControlHandle) itemHandle, 4);
			break;
		case 1024:
			SetControlValue ((ControlHandle) itemHandle, 5);
			break;
		case 1280:
			SetControlValue ((ControlHandle) itemHandle, 6);
			break;
		case 1600:
			SetControlValue ((ControlHandle) itemHandle, 7);
			break;
	}
	GetDialogItem (pDialog, 3, &itemType, &itemHandle, &itemRect);
	switch (pSettings->wTextureSize)
	{
		case 32:
			SetControlValue ((ControlHandle) itemHandle, 1);
			break;
		case 64:
			SetControlValue ((ControlHandle) itemHandle, 2);
			break;
		case 128:
			SetControlValue ((ControlHandle) itemHandle, 3);
			break;
		case 256:
			SetControlValue ((ControlHandle) itemHandle, 4);
			break;
		case 512:
			SetControlValue ((ControlHandle) itemHandle, 5);
			break;
		case 1024:
			SetControlValue ((ControlHandle) itemHandle, 6);
			break;
		case 2048:
			SetControlValue ((ControlHandle) itemHandle, 7);
			break;
	}
	GetDialogItem (pDialog, 4, &itemType, &itemHandle, &itemRect);
	switch (pSettings->wOffScreenDepth)
	{
		case 16:
			SetControlValue ((ControlHandle) itemHandle, 1);
			break;
		case 32:
			SetControlValue ((ControlHandle) itemHandle, 2);
			break;
	}
	GetDialogItem (pDialog, 5, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fFullScreen);
	GetDialogItem (pDialog, 6, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fTryPackedPixel);
	GetDialogItem (pDialog, 7, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fDirectTexturing);
	GetDialogItem (pDialog, 8, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fUseFog);
	GetDialogItem (pDialog, 9, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fVBLSync);
	GetDialogItem (pDialog, 10, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fLoopMovie);
	GetDialogItem (pDialog, 11, &itemType, &itemHandle, &itemRect);
	SetControlValue ((ControlHandle) itemHandle, pSettings->fUseWaitNextEvent);

	// ensure proper initial highlighting
	GetDialogItem (pDialog, 6, &itemType, &itemHandle, &itemRect);
	if (GetControlValue ((ControlHandle) itemHandle) == false)
	{
            GetDialogItem (pDialog, 7, &itemType, &itemHandle, &itemRect);
            HiliteControl ((ControlHandle)itemHandle, 255);
            GetDialogItem (pDialog, 4, &itemType, &itemHandle, &itemRect);
            HiliteControl ((ControlHandle)itemHandle, 255);
	}
	else
	{
            if (!IsMacOSXRuntime ()) // glm not available in Mac OS X
            {
		GetDialogItem (pDialog, 7, &itemType, &itemHandle, &itemRect);
		HiliteControl ((ControlHandle)itemHandle, 0);
            }
            GetDialogItem (pDialog, 4, &itemType, &itemHandle, &itemRect);
            HiliteControl ((ControlHandle)itemHandle, 0);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

Boolean GetSettings (void)
{
	DialogPtr	optionsDialog;
	short		itemType, theItem; 
	Rect		itemRect;
	Handle		itemHandle;
	Boolean		done = false;
	short		theValue;
	ModalFilterUPP myFilterUPP;
	
	optionsDialog = GetNewDialog (128, NULL, (WindowPtr) -1);
	if (!optionsDialog)
		return false;
#if !TARGET_API_MAC_CARBON
	SetPort ((GrafPtr) optionsDialog);
#else
	SetPort (GetDialogPort (optionsDialog));
#endif
	SetDialogDefaultItem (optionsDialog, 1) ;
	myFilterUPP = NewModalFilterUPP (OptionsFilter);
	GetPrefs (&gMovieSettings); // will get prefs
	SetDialogValues (optionsDialog, &gMovieSettings);

	if (IsMacOSXRuntime ()) // glm not available in Mac OS X
	{
		// Set to true and disable for direct texturing always
		GetDialogItem (optionsDialog, 7, &itemType, &itemHandle, &itemRect);
		SetControlValue ((ControlHandle) itemHandle, 1);
		HiliteControl ((ControlHandle)itemHandle, 255);
	}

	do
	{
		ModalDialog (myFilterUPP, &theItem);
		if (theItem == 1)
		{
			done = true;
			GetDialogValues (optionsDialog, &gMovieSettings);
			SetPrefs (&gMovieSettings); // save prefs
		}
		else if (theItem > 4) // handle check boxes
		{
			GetDialogItem (optionsDialog, theItem, &itemType, &itemHandle, &itemRect);
			theValue = GetControlValue ((ControlHandle) itemHandle);
			theValue = 1 - theValue;
			SetControlValue ((ControlHandle) itemHandle, theValue);
		}
		GetDialogItem (optionsDialog, 6, &itemType, &itemHandle, &itemRect);
		if (GetControlValue ((ControlHandle) itemHandle) == false)
		{
			GetDialogItem (optionsDialog, 7, &itemType, &itemHandle, &itemRect);
			HiliteControl ((ControlHandle)itemHandle, 255);
			GetDialogItem (optionsDialog, 4, &itemType, &itemHandle, &itemRect);
			HiliteControl ((ControlHandle)itemHandle, 255);
		}
		else
		{
                        if (!IsMacOSXRuntime ()) // glm not available in Mac OS X
                        {
                            GetDialogItem (optionsDialog, 7, &itemType, &itemHandle, &itemRect);
                            HiliteControl ((ControlHandle)itemHandle, 0);
                        }
			GetDialogItem (optionsDialog, 4, &itemType, &itemHandle, &itemRect);
			HiliteControl ((ControlHandle)itemHandle, 0);
		}
	} while (!done);
	DisposeDialog (optionsDialog);
	DisposeModalFilterUPP (myFilterUPP);
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

OSErr OpenMovie (Movie * pMovie)
{
	NavReplyRecord replyNav;
	NavDialogOptions optionsNav;
	NavTypeListHandle	openList	= NULL;
	AEKeyword theKeyword;
	DescType actualType;
	Size actualSize;
	FSSpec fsspecMovie;
	OSType typeList [1] = {MovieFileType};
	short numTypes = 1;
	OSType applicationSignature = FOUR_CHAR_CODE('????');
	short resFile = 0;
	short resID = 0;
	Str255 movieName;
	Boolean wasChanged;
	OSErr anErr;
	
	//Initialize dialog options structure and set default values
	anErr = NavGetDefaultDialogOptions (&optionsNav);
	optionsNav.dialogOptionFlags -= kNavDontAddTranslateItems;
	optionsNav.dialogOptionFlags = kNavNoTypePopup;
	PStrCopy (optionsNav.clientName, "\pOpenGL Movie");
	optionsNav.location.h = -1;
	optionsNav.location.v = -1;
	PStrCopy (optionsNav.windowTitle, "\p");
	PStrCopy (optionsNav.actionButtonLabel, "\pSelect");
	PStrCopy (optionsNav.cancelButtonLabel, "\p");
	PStrCopy (optionsNav.savedFileName, "\p");
	PStrCopy (optionsNav.message, "\pSelect movie to play.");
	optionsNav.preferenceKey = NULL;
	optionsNav.popupExtension = NULL;

	openList = (NavTypeListHandle) NewOpenHandle (applicationSignature, numTypes, typeList);
	if ( openList )
		HLock((Handle)openList);
	
	anErr = NavGetFile(NULL, &replyNav, &optionsNav, NULL, NULL, 0, openList, NULL);

	if (anErr == noErr && replyNav.validRecord)
	{
		anErr = AEGetNthPtr (&(replyNav.selection), 1, typeFSS, &theKeyword, &actualType, &fsspecMovie, sizeof (fsspecMovie), &actualSize);
		anErr = OpenMovieFile (&fsspecMovie, &resFile, fsRdPerm);
		if (anErr == noErr)
		{
			anErr = NewMovieFromFile (pMovie, resFile, &resID, movieName, newMovieActive, &wasChanged);
			CloseMovieFile (resFile);
			if (noErr != anErr)
				DebugStr ("\pCould not get movie from file");
		}
		else
			DebugStr ("\pCould open movie file");
		
		NavDisposeReply(&replyNav);
	}
	
	if (openList != NULL)
	{
		HUnlock((Handle)openList);
		DisposeHandle((Handle)openList);
	}
	
	if (NULL == pMovie)
	{
		// user did not select file
		return fnfErr;	
	}
	return noErr;
}