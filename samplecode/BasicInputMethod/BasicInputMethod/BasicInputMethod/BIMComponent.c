/*
    File:	 BIMComponent.c

    Description: Main entry points for our text service routines. These routines are called by
                 the Component Manager and the Text Services Manager. Here we do some basic
                 housekeeping, but for the actual functionality of our text service, we will call
                 our core implmentation routines in BIM.c.

    Author:	 SC

    Copyright: 	 © Copyright 2000-2001 Apple Computer, Inc. All rights reserved.

    Disclaimer:	 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

                 2001/04/18	SC	Renamed component routines to start with BIM for consistency
                 2001/03/09	SC	Removed the hack in BIMGetScriptLangSupport to tell TSM to
                                        call TextServiceEventRef instead of UCTextServiceEvent.
                 2001/03/07	SC	Removed the handler for UCTextServiceEvent since it is
                                        never called on Mac OS X.
                 2000/07/28	SC	Changed to include Carbon.h
                 2000/06/01	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BIM.h"
#include "BIMComponent.h"
#include "BIMScript.h"

//  Main entry point

pascal ComponentResult BIMComponentDispatch( ComponentParameters *inParams, Handle inSessionHandle );

//  Global variables

long		gInstanceRefCount = 0;
MenuRef		gTextServiceMenu = nil;

//  Local function declarations

static ComponentResult CallBIMFunction( ComponentParameters *inParams, ProcPtr inProcPtr,
                                        SInt32 inProcInfo );
static ComponentResult CallBIMFunctionWithStorage( Handle inStorage,
                                                   ComponentParameters *inParams,
                                                   ProcPtr inProcPtr, SInt32 inProcInfo );

/************************************************************************************************
*
*  BIMComponentDispatch
*
*  This routine is the main entry point for our text service component. All calls to our
*  component go through this entry point.
*
*  We examine the selector (inParams->what) and dispatch the call to the appropriate handler.
*
*	In	inParams		Parameters for this call.
*		inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMComponentDispatch( ComponentParameters *inParams, Handle inSessionHandle )
{
    ComponentResult result;

    result = noErr;

    //	Dispatch to the appropriate handler, based on the selector in the inParams record.

    switch (inParams->what) {

        //  These first four calls are made by the Component Manager to every component.

        case kComponentOpenSelect:
            BIMLog( "BIMComponentDispatch::kComponentOpenSelect\n" );
            result = CallBIMFunction( inParams, (ProcPtr) BIMOpenComponent,
                                      uppOpenComponentProcInfo );
            break;

        case kComponentCloseSelect:
            BIMLog( "BIMComponentDispatch::kComponentCloseSelect\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMCloseComponent,
                                                 uppCloseComponentProcInfo );
            break;

        case kComponentCanDoSelect:
            BIMLog( "BIMComponentDispatch::kComponentCanDoSelect\n" );
            result = CallBIMFunction( inParams, (ProcPtr) BIMCanDo, uppCanDoProcInfo );
            break;

        case kComponentVersionSelect:
            BIMLog( "BIMComponentDispatch::kComponentVersionSelect\n" );
            result = CallBIMFunction( inParams, (ProcPtr) BIMGetVersion, uppGetVersionProcInfo );
            break;

	//  These calls are made by the Text Services Manager to text service components.

        case kCMGetScriptLangSupport:
            BIMLog( "BIMComponentDispatch::kCMGetScriptLangSupport\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMGetScriptLangSupport,
                                                 uppGetScriptLangSupportProcInfo );
            break;

        case kCMInitiateTextService:
            BIMLog( "BIMComponentDispatch::kCMInitiateTextService\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMInitiateTextService,
                                                 uppInitiateTextServiceProcInfo );
            break;

        case kCMTerminateTextService:
            BIMLog( "BIMComponentDispatch::kCMTerminateTextService\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMTerminateTextService,
                                                 uppTerminateTextServiceProcInfo );
            break;

        case kCMActivateTextService:
            BIMLog( "BIMComponentDispatch::kCMActivateTextService\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMActivateTextService,
                                                 uppActivateTextServiceProcInfo );
            break;

        case kCMDeactivateTextService:
            BIMLog( "BIMComponentDispatch::kCMDeactivateTextService\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMDeactivateTextService,
                                                 uppDeactivateTextServiceProcInfo );
            break;

        case kCMTextServiceEvent:
            BIMLog( "BIMComponentDispatch::kCMTextServiceEvent\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMTextServiceEventRef,
                                                 uppTextServiceEventRefProcInfo );
            break;

        case kCMGetTextServiceMenu:
            BIMLog( "BIMComponentDispatch::kCMGetTextServiceMenu\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMGetTextServiceMenu,
                                                 uppGetTextServiceMenuProcInfo );
            break;

        case kCMFixTextService:
            BIMLog( "BIMComponentDispatch::kCMFixTextService\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMFixTextService,
                                                 uppFixTextServiceProcInfo );
            break;

        case kCMHidePaletteWindows:
            BIMLog( "BIMComponentDispatch::kCMHidePaletteWindows\n" );
            result = CallBIMFunctionWithStorage( inSessionHandle, inParams,
                                                 (ProcPtr) BIMHidePaletteWindows,
                                                 uppHidePaletteWindowsProcInfo );
            break;

            //	We were called with an unknown selector.

        default:
            result = badComponentSelector;
            break;
    }

    // Finally return the result of the call.

    return result;
}

/************************************************************************************************
*
*  BIMOpenComponent
*
*  This routine is called directly via OpenComponent, or indirectly via NewTSMDocument.
*
*  If this the first instance of our component, we initialize our global state (IMInitialize).
*  Then we initialize a new session context (IMOpenSession). 
*
*	In	inComponentInstance		The component instance.
*
*	Out	ComponentResult			A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMOpenComponent( ComponentInstance inComponentInstance )
{
    ComponentResult result;
    Handle sessionHandle;

    result = noErr;
    sessionHandle = nil;

    //  If this is the first instance of our component, initalize our global state. Normally,
    //  this means that we load our text service menu, install event handlers for the menu,
    //  and initialize any global variables that persist across sessions.

    if( gInstanceRefCount == 0 )
        result = BIMInitialize( inComponentInstance, &gTextServiceMenu );
    gInstanceRefCount++;

    //	Now initialize a new session context. We store our per-session data in a session
    //	handle that is stored with the component instance.

    if( result == noErr ) {

        //  Get our component instance storage.

        sessionHandle = GetComponentInstanceStorage( inComponentInstance );

        //  Initialize the new session.

        result = BIMSessionOpen( inComponentInstance, (BIMSessionHandle *) &sessionHandle );

        //  Save the returned handle as our component instance storage.

        if( result == noErr )
            SetComponentInstanceStorage( inComponentInstance, sessionHandle );
    }

    return result;
}

/************************************************************************************************
*
*  BIMCloseComponent
*
*  This routine is called directly via CloseComponent, or indirectly via DeleteTSMDocument.
*
*  In this routine we terminate the current session context (IMCloseSession). If this the last
*  remaining instance of our component, we also terminate our global state (IMTerminate). 
*
*	In	inSessionHandle		Our session context.
                inComponentInstance	The component instance.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMCloseComponent( Handle inSessionHandle,
                                          ComponentInstance inComponentInstance )
{
    ComponentResult result;

    result = noErr;

    if( inComponentInstance == nil )
        result = paramErr;
    else {
    
        //  Terminate the current session context. Note that if OpenComponent failed, the session
        //  handle may be NULL.
        
        BIMSessionClose( (BIMSessionHandle) inSessionHandle );
        SetComponentInstanceStorage( inComponentInstance, nil );
    
        //  If this is the last instance of our component, terminate our global state. Normally,
        //  this means that we dispose of any global data allocated during BIMInitialize().

	gInstanceRefCount--;
        if( gInstanceRefCount == 0 )
            BIMTerminate( inComponentInstance );
    }
    return result;
}

/************************************************************************************************
*
*  BIMCanDo
*
*  Return true if the routine indicated by "selector" is one that we support, otherwise return
*  false. The Text Services Manager does not currently call this routine.
*
*	In	inSelector		The selector to check for.
*
*	Out	ComponentResult		True if we support the selector, otherwise false.
*
************************************************************************************************/

