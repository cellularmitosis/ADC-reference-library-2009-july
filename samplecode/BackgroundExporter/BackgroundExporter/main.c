/*
	File:        main.c
	
	Description: Implementation of the Background Movie Export Server
    
	Author:      QuickTime DTS

	Copyright: 	 © Copyright 2004 Apple Computer, Inc. All rights reserved.
    
	Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
                consideration of your agreement to the following terms, and your use, installation, modification 
                or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
                not agree with these terms, please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject to these terms, 
                Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
                original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
                Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
                redistribute the Apple Software in its entirety and without modifications, you must retain this 
                notice and the following text and disclaimers in all such redistributions of the Apple Software. 
                Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
                endorse or promote products derived from the Apple Software without specific prior written 
                permission from Apple.  Except as expressly stated in this notice, no other rights or 
                licenses, express or implied, are granted by Apple herein, including but not limited to any 
                patent rights that may be infringed by your derivative works or by other works in which the 
                Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
                IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
                AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
                OR IN COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
                DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
                OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
                REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
                UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
                IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
                
	Change History (most recent first): 07/20/04 initial release
*/

#pragma mark * includes & imports *

#include <CoreFoundation/CoreFoundation.h>
#include <QuickTime/QuickTime.h>
#include <pthread.h>
#include <unistd.h> // for sleep()

#include "ExportCommon.h"

#pragma mark-
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef struct {
    SInt32				numberOfActiveRequests;		// number of requests that have been created but not yet released
    pthread_t			workerThread;
    CFMessagePortRef    localPort;
    CFMessagePortRef    remotePort;
    CFRunLoopSourceRef  runLoopSource;
    CFRunLoopRef        runLoopRef;
    QHdr				requestQueue;				// messages going from main thread to worker thread
    Boolean				shutdown;					// set to ask worker thread to shut down when all request queue is empty
} WorkerThread, *WorkerThreadRef;

typedef struct WorkerRequest {
    SInt32				referenceCount;
    WorkerThreadRef		worker;						// note: each active request maintains a reference to its worker
    QElemPtr			nextRequest;				// used when linked into worker->requestQueue
    Boolean				wasCancelledOrStarted;
    Boolean				actionFinished;
    
    // client-use fields
    CFDataRef           data;
} WorkerRequest, *WorkerRequestRef;

#pragma mark -
#pragma mark * local (static) globals *

static pthread_mutex_t mutex;
static pthread_cond_t  condition;

#pragma mark -
#pragma mark * exported function implementations *

OSErr CreateWorkerRequest(WorkerThreadRef worker, WorkerRequestRef *outRequest)
{
    WorkerRequestRef request;
    
    if (!outRequest) return paramErr;

    request = calloc(1, sizeof(WorkerRequest));
    if (!request) return memFullErr;
    
    request->worker = worker;
    
    IncrementAtomic(&request->referenceCount);
    IncrementAtomic(&worker->numberOfActiveRequests);
    
    *outRequest = request;
    
    return noErr;
}

void ReleaseWorkerRequest(WorkerRequestRef request)
{
    if (1 == DecrementAtomic(&request->referenceCount)) {
        DecrementAtomic(&request->worker->numberOfActiveRequests);
        
        CFRelease(request->data);
        free(request);
    }
}

pascal OSErr myMovieProgressProc(Movie theMovie, short theMessage, short theOperation, Fixed thePercentDone, long refcon)
{
    char message[255];
    CFDataRef data, ignore = NULL;
    WorkerThreadRef worker = (WorkerThreadRef)refcon;
    
    // if we're shutting down don't bother sending anything back
    // to the remote port because the client is gone
    if (worker->shutdown) return noErr;
     
     switch (theMessage) {
     case movieProgressOpen:
        sprintf(message, "open");
        break;
     case movieProgressUpdatePercent:
        sprintf(message, "%% complete: %ld\n", Fix2Long(FixMul(thePercentDone, Long2Fix(100))));
        break;
     case movieProgressClose:
        sprintf(message, "close");
        break;
    } 
     
     data = CFDataCreate(NULL, (UInt8 *)message, strlen(message)+1);
     
     CFMessagePortSendRequest(worker->remotePort, 0, data, 1, 1, NULL /*kCFRunLoopDefaultMode*/, &ignore);
     CFRelease(data);

    return noErr;
}

