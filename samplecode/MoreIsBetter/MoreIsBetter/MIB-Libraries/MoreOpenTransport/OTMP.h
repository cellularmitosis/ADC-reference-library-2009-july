/*
	File:		OTMP.h

	Contains:	MP task friendly OT library.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: OTMP.h,v $
Revision 1.9  2002/11/08 23:42:32         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.8  2001/11/07 15:56:59         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>      8/2/01    Quinn   [2619462] The fix for the MP bug is in CarbonLib 1.2.5.
         <5>      8/2/01    Quinn   Add missing word to comment.
         <4>      8/2/01    Quinn   Add OTMPGetMessage and OTMPPutMessage.
         <3>     5/12/00    Quinn   Looks like the bug fix won't make it into CarbonLib 1.2.
         <2>    10/11/00    Quinn   Added OTMPGetCanRunStatus.
         <1>     7/11/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <OpenTransport.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

typedef UInt32 OTMPCanRunStatus;
enum {
	kOTMPCanRun 		= 0,
	kOTMPSystemTooOld 	= 1,
	kOTMPBadCarbon 		= 2
};

extern pascal OTMPCanRunStatus OTMPGetCanRunStatus(void);
	// This routine returns a value of type OTMPCanRunStatus 
	// that indicates whether OTMP can run and, if not, why 
	// it can't.  Note that OTMP refuses to run if a CarbonLib 
	// older than version 1.2.5 is installed.  This is because a 
	// bug in older versions of CarbonLib [2563553] prevents 
	// MP tasks from using DTInstall (even those linked against 
	// InterfaceLib).

// This module logs its activities to MoreMPLog.  You can control which 
// bit in the log mask it uses (see "MoreMPLog.h" for a discussion of the 
// log mask) by changing this constant.

enum {
	kOTMPWaitRecordLogID 	= 1,
	kOTMPNotificationLogID 	= 2,
	kOTMPStdActionLogID 	= 3,
	kOTMPAPILogID 			= 4,
	kOTMPRetriesLogID 		= 5
};

// OTMPProviderRef is reference to a private structure defined inside this module.

typedef struct OTMPProvider OTMPProvider;
typedef OTMPProvider *OTMPProviderRef;

// Like OT itself, OTMP supports a limited degree of polymorphism.  Specifically, 
// OTMPEndpointRef is a subclass of OTMPProviderRef.  This structure is hard to 
// represent in C, so we just equate the two types.

typedef OTMPProviderRef OTMPEndpointRef;

extern pascal OSStatus InitOpenTransportMPInContext(
                                 OTInitializationFlags  flags,
                                 OTClientContextPtr *   clientContext);
	// You must call this routine at system task time.

extern pascal void CloseOpenTransportMPInContext(OTClientContextPtr clientContext);
	// You must call this routine at system task time.

extern pascal OSStatus OTMPPrepareThisTask(void);
	// You must call this from each MP task that is to call 
	// this library.  You must not call it for the main thread 
	// (or any other cooperative thread).

extern pascal void OTMPUnprepareThisTask(void);
	// You must call this from each MP task that you have 
	// prepared using OTMPPrepareThisTask before the task quits. 
	// This routine does not require that the preparation have 
	// been successful (or even to have occured at all), so you 
	// can safely call it at the end of any MP task that might 
	// have called OTMPPrepareThisTask.

extern pascal OTMPEndpointRef
OTMPOpenEndpointQInContext      (const char *           cfig,
                                 OTOpenFlags            oflag,
                                 TEndpointInfo *        info,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);
	// This "quick" version of OTOpenEndpoint uses a configuration 
	// string rather than a OTConfigurationRef.  I haven't 
	// yet implemented the real routine because I'm still trying 
	// to decide how to handle OTCreateConfiguration.  The obvious 
	// approach would require two round trips from MP to blue, which 
	// is unnecessarily slow.  The main advantage of using 
	// a configuration is that you can create endpoints quickly by 
	// creating a single configuration and cloning it each time you 
	// create an endpoint.  This speeds up endpoint creation a little. 
	// However, if your application is creating lots of endpoints you 
	// will get a much better speed increase by maintaining a cache of 
	// created endpoints.
	// 
	// Regardless, this routine is just fine for 99% of clients.
	
extern pascal OSStatus
OTMPCloseProvider               (OTMPProviderRef        ref);

extern pascal SInt32
OTMPIoctl                       (OTMPProviderRef        ref,
                                 UInt32                 cmd,
                                 void *                 data);

extern pascal OSStatus
OTMPCancelSynchronousCalls      (OTMPProviderRef        ref,
                                 OSStatus               errParam);


extern pascal OSStatus
OTMPGetEndpointInfo             (OTMPEndpointRef        ref,
                                 TEndpointInfo *        info);

extern pascal OTResult
OTMPGetEndpointState            (OTMPEndpointRef        ref);

extern pascal OTResult
OTMPLook                        (OTMPEndpointRef        ref);

extern pascal OTResult
OTMPCountDataBytes              (OTMPEndpointRef        ref,
                                 OTByteCount *          countPtr);

extern pascal OSStatus
OTMPGetProtAddress              (OTMPEndpointRef        ref,
                                 TBind *                boundAddr, /* can be NULL */
                                 TBind *                peerAddr)  /* can be NULL */;