pascal ComponentResult BIMCanDo( SInt16 inSelector )
{
    Boolean result;

    switch( inSelector ) {

        //  These first four calls are made by the Component Manager to every component.

        case kComponentOpenSelect:
        case kComponentCloseSelect:
        case kComponentCanDoSelect:
        case kComponentVersionSelect:
            result = true;
            break;

	//  These calls are made by the Text Services Manager to text service components. We
        //  don't need to support kCMTextServiceMenuSelect or kCMSetTextServiceCursor because
        //  menu handling and cursor management are handled for us on Mac OS X.

        case kCMGetScriptLangSupport:
        case kCMInitiateTextService:
        case kCMTerminateTextService:
        case kCMActivateTextService:
        case kCMDeactivateTextService:
        case kCMTextServiceEvent:
        case kCMGetTextServiceMenu:
        case kCMFixTextService:
        case kCMHidePaletteWindows:
            result = true;
            break;

	//  Return false for unknown request codes.

        default:
            result = false;
            break;
    }
    return result;
}

/************************************************************************************************
*
*  BIMGetVersion
*
*  This routine is called directly via GetComponentVersion. The Text Services Manager does not
*  currently call this routine
*
*	Out	ComponentResult		The version of this component.
*
************************************************************************************************/

pascal ComponentResult BIMGetVersion( void )
{
    return 0x00010000;
}

