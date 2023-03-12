/*
	File:		WorkerThread.c
	
	Description: Sample code showing how to send messages between the main thread and a worker pthread.
			     You could have an instance of this worker pthread interface for each window, for example.

	Author:		QuickTime Engineering, dts

	Copyright: 	© Copyright 2003-2004 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):  <1> qte 04/05/04 minor fixes for initial release
*/

//////////
//
// header files
//
//////////

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h> // for offsetof() macro
// Enqueue etc. are in OSUtils.h
// TestAndSet etc. and IncrementAtomic etc. are in DriverSynchronization.h
// EventLoopTimer APIs are in CarbonEventsCore.h

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include "WorkerThread.h"

//////////
//
// data structures
//
//////////

typedef struct WorkerThread {
    SInt32								referenceCount;
    WorkerActionRoutine					actionRoutine;
    WorkerCancelRoutine					cancelRoutine;
    WorkerResponseMainThreadCallback	responseCallback;
    void *				refcon;
    SInt32				numberOfActiveRequests;		// number of requests that have been created but not yet released
    pthread_t			workerThread;
    //POSIX sem_t				shutdownSemaphore;
    //POSIX sem_t				requestSemaphore;
    MPSemaphoreID       shutdownSemaphore;
    MPSemaphoreID       requestSemaphore;
    QHdr				requestQueue;				// messages going from main thread to worker thread
    EventLoopTimerUPP   responseEventLoopTimerUPP;
    EventLoopTimerRef   responseEventLoopTimer;
    QHdr				responseQueue;				// messages going from worker thread to main thread
    UInt8				responseQueueArmed;
    Boolean				shutdown;					// set to ask worker thread to shut down when all request queue is empty
    unsigned long       osVersion;                  // the OS version we are running on
} WorkerThread;

typedef struct WorkerRequest {
    SInt32				referenceCount;
    WorkerThreadRef		worker;						// note: each active request maintains a reference to its worker
    QElemPtr			nextRequest;				// used when linked into worker->requestQueue
    QElemPtr			nextResponse;				// used when linked into worker->responseQueue
    Boolean				wasSent;
    Boolean				wasCancelled;
    Boolean				wasStartedOrCancelled;
    Boolean				actionFinished;
    
    // add client-use fields here
    FSRef				fileRef;
    UInt32				doc;
    void *				threadData;
} WorkerRequest;

//////////
//
// function prototypes
//
//////////

static void *runWorkerThread ( void *argWorkerThreadRef );
static void runWorkerResponseEventLoopTimer ( EventLoopTimerRef timer, void *refcon );

#pragma mark-

//////////
//
// worker thread routines
//
//////////

OSErr createWorkerThread (
    WorkerActionRoutine actionRoutine,
    WorkerCancelRoutine cancelRoutine,
    WorkerResponseMainThreadCallback responseCallback,
    void *refcon,
    WorkerThreadRef *outWorker )
{
    WorkerThreadRef worker;
    
    if ( !actionRoutine || !responseCallback || !outWorker ) return paramErr;
    
    worker = calloc( 1, sizeof( WorkerThread ) );
    if ( ! worker ) return memFullErr;
    
    worker->referenceCount = 1;
    worker->actionRoutine = actionRoutine;
    worker->cancelRoutine = cancelRoutine;
    worker->responseCallback = responseCallback;
    worker->refcon = refcon;
    
    // get the OS version
    Gestalt(gestaltSystemVersion, &worker->osVersion);
    
    // create a one-shot event loop timer
    worker->responseEventLoopTimerUPP = NewEventLoopTimerUPP( runWorkerResponseEventLoopTimer );
    InstallEventLoopTimer( GetMainEventLoop(), kEventDurationForever, kEventDurationForever, 
        worker->responseEventLoopTimerUPP, worker, &worker->responseEventLoopTimer );
    
    // sem_init() not yet implemented as of MacOS X 10.3 - so we're going to use the MP APIs instead
    // the POSIX calls we could use when sem_init() is supported are clearly marked with the POSIX prefix
    // Named semaphors are available but not really ideal for our needs in this sample
    // Reference: http://developer.apple.com/documentation/Carbon/Reference/Multiprocessing_Services/multiproc_ref/function_group_3.html

    //POSIX sem_init( &worker->requestSemaphore, 0 /* not shared */, 0 /* initial value */ );
    //POSIX sem_init( &worker->shutdownSemaphore, 0 /* not shared */, 0 /* initial value */ );
    MPCreateSemaphore(1, 0, &worker->requestSemaphore);
    MPCreateSemaphore(1, 0, &worker->shutdownSemaphore);
    
    pthread_create( &worker->workerThread, NULL, runWorkerThread, worker );

    *outWorker = worker;
    return noErr;
}

