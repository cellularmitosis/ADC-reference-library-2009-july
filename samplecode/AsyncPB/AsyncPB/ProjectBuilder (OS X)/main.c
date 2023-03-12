/*
    File:       AsyncPB.c

    Contains:   AsyncPB is an example of how File System calls can be made 
				in a chain from an interrupt handler.
				Once you select DoIt from the menu give it a little time to actually
				process the input file.

    Written by: DTS

    Copyright:  Copyright (c) 2003 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

				23 Jul 2003	Rewrote for modern coding conventions (Quinn)
				Jul 2003	Updated for Project Builder (MK)
				6/24/99		Updated for Metrowerks Codewarror Pro 2.1 (KG)
                1984 ?		First written (JB)
$Log$

*/

/////////////////////////////////////////////////////////////////

#include <Carbon/Carbon.h>
#include <assert.h>

/////////////////////////////////////////////////////////////////
#pragma mark ***** User Interface Boilerplate

static const ControlID kDoItButtonID = { 'DoIt', 0 };

static ControlRef gDoItButton;

static WindowRef gWindow;

static void DisplayError(OSStatus errNum)
    // Displays an error dialog
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

/////////////////////////////////////////////////////////////////
#pragma mark ***** Start of interesting code

/////////////////////////////////////////////////////////////////
#pragma mark ***** Chained I/O handling

enum {
    kStateRecordMagic = 'mjik'
};

// StateRecord records all of the information about a particular 
// chained I/O operation.

struct StateRecord {
    OSStatus			magic;			// must be kStateRecordMagic

    Boolean				inUse;			// primarily for debugging
    
	FSForkIOParam		readParam;		// parameter block used for reads
	FSForkIOParam		writeParam;		// parameter block used for writes

    char				buffer[1];		// The small buffer size is great for testing because 
                                        // you can see the "Do It" button grey out and then 
                                        // ungrey on completion.  For better efficiency, you can 
                                        // increase the size of the buffer (and everything works!).
    
    EventRef			doneEvent;		// we post doneEvent to doneQueue when the chain completes
    EventQueueRef		doneQueue;
    OSStatus			doneErr;
};
typedef struct StateRecord StateRecord;

static void SignalDone(StateRecord *sr, OSStatus err)
    // Called when the I/O chain terminates.  It posts doneEvent 
    // to doneQueue, which causes the main event loop to wake up 
    // and call the DoneIt function, which displays the results of 
    // the I/O chain (that is, doneErr).
{
    OSStatus junk;
    
    sr->doneErr = err;
    
    junk = PostEventToQueue(sr->doneQueue, sr->doneEvent, kEventPriorityStandard);
    assert(junk == noErr);
}

static IOCompletionUPP gReadCompletionUPP;			// -> ReadCompletion

static pascal void ReadCompletion(ParmBlkPtr pb)
    // Called when the async read request completes.
    // pb points to the parameter block that was used to 
    // issue the read.  We can map back from that to get our 
    // state record.
{	
    StateRecord	* sr;
    
    // Get our state record.

    sr = (StateRecord *) (((char *) pb) - offsetof(StateRecord, readParam));
    assert(sr->magic == kStateRecordMagic);
	
    // We had a partially successful read.  Let's just treat it 
    // as a successful read for the moment.  This ensures that 
    // the last chunk of the file gets written.  The next time 
    // we read, actualCount will be zero and we'll terminate 
    // the I/O chain.
    
    if ( (sr->readParam.ioResult == eofErr) && (sr->readParam.actualCount > 0) ) {
        sr->readParam.ioResult = noErr;
    }
    
    if (sr->readParam.ioResult == noErr) {        
        // If the read was successful, kick off the write.  We copy 
        // across the number of bytes read into requestCount so that 
        // we only write the bytes that we read in the last read.
        
        sr->writeParam.requestCount = sr->readParam.actualCount;
		(void) PBWriteForkAsync( &sr->writeParam );
    } else {
        // If the read errored, call SignalDone to tell the main event loop.
        
        if (sr->readParam.ioResult == eofErr) {
            // If the read hit the end of file, that's not an error, it's 
            // just the end of the I/O chain.
            sr->readParam.ioResult = noErr;
        }
        SignalDone(sr, sr->readParam.ioResult);
    }
}

