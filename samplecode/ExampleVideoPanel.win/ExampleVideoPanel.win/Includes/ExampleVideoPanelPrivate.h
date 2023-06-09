/*
	File:		ExampleVideoPanelPrivate.h
	
	Description: 	Example video panel component private header file.

	Author:		Apple Developer Support, original code by Gary Woodcock

	Copyright: 	� Copyright 1992-1993 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
	
	   <2>		2/09/02		SRK				Carbonized/Win32
	   <1>	 	1992		Gary Woodcock		first file
*/

//-----------------------------------------------------------------------

// Includes
#ifndef __Prefix_File__
#include "WinPrefix.h"
#endif

#ifndef	_EXAMPLEVIDEOPANELPRIVATE_
#define	_EXAMPLEVIDEOPANELPRIVATE_

#include <QuickTimeComponents.h>
#include "DebugFlags.h"

//-----------------------------------------------------------------------

// Private constants

// Base resource ID
enum
{
	kBaseResID = 500
};

// Maximum number of instances that can be opened
enum
{
	kMaxExampleVideoPanelInstances = 1
};

// Component and interface revision levels
enum
{
	exampleVideoPanelInterfaceRevision = 0x00010001	
};

enum
{
	kGenericError = -1L	// Handy return code
};

// DITL ID
enum
{
	kExampleVideoPanelDITLID = kBaseResID
};

// DITL items
enum
{
	kZeroBlackButton = 1,
	kResetButton,
	kSeparator,
	kTextBlurb
};

enum
{
	kDKey = 0x02,
	kRKey = 0x0F
};

//-----------------------------------------------------------------------
// Private types
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef	struct	PanelGlobals
{
	Component			self;
	SeqGrabComponent	seqGrabber;
	SGChannel			videoChannel;
	short				resRefNum;
	unsigned short		savedBlackLevel;
}
PanelGlobals, *PanelGlobalsPtr, **PanelGlobalsHdl;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

//-----------------------------------------------------------------------
// Private prototypes

#if 0
#ifdef DEBUG_ME

// Only need this prototype if we're running linked (for debugging)
pascal ComponentResult
ExampleVideoPanelDispatcher (ComponentParameters *params, Handle storage);

#else

// Use this declaration when we're a standalone component
pascal ComponentResult
main (ComponentParameters *params, Handle storage);

#endif DEBUG_ME
#endif
							 
pascal ComponentResult
_ExampleVideoPanelOpen (PanelGlobalsHdl storage, ComponentInstance self);
	
pascal ComponentResult
_ExampleVideoPanelClose (PanelGlobalsHdl storage, ComponentInstance self);

pascal ComponentResult
_ExampleVideoPanelVersion (PanelGlobalsHdl storage);

pascal ComponentResult
_ExampleVideoPanelPanelGetDitl (PanelGlobalsHdl storage, Handle *ditl);
														 
pascal ComponentResult
_ExampleVideoPanelPanelGetTitle (PanelGlobalsHdl storage, Str255 title);
														 
pascal ComponentResult
_ExampleVideoPanelPanelCanRun (PanelGlobalsHdl storage, SGChannel c);
														 
pascal ComponentResult
_ExampleVideoPanelPanelInstall (PanelGlobalsHdl storage, SGChannel c, DialogPtr d, short itemOffset);
														 
pascal ComponentResult
_ExampleVideoPanelPanelEvent (PanelGlobalsHdl storage, SGChannel c, DialogRef d, short itemOffset,
	const EventRecord *theEvent, short *itemHit, Boolean *handled);
														 
pascal ComponentResult
_ExampleVideoPanelPanelItem (PanelGlobalsHdl storage, SGChannel c, DialogPtr d, short itemOffset, short itemNum);
														 
pascal ComponentResult
_ExampleVideoPanelPanelRemove (PanelGlobalsHdl storage, SGChannel c, DialogPtr d, short itemOffset);
														 
pascal ComponentResult
_ExampleVideoPanelPanelSetGrabber (PanelGlobalsHdl storage, SeqGrabComponent sg);
														 
pascal ComponentResult
_ExampleVideoPanelPanelSetResFile (PanelGlobalsHdl storage, short resRef);
														 
pascal ComponentResult
_ExampleVideoPanelPanelGetSettings (PanelGlobalsHdl storage, SGChannel c, UserData *ud, long flags);
														
pascal ComponentResult
_ExampleVideoPanelPanelSetSettings (PanelGlobalsHdl storage, SGChannel c, UserData ud, long flags);
														 
pascal ComponentResult
_ExampleVideoPanelPanelValidateInput (PanelGlobalsHdl storage, Boolean *ok);

static void 
Carbon_SetControlState(DialogPtr theDialog, short theItem, Boolean enable);

static void 
OldStyle_SetControlState(DialogPtr theDialog, short theItem, Boolean enable);

static void 
DoSetControlState(DialogPtr theDialog, short theItem, Boolean enable);

static void
DoEnableControl (DialogPtr theDialog, short theItem);

static void
DoDisableControl (DialogPtr theDialog, short theItem);

static OSErr
FakeDialogButtonHit (DialogPtr theDialog, short theButtonItem);

static void
GetItemBox (DialogPtr theDialog, short theItem, Rect *theRect);

#ifdef DEBUG_ME

Component
RegisterExampleVideoPanel (void);

#endif DEBUG_ME

//-----------------------------------------------------------------------

#endif	_EXAMPLEVIDEOPANELPRIVATE_

//-----------------------------------------------------------------------