/************************************************************************************************
*
*  BIMGetScriptLangSupport
*
*  This routine is called by the Text Services Manager to determine our input method type.
*
*	In	inSessionHandle		Our session context.
*
*	Out	outScriptHandle		A handle to an array of script/language records.
*		ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMGetScriptLangSupport( Handle inSessionHandle,
                                              ScriptLanguageSupportHandle *outScriptHandle )
{
#pragma unused (inSessionHandle)

    OSStatus			result;
    ScriptLanguageRecord	scriptLanguageRecord;

    result = noErr;

    //	Allocate a handle to store our script/language records.

    if( *outScriptHandle == NULL ) {
        *outScriptHandle = (ScriptLanguageSupportHandle) NewHandle ( sizeof( SInt16 ) );
        if( *outScriptHandle == NULL )
            result = memFullErr;
    }

    //  Set up the handle so that it is empty.

    if( result == noErr ) {
        SetHandleSize( (Handle) *outScriptHandle, sizeof( SInt16 ) );
        result = MemError();
        if( result == noErr )
            ( **outScriptHandle )->fScriptLanguageCount = 0;        
    }

    //  Add a script/language record to tell TSM that we are a type 3 input method (we
    //  work with Unicode but only send back Unicode characters that are part of some
    //  Macintosh script encoding). We put our Macintosh script into our 'thng' resource
    //  component flags (as always), and return kTextEncodingUnicodeDefault from this
    //  routine.
    //
    //	Input   Component       ScriptLanguage-         xCHR Resource
    //	Method  Description     Support               ('KCHR' or 'uchr')
    //	Type    componentFlags  fScript              (chosen from KB Menu)
    //	------  --------------  -------------------  ---------------------
    //	type 1    Mac Script    Mac Script (or err)           KCHR
    //	type 2    Mac Script            0x7E             KCHR and uchr
    //	type 3    Mac Script           0x100                  uchr
    //	type 4       0x7E              0x100           uchr (negative ID)
    //	------  --------------  -------------------  ---------------------
    //

    if( result == noErr ) {
        scriptLanguageRecord.fScript = kTextEncodingUnicodeDefault;
        scriptLanguageRecord.fLanguage = kBIMLanguage;
        result = PtrAndHand( &scriptLanguageRecord, (Handle) *outScriptHandle,
                             sizeof( ScriptLanguageRecord ) );
        if( result == noErr )
            (**outScriptHandle)->fScriptLanguageCount++;
    }
        
    //	If an error occurred, dispose of everything.

    if( result ){
        if( *outScriptHandle ) {
            DisposeHandle( (Handle) *outScriptHandle );
            *outScriptHandle = NULL;
        }
    }
    return result;
}

/************************************************************************************************
*
*  BIMInitiateTextService
*
*  This routine is called by the Text Services Manager whenever an application calls
*  NewTSMDocument. However, since BIMTerminateTextService is never called, we do nothing.
*
*	In	inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMInitiateTextService( Handle inSessionHandle )
{
#pragma unused (inSessionHandle)

    return noErr;
}

/************************************************************************************************
*
*  BIMTerminateTextService
*
*  This routine is never called by the Text Services Manager. Do nothing.
*
*	In	inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMTerminateTextService( Handle inSessionHandle )
{
#pragma unused (inSessionHandle)

    return noErr;
}

/************************************************************************************************
*
*  BIMActivateTextService
*
*  This routine is called by the Text Services Manager whenever an application calls
*  NewTSMDocument or ActivateTSMDocument. The appropriate response to ActivateTextService is to
*  restore our active state, including displaying all floating windows if they have been hidden,
*  and redisplaying any inconfirmed text in the currently active input area.
*
*  We call our core routine BIMSessionActivate() to handle activation.
*
*	In	inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMActivateTextService( Handle inSessionHandle )
{
    return BIMSessionActivate( (BIMSessionHandle) inSessionHandle );
}

/************************************************************************************************
*
*  BIMDeactivateTextService
*
*  This routine is called by the Text Services Manager whenever an application calls
*  DeactivateTSMDocument. We are responsible for saving whatever state information we need to
*  save so that we can restore it again when we are reactivated. We should not confirm any
*  unconfirmed text in the active input area, but save it until reactivation. We should not hide
*  our floating windows either.
*
*  We call our core routine BIMSessionDeactivate() to handle deactivation.
*
*	In	inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMDeactivateTextService( Handle inSessionHandle )
{
    return BIMSessionDeactivate( (BIMSessionHandle) inSessionHandle );
}

/************************************************************************************************
*
*  BIMTextServiceEventRef
*
*  This routine is called in response to a user event (currently only key events and mouse
*  events are passed) within our current context.
*
*  We call our core routine BIMSessionEvent() to handle the event.
*
*	In	inSessionHandle		Our session context.
*		inEventRef		The event that needs to be handled.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMTextServiceEventRef( Handle inSessionHandle, EventRef inEventRef )
{
    return BIMSessionEvent( (BIMSessionHandle) inSessionHandle, inEventRef );
}

/************************************************************************************************
*
*  BIMGetTextServiceMenu
*
*  This routine is called by the Text Services Manager when our text service component is opened
*  or activated, so that it can put our component's menu on the menu bar.
*
*  We return a reference to our text service (pencil) menu handle in outMenuHandle.
*
*	In	inSessionHandle		Our session context.
*
*	Out	outMenuHandle		A reference to our text service (pencil) menu.
                ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMGetTextServiceMenu( Handle inSessionHandle, MenuHandle *outMenuHandle )
{
    *outMenuHandle = gTextServiceMenu;
    return noErr;
}

/************************************************************************************************
*
*  BIMFixTextService
*
*  This routine is called by the Text Services Manager to notify us that we must "fix" or
*  complete processing of any input that is in progress.
*
*  We call our core routine BIMSessionFix() to handle the event.
*
*	In	inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMFixTextService( Handle inSessionHandle )
{
    return BIMSessionFix( (BIMSessionHandle) inSessionHandle );
}

/************************************************************************************************
*
*  BIMHidePaletteWindows
*
*  This routine is called by the Text Services Manager to notify us that we must hide our
*  palette windows because another text service is becoming active. This routine is called
*  directly via HidePaletteWindows or indirectly via DeactivateTSMDocument.
*
*  We call our core routine BIMSessionHidePalettes() to handle the event.
*
*	In	inSessionHandle		Our session context.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

pascal ComponentResult BIMHidePaletteWindows( Handle inSessionHandle )
{
    return BIMSessionHidePalettes( (BIMSessionHandle) inSessionHandle );
}

/************************************************************************************************
*
*  CallBIMFunction
*
*  Glue code to create a Universal Procedure Pointer for our internal dispatch routines
*  and call them.
*
*	In	inParams		Component Manager parameters.
*		inProcPtr		A pointer to the procedure to call.
*		inProcInfo		Paramters accepted by the procedure.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

static ComponentResult CallBIMFunction( ComponentParameters *inParams, ProcPtr inProcPtr,
                                        SInt32 inProcInfo )
{
    ComponentResult		result;
    ComponentFunctionUPP	componentFunctionUPP;

    result = noErr;
    componentFunctionUPP = NewComponentFunctionUPP( inProcPtr, inProcInfo );
    result = CallComponentFunction( inParams, componentFunctionUPP );
    DisposeComponentFunctionUPP( componentFunctionUPP );
    return result;
}

/************************************************************************************************
*
*  CallBIMFunctionWithStorage
*
*  Glue code to create a Universal Procedure Pointer for our internal dispatch routines
*  and call them. This takes an additional inStorage parameter for routines that require a
*  reference to the current session handle.
*
*	In	inStorage		The session handle.
*		inParams		Component Manager parameters.
*		inProcPtr		A pointer to the procedure to call.
*		inProcInfo		Paramters accepted by the procedure.
*
*	Out	ComponentResult		A toolbox error code.
*
************************************************************************************************/

static ComponentResult CallBIMFunctionWithStorage( Handle inStorage,
                                                   ComponentParameters *inParams,
                                                   ProcPtr inProcPtr, SInt32 inProcInfo )
{
    ComponentResult		result;
    ComponentFunctionUPP	componentFunctionUPP;

    result = noErr;
    componentFunctionUPP = NewComponentFunctionUPP( inProcPtr, inProcInfo );
    result = CallComponentFunctionWithStorage( inStorage, inParams, componentFunctionUPP );
    DisposeComponentFunctionUPP( componentFunctionUPP );
    return result;
}
