/*
 *  OpenGL_Image_Options-Dialog.c
 *  OpenGL Image
 *
 *  Created by gstahl on Fri Aug 17 2001.
 
 	Copyright:	Copyright © 2001 Apple Computer, Inc., All Rights Reserved

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

#ifdef __APPLE_CC__ // project builder
	#include <Carbon/Carbon.h> // standard carbon

#else // CodeWarrior
	#include <CarbonEvents.h> 
	#include <IBCarbonRuntime.h>
	#include <Gestalt.h>
	#include <Controls.h>

#endif

#include "OpenGL_Image.h" // our header

// ---------------------------------

EventHandlerUPP gTexWinEvtHandler;		// window event handler

// ---------------------------------

static Boolean CompareControlIDs (OSType LHSsignature, SInt32 LHSid, OSType RHSsignature, SInt32 RHSid);
static SInt16 GetControlValueByID (WindowRef window, OSType signature, SInt32 id);
static void SetControlValueByID (WindowRef window, OSType signature, SInt32 id, SInt16 value);
static pascal OSStatus myTexWinEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData);

// ---------------------------------

enum { // control enums from nib 
    kCntlRadioScaleToTex = 1,
    kCntlRadioTileToTex,
    kCntlPopUpScale,
    kCntlCheckOverlap,
    kCntlPopUpMaxTex,
    kCntlCheckClientTex,
    kCntlRadioAGP,
	kCntlCheckNPOTTextures,

    kNumControlsPlus1
};

Boolean gfNPOTCheckValue;
Boolean	gfOverlapCheckValue;
Boolean gfTileTextureRadioValue;

// ---------------------------------

// returns true if they match
static Boolean CompareControlIDs (OSType LHSsignature, SInt32 LHSid, OSType RHSsignature, SInt32 RHSid)
{
	if ((LHSsignature == RHSsignature) && (LHSid == RHSid))
		return true;
	else
		return false;
}

// ---------------------------------

// sets a control of id based on value
static SInt16 GetControlValueByID (WindowRef window, OSType signature, SInt32 id)
{
	SInt16 value;
    ControlID idControl;
    ControlRef control;
    
    idControl.signature = signature;
    idControl.id = id; 
    GetControlByID (window, &idControl, &control);
    value = GetControlValue (control);
	return value;
}

// ---------------------------------

// sets a control of id based on value
static void SetControlValueByID (WindowRef window, OSType signature, SInt32 id, SInt16 value)
{
    ControlID idControl;
    ControlRef control;
    
    idControl.signature = signature;
    idControl.id = id; 
    GetControlByID (window, &idControl, &control);
    SetControlValue (control, value);
}

// ---------------------------------

static pascal OSStatus myTexWinEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler)
	WindowRef			window = (WindowRef) userData;
    OSStatus			result = eventNotHandledErr;

    if (GetEventKind (event) == kEventControlHit)
    {
        ControlRef control;
		ControlID controlID = { 'ImAt', 0 };
		
        GetEventParameter (event, kEventParamDirectObject, typeControlRef, NULL, sizeof(control), NULL, &control);
		GetControlID (control, &controlID);

		if (CompareControlIDs ('ImAt', kCntlRadioScaleToTex, controlID.signature, controlID.id) && gfTileTextureRadioValue) // if the scale radio was clicked
		{
			gfTileTextureRadioValue = 1 - gfTileTextureRadioValue;
			SetControlValueByID (window, 'ImAt', kCntlRadioTileToTex, false); //set tile radio to false
			// disable overlap selection while selected to false
			gfOverlapCheckValue = GetControlValueByID (window, 'ImAt', kCntlCheckOverlap);
			SetControlValueByID (window, 'ImAt', kCntlCheckOverlap, false); //set tile radio to false
			controlID.id = kCntlCheckOverlap;
			GetControlByID (window, &controlID, &control);
			DeactivateControl (control);
			// disable NPOT texture selection while selected to false
			gfNPOTCheckValue = GetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures);
			SetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures, false); //set check to false
			controlID.id = kCntlCheckNPOTTextures;
			GetControlByID (window, &controlID, &control);
			DeactivateControl (control);
			// enable scale selection
			controlID.id = kCntlPopUpScale;
			GetControlByID (window, &controlID, &control);
			ActivateControl (control);
		}
		else if (CompareControlIDs ('ImAt', kCntlRadioTileToTex, controlID.signature, controlID.id) && !gfTileTextureRadioValue) // if the tile radio was clicked
		{
			gfTileTextureRadioValue = 1 - gfTileTextureRadioValue;
			// disable text scale selection
			SetControlValueByID (window, 'ImAt', kCntlRadioScaleToTex, false); //set tile radio to false
			controlID.id = kCntlPopUpScale;
			GetControlByID (window, &controlID, &control);
			DeactivateControl (control);
			// enable overlap selection
			SetControlValueByID (window, 'ImAt', kCntlCheckOverlap, gfOverlapCheckValue); //set overlap to orginal value
			controlID.id = kCntlCheckOverlap;
			GetControlByID (window, &controlID, &control);
			ActivateControl (control);
			// enable NPOT textures selection
			if (false == gpOpenGLCaps->f_ext_texture_rectangle)
			{
				// disable NPOT Textures selection while selected to false
				SetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures, false); //set tile radio to false
				controlID.id = kCntlCheckNPOTTextures;
				GetControlByID (window, &controlID, &control);
				DeactivateControl (control);
			}
			else
			{
				SetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures, gfNPOTCheckValue); //set overlap to orginal value
				controlID.id = kCntlCheckNPOTTextures;
				GetControlByID (window, &controlID, &control);
				ActivateControl (control);
			}
		}
		else if (CompareControlIDs ('ImAt', 0, controlID.signature, controlID.id)) // if the tile radio was clicked
		{
			// done
			QuitAppModalLoopForWindow (window);
		}
		else if (CompareControlIDs ('ImAt', kCntlPopUpMaxTex, controlID.signature, controlID.id)) // if the tile radio was clicked
		{
			MenuRef refMenu;
			unsigned short i;
			long currentMaxTexSize = GetControlValueByID (window, 'ImAt', kCntlPopUpMaxTex);
			long currentTexScale = GetControlValueByID (window, 'ImAt', kCntlPopUpScale);
		
			// ensure texture scale seeting is also in valid range for GPU
			SetControlValueByID (window, 'ImAt', kCntlPopUpScale, (currentTexScale - 3) < currentMaxTexSize ? currentTexScale : (currentMaxTexSize + 3));
			controlID.id = kCntlPopUpScale; 
			GetControlByID (window, &controlID, &control);
			refMenu = GetControlPopupMenuHandle (control);
			for (i = 1; i <= CountMenuItems (refMenu); i++)
				if (i < currentMaxTexSize + 3 + 1)
					EnableMenuItem (refMenu, i);
				else
					DisableMenuItem (refMenu, i);
		}
	}
    return result;
}

#pragma mark -
// ==================================
// public

void GetImageOptions (pRecGLCap pOpenGLCaps, Boolean *pfTileTextures, short * pTextureScale, Boolean * pfOverlapTextures, 
					  short * pMaxTextureSize, Boolean * pfClientTextures, Boolean *pfAGPTextures, Boolean *pfNPOTTextures)
{
	// build window
 	UInt32 			response;
	IBNibRef 		nibRef;
    OSStatus		err;
	WindowRef		window;
    EventHandlerRef	ref;
    EventTypeSpec	list[] = { { kEventClassControl, kEventControlHit } };
	ControlRef 		control;
	ControlID 		controlID = { 'ImAt', 0 };
 
	if ((Gestalt (gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000))
		err = CreateNibReference (CFSTR ("main X"), &nibRef);
	else
		err = CreateNibReference (CFSTR ("main 9"), &nibRef);
	if (noErr == err)
		err = CreateWindowFromNib (nibRef, CFSTR ("TextureWindow"), &window);
    DisposeNibReference (nibRef);
    gTexWinEvtHandler = NewEventHandlerUPP (myTexWinEvtHndlr);
    InstallWindowEventHandler (window, gTexWinEvtHandler, GetEventTypeCount (list), list, (void *) window, &ref);
	
	// set initial values
	{
		MenuRef refMenu;
		unsigned short i;
		short maxUserSizeBits = 0, maxGPUSizeBits = 0, maxGPUSize = pOpenGLCaps->maxTextureSize;
		
		while (maxGPUSize > 1) 
		{ 
			maxGPUSizeBits++;
			maxGPUSize = maxGPUSize >> 1;
		}
		while (*pMaxTextureSize > 1) 
		{ 
			maxUserSizeBits++;
			*pMaxTextureSize = *pMaxTextureSize >> 1;
		}
		// restrict texture size to GPU maximum
		maxUserSizeBits = maxUserSizeBits < maxGPUSizeBits ? maxUserSizeBits : maxGPUSizeBits;
		
		// ensure menu reflects max sizes for GPU
		controlID.id = kCntlPopUpMaxTex; 
		GetControlByID (window, &controlID, &control);
		refMenu = GetControlPopupMenuHandle (control);
		for (i = (unsigned short) maxGPUSizeBits - 3; i <= CountMenuItems (refMenu); i++)
			DisableMenuItem (refMenu, i);		
		SetControlValueByID (window, 'ImAt', kCntlPopUpMaxTex, maxUserSizeBits - 4);
		
		// ensure texture scale seeting is also in valid range for GPU
		*pTextureScale = *pTextureScale < maxUserSizeBits ? *pTextureScale : maxUserSizeBits;
		if (*pTextureScale == kNone) // look for kNone settings (from using NPOT textures)
			*pTextureScale = maxUserSizeBits;
		controlID.id = kCntlPopUpScale; 
		GetControlByID (window, &controlID, &control);
		refMenu = GetControlPopupMenuHandle (control);
		for (i = (unsigned short) maxUserSizeBits; i <= CountMenuItems (refMenu); i++)
			DisableMenuItem (refMenu, i);		
		SetControlValueByID (window, 'ImAt', kCntlPopUpScale, *pTextureScale - 1); // enumerated value
	}
	if (pOpenGLCaps->f_ext_client_storage)
		SetControlValueByID (window, 'ImAt', kCntlCheckClientTex, *pfClientTextures); 
	else
	{
		*pfClientTextures = false;
		// disable client textures
		controlID.id = kCntlCheckClientTex;
		GetControlByID (window, &controlID, &control);
		DeactivateControl (control);
	}
	gfOverlapCheckValue = *pfOverlapTextures;
	SetControlValueByID (window, 'ImAt', kCntlCheckOverlap, *pfOverlapTextures); 
	if (true == gpOpenGLCaps->f_ext_texture_rectangle)
	{
		gfNPOTCheckValue = *pfNPOTTextures;
		SetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures, gfNPOTCheckValue);
	}
	else
	{
		gfNPOTCheckValue = false;
		SetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures, gfNPOTCheckValue);
	}
	gfTileTextureRadioValue = *pfTileTextures;
	if (true == gfTileTextureRadioValue) // if tiling
	{
		SetControlValueByID (window, 'ImAt', kCntlRadioTileToTex, true); //set tile radio to false
		SetControlValueByID (window, 'ImAt', kCntlRadioScaleToTex, false); //set tile radio to false
		// disable scale selection
		controlID.id = kCntlPopUpScale;
		GetControlByID (window, &controlID, &control);
		DeactivateControl (control);
		// enable overlap selection
		controlID.id = kCntlCheckOverlap;
		GetControlByID (window, &controlID, &control);
		ActivateControl (control);
		// enable NPOT textures selection
		controlID.id = kCntlCheckNPOTTextures;
		GetControlByID (window, &controlID, &control);
		ActivateControl (control);
	}
	else
	{
		SetControlValueByID (window, 'ImAt', kCntlRadioTileToTex, false); //set tile radio to false
		SetControlValueByID (window, 'ImAt', kCntlRadioScaleToTex, true); //set tile radio to false
		// disable overlap selection while selected to false
		SetControlValueByID (window, 'ImAt', kCntlCheckOverlap, false); //set tile radio to false
		controlID.id = kCntlCheckOverlap;
		GetControlByID (window, &controlID, &control);
		DeactivateControl (control);
		// disable NPOT Textures selection while selected to false
		SetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures, false); //set tile radio to false
		controlID.id = kCntlCheckNPOTTextures;
		GetControlByID (window, &controlID, &control);
		DeactivateControl (control);
		// enable scale selection
		controlID.id = kCntlPopUpScale;
		GetControlByID (window, &controlID, &control);
		ActivateControl (control);
	}
	// ensure texture rectangle disabled if not avialable
	if (false == gpOpenGLCaps->f_ext_texture_rectangle)
	{
		// disable NPOT Textures selection
		controlID.id = kCntlCheckNPOTTextures;
		GetControlByID (window, &controlID, &control);
		DeactivateControl (control);
	}
	// set agp texturing option
	if (CheckMacOSX ())
	{
		SetControlValueByID (window, 'ImAt', kCntlRadioAGP, *pfAGPTextures + 1);
	} 
	else
	{
		*pfAGPTextures = false;
		SetControlValueByID (window, 'ImAt', kCntlRadioAGP, *pfAGPTextures + 1);
		controlID.id = kCntlRadioAGP;
		GetControlByID (window, &controlID, &control);
		DeactivateControl (control);
	} 

    ShowWindow (window);

	// run window
	RunAppModalLoopForWindow (window);
	
	// read values
	*pfAGPTextures = GetControlValueByID (window, 'ImAt', kCntlRadioAGP) - 1;
	*pfClientTextures = GetControlValueByID (window, 'ImAt', kCntlCheckClientTex);
	*pMaxTextureSize = 1 << (GetControlValueByID (window, 'ImAt', kCntlPopUpMaxTex) + 4);
	*pfOverlapTextures = GetControlValueByID (window, 'ImAt', kCntlCheckOverlap);
	*pfNPOTTextures = GetControlValueByID (window, 'ImAt', kCntlCheckNPOTTextures);
	*pTextureScale = GetControlValueByID (window, 'ImAt', kCntlPopUpScale) + 1;
	*pfTileTextures = GetControlValueByID (window, 'ImAt', kCntlRadioTileToTex);
	
	RemoveEventHandler (ref);
	DisposeEventHandlerUPP (gTexWinEvtHandler);
	DisposeWindow (window);
}