void retainWorkerThread ( WorkerThreadRef worker )
{
    if ( !worker ) return;

    IncrementAtomic( &worker->referenceCount );
}

void releaseWorkerThread ( WorkerThreadRef worker )
{
    if ( !worker ) return;

    if ( 1 == DecrementAtomic( &worker->referenceCount ) ) {
        if ( 0 != worker->numberOfActiveRequests ) {
            DebugStr("\preleaseWorkerThread: reference count went to zero, but there are still active requests" );
        }
        
        ExitMoviesOnThread();
        
        pthread_detach( worker->workerThread );
        
        // ask worker thread to clean up and exit
        worker->shutdown = true;
        //POSIX sem_post( &worker->requestSemaphore );
        //POSIX sem_post( &worker->shutdownSemaphore ); // avoid race condition where busy thread disposes requestSemaphore before we post to it
        MPSignalSemaphore(worker->requestSemaphore);
        MPSignalSemaphore(worker->shutdownSemaphore); // avoid race condition where busy thread disposes requestSemaphore before we post to it
    }
}

#pragma mark-

//////////
//
// worker request routines
//
//////////

OSErr createWorkerRequest ( 
    WorkerThreadRef worker, 
    WorkerRequestRef *outRequest )
{
    if ( !outRequest ) return paramErr;

    WorkerRequestRef request = calloc( 1, sizeof( WorkerRequest ) );
    if ( !request ) return memFullErr;
    
    request->referenceCount = 1;
    request->worker = worker;
    
    IncrementAtomic( &worker->numberOfActiveRequests );
    retainWorkerThread( worker );
    
    *outRequest = request;
    return noErr;
}

static void *runWorkerThread ( void *argWorkerThreadRef )
{
    WorkerThreadRef worker = argWorkerThreadRef;
    
    // protect this thread from calling non-thread-safe components
    EnterMoviesOnThread(0);

    while ( true ) {
        // handle all queued requests, then wait on the semaphore
        QElemPtr requestElem = worker->requestQueue.qHead; // NULL, or the address of a nextRequest field in a WorkerRequest
        if ( requestElem ) {
            WorkerRequestRef request = (WorkerRequestRef)((long)requestElem - offsetof(WorkerRequest, nextRequest));
            Dequeue( requestElem, &worker->requestQueue );
            
            if ( request->worker != worker ) {
                DebugStr("\prunWorkerThread: bad request in requestQueue" );
                continue;
            }
            
            if ( TestAndSet( 0, &request->wasStartedOrCancelled ) ) {
                // this request was cancelled.
                request->wasCancelled = true;
            }
            else {
                // this request was not cancelled.  run it.
                (*worker->actionRoutine)( worker->refcon, request );
                request->actionFinished = true;
            }
            
            // queue the request on the response queue.
            Enqueue( (QElemPtr)&request->nextResponse, &worker->responseQueue );
            if ( 0 == TestAndSet( 0, &worker->responseQueueArmed ) ) {
                SetEventLoopTimerNextFireTime( worker->responseEventLoopTimer, kEventDurationNoWait );
            }
        }
        else if( worker->shutdown ) {
            // go peacefully
            break;
        }
        else {
            //POSIX sem_wait( &worker->requestSemaphore );
            MPWaitOnSemaphore(worker->requestSemaphore, kDurationForever);
        }
    }
    
    // wait so that we can be sure that the main thread is done signalling requestSemaphore.
    //POSIX sem_wait( &worker->shutdownSemaphore );
    MPWaitOnSemaphore(worker->shutdownSemaphore, kDurationForever);
    
    // clean up.
    //POSIX sem_destroy( &worker->requestSemaphore );
    //POSIX sem_destroy( &worker->shutdownSemaphore );
    MPDeleteSemaphore(worker->requestSemaphore);
    MPDeleteSemaphore(worker->shutdownSemaphore);
    RemoveEventLoopTimer( worker->responseEventLoopTimer );
    DisposeEventLoopTimerUPP( worker->responseEventLoopTimerUPP );
    free( worker );
    
    return NULL;
}