static IOCompletionUPP gWriteCompletionUPP;			// -> WriteCompletion

static pascal void WriteCompletion(ParmBlkPtr pb)
    // Called when the async write request completes.
    // pb points to the parameter block that was used to 
    // issue the write.  We can map back from that to get our 
    // state record.
{
    StateRecord	* sr;

    // Get our state record.

    sr = (StateRecord *) (((char *) pb) - offsetof(StateRecord, writeParam));
    assert(sr->magic == kStateRecordMagic);
	
    if (sr->readParam.ioResult == noErr) {

        // If the read was successful, kick off the write.

		(void) PBReadForkAsync( &sr->readParam );
    } else {
        // If the write errored, call SignalDone to tell the main event loop.

        SignalDone(sr, sr->readParam.ioResult);
    }
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Main event loop-based set up/tear down

static OSStatus CreateSignalDoneEvent(void *refCon, EventRef *doneEventPtr)
    // Create the event that is posted when the I/O chain is 
    // done.  In this case, we create a 'Done' HICommand, which 
    // we dispatch to the DoneIt routine.  We store the refCon 
    // in the event so that DoneIt can recover the correct 
    // StateRecord pointer.
{
    OSStatus err;
    
    assert( doneEventPtr != NULL );
    assert(*doneEventPtr == NULL );

    err = MacCreateEvent(
        NULL, 
        kEventClassCommand, 
        kEventCommandProcess, 
        GetCurrentEventTime(), 
        kEventAttributeNone, 
        doneEventPtr
    );
    if (err == noErr) {
        HICommand command;
        
        BlockZero(&command, sizeof(command));
        command.commandID = 'Done';
        err = SetEventParameter(
            *doneEventPtr, 
            kEventParamDirectObject, 
            typeHICommand, 
            sizeof(command), 
            &command
        );
    }
    if (err == noErr) {
        err = SetEventParameter(*doneEventPtr, 'REFC', typeUInt32, sizeof(refCon), &refCon);
    }
    
    // Clean up.
    
    if (err != noErr) {
        ReleaseEvent(*doneEventPtr);
        *doneEventPtr = NULL;
    }
    
    assert( (err == noErr) == (*doneEventPtr != NULL) );
    
    return err;
}

// In this example we use a single global state record for the I/O. 
// In a real program, you would allocate one of these for each async 
// I/O chain you wanted to operate concurrently.

static StateRecord gStateRecord;

static void DoIt(void)
    // Called by our Carbon event command handler when the user click's 
    // the "Do It" button.  This kicks off the async I/O chain.
{
	OSStatus			err;
    OSStatus			junk;
    StateRecord *		sr;
    ProcessSerialNumber ourPSN;
    FSRef				ourRef;
    FSRef				parentDirRef;
    FSRef				srcFileRef;
    FSRef				dstFileRef;
    SInt16				srcForkRef;
    SInt16				dstForkRef;
    EventRef			doneEvent;
    static const UniChar kSrcName[] =      { 'i', 'n', 'D', 'a', 't', 'a' };
    static const UniChar kDstName[] = { 'o', 'u', 't', 'D', 'a', 't', 'a' };
    
    srcForkRef = 0;
    dstForkRef = 0;
    doneEvent = NULL;
    
    // 'Allocate' our state record.
    
    assert( ! gStateRecord.inUse );
    sr = &gStateRecord;
    
    // Disable the button so the user can't click it again.
    
    junk = DisableControl(gDoItButton);
    assert(junk == noErr);

    // Get an FSRef to the directory in which the application resides.
    
    err = GetCurrentProcess(&ourPSN);
    if (err == noErr) {
        err = GetProcessBundleLocation(&ourPSN, &ourRef);
    }
    if (err == noErr) {
        err = FSGetCatalogInfo(&ourRef, kFSCatInfoNone, NULL, NULL, NULL, &parentDirRef);
    }
    
    // Open up the source file.
    
    if (err == noErr) {
        err = FSMakeFSRefUnicode(
            &parentDirRef, 
            sizeof(kSrcName) / sizeof(UniChar), 
            kSrcName, 
            kTextEncodingUnknown, 
            &srcFileRef
        );
    }
    if (err == noErr) {
        err = FSOpenFork(&srcFileRef, 0, NULL, fsRdPerm, &srcForkRef);
    }
    
    // Open up the destination file (deleting the old one if necessary).
    
    if (err == noErr) {
        if ( 	FSMakeFSRefUnicode(
                    &parentDirRef, 
                    sizeof(kDstName) / sizeof(UniChar), 
                    kDstName, 
                    kTextEncodingUnknown, 
                    &dstFileRef
                ) == noErr ) {
            junk = FSDeleteObject(&dstFileRef);
            assert(junk == noErr);
        }

        FSCatalogInfo 	catInfo;
        FInfo *			fInfoPtr;
        
        BlockZero(&catInfo, sizeof(catInfo));
        fInfoPtr = (FInfo *) catInfo.finderInfo;
        fInfoPtr->fdCreator = '????';
        fInfoPtr->fdType    = 'TEXT';

        err = FSCreateFileUnicode(
            &parentDirRef, 
            sizeof(kDstName) / sizeof(UniChar), 
            kDstName, 
            kFSCatInfoFinderInfo, 
            &catInfo, 
            &dstFileRef, 
            NULL
        );
    }
    if (err == noErr) {
        err = FSOpenFork(&dstFileRef, 0, NULL, fsRdWrPerm, &dstForkRef);
    }
    
    // Create the event that the async I/O chain posts in order to signal 
    // its completion.
    
    if (err == noErr) {
        err = CreateSignalDoneEvent(sr, &doneEvent);
    }
    
    // If everything is good, let's kick off the chained I/O completion routines.
    
    if (err == noErr) {
        BlockZero(sr, sizeof(*sr));
        
        sr->magic = kStateRecordMagic;
        
        sr->inUse = true;

        sr->readParam.ioCompletion   = gReadCompletionUPP;
        sr->readParam.forkRefNum     = srcForkRef;
        sr->readParam.buffer         = sr->buffer;
        sr->readParam.requestCount   = sizeof(sr->buffer);
        sr->readParam.positionMode   = fsAtMark;
        sr->readParam.positionOffset = 0;

        sr->writeParam.ioCompletion   = gWriteCompletionUPP;
        sr->writeParam.forkRefNum     = dstForkRef;
        sr->writeParam.buffer         = sr->buffer;
        // Don't need to initialise sr->writeParam.requestCount because 
        // ReadCompletion always sets it up.
        sr->writeParam.positionMode   = fsAtMark;
        sr->writeParam.positionOffset = 0;
        
        sr->doneEvent = doneEvent;
        sr->doneQueue = GetMainEventQueue();

        PBReadForkAsync(&sr->readParam);
        
        // Once we've kicked off the async I/O chain, it's responsible for 
        // for cleaning up this information by calling SignalDone, which 
        // results in DoneIt being called.
                
        srcForkRef = 0;
        dstForkRef = 0;
        doneEvent  = NULL;
    }
    
    // Clean up.
 
    if (srcForkRef != NULL) {
        junk = FSCloseFork(srcForkRef);
        assert(junk == noErr);
    }
    if (dstForkRef != NULL) {
        junk = FSCloseFork(dstForkRef);
        assert(junk == noErr);
    }
    if (doneEvent != NULL) {
        ReleaseEvent(doneEvent);
    }
    if (err != noErr) {
        junk = EnableControl(gDoItButton);
        assert(junk == noErr);
    }
    
    DisplayError(err);
}

static void DoneIt(EventRef event)
    // Called by our Carbon event command handler when it gets 'Done' 
    // event, which is posted when the I/O chain completes.  This 
    // cleans up resources and displays the result.
{
    OSStatus 			err;
    OSStatus 			junk;
    StateRecord *		sr;

    assert( gStateRecord.inUse );
    
    // Get a reference to our state record from the event's refCon.
    
    err = GetEventParameter(
        event,
        'REFC', 
        typeUInt32,
        NULL,
        sizeof(sr),
        NULL,
        &sr
    );
    
    // Clean up the state record.
    
    if (err == noErr) {
        assert(sr->magic == kStateRecordMagic);
        
        junk = FSCloseFork(sr->readParam.forkRefNum);
        assert(junk == noErr);      
        sr->readParam.forkRefNum = 0;     
          
        junk = FSCloseFork(sr->writeParam.forkRefNum);
        assert(junk == noErr);
        sr->writeParam.forkRefNum = 0;     

        ReleaseEvent(sr->doneEvent);
        sr->doneEvent = NULL;

        DisplayError(sr->doneErr);    
    }
    
    // Mark the global state record as no longer being in use and 
    // re-enable the "Do It" button.  This is kinda bogus because 
    // it breaks our model that DoneIt knows nothing about the 
    // global state record.  In a real application this might 
    // recycle sr on to the end of a queue of StateRecords that is 
    // is available to the application.
    
    gStateRecord.inUse = false;
    
    junk = EnableControl(gDoItButton);
    assert(junk == noErr);
}
    
/////////////////////////////////////////////////////////////////
#pragma mark ***** End of interesting code

/////////////////////////////////////////////////////////////////
#pragma mark ***** More User Interface Boilerplate

// The code below is mostly standard Carbon NIB-based application boilerplate.

static const EventTypeSpec kMyEventTypes[] = {
    {kEventClassCommand, kEventCommandProcess}
};

static EventHandlerUPP gMyEventHandlerUPP;         // -> MyEventHandler

static OSStatus MyEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
    // A standard Carbon event handler.
{
    OSStatus    err;
    HICommand   command;

    err = eventNotHandledErr;
    switch ( GetEventClass(inEvent) ) {
        case kEventClassCommand:
            switch ( GetEventKind(inEvent) ) {
                case kEventCommandProcess:
                    err = GetEventParameter(
                        inEvent,
                        kEventParamDirectObject, 
                        typeHICommand,
                        NULL,
                        sizeof(command),
                        NULL,
                        &command
                    );
                    if (err == noErr) {
                        switch (command.commandID) {
                            case 'DoIt':
                                DoIt();
                                break;
                            case 'Done':
                                DoneIt(inEvent);
                                break;
                            default:
                                err = eventNotHandledErr;
                                break;
                        }
                    }
                    break;
                default:
                    // do noting
                    break;
            }
            break;
        default:
            // do noting
            break;
    }
    
    return err;
}

int main(int argc, char* argv[])
{
    OSStatus		err;
    IBNibRef 		nibRef;

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

    // Initialise our UPPs.
    
    gReadCompletionUPP  = NewIOCompletionUPP(ReadCompletion);
    assert(gReadCompletionUPP  != NULL);

    gWriteCompletionUPP = NewIOCompletionUPP(WriteCompletion);
    assert(gWriteCompletionUPP != NULL);

    // Grab a reference to the "Do It" button.

    err = GetControlByID(gWindow, &kDoItButtonID, &gDoItButton);
    require_noerr( err, CantGetDoItButtonControl );

    // Install an application event handler.

    gMyEventHandlerUPP = NewEventHandlerUPP(MyEventHandler);
    assert(gMyEventHandlerUPP != NULL);

    err = InstallApplicationEventHandler(gMyEventHandlerUPP, GetEventTypeCount(kMyEventTypes), kMyEventTypes, NULL, NULL);
    require_noerr( err, CantInstallHandler );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( gWindow );
    
    // Call the event loop
    RunApplicationEventLoop();

CantInstallHandler:
CantGetDoItButtonControl:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