OSErr DoExport(WorkerRequestRef request)
{    
    Movie sourceMovie = 0;
    Handle sourceDataRef = NULL,
           destDataRef = NULL;
    OSType sourceDataRefType,
           destDataRefType;    
    MovieProgressUPP theMovieProgressUPP = NULL;
    ComponentInstance ci = 0;
    QTAtomContainer theSettingsAtomContainer = 0;
    short resId = movieInDataForkResID;
    OSErr err = paramErr;
    
    CFDataRef data = request->data;
    if (NULL == data) goto bail;
    
    // myMovieExportData should now contain the needed export data
    MovieExportData *myMovieExportData = (MovieExportDataRef)CFDataGetBytePtr(data);
    if (NULL == myMovieExportData) goto bail;
    
    // create data references from the FSRefs
    err = QTNewDataReferenceFromFSRef(&myMovieExportData->sourceRef, 0, &sourceDataRef, &sourceDataRefType);
    if (err) goto bail;
    
    err = QTNewDataReferenceFromFSRef(&myMovieExportData->destRef, 0, &destDataRef, &destDataRefType);
    if (err) goto bail;
    
    // open the source movie
    err = NewMovieFromDataRef(&sourceMovie, newMovieActive, &resId, sourceDataRef, sourceDataRefType);
    if (err) goto bail;
    
    // set the movie progress proc - used to send update status to the client
    theMovieProgressUPP = NewMovieProgressUPP(myMovieProgressProc);
    if (NULL == theMovieProgressUPP) goto bail;
    
    //SetMovieProgressProc(sourceMovie, theMovieProgressUPP, (long)request->worker->remotePort);
    SetMovieProgressProc(sourceMovie, theMovieProgressUPP, (long)request->worker);
    
    // open the movie export component we want
    err = OpenADefaultComponent(myMovieExportData->componentType, myMovieExportData->componentSubType, &ci);
    if (err) goto bail;
        
    // turn the settings into the atom container
    PtrToHand(myMovieExportData->exportSettings, &theSettingsAtomContainer, myMovieExportData->exportSettingsSize);
    
    // configre the movie export component
    err = MovieExportSetSettingsFromAtomContainer(ci, theSettingsAtomContainer);
    if (err) goto bail;
    
    // export the movie
    err = ConvertMovieToDataRef(sourceMovie, 0, destDataRef, destDataRefType,
                                kQTFileTypeMovie, FOUR_CHAR_CODE('TVOD'),
                                createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
                                ci);
bail:
    request->actionFinished = true;
    
    if (ci) CloseComponent(ci);
    if (sourceDataRef) DisposeHandle(sourceDataRef);
    if (destDataRef) DisposeHandle(destDataRef);
    if (theSettingsAtomContainer) QTDisposeAtomContainer(theSettingsAtomContainer);
    if (sourceMovie) DisposeMovie(sourceMovie);
    if (theMovieProgressUPP) DisposeMovieProgressUPP(theMovieProgressUPP);
    
    return err;
}

void DoCancelExport(WorkerRequestRef request)
{
    // myMovieExportData will contain our needed export data including our settings
    MovieExportData *myMovieExportData = (MovieExportDataRef)CFDataGetBytePtr((CFDataRef)request->data);
    if (NULL == myMovieExportData) return;
    
    FSDeleteObject(&myMovieExportData->destRef);
}

CFDataRef myMessagePortListenerProc(CFMessagePortRef local, SInt32 msgid, CFDataRef inData, void *info)
{
    WorkerThreadRef worker = NULL;
    WorkerRequestRef request = NULL;
    CFDataRef replyData = NULL;
    char reply[255];
    
    UInt8 requestType;
    
    if (!inData || !info) return NULL;
    
    worker = (WorkerThreadRef)info;
    
    pthread_mutex_lock(&mutex);
    
    // what type of request is this?
    CFDataGetBytes(inData, CFRangeMake(0, sizeof(UInt8)), &requestType);
    
    switch(requestType) {
    case kExportRequest:
        // create and configure the request
        if (memFullErr == CreateWorkerRequest(worker, &request)) {
            fprintf(stderr, "MovieExportServer: CreateWorkerRequest memFullErr\n");
            break;
        }
    
        // make a copy of the export data
        request->data = CFDataCreateCopy(kCFAllocatorDefault, inData);
    
        Enqueue((QElemPtr)&request->nextRequest, &request->worker->requestQueue);
    
        sprintf(reply, "Reply: Reqest %ld queued.\n\n", request->worker->numberOfActiveRequests);
    
        replyData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)reply, strlen(reply)+1);

        break;
    case kCancelRequest:
    {
        // walk the pending request queue and cancel everything
        // requestElem will be NULL, or the address of a nextRequest field in a WorkerRequest
        QElemPtr requestElem = worker->requestQueue.qHead;
        if (requestElem) {
           
           do {
                request = (WorkerRequestRef)((long)requestElem - offsetof(WorkerRequest, nextRequest));
                if (request->worker != worker) {
                    fprintf(stderr, "MovieExportServer: bad request in queue!\n");
                } else {
                    TestAndSet(0, &request->wasCancelledOrStarted);
                }
            } while (requestElem = request->nextRequest);
            
            sprintf(reply, "Reply: All pending requests canceled.\n\n");

        } else {
            sprintf(reply, "Reply: No pending requests.\n\n");
        }
        
        replyData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)reply, strlen(reply)+1);
        break;
    }
    case kShutdownRequest:
        TestAndSet(0, &worker->shutdown);
        fprintf(stderr, "MovieExportServer: will shutdown\n");
        break;
    default:
        break;
    }
    
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
    
    return replyData;   // inData and replyData will be released for us after callback returns
}

