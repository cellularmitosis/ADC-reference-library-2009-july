/*
	File:		main.c
	
	Description:

	Author:		JM

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
    7/2003	MK	Updated for Project Builder

*/


#include <Carbon/Carbon.h>
#include <stdio.h>
#include <assert.h>

#include "FinderLaunch.h"

//some constants

	/* constants referring to the main STR# resource */
enum {
	kMainStrings = 128,
	kNavTextMessage = 1
};

//navigation services  globals
NavObjectFilterUPP gFileFilter;
NavEventUPP gNavEventProc;

//user interface stuff
static WindowRef gWindow;
static EventHandlerUPP gMyEventHandlerUPP;


//display error handler
static void DisplayError(OSStatus errNum)
{
    OSStatus    junk;
    SInt16      junkItemHit;
    Str255      expStr;

    if ( (errNum != noErr) && (errNum != userCanceledErr) ) {

        NumToString(errNum, expStr);

        junk = StandardAlert(
            kAlertStopAlert, 
            "\pAn error occured.", 
            expStr, 
            NULL, 
            &junkItemHit
        );
        assert(junk == noErr);
    }
}


//finder launch specifics

//file filter for the selection
//window
static Boolean InvisoFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode){
    NavFileOrFolderInfo* theInfo;
    
    if ( theItem->descriptorType == typeFSS ) {
        theInfo = (NavFileOrFolderInfo*) info;
        if( theInfo->visible ){
            return true;
        }
    }
    return false;
}

/* GetHFSObjectList opens a communication session with the user allowing them to choose
	one more file system objects.  After the user has made a selection, a list of the items
	chosen is passed back as a AEDescList containing a list of FSSpec records. if the user
	cancels the interaction, then no list is returned and a userCanceledErr is returned.*/
static OSErr GetHFSObjectList(AEDescList *documents) {
	NavReplyRecord theReply;
	Boolean hasNavReply;
	OSErr err;
		/* set up locals */
	AECreateDesc(typeNull, NULL, 0, documents);
	hasNavReply = false;

    NavDialogOptions dialogOptions;
        /* set the message in the navigation window to indicated multiple
        selections are allowed */
    memset(&theReply, 0, sizeof(theReply));
    err = NavGetDefaultDialogOptions(&dialogOptions);
    if (err == noErr){
        dialogOptions.dialogOptionFlags = (kNavDontAutoTranslate | kNavAllowMultipleFiles);
        GetIndString(dialogOptions.message, kMainStrings, kNavTextMessage);
            /* run the navigation window */
        err = NavChooseObject( NULL, &theReply, &dialogOptions, gNavEventProc, NULL, NULL);
    }
    if (err == noErr){
        if (!theReply.validRecord) { 
            err = userCanceledErr; 
        }
        if( err == noErr ){
            hasNavReply = true;
                /* duplicate the returned document list */
            err = AEDuplicateDesc(&theReply.selection, documents);
        }
    }
    
    if (err == noErr){
            /* clean up the navigation stuff */
        NavDisposeReply(&theReply);
    }
    
    if( err != noErr ){
        if (hasNavReply) NavDisposeReply(&theReply);
        AEDisposeDesc(documents);
    }
    
	return err;
}

/* SelectTargetsToLaunch calls GetHFSObjectList to retrieve a list of files or folders	
	 and then it passes the selected folders and files to the FinderLaunch routine. */
static void SelectTargetsToLaunch(void) {
	AEDescList documents;
	OSErr err;
	long index, count;
	FSSpec *targets;
	AEKeyword keyword;
	DescType typecode;
	Size actualSize;
		/* set up locals */
	AECreateDesc(typeNull, NULL, 0, &documents);
	targets = NULL;
		/* get a list of files from the user */
	err = GetHFSObjectList(&documents);
	if (err == noErr){
            /* count the items in the list */
        err = AECountItems(&documents, &count);
    }
        
	if (err == noErr){
            /* allocate an array to store the records */
        targets = (FSSpec *) NewPtr(count * sizeof(FSSpec));
        
        if (targets == NULL) { 
            err = memFullErr; 
        }
        
        if( err == noErr ){
                /* copy each record from the list to the array */
            for (index = 0; index < count; index++) {
                err = AEGetNthPtr(&documents, (index + 1), typeFSS, &keyword, &typecode,
                    (targets + index), sizeof(FSSpec), &actualSize);
            }
        }
    }
    
    if( err == noErr ){
            /* ask the Finder to launch the items */
        err = FinderLaunch(count, targets);
    }
        
		/* clean up and leave, report any 'real' errors */
	if ((err != noErr) && (err != userCanceledErr)) {
		DisplayError( err );
	}
	if (targets != NULL) DisposePtr((Ptr) targets);
	AEDisposeDesc(&documents);
}

/* NavEventCallBack is a callback routine provided to the NavChooseObject routine.  In
	this routine we process update and activate events for the main window while
	navigation services is displaying its window. */ 
static pascal void NavEventCallBack( NavEventCallbackMessage callBackSelector,
			NavCBRecPtr callBackParms, NavCallBackUserData callBackUD) {
	if (callBackSelector == kNavCBEvent) {
		short ewhat;
		ewhat = callBackParms->eventData.eventDataParms.event->what;
		if ((ewhat == updateEvt) || (ewhat == activateEvt)) {
		
			//HandleNextEvent(callBackParms->eventData.eventDataParms.event);

		}
	}
}


//carbon event handler
static OSStatus MyEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    OSStatus    err;
    HICommand   command;

    err = eventNotHandledErr;
    switch ( GetEventClass(inEvent) ) {
        case kEventClassCommand:
        err = GetEventParameter( 
                inEvent, 
                kEventParamDirectObject,
				typeHICommand, 
                NULL, 
                sizeof(command), 
                NULL,
                &command);
        
        if( err == noErr && command.commandID == 'open' ){
            SelectTargetsToLaunch();
        } else {
            err = eventNotHandledErr;
        }
        break;

        default:
        break;
    }
    
    return err;
}

static const EventTypeSpec kMyEventTypes[] = {
    {kEventClassCommand, kEventCommandProcess}
};


//main
int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gWindow);
    require_noerr( err, CantCreateWindow );

        //load navigation services
	assert( NavServicesAvailable() );
    NavLoad();

    //initialize our upps
    gNavEventProc = NewNavEventUPP( NavEventCallBack );
	assert( gNavEventProc != NULL );
	
    gFileFilter = NewNavObjectFilterUPP( InvisoFilter );
	assert( gFileFilter != NULL );

    //install application event handler
    gMyEventHandlerUPP = NewEventHandlerUPP(MyEventHandler);
    assert(gMyEventHandlerUPP != NULL);

    err = InstallApplicationEventHandler(
            gMyEventHandlerUPP, 
            GetEventTypeCount(kMyEventTypes), 
            kMyEventTypes, 
            NULL, 
            NULL);
    require_noerr( err, CantInstallHandler );

    //unload navigation services
    NavUnload();

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( gWindow );
    
    // Call the event loop
    RunApplicationEventLoop();

    //error labels
    CantCreateWindow:
    CantSetMenuBar:
    CantGetNibRef:
    CantInstallHandler:
    
    //done
	return err;
}