extern pascal OSStatus
OTMPResolveAddress              (OTMPEndpointRef        ref,
                                 TBind *                reqAddr,
                                 TBind *                retAddr,
                                 OTTimeout              timeOut);

extern pascal OSStatus
OTMPBind                        (OTMPEndpointRef        ref,
                                 TBind *                reqAddr, /* can be NULL */
                                 TBind *                retAddr) /* can be NULL */;

extern pascal OSStatus
OTMPUnbind                      (OTMPEndpointRef        ref);

extern pascal OSStatus
OTMPConnect                     (OTMPEndpointRef        ref,
                                 TCall *                sndCall,
                                 TCall *                rcvCall) /* can be NULL */;

extern pascal OSStatus
OTMPListen                      (OTMPEndpointRef        ref,
                                 TCall *                call);

extern pascal OSStatus
OTMPAccept                      (OTMPEndpointRef        listener,
                                 OTMPEndpointRef        worker,
                                 TCall *                call);
	// The implementation of this routine has a race condition which 
	// might be exposed if the worker endpoint is active (ie there are 
	// outstanding synchronous operations on the endpoint).  Rather than 
	// fix the problem (which would require a major rework of the 
	// library's global concurrency model), I've decided to outlaw this 
	// behaviour.  You must call this routine only with worker endpoints 
	// that are quiescent (ie have no outstanding synchronous operations).
	// 
	// See the comments in the implementation for details.
	
extern pascal OSStatus
OTMPSndDisconnect               (OTMPEndpointRef        ref,
                                 TCall *                call) /* can be NULL */;

extern pascal OSStatus
OTMPRcvDisconnect               (OTMPEndpointRef        ref,
                                 TDiscon *              discon) /* can be NULL */;

extern pascal OSStatus
OTMPSndOrderlyDisconnect        (OTMPEndpointRef        ref);

extern pascal OSStatus
OTMPRcvOrderlyDisconnect        (OTMPEndpointRef        ref);

extern pascal OSStatus
OTMPOptionManagement            (OTMPEndpointRef        ref,
                                 TOptMgmt *             req,
                                 TOptMgmt *             ret);

extern pascal OTResult
OTMPRcv                         (OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags *              flags);

extern pascal OTResult
OTMPSnd                         (OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags                flags);

extern pascal OSStatus
OTMPSndUData                    (OTMPEndpointRef        ref,
                                 TUnitData *            udata);

extern pascal OSStatus
OTMPRcvUData                    (OTMPEndpointRef        ref,
                                 TUnitData *            udata,
                                 OTFlags *              flags);

extern pascal OSStatus
OTMPRcvUDErr                    (OTMPEndpointRef        ref,
                                 TUDErr *               uderr);

#if CALL_NOT_IN_CARBON

	// IMPORTANT:
	// These routines are a preliminary implementation and have 
	// not yet been tested properly.  Specifically, I have not 
	// tested the flow control execution paths.
	
	extern pascal OTResult
	OTMPGetMessage                  (OTMPProviderRef        ref,
	                                 strbuf *               ctlbuf,
	                                 strbuf *               databuf,
	                                 OTFlags *              flagsPtr);

	extern pascal OSStatus
	OTMPPutMessage                  (OTMPProviderRef        ref,
	                                 const strbuf *         ctlbuf,
	                                 const strbuf *         databuf,
	                                 OTFlags                flags);

#endif

/////////////////////////////////////////////////////////////////
// Cooperative Threading

// During the bring up and debugging effort it's often useful to run 
// everything from the main thread (or from Thread Manager threads). 
// To limit OTMP's external dependencies, I allow you to install a 
// callback that OTMP calls while is waiting for async operations
// to complete.  You can use this callback to call WaitNextEvent, or 
// yield time in whatever way is appropriate for your application.
// This is very similar to the kOTSyncIdleEvent idea, except that 
// there is only one callback for the entire library.
//
// IMPORTANT: You only need to install this callback if you call 
// OTMP from your application's main thread or a Thread Manager thread. 
// If you always call OTMP from preemptively scheduled MP tasks this 
// callback is unnecessary.

typedef pascal OSStatus (*OTMPMainThreadYielder)(void);
	// OTMP calls this routine when it is waiting "sync waiting", 
	// ie waiting for an asynchronous event to complete.  The routine 
	// should yield time in whatever way that's appropriate for your 
	// application.
	// 
	// If the routine returns an error, OTMP will break out of its 
	// sync wait loop.  You should only do this under the most dire 
	// circumstances!!!  It will leave the entire library in a very 
	// base state.  It's useful for breaking out of endless loops while 
	// debugging, and not much else.  For production code you should 
	// cancel the synchronous call using OTMPCancelSynchronousCalls.

extern pascal void InstallOTMPMainThreadYielder(OTMPMainThreadYielder yielder);
	// Installs, or removes if yielder is NULL, the yielder callback.

/////////////////////////////////////////////////////////////////
// Debugging Stuff

