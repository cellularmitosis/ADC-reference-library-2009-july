/*
	File:		WorkerThread.h
	
	Description: Sample code showing how to send messages between the main thread and a worker pthread.
			     You could have an instance of this worker pthread interface for each window, for example.

	Author:		QTEngineering

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

*/

#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <CoreServices/CoreServices.h>
#include "ThreadData.h"

// This is the interface to a worker thread.
struct WorkerThread;
typedef struct WorkerThread *WorkerThreadRef;

// This encapsulates individual work requests that will be performed by a worker thread.
struct WorkerRequest;
typedef struct WorkerRequest *WorkerRequestRef;


// This is the routine called on the worker thread
typedef void (*WorkerActionRoutine)( void *refcon, WorkerRequestRef request );

// This is a routine that can be called to cancel the current action
// while WorkerActionRoutine is running.  (It's not called for requests that are 
// cancelled before the action starts.)  
/// If there's no way to do that, pass NULL to createWorkerThread instead.
typedef void (*WorkerCancelRoutine)( void *refcon, WorkerRequestRef request );

// This is called on the main thread after a request is completed or cancelled
typedef void (*WorkerResponseMainThreadCallback)( void *refcon, WorkerRequestRef request );


// Call this on the main thread to create a worker thread to run your requests.
OSErr createWorkerThread(
	WorkerActionRoutine actionRoutine,
	WorkerCancelRoutine cancelRoutine,
	WorkerResponseMainThreadCallback responseCallback,
	void *refcon,
	WorkerThreadRef *outWorker );

// In case you need it.
void retainWorkerThread(
	WorkerThreadRef worker );

// Call this on the main thread to release the worker thread.  
// This does not block; when when the reference count drops to zero and the worker thread 
// has finished all work, it will clean up by itself.
void releaseWorkerThread(
	WorkerThreadRef worker );


// Call this from the main thread to create a request object; then set attributes 
// of the request object and send it.  When the request is done, your responseCallback 
// will be called on the main thread.  If you want to cancel the request early,
// call cancelWorkerRequest from the main thread.  If not cancelled before it finishes,
// the request will be handled by a call to your actionRoutine on the worker thread.
OSErr createWorkerRequest( 
	WorkerThreadRef worker, 
	WorkerRequestRef *outRequest );

// These are setters and getters for objects or values that are passed between the 
// main thread and worker thread.  The messaging code doesn't care about these, and you
// can add whatever extra fields and accessors you want.
// These accessors are not protected; you must prevent your code from manipulating 
// a request's objects from the main thread between sending the request and receiving 
// the response.
OSErr setWorkerRequestFile( 
	WorkerRequestRef request, 
	FSRef fileRef );
        
OSErr getWorkerRequestFile( 
	WorkerRequestRef request, 
	FSRef *fileRef );
        
OSErr setWorkerRequestDoc( 
	WorkerRequestRef request, 
	UInt32 doc );
        
OSErr getWorkerRequestDoc ( 
        WorkerRequestRef request, 
        UInt32 *doc );

OSErr setWorkerRequestThreadData ( 
	WorkerRequestRef request, 
	ThreadData *threadData );
        
OSErr getWorkerRequestThreadData ( 
        WorkerRequestRef request, 
        ThreadData **threadDataPtr );

// ++ add more accessors as you like ++

// Call this to schedule the request to be sent to the worker thread.
// The worker thread will process them in first-in, first-out order.
// If you don't want to send the request after all, just release it 
// before sending.
OSErr sendWorkerRequest( 
	WorkerRequestRef request );

// Call this from the main thread to cancel an already-scheduled request.
// If the request has not yet started, the action routine will not be called;
// if the action routine has started running, the cancel routine (if provided) 
// will be called.  If the action routine has completed, this will have no effect.
void cancelWorkerRequest( 
	WorkerRequestRef request );

// Call this from your response callback to find out whether the request was cancelled.
Boolean wasWorkerRequestCancelled( 
	WorkerRequestRef request );

// I'm not sure if clients will ever need to call this, but here it is.
void retainWorkerRequest( 
	WorkerRequestRef request );

// Call this from your response callback to release the worker request.
// If you don't release requests, you will leak not only the request entries but the worker thread too.
void releaseWorkerRequest( 
	WorkerRequestRef request );

#endif // WORKER_THREAD_H