static void runWorkerResponseEventLoopTimer ( EventLoopTimerRef timer, void *refcon )
{
    WorkerThreadRef worker = refcon;
    
    while ( TestAndClear( 0, &worker->responseQueueArmed ) ) {
        QElemPtr responseElem = worker->responseQueue.qHead; // NULL, or the address of a nextResponse field in a WorkerRequest
        while ( responseElem ) {
            WorkerRequestRef request = (WorkerRequestRef)((long)responseElem - offsetof(WorkerRequest, nextResponse));
            Dequeue( responseElem, &worker->responseQueue );
            
            // run the response callback.
            (*worker->responseCallback)( worker->refcon, request );
            
            // (note that the response callback may release the request)
            responseElem = worker->responseQueue.qHead;
        }
    }
}

#pragma mark-

// accessors

OSErr setWorkerRequestFile ( 
    WorkerRequestRef request, 
    FSRef fileRef )
{
    if ( !request ) return paramErr;

    request->fileRef = fileRef;
    return noErr;
}

OSErr getWorkerRequestFile ( 
    WorkerRequestRef request, 
    FSRef *fileRef )
{
    if (( !request ) || ( !fileRef ) ) return paramErr;

    *fileRef = request->fileRef;
    return noErr;
}

OSErr setWorkerRequestDoc ( 
    WorkerRequestRef request, 
    UInt32 doc )
{
    if ( !request ) return paramErr;

    request->doc = doc;
    return noErr;
}

OSErr getWorkerRequestDoc ( 
    WorkerRequestRef request, 
    UInt32 *doc )
{
    if (( !request ) || ( !doc ) ) return paramErr;

    *doc = request->doc;
    return noErr;
}

OSErr setWorkerRequestThreadData ( 
	WorkerRequestRef request, 
	void *threadData )
{
    if ( !request ) return paramErr;

    request->threadData = threadData;
    return noErr;
}
        
OSErr getWorkerRequestThreadData ( 
        WorkerRequestRef request, 
        void **threadDataPtr )
{
    if (( !request ) || ( !threadDataPtr ) ) return paramErr;

    *threadDataPtr = request->threadData;
    return noErr;
}

// add more accessors as you like

#pragma mark-

OSErr sendWorkerRequest ( WorkerRequestRef request )
{
    if ( !request ) return paramErr;

    if ( request->wasSent ) {
        DebugStr("\psendWorkerRequest: request was already sent");
        return paramErr;
    }
        
    request->wasSent = true;
    Enqueue( (QElemPtr)&request->nextRequest, &request->worker->requestQueue );
    //POSIX sem_post( &request->worker->requestSemaphore );
    MPSignalSemaphore(request->worker->requestSemaphore);
        
    return noErr;
}

void cancelWorkerRequest ( WorkerRequestRef request )
{
    if ( !request ) return;

    if ( !request->wasSent ) {
        DebugStr("\pcancelWorkerRequest: request was not sent");
        return;
    }
    if ( request->wasCancelled ) {
        DebugStr("\pcancelWorkerRequest: request was already cancelled");
        return;
    }
    if ( TestAndSet( 0, &request->wasStartedOrCancelled ) ) {
        // request has already started
        if ( request->actionFinished ) {
            // request has already finished: nothing we can do
        }
        else {
            // request action routine is currently running: call the cancel function, if set
            request->wasCancelled = true;
            if ( request->worker->cancelRoutine )
                (*request->worker->cancelRoutine)( request->worker->refcon, request );
        }
    }
    else {
        // request had not yet started, and by setting wasStartedOrCancelled we've cancelled it
        request->wasCancelled = true;
    }
}

Boolean wasWorkerRequestCancelled ( WorkerRequestRef request )
{
    if ( !request ) return false;

    return request->wasCancelled;
}

void releaseWorkerRequest ( WorkerRequestRef request )
{
    if ( 1 == DecrementAtomic( &request->referenceCount ) ) {
        DecrementAtomic( &request->worker->numberOfActiveRequests );
        releaseWorkerThread( request->worker );
            
        free( request );
    }
}

void retainWorkerRequest ( WorkerRequestRef request )
{
    IncrementAtomic( &request->referenceCount );
}