#if MORE_DEBUG

	// The following routines were useful for bringing up OTMP 
	// and may also be useful when debugging your applications.

	extern void OTMPPrintSndRetryFreqDist(void);
		// Prints, to stdout, a frequency distribution of the 
		// number of memory retries by OTMPSnd.  See the 
		// implementation for more details.

	extern pascal void RunOTMPAsSystemTasks(Boolean asSystemTasks);
		// If asSystemTasks is true, the module will run entirely at 
		// system task time.  This was very useful when debugging 
		// OTMP but is of limited usefulness you OTMP clients.  Call this 
		// routine after initialising the library but before calling 
		// any other library  routines.  This routine also forces all 
		// Blue tasks to run at system task time (see "MoreBlueActions.h").
		// Must be called at system task time.
	
#endif

/////////////////////////////////////////////////////////////////

// OTMPX
// -----
// These routines look at the current system version and do the right 
// thing on both traditional Mac OS (call OTMP) and Mac OS X 
// (call Mac OS X's built in OT compatibility directly).

extern pascal OSStatus InitOpenTransportMPXInContext(
                                 OTInitializationFlags  flags,
                                 OTClientContextPtr *   clientContext);

extern pascal void CloseOpenTransportMPXInContext(OTClientContextPtr clientContext);

extern pascal OSStatus OTMPXPrepareThisTask(void);

extern pascal void OTMPXUnprepareThisTask(void);

extern pascal OTMPEndpointRef
OTMPXOpenEndpointQInContext     (const char *           cfig,
                                 OTOpenFlags            oflag,
                                 TEndpointInfo *        info,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);
	
extern pascal OSStatus
OTMPXCloseProvider              (OTMPProviderRef        ref);

extern pascal SInt32
OTMPXIoctl                      (OTMPProviderRef        ref,
                                 UInt32                 cmd,
                                 void *                 data);

extern pascal OSStatus
OTMPXCancelSynchronousCalls     (OTMPProviderRef        ref,
                                 OSStatus               errParam);


extern pascal OSStatus
OTMPXGetEndpointInfo            (OTMPEndpointRef        ref,
                                 TEndpointInfo *        info);

extern pascal OTResult
OTMPXGetEndpointState           (OTMPEndpointRef        ref);

extern pascal OTResult
OTMPXLook                       (OTMPEndpointRef        ref);

extern pascal OTResult
OTMPXCountDataBytes             (OTMPEndpointRef        ref,
                                 OTByteCount *          countPtr);

extern pascal OSStatus
OTMPXGetProtAddress             (OTMPEndpointRef        ref,
                                 TBind *                boundAddr, /* can be NULL */
                                 TBind *                peerAddr)  /* can be NULL */;
extern pascal OSStatus
OTMPXResolveAddress             (OTMPEndpointRef        ref,
                                 TBind *                reqAddr,
                                 TBind *                retAddr,
                                 OTTimeout              timeOut);

extern pascal OSStatus
OTMPXBind                       (OTMPEndpointRef        ref,
                                 TBind *                reqAddr, /* can be NULL */
                                 TBind *                retAddr) /* can be NULL */;

extern pascal OSStatus
OTMPXUnbind                     (OTMPEndpointRef        ref);

extern pascal OSStatus
OTMPXConnect                    (OTMPEndpointRef        ref,
                                 TCall *                sndCall,
                                 TCall *                rcvCall) /* can be NULL */;

extern pascal OSStatus
OTMPXListen                     (OTMPEndpointRef        ref,
                                 TCall *                call);

extern pascal OSStatus
OTMPXAccept                     (OTMPEndpointRef        listener,
                                 OTMPEndpointRef        worker,
                                 TCall *                call);

extern pascal OSStatus
OTMPXSndDisconnect              (OTMPEndpointRef        ref,
                                 TCall *                call) /* can be NULL */;

extern pascal OSStatus
OTMPXRcvDisconnect              (OTMPEndpointRef        ref,
                                 TDiscon *              discon) /* can be NULL */;

extern pascal OSStatus
OTMPXSndOrderlyDisconnect       (OTMPEndpointRef        ref);

extern pascal OSStatus
OTMPXRcvOrderlyDisconnect       (OTMPEndpointRef        ref);

extern pascal OSStatus
OTMPXOptionManagement           (OTMPEndpointRef        ref,
                                 TOptMgmt *             req,
                                 TOptMgmt *             ret);

extern pascal OTResult
OTMPXRcv                        (OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags *              flags);

extern pascal OTResult
OTMPXSnd                        (OTMPEndpointRef        ref,
                                 void *                 buf,
                                 OTByteCount            nbytes,
                                 OTFlags                flags);

extern pascal OSStatus
OTMPXSndUData                   (OTMPEndpointRef        ref,
                                 TUnitData *            udata);

extern pascal OSStatus
OTMPXRcvUData                   (OTMPEndpointRef        ref,
                                 TUnitData *            udata,
                                 OTFlags *              flags);

extern pascal OSStatus
OTMPXRcvUDErr                   (OTMPEndpointRef        ref,
                                 TUDErr *               uderr);

#ifdef __cplusplus
}
#endif