void *MessagePortListener(void *inWorkerThreadRef)
{
    CFMessagePortContext messagePortContext = { 0 };
    WorkerThreadRef worker = (WorkerThreadRef)inWorkerThreadRef;
    
    messagePortContext.info = (void *)worker;
    
    worker->localPort = CFMessagePortCreateLocal(kCFAllocatorDefault, CFSTR("TheMessagePort"), myMessagePortListenerProc, &messagePortContext, false);
    worker->remotePort = CFMessagePortCreateRemote(kCFAllocatorDefault, CFSTR("TheProgressPort"));
    worker->runLoopSource = CFMessagePortCreateRunLoopSource(kCFAllocatorDefault, worker->localPort, 0);
    worker->runLoopRef = CFRunLoopGetCurrent();
    
    CFRunLoopAddSource(worker->runLoopRef, worker->runLoopSource, kCFRunLoopDefaultMode);
     
    CFRunLoopRun();
    sleep(1);
    
    CFMessagePortInvalidate(worker->localPort);
    CFRunLoopRemoveSource(worker->runLoopRef, worker->runLoopSource, kCFRunLoopDefaultMode);
    
    CFRelease(worker->localPort);
    CFRelease(worker->remotePort);
    CFRelease(worker->runLoopSource);
    
    pthread_exit(NULL);
}

int main (int argc, const char * argv[])
{
    OSErr err;

    WorkerThreadRef worker;
    
    EnterMovies();
    
    fprintf(stderr, "MovieExportServer: started\n");
    
    // allocate memory for the worker thread data
    worker = calloc(1, sizeof(WorkerThread));
    if (!worker) return memFullErr;
    
    // create the actual worker thread to listen for messages from the client
    err = pthread_create(&worker->workerThread, NULL, MessagePortListener, worker);
    if (err) return err;
    
    // initialize mutex and condition variable objects
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);
    
    pthread_mutex_lock(&mutex);
    
    while (1) {
        // when there is work to be done, handle all queued requests then wait again
        
        // requestElem will be NULL, or the address of a nextRequest field in a WorkerRequest
        QElemPtr requestElem = worker->requestQueue.qHead; 
        
        if (requestElem) {
            WorkerRequestRef request = (WorkerRequestRef)((long)requestElem - offsetof(WorkerRequest, nextRequest));
            Dequeue(requestElem, &worker->requestQueue);
            
            if (request->worker != worker) {
                fprintf(stderr, "MovieExportServer: bad request in queue!\n");
                continue;
            }
            
            if (TestAndSet(0, &request->wasCancelledOrStarted)) {
                // request was cancelled
                DoCancelExport(request);
                ReleaseWorkerRequest(request);
            } else {
                // request started, so run it
                err = DoExport(request);
                ReleaseWorkerRequest(request);
                if (err) fprintf(stderr, "MovieExportServer: DoExport error %d\n", err);
            }
            
        } else if (worker->shutdown) {
            // shutting down
            CFRunLoopStop(worker->runLoopRef);
            pthread_join(worker->workerThread, NULL);
            break;
        } else {
            // hang out and wait
            pthread_cond_wait(&condition, &mutex);
            pthread_mutex_unlock(&mutex);
        }
    }
    
    free(worker);
    
    fprintf(stderr, "MovieExportServer: shutdown\n");
     
    return 0;
}