/*
	File:		OTMPSimpleServerHTTP.c

	Contains:	Implementation of the simple HTTP server sample (using MP tasks).

	Written by:	Quinn "The Eskimo!"

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: OTMPSimpleServerHTTP.c,v $
Revision 1.8  2002/11/08 23:44:17         
Include <Files.h> before <OpenTransportProviders.h> because of a bug in UI. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.7  2001/11/07 15:57:03         
Tidy up headers, add CVS logs, update copyright.


         <6>     21/9/01    Quinn   Fix Project Builder warning.
         <5>     21/9/01    Quinn   Version 1.0a5 update.
         <4>      9/7/01    Quinn   Fixed bug in server connection accept loop. Also eliminated use
                                    of MPLogPrintfSlow.
         <3>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <2>    22/11/00    Quinn   Fix CWPro6 type check nitpick.
         <1>     7/11/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MIB Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Files.h>
	#include <OpenTransport.h>
	#include <OpenTransportProviders.h>
	#include <Threads.h>
	#include <Files.h>
	#include <TextUtils.h>
	#include <Multiprocessing.h>
#endif

// Standard C interfaces

#include <stdio.h>

// MIB interfaces

#include "MoreMPLog.h"
#include "OTMP.h"

// Pick up our own prototype.

#include "OTMPSimpleServerHTTP.h"

/////////////////////////////////////////////////////////////////

// Forward declarations.

static OSStatus RegisterEndpoint(OTMPEndpointRef mpEP);
static void     UnregisterEndpoint(OTMPEndpointRef mpEP);

/////////////////////////////////////////////////////////////////
#pragma mark ***** Misc Utilities *****

static OTResult SetFourByteOption(OTMPEndpointRef ep, OTXTILevel level, OTXTIName name, UInt32 value)
	// Standard boilerplate code, copied from "Inside Macintosh: 
	// Networking with Open Transport".  The only change I made 
	// was to substitute OTMPXOptionManagement for OTOptionManagement.
	// Sweet!
{
	OTResult err;
	TOption option;
	TOptMgmt request;
	TOptMgmt result;

	/* Set up the option buffer to specify the option and value to set. */

	option.len = kOTFourByteOptionSize;
	option.level = level;
	option.name = name;
	option.status = 0;
	option.value[0] = value;

	/* Set up request parameter for OTOptionManagement */

	request.opt.buf = (UInt8 *) &option;
	request.opt.len = sizeof(option);
	request.flags = T_NEGOTIATE;

	/* Set up reply parameter for OTOptionManagement. */
	
	result.opt.buf = (UInt8 *) &option;
	result.opt.maxlen = sizeof(option);
	err = OTMPXOptionManagement(ep, &request, &result);
	if (err == noErr) {
		if (option.status != T_SUCCESS) {
			err = option.status;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////

static OSStatus OTMPXSndQ(OTMPEndpointRef ep, void *buf, size_t nbytes)
	// My own personal wrapper around the OTSnd routine that cleans
	// up the error result.
{
	OTResult bytesSent;
	
	bytesSent = OTMPXSnd(ep, buf, nbytes, 0);
	if (bytesSent >= 0) {
	
		// Because we're running in synchronous blocking mode, OTSnd
		// should not return until it has sent all the bytes unless it
		// gets an error.  If it does, we want to hear about it.
		assert(bytesSent == nbytes);
	
		return noErr;
	} else {
		return bytesSent;
	}
}

/////////////////////////////////////////////////////////////////

static OSStatus OTMPXAcceptQ(OTMPEndpointRef listener, OTMPEndpointRef worker, TCall *call)
	// My own personal wrapper around the OTAccept routine that handles 
	// the connection attempt disappearing cleanly.
{
	OSStatus err;
	OSStatus junk;
	OTResult look;
	TDiscon  discon;
	
	// First try the Accept.
	
	err = OTMPXAccept(listener, worker, call);
	MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLa', (void *) err);
	
	// If that fails with a look error, let’s see what the problem is.
	
	if (err == kOTLookErr) {
		look = OTMPXLook(listener);
		MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLl', (void *) look);

		// Only two async events should be able to cause Accept to "look", namely 
		// T_LISTEN and T_DISCONNECT.  However, the "tilisten" module prevents 
		// further connection attempts coming up while we’re still thinking about 
		// this one, so the only event that should come up is T_DISCONNECT.
		
		assert(look == T_DISCONNECT);
		if (look == T_DISCONNECT) {
		
			// If we get a T_DISCONNECT, it should be for the current pending 
			// connection attempt.  We receive the disconnect info and check 
			// the sequence number against that in the call.  If they match, 
			// the connection attempt disappeared and we return kOTNoDataErr.
			// If they don’t match, that’s weird.
			
			OTMemzero(&discon, sizeof(discon));

			junk = OTMPXRcvDisconnect(listener, &discon);
			assert(junk == noErr);
			MPLog2(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLr', (void *) junk, (void *) discon.sequence);
			
			if (discon.sequence == call->sequence) {
				err = kOTNoDataErr;
			} else {
				assert(false);
				// Leave err set to kOTLookErr.
			}
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////

static OSErr FSReadQ(short refNum, long count, void *buffPtr)
	// My own wrapper for FSRead.  Whose bright idea was it for
	// it to return the count anyway!
{
	OSStatus err;
	long tmpCount;
	
	tmpCount = count;
	err = FSRead(refNum, &count, buffPtr);
	
	assert((err != noErr) || (count == tmpCount));
	
	return err;
}

/////////////////////////////////////////////////////////////////

static OSStatus OpenAndRegisterEndpoint(const char *cfig, OTMPEndpointRef *mpEP)
	// A wrapper for OTMPXOpenEndpointQInContext which also 
	// registers the endpoint with endpoint registry.  We 
	// need to register our endpoints so that we can quit 
	// cleanly.  See TermOTMPSimpleServerHTTP for more details.
{
	OSStatus err;
	OSStatus junk;
	
	*mpEP = OTMPXOpenEndpointQInContext(cfig, 0, NULL, &err, NULL);
	if (err == noErr) {
		err = RegisterEndpoint(*mpEP);
		if (err != noErr) {
			junk = OTMPXCloseProvider(*mpEP);
			assert(junk == noErr);
			*mpEP = NULL;
		}
	}
	
	assert( (err == noErr) == (*mpEP != NULL) );
	return err;
}

static OSStatus OrderlyDisconnect(OTMPEndpointRef ep)
	// Gosh XTI is lame.  RcvOrderlyDisconnect is non-blocking 
	// (it doesn't wait for the T_ORDREL event) so we have to 
	// block in a Rcv call.  This is standard XTI practice.
{
	OSStatus 	err;
	UInt8 		tmp;
	OTFlags 	junkFlags;
	OTResult 	look;
	
	err = OTMPXSndOrderlyDisconnect(ep);
	if (err == noErr) {
		err = OTMPXRcv(ep, &tmp, sizeof(tmp), &junkFlags);
		if (err == kOTLookErr) {
			look = OTMPXLook(ep);
			switch (look) {
				case T_ORDREL:
					err = OTMPXRcvOrderlyDisconnect(ep);
					break;
				default:
					// leave err set to kOTLookErr
					break;
			}
		} else if (err == noErr) {
			err = kOTLookErr;			// something is happening, but it's not a disconnect
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Worker Thread Stuff *****

static Boolean StringHasSuffix(const char *str, const char *suffix)
	// Returns true if the end of str is suffix.
{
	Boolean result;
	
	result = false;
	if ( OTStrLength(str) >= OTStrLength(suffix) ) {
		if ( OTStrEqual(str + OTStrLength(str) - OTStrLength(suffix) , suffix) ) {
			result = true;
		}
	}
	
	return result;
}

static OSStatus ExtractRequestedFileName(const char *buffer,
											char *fileName, char *mimeType)
	// Assuming that buffer is a C string contain an HTTP request,
	// extract the name of the file that's being requested.
	// Also check to see if the file has one of the common suffixes,
	// and set mimeType appropriately.
	//
	// Obviously this routine should use Internet Config to
	// map the file type/creator/extension to a MIME type,
	// but I don't want to complicate the sample with that code.
	//
	// Also, we don't handle malformed HTTP requests, or indeed 
	// check for buffer overrun on fileName.  This is an OT sample, 
	// not a web server sample.
{
	OSStatus err;
	
	// Default the result to empty.
	fileName[0] = 0;
	
	// Scan the request looking for the fileName.  Obviously this is not
	// a very good validation of the request, but this is an OT sample,
	// not an HTTP one.  Also note that we require HTTP/1.0, but some
	// ancient clients might just generate "GET %s<cr><lf>"
	
	(void) sscanf(buffer, "GET %s HTTP/1.0", fileName);		// MP?
	
	// If the file name is still blank, scanf must have failed.
	// Note that I don't rely on the result from scanf because in a
	// previous life I learnt to mistrust it.
	
	if (fileName[0] == 0) {
		err = -1;
	} else {
	
		// So the request is cool.  Normalise the file name.
		// Requests for the root return "index.html".
		
		if ( OTStrEqual(fileName, "/") ) {
			OTStrCopy(fileName, "index.html");
		}
		
		// Remove the prefix slash.  Note that we don't deal with
		// "slashes" embedded in the fileName, so we don't handle
		// any directories other than the root.  This would be
		// easy to do, but again this is not an HTTP sample.
		
		if ( fileName[0] == '/' ) {
			BlockMoveData(&fileName[1], &fileName[0], OTStrLength(fileName));
		}
	
		// Set mimeType based on the file's suffix.
		
		if ( StringHasSuffix(fileName, ".html") ) {
			OTStrCopy(mimeType, "text/html");
		} else if ( StringHasSuffix(fileName, ".gif") ) {
			OTStrCopy(mimeType, "image/gif");
		} else if ( StringHasSuffix(fileName, ".jpg") ) {
			OTStrCopy(mimeType, "image/jpeg");
		} else {
			OTStrCopy(mimeType, "text/plain");
		}
		err = noErr;
	}
	
	#if MORE_DEBUG
		MPLogPrintf("ExtractRequestedFileName: Returning %d, “%s”, “%s”\n", err, fileName, mimeType);
	#endif
	
	return err;
}

/////////////////////////////////////////////////////////////////

// gRunningThreads contains the number of running HTTP listener and 
// worker threads.
//
// We spool an HTTP listener for each IP address on the computer.
// Normally you would only spool one listener for the entire machine
// (listening on kOTAnyInetAddress), but we want to actively distinguish
// between each IP address so that we can server different information
// for each IP address.
// 
// We also spool a thread for each individual connection.  This is 
// obviously bad design (we should preallocate a thread pool and hand 
// requests off to threads in the pool) but, again, this is sample code 
// not Apache.
//
// Two differences between this sample and the non-preemptive version:
//
// o We count both listener and worker threads in this value.
// o We adjust this value with atomic operations.

volatile SInt32 gRunningThreads = 0;

// The worker thread reads characters one at a time from the endpoint
// and uses the following state machine to determine when the request is
// finished.  For HTTP/1.0 requests, the request is terminated by
// two consecutive CR LF pairs.  Each time we read one of the appropriate
// characters we increment the state until we get to kDone, at which
// point we go off to process the request.

enum {
	kWorkerWaitingForCR1,
	kWorkerWaitingForLF1,
	kWorkerWaitingForCR2,
	kWorkerWaitingForLF2,
	kWorkerDone
};

// This is the size of the transfer buffer that each worker thread
// allocates to read file system data and write network data.  Real 
// servers should use a bigger buffer and probably should cache 
// file data (at least on traditional Mac OS).

enum {
	kTransferBufferSize = 4096
};

// WorkerContext holds the information needed by a worker endpoint to
// operate.  A WorkerContext is created by the listener endpoint
// and passed as the thread parameter to the worker thread.  If the
// listener successfully does this, it's assumed that the worker
// thread has taken responsibility for freeing the context.

struct WorkerContext {
	OTMPEndpointRef worker;
	short vRefNum;
	long dirID;
};
typedef struct WorkerContext WorkerContext, *WorkerContextPtr;

// The two buffers hold standard HTTP responses.  The first is the 
// default text we spit out when we get an error.  The second is
// the header that we use when we successfully field a request.
// Again note that this sample is not about HTTP, so these responses
// are probably not particularly compliant to the HTTP protocol.

char gDefaultOutputText[] = "HTTP/1.0 200 OK\x0D\x0A" "Content-Type: text/html\x0D\x0A\x0D\x0A" "<H1>Say what!</H1><P>\x0D\x0A" "Error Number (%d), Error Text (%s)";
char gHTTPHeader[] = "HTTP/1.0 200 OK\x0D\x0A" "Content-Type: %s\x0D\x0A\x0D\x0A";

/////////////////////////////////////////////////////////////////

static OSStatus ReadHTTPRequest(OTMPEndpointRef worker, char *buffer)
	// This routine reads the HTTP request from the worker endpoint,
	// using the state machine described above, and puts it into the
	// indicated buffer.  The buffer must be at least kTransferBufferSize
	// bytes big.
	//
	// This is pretty feeble
	// code (reading data one byte at a time is bad for performance),
	// but it works and I'm not quite sure how to fix it.  Perhaps
	// OTCountDataBytes?
	//
	// Also, the code does not support requests bigger than
	// kTransferBufferSize.  In practise, this isn't a problem.
{
	OSStatus err;
	long 	 bufferIndex;
	int 	 state;
	char 	 ch;
	OTResult bytesReceived;
	OTFlags  junkFlags;
	
	MPLogPrintf(">ReadHTTPRequest");
	
	assert(worker != NULL);
	assert(buffer != NULL);
	
	bufferIndex = 0;
	state = kWorkerWaitingForCR1;
	do {	
		bytesReceived = OTMPXRcv(worker, &ch, sizeof(char), &junkFlags);
		if (bytesReceived >= 0) {
			assert(bytesReceived == sizeof(char));
			
			err = noErr;

			// Put the character into the buffer.
			
			buffer[bufferIndex] = ch;
			bufferIndex += 1;
			
			// Check that we still have space to include our null terminator.
			
			if (bufferIndex >= kTransferBufferSize) {
				err = -1;
			}
			
			// Do the magic state machine.  Note the use of
			// hardwired numbers for CR and LF.  This is correct
			// because the Internet standards say that these
			// numbers can't change.  I don't use \n and \r
			// because these values change between various C
			// compilers on the Mac.
			
			switch (ch) {
				case 0x0D:
					switch (state) {
						case kWorkerWaitingForCR1:
							state = kWorkerWaitingForLF1;
							break;
						case kWorkerWaitingForCR2:
							state = kWorkerWaitingForLF2;
							break;
						default:
							state = kWorkerWaitingForCR1;
							break;
					}
					break;
				case 0x0A:
					switch (state) {
						case kWorkerWaitingForLF1:
							state = kWorkerWaitingForCR2;
							break;
						case kWorkerWaitingForLF2:
							state = kWorkerDone;
							break;
						default:
							state = kWorkerWaitingForCR1;
							break;
					}
					break;
				default:
					state = kWorkerWaitingForCR1;
					break;
			}
		} else {
			err = bytesReceived;
		}
	} while ( err == noErr && state != kWorkerDone );

	if (err == noErr) {
		// Append the null terminator that turns the HTTP request into a C string.
		buffer[bufferIndex] = 0;
	}

	MPLogPrintf("<ReadHTTPRequest");

	return err;
}

/////////////////////////////////////////////////////////////////

static OSStatus CopyFileToEndpoint(const FSSpec *fileSpec, char *buffer, OTMPEndpointRef worker)
	// Copy the file denoted by fileSpec to the endpoint.  buffer is a
	// temporary buffer of size kTransferBufferSize.  Initially buffer
	// contains a C string that is the HTTP header to output.  After that,
	// the routine uses buffer as temporary storage.  We do this because
	// we want any errors opening the file to be noticed before we send
	// the header saying that the request went through successfully.
{
	OSStatus err;
	OSStatus junk;
	long bytesToSend;
	long bytesThisTime;
	short fileRefNum;
	
	MPLogPrintf(">CopyFileToEndpoint");

	err = FSpOpenDF(fileSpec, fsRdPerm, &fileRefNum);
	if (err == noErr) {
		err = GetEOF(fileRefNum, &bytesToSend);
		
		// Write the HTTP header out to the endpoint.
		
		if (err == noErr) {
			err = OTMPXSndQ(worker, buffer, OTStrLength(buffer));
		}
		
		// Copy the file in kTransferBufferSize chunks to the endpoint.
		
		while (err == noErr && bytesToSend > 0) {
			if (bytesToSend > kTransferBufferSize) {
				bytesThisTime = kTransferBufferSize;
			} else {
				bytesThisTime = bytesToSend;
			}
			err = FSReadQ(fileRefNum, bytesThisTime, buffer);
			if (err == noErr) {
				err = OTMPXSndQ(worker, buffer, bytesThisTime);
			}
			bytesToSend -= bytesThisTime;
		}
		
		// Clean up.
		junk = FSClose(fileRefNum);
		assert(junk == noErr);
	}
	
	MPLogPrintf("<CopyFileToEndpoint");

	return err;
}

/////////////////////////////////////////////////////////////////

static OSStatus WorkerThreadProc(void *param)
	// This routine is the starting routine for the worker thread.
	// The thread is responsible for reading an HTTP request from
	// an endpoint, processing the requesting and writing the results
	// back to the endpoint.
{
	OSStatus 	err;
	OSStatus 	junk;
	char *		buffer = NULL;
	char *		errStr;
	char 		fileName[256];
	char 		mimeType[256];
	FSSpec 		fileSpec;
	WorkerContextPtr context;
	ByteCount	fileNameLen;

	(void) OTAtomicAdd32(1, (SInt32 *) &gRunningThreads);
	
	MPLogPrintf(">WorkerThreadProc");

	context = (WorkerContextPtr) param;
	
	MPLogPrintf("WorkerThreadProc: Starting\n");
	assert(context != NULL);
	assert(context->worker != NULL);

	err = OTMPXPrepareThisTask();
	
	// Allocate the transfer buffer in the heap.
	
	if (err == noErr) {
		buffer = MPAllocateAligned(kTransferBufferSize, kMPAllocateDefaultAligned, kNilOptions);
		if (buffer == NULL) {
			err = kENOMEMErr;
		}
	}

	// Read the request into the transfer buffer.
	
	if (err == noErr) {
		err = ReadHTTPRequest(context->worker, buffer);
	}
	
	if (err == noErr) {

		// Get the requested file name (and its mimeType) from the
		// HTTP request in the transfer buffer.
		
		err = ExtractRequestedFileName(buffer, fileName, mimeType);
		
		if (err == noErr) {

			// Create the appropriate HTTP response in the buffer.
			
			sprintf(buffer, gHTTPHeader, mimeType);		// MP?

			// Copy the file (with preceding HTTP header) to the endpoint.

			fileNameLen = OTStrLength(fileName);
			assert(fileNameLen <= 255);
			BlockMoveData(&fileName[0], &fileName[1], fileNameLen);
			fileName[0] = fileNameLen;
			(void) FSMakeFSSpec(context->vRefNum, context->dirID, (UInt8 *) fileName, &fileSpec);
			err = CopyFileToEndpoint(&fileSpec, buffer, context->worker);
		}
		
		// Handle any errors by sending back an appropriate error header.
		
		if (err != noErr) {
			switch (err) {
				case fnfErr:
					errStr = "File Not Found";
					break;
				default:
					errStr = "Unknown Error";
					break;
			}
			sprintf(buffer, gDefaultOutputText, err, errStr);		// MP?
			err = OTMPXSndQ(context->worker, buffer, OTStrLength(buffer));
		}
	}
	
	// Shut down the endpoint and clean up the WorkerContext.
	
	if (err == noErr) {
		err = OrderlyDisconnect(context->worker);
	}

	UnregisterEndpoint(context->worker);
	junk = OTMPXCloseProvider(context->worker);
	assert(junk == noErr);
	
	MPFree(context);

	if (buffer != NULL) {
		MPFree(buffer);
	}

	MPLogPrintf("WorkerThreadProc: Stopping with final result %d.\n", err);

	OTMPXUnprepareThisTask();
		
	MPLogPrintf(">WorkerThreadProc");

	(void) OTAtomicAdd32(-1, (SInt32 *) &gRunningThreads);
	assert(gRunningThreads >= 0);
	
	return noErr;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Listener Thread Stuff *****

extern pascal OSStatus RunHTTPServerMP(InetHost ipAddr, short vRefNum, long dirID)
	// This routine runs an HTTP server.  It doesn't return until
	// someone cancels synchronous calls on its listening endpoint,
	// which it registers with the endpoint registry (see 
	// TermOTMPSimpleServerHTTP for details).
	//
	// ipAddr is the IP address that the server listens on.  Specify 
	// kOTAnyInetAddress to listen on all IP addresses on the machine; 
	// specify an IP address to listen on a specific address.  vRefNum and 
	// dirID point to the root directory of the HTTP information to be served.
	//
	// The routine creates a listening endpoint and listens for connection 
	// requests on that endpoint.  When a connection request arrives, it creates 
	// a new worker thread (with accompanying endpoint) and accepts the connection
	// on that thread.
	//
	// Note the use of the "tilisten" module which prevents multiple
	// simultaneous T_LISTEN events coming from the transport provider,
	// thereby greatly simplifying the listen/accept sequence.
{
	OSStatus 		err;
	OTMPEndpointRef listener;
	TBind 			bindReq;
	InetAddress 	ipAddress;
	InetAddress 	remoteIPAddress;
	TCall 			call;
	MPTaskID 		workerThread;
	OSStatus 		junk;
	WorkerContextPtr workerContext;
	char			buf[128];

	(void) OTAtomicAdd32(1, (SInt32 *) &gRunningThreads);
	
	// display IP address in String
	OTInetHostToString(ipAddr, buf);
	MPLogPrintf("HTTP Server on <%s> Starting.\n", buf);

	listener = NULL;
	
	err = OTMPXPrepareThisTask();
	
	// Create the listener endpoint.
	
	if (err == noErr) {
		err = OpenAndRegisterEndpoint("tilisten,tcp", &listener);
	}

	// In order for the IP address to be re-used after quitting this sample program and
	// restarting it, the IP_REUSEADDR option must be set.

	if (err == noErr) {
		err = SetFourByteOption(listener, INET_IP, kIP_REUSEADDR, 1);
	}	

	// Set the endpoint mode and bind it to the appropriate IP address.
	
	if (err == noErr) {
		OTInitInetAddress(&ipAddress, 80, ipAddr);	// port & host ip
		bindReq.addr.buf = (UInt8 *) &ipAddress;
		bindReq.addr.len = sizeof(ipAddress);
		bindReq.qlen = 1;
		err = OTMPXBind(listener, &bindReq, NULL);
	}
	
	MPLog(kOTMPSimpleServerHTTPListenLoopLogID, '>SLL');
	
	while (err == noErr) {
		Boolean listenPending;

		// Listen for connection attempts...
		
		listenPending = false;
		
		OTMemzero(&call, sizeof(TCall));
		call.addr.buf = (UInt8 *) &remoteIPAddress;
		call.addr.maxlen = sizeof(remoteIPAddress);

		err = OTMPXListen(listener, &call);
		MPLog2(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLl', (void *) err, (void *) call.sequence);

		// ... then spool a worker thread for this connection.
		
		if (err == noErr) {
			listenPending = true;
		
			// Create the worker context.
		
			workerThread = kInvalidID;
			workerContext = MPAllocateAligned(sizeof(*workerContext), kMPAllocateDefaultAligned, kMPAllocateClearMask);
			if (workerContext == NULL) {
				err = kENOMEMErr;
			} else {
				workerContext->worker = NULL;
				workerContext->vRefNum = vRefNum;
				workerContext->dirID = dirID;
			}
			MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLm', (void *) err);
			
			// Open the worker endpoint.
			
			if (err == noErr) {
				err = OpenAndRegisterEndpoint("tcp", &workerContext->worker);
				MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLo', (void *) err);
			}
			
			// Accept the connection.  OTMPXAcceptQ handles the case where the connection 
			// disappears before we get to accept it and returns kOTNoDataErr.  We know 
			// that we only have to handle one of these because "tilisten" only allows 
			// one connection to come up at a time.
			
			if (err == noErr) {
				err = OTMPXAcceptQ(listener, workerContext->worker, &call);
				listenPending = (err != noErr) && (err != kOTNoDataErr);
			}

			// Create the worker thread.

			if (err == noErr) {
				err = MPCreateTask(WorkerThreadProc, workerContext, 65536, kInvalidID, NULL, NULL, kNilOptions, &workerThread);
				MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLc', (void *) err);
			}
			
			// Clean up on error.
			
			if (err != noErr) {
				// As we create the thread last, we don't need to worry about disposing 
				// it if we get an error.
				if (workerContext != NULL) {
					if (workerContext->worker != NULL) {
						UnregisterEndpoint(workerContext->worker);
						junk = OTMPXCloseProvider(workerContext->worker);
						assert(junk == noErr);
					}
					MPFree(workerContext);
				}
				err = noErr;
			}
		}
		
		// If we got a call but we didn't accept it for any reason, 
		// force a disconnect to remove it from the listener.
		
		if ( listenPending ) {
			junk = OTMPXSndDisconnect(listener, &call);
			assert(junk == noErr);
			MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, 'SLLd', (void *) junk);
		}
	}

	MPLog1(kOTMPSimpleServerHTTPListenLoopLogID, '<SLL', (void *) err);
	
	// Clean up the listener endpoint.
	
	if (listener != NULL) {
		UnregisterEndpoint(listener);
		junk = OTMPXCloseProvider(listener);
		assert(junk == noErr);
	}

	MPLogPrintf("HTTP Server on %08x: Stopping (%ld).\n", ipAddr, err);
	
	OTMPXUnprepareThisTask();

	(void) OTAtomicAdd32(-1, (SInt32 *) &gRunningThreads);
	assert(gRunningThreads >= 0);

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Endpoint Registry *****

// Preemptive threading poses an interesting problem when you need to 
// quit your application.  The network threads are blocked waiting for 
// networking activity (listeners are blocked waiting for connections, 
// workers are block waiting to receive or send data), so they can't 
// poll for a "gQuitNow" Boolean.  We have to come up with some way 
// to unblock synchronous network threads.
//
// One possible way to do this is to use MPTerminateTask.  I don't like 
// this approach because it stops the task dead: the task does not have 
// an opportunity to clean up its own data structures, such as the 
// endpoint on which it's listening.  If you use this approach you can't 
// shut down the HTTP server and then restart it, because the shutdown 
// has left a dangling listener endpoint bound to port 80.
// 
// The approach I chose was to register all endpoints with a central 
// endpoint registry.  When I want to quit, I call 
// OTMPCancelSynchronousCalls on each endpoint in the registry.  If 
// a thread is blocked waiting for the network, it will unblock with 
// an error.  If the task isn't blocked on the network, it will get 
// the error the next time it tries to use the endpoint.  Either way, 
// the task handles the error and shuts down cleanly.
//
// This approach is pretty cool, although it does require that my MP 
// threads not block in places other than endpoints.  For example, if 
// an MP thread blocks a long time waiting for a File Manager request, 
// the shutdown code will have to wait for that time before the server 
// shuts down.
//
// The endpoint registry is a global database, so I protect it with a 
// critical section (mutex).

static MPCriticalRegionID gSyncEndpointRegistry;

// The endpoit registry is a linked list of EndpointEntry structures.
// Each is allocated with the MP memory allocator.

enum {
	kEndpointEntryMagic = 'Entr'
};

struct EndpointEntry {
	OTLink 			fLink;
	OSStatus		magic;			// must be kEndpointEntryMagic
	OTMPEndpointRef mpEP;
};
typedef struct EndpointEntry EndpointEntry;

static OTList gEndpointRegistry;			// of EndpointEntry

static OSStatus RegisterEndpoint(OTMPEndpointRef mpEP)
	// This routine adds the mpEP to the registry.
{
	OSStatus       	err;
	OSStatus		junk;
	EndpointEntry *	entry;

	// Allocate an endpoint tracking entry.  Do this before entering 
	// the critical section so that we hold the critical section for the 
	// minimum amount of time.
	
	err = noErr;
	entry = MPAllocateAligned(sizeof(*entry), kMPAllocateDefaultAligned, kMPAllocateClearMask);
	if (entry == NULL) {
		err = memFullErr;
	}
	
	// Inside a critical region, record that entry in our registry.
	// Nil out the pointer if we succeed so that the clean up code doesn't 
	// dispose it.
	
	if (err == noErr) {
		entry->magic = kEndpointEntryMagic;
		entry->mpEP  = mpEP;
		err = MPEnterCriticalRegion(gSyncEndpointRegistry, kDurationForever);
		if (err == noErr) {
			
			OTAddFirst(&gEndpointRegistry, &entry->fLink);
			entry = NULL;
			
			junk = MPExitCriticalRegion(gSyncEndpointRegistry);
			assert(junk == noErr);
		}
	}
	
	// Clean up.
	
	if (err != noErr && entry != NULL) {
		MPFree(entry);
	}
	return err;
}

static Boolean RegistrySearch(const void *ref, OTLink *linkToCheck)
	// This routine is a callback for OTFindAndRemoveLink that is 
	// used to find an endpoint entry so that we can remove it.
{
	EndpointEntry *thisEntry;
	
	thisEntry = OTGetLinkObject(linkToCheck, EndpointEntry, fLink);
	assert(thisEntry->magic == kEndpointEntryMagic);

	return (thisEntry->mpEP == ref);
}

static OTListSearchUPP gRegistrySearchUPP;		// -> RegistrySearch

static void     UnregisterEndpoint(OTMPEndpointRef mpEP)
	// Remove mpEP from the endpoint registry.
{
	OSStatus 		err;
	OSStatus 		junk;
	EndpointEntry *	foundEntry;
	
	err = MPEnterCriticalRegion(gSyncEndpointRegistry, kDurationForever);
	if (err == noErr) {
		foundEntry = OTGetLinkObject( OTFindAndRemoveLink(&gEndpointRegistry, gRegistrySearchUPP, mpEP) , EndpointEntry, fLink);
		assert(foundEntry != NULL && foundEntry->magic == kEndpointEntryMagic);
		
		junk = MPExitCriticalRegion(gSyncEndpointRegistry);
		assert(junk == noErr);
		
		// Free'ing foundEntry might take significant time, so we do it 
		// outside of the critical section.

		if (foundEntry != NULL) {
			MPFree(foundEntry);
		}
	}
}

static void CancelRegisteredEndpoints(void)
	// Walks the list of endpoints in the registry and cancels each one. 
	// Any threads waiting for a synchronous network request will unblock 
	// with an error, which they will handle appropriately.  Threads that 
	// aren't waiting for a network request will get the error the next 
	// time they do an operation on their endpoint.
{
	OSStatus 		junk;
	EndpointEntry *	thisEntry;

	while ( gEndpointRegistry.fHead != NULL ) {
		thisEntry = OTGetLinkObject( OTRemoveFirst(&gEndpointRegistry), EndpointEntry, fLink);
		assert(thisEntry->magic == kEndpointEntryMagic);
		
		junk = OTMPXCancelSynchronousCalls(thisEntry->mpEP, kOTCanceledErr);
		assert(junk == noErr);
		
		MPFree(thisEntry);
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Init/Term *****

extern pascal OSStatus InitOTMPSimpleServerHTTP(void)
	// See comments in header file.
{
	OSStatus err;

	gEndpointRegistry.fHead = NULL;
	gRunningThreads	= 0;
	
	err = noErr;
	gRegistrySearchUPP = NewOTListSearchUPP(RegistrySearch);
	if (gRegistrySearchUPP == NULL) {
		err = memFullErr;
	}
	if (err == noErr) {
		err = MPCreateCriticalRegion(&gSyncEndpointRegistry);
	}
	
	return err;
}

extern pascal void     TermOTMPSimpleServerHTTP(void)
	// See comments in header file.
{
	OSStatus 		err;
	OSStatus 		junk;
	UInt32			lastPrinted;
	MPCriticalRegionID tmpID;
	
	assert(gSyncEndpointRegistry != kInvalidID);

	// First grab the endpoint registry critical section.  Because we're 
	// running in the main task we must poll for it rather than wait.
		
	do {
		err = MPEnterCriticalRegion(gSyncEndpointRegistry, kDurationImmediate);
		if (err == kMPTimeoutErr) {
			printf("+");
			fflush(stdout);
		}
	} while (err == kMPTimeoutErr);
	assert(err == noErr);

	// Now delete the critical section.  This will cause anyone waiting to 
	// unblock  with a "deleted" error.  Anyone not yet waiting will try to enter 
	// the mutex and fail because gSyncEndpointRegistry is kInvalidID.
	// Hmmm, there is a race condition here, where an MP task can sample 
	// gSyncEndpointRegistry, then yield to us, and we delete gSyncEndpointRegistry.
	// This window is small, so I rely an the fact that MP kernel IDs (like
	// MPCriticalRegionID) are not reused quickly).
	
	tmpID = gSyncEndpointRegistry;
	gSyncEndpointRegistry = kInvalidID;
	junk = MPDeleteCriticalRegion(tmpID);
	assert(junk == noErr);

	// Cancel any synchronous network operations, unblocking their 
	// threads.
	
	CancelRegisteredEndpoints();
	
	// Now we just wait for all the threads to terminate.  Note that 
	// gRunningThreads is declared volatile to counteract any optimisations 
	// that the compiler might be doing.

	lastPrinted = TickCount();
	while ( gRunningThreads != 0 ) {
		if ( TickCount() > (lastPrinted + 10) ) {
			printf(".");
			fflush(stdout);
			lastPrinted = TickCount();
		}
	}

	if (gRegistrySearchUPP != NULL) {
		DisposeOTListSearchUPP(gRegistrySearchUPP);
	}
}
