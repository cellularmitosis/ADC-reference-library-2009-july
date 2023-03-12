/*
	File:		SuspendEvent.h

	Contains:	

	Written by: C.K. Haun	

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
    8/2003		MK	Updated for Project Builder
    7/21/1999	KG	Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include <Carbon/Carbon.h>

/* windowControl is the structure attached to every window I create (in the refCon */
/* field) that contains all the information I need to know about the window. */
/* data, procedure pointers for controlling, and anything else gets put in this */
/* struct.  That makes my windows autonomous */
struct windowControl {
    unsigned long windowID;                                 /* master ID number  */
    ProcPtr drawMe;                                         /* content drawing procedure pointer */
	ProcPtr clickMe;										/* content click routine */
    ProcPtr closeMe;                                        /* document close procedure pointer */
	ProcPtr sizeMe;											/* size procedure */
    AliasHandle fileAliasHandle;                            /* alias for this document */
    Boolean windowDirty;
	Handle generalData;										/* cast to whatever you need as you need it */
};
typedef struct windowControl windowControl, *windowCPtr, **windowCHandle;

enum {kDocWindowResID = 128,kMyDocumentWindow = 1000};
/* menu enums */
enum {kMBarID = 128};
enum {kAppleMenu = 128,kFileMenu,kEditMenu,kToolsMenu};

/* file menu enums */
enum {kNewItem = 1,kOpenItem,kCloseItem,kSaveItem,kSaveAsItem,kFileBlank1,kPageSetupItem,kPrintItem,kFileBlank2,kQuitItem};

/* general purpose enums */
enum {kResumeMask=1,kSampHelp=129,kAboutBox=128,kHelpString=128,kBadSystem=130};

enum {kMinHeight = 200};

enum {kGeneralStrings=128};
enum {kPendingWords1=1,kPendingWords2};



#define kSuspCreator 0x11171915

#define kFredDocType 'FRED'
#define kAlDocType 'AL  '