/*
	File:		OTMPTest.c

	Contains:	Test application for the OTMP library.

	Written by:	Quinn

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

$Log: OTMPTest.c,v $
Revision 1.6  2002/11/08 23:45:25         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2001/11/07 15:57:16         
Tidy up headers, add CVS logs, update copyright.


         <4>      9/7/01    Quinn   Eliminated a bunch of MPLogPrintfSlow calls because I'm having
                                    problems with that routine.
         <3>      5/7/01    Quinn   Changes required for a new "connect" test.
         <2>      8/2/01    Quinn   Added code to test OTMPGetMessage and OTMPPutMessage.
         <1>     7/11/00    Quinn   First checked in.
*/

#include "MoreSetup.h"

#include "OTMP.h"
#include "MoreMPLog.h"

#include <Multiprocessing.h>
#include <Events.h>
#include <OpenTransportProtocol.h>
#include <OpenTransportProviders.h>
#include <DriverServices.h>
#include <Folders.h>

#include <stdio.h>
#include <stdlib.h>

static OSStatus OrderlyDisconnect(OTMPEndpointRef ep)
	// Gosh XTI is lame.  RcvOrderlyDisconnect is non-blocking 
	// (it doesn't wait for the T_ORDREL event) so we have to 
	// block in a Rcv call.
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

static pascal OSStatus MyMainThreadYielder(void)
{
	OSStatus err;
	static UInt32 gTimeLastPrinted;
	
	if ( TickCount() > (gTimeLastPrinted + 30) ) {
		printf(".");
		fflush(stdout);
		gTimeLastPrinted = TickCount();
	}

	#if TARGET_API_MAC_CARBON
		{
			err = noErr;
			if (GetCurrentKeyModifiers() & alphaLock) {
				err = kOTCanceledErr;
			}
		}
	#else
		{
			EventRecord event;
			
			err = noErr;
			(void) OSEventAvail(0, &event);
			if (event.modifiers & alphaLock) {
				err = kOTCanceledErr;
			}
		}
	#endif
	return err;
}

static pascal OSStatus IoctlTest(OTMPEndpointRef ep)
{
	OSStatus err;
	str_mlist moduleList[10];
	str_list  iList;
	
	iList.sl_nmods = 10;
	iList.sl_modlist = moduleList;
	
	err = OTMPXIoctl(ep, I_LIST, &iList);
	if (err == noErr) {
		SInt32 i;
		
		printf("\n");
		printf("Module List\n");
		printf("-----------\n");
		for (i = 0; i < iList.sl_nmods; i++) {
			printf("  %ld = “%s”\n", i, &moduleList[i]);
		}
	}
	return err;
}

static pascal OSStatus DoOutgoingBind(OTMPEndpointRef ep)
{
	OSStatus err;
	TBind ret;
	InetAddress boundAddr;
	
	OTMemzero(&ret, sizeof(ret));
	ret.addr.maxlen = sizeof(boundAddr);
	ret.addr.buf    = (UInt8 *) &boundAddr;
	
	err = OTMPXBind(ep, NULL, &ret);
	if (err == noErr) {
		char addrAsStr[32];
		
		OTInetHostToString(boundAddr.fHost, addrAsStr);
		printf("\n");
		printf("boundAddr = %s:%d\n", addrAsStr, boundAddr.fPort);
	}
	return err;
}

static pascal OSStatus DoIncomingBind(OTMPEndpointRef ep, InetPort portNum, OTQLen qlen)
{
	OSStatus err;
	TBind req;
	TBind ret;
	InetAddress reqAddr;
	InetAddress retAddr;

	OTInitInetAddress(&reqAddr, portNum, kOTAnyInetAddress);
	OTMemzero(&req, sizeof(req));
	req.addr.len    = sizeof(reqAddr);
	req.addr.buf    = (UInt8 *) &reqAddr;
	req.qlen        = qlen;
		
	OTMemzero(&ret, sizeof(ret));
	ret.addr.maxlen = sizeof(retAddr);
	ret.addr.buf    = (UInt8 *) &retAddr;
	
	err = OTMPXBind(ep, &req, &ret);
	if (err == noErr) {
		char addrAsStr[32];
		
		OTInetHostToString(retAddr.fHost, addrAsStr);
		MPLogPrintf("\n");
		MPLogPrintf("Bound to = %s:%d with qlen of %d\n", addrAsStr, retAddr.fPort, ret.qlen);
	}
	return err;
}

static OTResult SetFourByteOption(OTMPEndpointRef ep, OTXTILevel level, OTXTIName name, UInt32 value)
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
	return (err);
}

static OSStatus BasicEchoTest(OTMPEndpointRef ep)
{
	OSStatus err;
	OSStatus err2;
	
	err = DoOutgoingBind(ep);
	
	if (err == noErr) {
		InetAddress sndAddr;
		InetAddress rcvAddr;
		TCall sndCall;
		TCall rcvCall;
		
		OTInitInetAddress(&sndAddr, 7, 0x7f000001);		// localhost addr
		OTMemzero(&rcvAddr, sizeof(rcvAddr));

		OTMemzero(&sndCall, sizeof(sndCall));
		sndCall.addr.buf = (UInt8 *) &sndAddr;
		sndCall.addr.len = sizeof(sndAddr);
		
		OTMemzero(&rcvCall, sizeof(rcvCall));
		rcvCall.addr.buf    = (UInt8 *) &rcvAddr;
		rcvCall.addr.maxlen = sizeof(rcvAddr);
		
		err = OTMPXConnect(ep, &sndCall, &rcvCall);
		if (err == kOTLookErr) {
			OTResult look;
			
			look = OTMPXLook(ep);
			if (look == T_DISCONNECT) {
				TDiscon discon;
				
				OTMemzero(&discon, sizeof(discon));
				err = OTMPXRcvDisconnect(ep, &discon);
				if (err == noErr) {
					printf("\n");
					printf("Connect failed with reason %ld.\n", discon.reason);
				}
			}
		} else if (err == noErr) {
			err = OTMPXSnd(ep, "Hello Cruel World!\x0d\x0a", 20, 0);
			if (err == 20) {
				err = noErr;
			}
			if (err == noErr) {
				OTFlags flags;
				char rcvBuf[100];
				
				OTMemzero(rcvBuf, sizeof(rcvBuf));
				flags = 0;
				err = OTMPXRcv(ep, rcvBuf, sizeof(rcvBuf), &flags);
				if (err == 20) {
					err = noErr;
				}
			}
		
			err2 = OTMPXSndDisconnect(ep, NULL);
			if (err == noErr) {
				err = err2;
			}
		}
	}
	
	if (err == noErr) {
		err = OTMPXUnbind(ep);
	}

	return err;
}

static OSStatus EchoDisconnectTest(OTMPEndpointRef ep)
{
	OSStatus err;
	
	err = DoOutgoingBind(ep);
	
	if (err == noErr) {
		InetAddress sndAddr;
		InetAddress rcvAddr;
		TCall sndCall;
		TCall rcvCall;
		
		OTInitInetAddress(&sndAddr, 7, 0x7f000001);		// localhost addr
		OTMemzero(&rcvAddr, sizeof(rcvAddr));

		OTMemzero(&sndCall, sizeof(sndCall));
		sndCall.addr.buf = (UInt8 *) &sndAddr;
		sndCall.addr.len = sizeof(sndAddr);
		
		OTMemzero(&rcvCall, sizeof(rcvCall));
		rcvCall.addr.buf    = (UInt8 *) &rcvAddr;
		rcvCall.addr.maxlen = sizeof(rcvAddr);
		
		err = OTMPXConnect(ep, &sndCall, &rcvCall);
		if (err == noErr) {
			OTFlags flags;
			char rcvBuf[100];
			
			printf("\n");
			printf("Now quit Mac TCP Watcher.\n");
			
			OTMemzero(rcvBuf, sizeof(rcvBuf));
			flags = 0;
			err = OTMPXRcv(ep, rcvBuf, sizeof(rcvBuf), &flags);
			if (err == kOTLookErr) {
				OTResult look;

				look = OTMPXLook(ep);
				if (look == T_DISCONNECT) {
					TDiscon discon;
					
					OTMemzero(&discon, sizeof(discon));
					err = OTMPXRcvDisconnect(ep, &discon);
					if (err == noErr) {
						printf("\n");
						printf("Received disconnected with reason %ld.\n", discon.reason);
					}
				} else {
					printf("\n");
					printf("look = %ld\n", look);
				}
			}
		}
	}
	
	if (err == noErr) {
		err = OTMPXUnbind(ep);
	}

	return err;
}

static OSStatus ListenTest(OTMPEndpointRef ep)
{
	OSStatus err;
	OSStatus junk;
	TCall call;
	InetAddress remoteAddr;
	OTMPEndpointRef worker;
	
	worker = NULL;
	err = SetFourByteOption(ep, INET_IP, IP_REUSEADDR, 0);
	if (err == noErr) {
		err = DoIncomingBind(ep, 1024, 10);
	}
	if (err == noErr) {
		OTMemzero(&remoteAddr, sizeof(remoteAddr));
		OTMemzero(&call, sizeof(call));
		
		call.addr.buf    = (UInt8 *) &remoteAddr;
		call.addr.maxlen = sizeof(remoteAddr);
		
		err = OTMPXListen(ep, &call);
		if (err == noErr) {
			char addrAsStr[100];
			
			OTInetHostToString(remoteAddr.fHost, addrAsStr);
			printf("\n");
			printf("Call from %s:%d\n", addrAsStr, remoteAddr.fPort);
			
			worker = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
		}
		if (err == noErr) {
			err = OTMPXAccept(ep, worker, &call);
		}
	}
	if (err == noErr) {
		err = OTMPXSndDisconnect(worker, NULL);
	}
	if (err == noErr) {
		err = OTMPXUnbind(ep);
	}
	if (worker != NULL) {
		junk = OTMPXCloseProvider(worker);
		assert(junk == noErr);
	}
	return err;
}

static OSStatus MiscEndpointTest(OTMPEndpointRef ep)
{
	OSStatus err;
	OSStatus junk;
	TEndpointInfo info;
	char addrAsStr[100];
	
	err = OTMPXGetEndpointInfo(ep, &info);
	if (err == noErr) {
		printf("addr     = %ld\n", info.addr);
		printf("options  = %ld\n", info.options);
		printf("tsdu     = %ld\n", info.tsdu);
		printf("etsdu    = %ld\n", info.etsdu);
		printf("connect  = %ld\n", info.connect);
		printf("discon   = %ld\n", info.discon);
		printf("servtype = %ld\n", info.servtype);
		printf("flags    = %ld\n", info.flags);
	}
	if (err == noErr) {
		err = OTMPXGetEndpointState(ep);
		if (err >= noErr) {
			printf("state1 = %ld\n", err);
			err = noErr;
		}
	}
	if (err == noErr) {
		err = OTMPXBind(ep, NULL, NULL);
	}
	if (err == noErr) {
		err = OTMPXGetEndpointState(ep);
		if (err >= noErr) {
			printf("state2 = %ld\n", err);
			err = noErr;
		}
		if (err == noErr) {
			TBind local;
			InetAddress localAddr;
			
			OTMemzero(&localAddr, sizeof(localAddr));
			OTMemzero(&local, sizeof(local));
			local.addr.maxlen = sizeof(localAddr);
			local.addr.buf    = (UInt8 *) &localAddr;
			err = OTMPXGetProtAddress(ep, &local, NULL);
			if (err == noErr) {
				OTInetHostToString(localAddr.fHost, addrAsStr);
				printf("localAddr = %s:%d\n", addrAsStr, localAddr.fPort);
			}
		}
		if (err == noErr) {
			TBind dns;
			TBind inet;
			DNSAddress  dnsAddr;
			InetAddress inetAddr;
			
			OTMemzero(&dns, sizeof(dns));
			OTMemzero(&inet, sizeof(inet));
			OTMemzero(&inetAddr, sizeof(inetAddr));
			
			dns.addr.len = OTInitDNSAddress(&dnsAddr, "1.2.3.4:80");
			dns.addr.buf = (UInt8 *) &dnsAddr;
			
			inet.addr.maxlen = sizeof(inetAddr);
			inet.addr.buf    = (UInt8 *) &inetAddr;
			
			err = OTMPXResolveAddress(ep, &dns, &inet, 1000);
			if (err == noErr) {
				OTInetHostToString(inetAddr.fHost, addrAsStr);
				printf("inetAddr = %s:%d\n", addrAsStr, inetAddr.fPort);
			}
		}
		junk = OTMPXUnbind(ep);
		assert(junk == noErr);
	}
	if (err == noErr) {
		OTByteCount byteCount;

		byteCount = 666;
		err = OTMPXCountDataBytes(ep, &byteCount);
		assert(err == kOTNoDataErr);
		assert(byteCount == 0);
		if (err == kOTNoDataErr) {
			printf("byteCount = %ld\n", byteCount);
			err = noErr;
		} else {
			err = -1;
		}
	}

	return err;
}

static OSStatus UnitDataTest(void)
{
	OSStatus err;
	OTMPEndpointRef ep;
	TUnitData udata;
	const char *message = "Hello Cruel World!\x0d\x0a";
	
	ep = OTMPXOpenEndpointQInContext("udp", 0, NULL, &err, NULL);
	if (err == noErr) {
		err = OTMPXBind(ep, NULL, NULL);
	}
	if (err == noErr) {
		InetAddress dest;
		
		OTMemzero(&udata, sizeof(udata));
		OTInitInetAddress(&dest, 7, 0x01020304);
		udata.addr.len = sizeof(dest);
		udata.addr.buf = (UInt8 *) &dest;
		udata.udata.len = OTStrLength(message);
		udata.udata.buf = (UInt8 *) message;
		err = OTMPXSndUData(ep, &udata);
	}
	if (err == noErr) {
		char buffer[256];
		InetAddress src;
		OTFlags junkFlags;
		
		OTMemzero(&udata, sizeof(udata));
		OTMemzero(&src, sizeof(src));
		udata.addr.maxlen  = sizeof(src);
		udata.addr.buf     = (UInt8 *) &src;
		udata.udata.maxlen = sizeof(buffer);
		udata.udata.buf    = (UInt8 *) buffer;
		
		err = OTMPXRcvUData(ep, &udata, &junkFlags);
		if (err == noErr) {
			if ( udata.udata.len != OTStrLength(message) ||
					! OTMemcmp(buffer, message, udata.udata.len) ) {
				err = -1;
			}
		} else if (err == kOTLookErr) {
			OTResult look;

			// If echo server isn't running, previous send will generate 
			// an ICMP port unreachable, which the above RcvUData will return 
			// as a look error and which we consume here.
			
			look = OTMPXLook(ep);
			if (look == T_UDERR) {
				TUDErr   uderr;
				
				OTMemzero(&uderr, sizeof(uderr));
				OTMemzero(&src, sizeof(src));
				uderr.addr.maxlen = sizeof(src);
				uderr.addr.buf    = (UInt8 *) &src;
				uderr.opt.maxlen  = sizeof(buffer);
				uderr.opt.buf     = (UInt8 *) buffer;
				
				err = OTMPXRcvUDErr(ep, &uderr);
				if (err == noErr) {
					printf("uderr src = %08xl\n", src.fHost);
					printf("uderr error = %ld\n", uderr.error);
				}
			}
		}
	}
	if (err == noErr) {
		err = OTMPXUnbind(ep);
	}
	
	if (ep != NULL) {
		OTMPXCloseProvider(ep);
	}
	
	return err;
}

static void BasicDebuggingTests(void)
{
	OSStatus err;
	OSStatus junk;
	OTMPEndpointRef ep;

	InstallOTMPMainThreadYielder(MyMainThreadYielder);
	RunOTMPAsSystemTasks(true);

	ep = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
	if (err == noErr) {
		err = IoctlTest(ep);
		if (err == noErr) {
			// err = BasicEchoTest(ep);
		}
		if (err == noErr) {
			// err = EchoDisconnectTest(ep);
		}
		if (err == noErr) {
			// err = ListenTest(ep);
		}
		if (err == noErr) {
			// err = MiscEndpointTest(ep);
		}
		if (err == noErr) {
			// err = UnitDataTest();
		}
	
		junk = OTMPXCloseProvider(ep);
		assert(junk == noErr);
	}

	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}

	InstallOTMPMainThreadYielder(NULL);
	RunOTMPAsSystemTasks(false);
}

static void NestedInitTest(void)	
	// Test nested inits.
{
	OSStatus err;
	OTClientContextPtr thisContext;

	err = InitOpenTransportMPInContext(kInitOTForExtensionMask, &thisContext);
	if (err == noErr) {
		CloseOpenTransportMPInContext(thisContext);
	}
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static UInt8 gJunkBuffer[1024];

static volatile Boolean gRcvStarted;

static volatile OTMPEndpointRef gLookerEP;

static volatile Boolean gQuitLooker;

static void RcvSleep()
{
	OSStatus 		junk;
	AbsoluteTime 	timeToWakeUp;

	MPLogPrintf("SFCRcv: Going to sleep for 1 seconds");
	timeToWakeUp = AddDurationToAbsolute(1000, UpTime());
	junk = MPDelayUntil(&timeToWakeUp);
	assert(junk == noErr);
	MPLogPrintf("SFCRcv: Waking up\n");
}

static OSStatus SFCRcv(void *parameter)
{
	#pragma unused(parameter)
	OSStatus err;
	OSStatus junk;
	OTMPEndpointRef ep;
	TCall call;
	OTFlags junkFlags;
	UInt32  sleepCounter;

	ep = NULL;
	
	err = OTMPXPrepareThisTask();
	if (err == noErr) {
		ep = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
	}
	if (err == noErr) {
		err = SetFourByteOption(ep, INET_IP, IP_REUSEADDR, 1);
	}
	if (err == noErr) {
		err = DoIncomingBind(ep, 1025, 1);
	}
	gRcvStarted = true;
	if (err == noErr) {
		OTMemzero(&call, sizeof(call));
		err = OTMPXListen(ep, &call);
	}
	if (err == noErr) {
		err = OTMPXAccept(ep, ep, &call);
	}
	if (err == noErr) {
		sleepCounter = 0;
		do {
			if (sleepCounter == 0) {
				RcvSleep();
			}
			err = OTMPXRcv(ep, gJunkBuffer, sizeof(gJunkBuffer), &junkFlags);
			if (err > noErr) {
				err = noErr;
			}
			sleepCounter += 1;
			if (sleepCounter > 64) {
				sleepCounter = 0;
			}
		} while (err == noErr);
		if (err == kOTLookErr) {
			OTResult look;
			
			look = OTMPXLook(ep);
			switch (look) {
				case T_ORDREL:
					err = OTMPXRcvOrderlyDisconnect(ep);
					if (err == noErr) {
						err = OTMPXSndOrderlyDisconnect(ep);
					}
					break;
				case T_DISCONNECT:
					err = OTMPXRcvDisconnect(ep, NULL);
					break;
				default:
					MPLogPrintf("look = %08lx\n", look);
					break;
			}
		}
	}
	
	if (ep != NULL) {
		junk = OTMPXCloseProvider(ep);
		assert(junk == noErr);
	}
	
	OTMPXUnprepareThisTask();
	return err;
}

static OSStatus SFCSnd(void *parameter)
{
	#pragma unused(parameter)
	OSStatus err;
	OSStatus junk;
	OTMPEndpointRef ep;
	TCall call;
	InetAddress inet;
	OTByteCount bytesThisTime;
	OTByteCount bytesRemaining;

	ep = NULL;
	
	err = OTMPXPrepareThisTask();
	if (err == noErr) {
		ep = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
	}
	if (err == noErr) {
		gLookerEP = ep;
		err = OTMPXBind(ep, NULL, NULL);
	}
	if (err == noErr) {
		OTInitInetAddress(&inet, 1025, 0x7f000001);			// 127.0.0.1:1025
		OTMemzero(&call, sizeof(call));
		call.addr.buf = (UInt8 *) &inet;
		call.addr.len = sizeof(inet);
		err = OTMPXConnect(ep, &call, NULL);
	}
	if (err == noErr) {
		if (false) {
			bytesRemaining = 0x01000000;						// 16 MB
		} else {
			bytesRemaining = 0x00100000;						// 1 MB
		}
		do {
			if (bytesRemaining > sizeof(gJunkBuffer)) {
				bytesThisTime = sizeof(gJunkBuffer);
			} else {
				bytesThisTime = bytesRemaining;
			}
			err = OTMPXSnd(ep, gJunkBuffer, bytesThisTime, 0);
			if (err > noErr) {
				bytesRemaining -= err;
				err = noErr;
			}
		} while ( (err == noErr) && (bytesRemaining != 0) );
	}
	if (err == noErr) {
		err = OTMPXSndDisconnect(ep, NULL);
	}
	
	gLookerEP = NULL;
	gQuitLooker = true;
	if (ep != NULL) {
		junk = OTMPXCloseProvider(ep);
		assert(junk == noErr);
	}
	
	OTMPXUnprepareThisTask();
	return err;
}

static SInt32 gLookCounter;

static OSStatus SFCLooker(void *parameter)
{
	#pragma unused(parameter)
	OSStatus err;
	OSStatus junk;
	
	err = OTMPXPrepareThisTask();
	if (err == noErr) {
		while ( ! gQuitLooker ) {
			if ( gLookerEP != NULL ) {
				(void) OTAtomicAdd32(1, &gLookCounter);
				
				junk = OTMPXLook(gLookerEP);
				
				if (1) {
					AbsoluteTime timeToWakeUp;
					
					timeToWakeUp = AddDurationToAbsolute(-64, UpTime());
					junk = MPDelayUntil(&timeToWakeUp);
					assert(junk == noErr);
					
					// MPLogPrintfSlow("<look>");
				}
			}
		}
	}
	
	OTMPXUnprepareThisTask();
	return err;
}

static void SendFlowControlTest(void)
{
	OSStatus  err;
	OSStatus  junk;
	MPQueueID deathQueue;
	MPTaskID  rcvTask;
	MPTaskID  sndTask;

	gRcvStarted = false;
	gLookerEP = NULL;
	gQuitLooker = false;
	
	deathQueue = kInvalidID;
	err = MPCreateQueue(&deathQueue);
	if (err == noErr) {
		err = MPCreateTask(SFCRcv, NULL, 65536, deathQueue, (void *) 1, (void *) 666, kNilOptions, &rcvTask);
	}
	if (err == noErr) {
		MPLogPrintf("Waiting for receiver to start.\n");
		while ( ! gRcvStarted ) {
			printf(".");
			fflush(stdout);
		}
		MPLogPrintf("\n");
	}
	if (err == noErr) {
		err = MPCreateTask(SFCSnd, NULL, 65536, deathQueue, (void *) 2, (void *) 666, kNilOptions, &sndTask);
	}
	if (err == noErr) {
		err = MPCreateTask(SFCLooker, NULL, 65536, deathQueue, (void *) 3, (void *) 666, kNilOptions, &rcvTask);
	}
	if (err == noErr) {
		UInt32 terminatedTaskCount;
		UInt32 taskNumber;
		OSStatus taskStatus;
		UInt32 lastPrinted;
		
		lastPrinted = 0;
		terminatedTaskCount = 0;
		do {
			err = MPWaitOnQueue(deathQueue, (void **) &taskNumber, NULL, (void **) &taskStatus, kDurationImmediate);
			if (err == noErr) {
				MPLogPrintf("Task number %ld completed with status %ld.\n", taskNumber, taskStatus);
				terminatedTaskCount += 1;
			} else if (err == kMPTimeoutErr) {
				#if !TARGET_API_MAC_CARBON
					SystemTask();
				#endif
				if (TickCount() > (lastPrinted + 60)) {
					printf(".");
					fflush(stdout);
					lastPrinted = TickCount();
				}
				err = noErr;
			}
		} while ( (err == noErr) && (terminatedTaskCount < 3) );
	}
	
	// Clean up.
	
	if (deathQueue != kInvalidID) {
		junk = MPDeleteQueue(deathQueue);
		assert(junk == noErr);
	}

	printf("gLookCounter = %ld\n", gLookCounter);
	gLookCounter = 0;
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoWriteMPLog(void)
{
	OSStatus err;
	FSSpec fss;
	
	err = FindFolder(kOnSystemDisk, kDesktopFolderType, true, &fss.vRefNum, &fss.parID);
	if (err == noErr) {
		(void) FSMakeFSSpec(fss.vRefNum, fss.parID, "\pMPLog", &fss);
		err = MPLogWriteToFile(&fss);
	}

	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static OSStatus EchoWorker(void *parameter)
{
	OSStatus err;
	OSStatus junk;
	OTMPEndpointRef worker;
	char buffer[1024];			// cheesy, but we do have 64 KB of stack for this thread (-:
	
	worker = (OTMPEndpointRef) parameter;

	err = OTMPXPrepareThisTask();
	if (err == noErr) {
	
		// Echo incoming data back out to the client.
		
		do {
			OTFlags junkFlags;
			
			err = OTMPXRcv(worker, buffer, sizeof(buffer), &junkFlags);
			if (err > noErr) {
				err = OTMPXSnd(worker, buffer, err, 0);
				if (err > noErr) {
					err = noErr;
				}
			}
		} while (err == noErr);

		// The above terminates when the client disconnects.  We 
		// are notified by a kOTLookErr, which we handle below.
		
		if (err == kOTLookErr) {
			OTResult look;
			
			look = OTMPXLook(worker);
			switch (look) {
				case T_DISCONNECT:
					err = OTMPXRcvDisconnect(worker, NULL);
					break;
				case T_ORDREL:
					err = OTMPXSndOrderlyDisconnect(worker);
					break;
				default:
					printf("look = %08lx\n", look);
					// leave err set to kOTLookErr
					break;
			}
		}
	}
	
	// Clean up.
	
	junk = OTMPXCloseProvider(worker);
	assert(junk == noErr);
	OTMPXUnprepareThisTask();
	
	return err;
}

static volatile Boolean gEchoListenerStarted;
static MPQueueID gEchoDeathQueue;
static SInt32 gEchoWorkerNumber;

static OSStatus EchoListener(void *parameter)
{
	#pragma unused(parameter)
	OSStatus err;
	OSStatus junk;
	TCall call;
	OTMPEndpointRef listener;

	listener = NULL;
	
	err = OTMPXPrepareThisTask();
	if (err == noErr) {
		listener = OTMPXOpenEndpointQInContext("tilisten,tcp", 0, NULL, &err, NULL);
	}
	if (err == noErr) {
		err = SetFourByteOption(listener, INET_IP, IP_REUSEADDR, 1);
	}
	if (err == noErr) {
		err = DoIncomingBind(listener, 1026, 10);
	}
	gEchoListenerStarted = true;
	if (err == noErr) {
		do {
			Boolean listenPending;
			OTMPEndpointRef worker;
			MPTaskID junkTaskID;
			
			worker = NULL;
			
			OTMemzero(&call, sizeof(call));
			err = OTMPXListen(listener, &call);
			listenPending = (err == noErr);
			
			if (err == noErr) {
				worker = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
			}
			if (err == noErr) {
				err = OTMPXAccept(listener, worker, &call);
				listenPending = (err != noErr);
			}
			if (err == noErr) {
				err = MPCreateTask(EchoWorker, worker, 65536, gEchoDeathQueue, (void *) OTAtomicAdd32(1, &gEchoWorkerNumber), NULL, kNilOptions, &junkTaskID);
				if (err == noErr) {
					worker = NULL;
				} else {
					// Typically we get here with a kMPInsufficientResourcesErr 
					// because we don't have the space to allocate the worker's 
					// stack.  If this happens, don't kill the server entirely.
					// Instead, swallow the error, which will cause this thread 
					// to close "worker" and all will be well.
					err = noErr;
				}
			}
			
			// Clean up.
			
			if (listenPending) {
				junk = OTMPXSndDisconnect(listener, &call);
				assert(junk == noErr);
			}
			if (worker != NULL) {
				junk = OTMPXCloseProvider(worker);
				assert(junk == noErr);
			}
		} while (err == noErr);
	}
	
	// Clean up.
	
	if (listener != NULL) {
		junk = OTMPXCloseProvider(listener);
		assert(junk == noErr);
	}
	
	OTMPXUnprepareThisTask();
	gEchoListenerStarted = false;
	return err;
}

static void StartEchoServer(void)
{
	OSStatus err;
	MPTaskID junkTaskID;
	
	err = noErr;
	if (gEchoDeathQueue != kInvalidID) {
		printf("Echo server is already running.\n");
		err = userCanceledErr;
	}
	if (err == noErr) {
		err = MPCreateQueue(&gEchoDeathQueue);
	}
	if (err == noErr) {
		err = MPCreateTask(EchoListener, NULL, 65536, gEchoDeathQueue, 0, NULL, kNilOptions, &junkTaskID);
	}

	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static void PrintEchoDeathInfo(void)
{
	OSStatus err;
	SInt32   taskNumber;
	OSStatus taskStatus;
	
	err = noErr;
	if (gEchoDeathQueue == kInvalidID) {
		printf("Echo server is not running.\n");
		err = userCanceledErr;
	}
	if (err == noErr) {
		do {
			err = MPWaitOnQueue(gEchoDeathQueue, (void **) &taskNumber, NULL, (void **) &taskStatus, kDurationImmediate);
			if (err == noErr) {
				printf("Task number %ld stopped with error %ld.\n", taskNumber, taskStatus);
			}
		} while (err == noErr);
		if (err == kMPTimeoutErr) {
			err = noErr;
		}
	}
	
	if (err != noErr) {
		printf("Failed with error %ld.\n", err);
	}
}

static void FillSendBuffer(UInt32 buf[], ItemCount numEntries)
{
	ItemCount i;
	
	for (i = 0; i < numEntries; i++) {
		buf[i] = i;
	}
}

static void CheckReceiveBuffer(UInt32 rcvBuf[], UInt32 sndBuf[], ItemCount numEntries)
{
	ItemCount i;
	
	for (i = 0; i < numEntries; i++) {
		assert(rcvBuf[i] == sndBuf[i]);
	}
}

#if TARGET_API_MAC_CARBON

EXTERN_API_C( UInt32 )
OTGetRandomSeed                 (void);
EXTERN_API_C( UInt32 )
OTGetRandomNumber               (UInt32 *               seed,
                                 UInt32                 lo,
                                 UInt32                 hi);

EXTERN_API_C( UInt32 )
OTGetRandomSeed                 (void)
{
	return 0;
}

EXTERN_API_C( UInt32 )
OTGetRandomNumber               (UInt32 *               seed,
                                 UInt32                 lo,
                                 UInt32                 hi)
{
	#pragma unused(seed)
	#pragma unused(lo)
	#pragma unused(hi)
	return 0;
}

#endif

static SInt32 gPoundersRunning;

static OSStatus EchoPounder(void *parameter)
{
	OSStatus 		err;
	OSStatus 		junk;
	InetHost		ipAddr;
	OTMPEndpointRef ep;
	InetAddress		inetAddr;
	TCall 			call;
	UInt32			sndBuffer[256];
	UInt32			rcvBuffer[256];
	UInt32			seed;

	ipAddr = (InetHost) parameter;
	
	ep = NULL;
	seed = OTGetRandomSeed();
	
	err = OTMPXPrepareThisTask();
	if (err == noErr) {
		ep = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
	}
	if (err == noErr) {
		err = OTMPXBind(ep, NULL, NULL);
	}
	if (err == noErr) {
		OTInitInetAddress(&inetAddr, 1026, ipAddr);
		OTMemzero(&call, sizeof(call));
		call.addr.buf = (UInt8 *) &inetAddr;
		call.addr.len = sizeof(inetAddr);
		err = OTMPXConnect(ep, &call, NULL);
	}
	if (err == noErr) {
		UInt32 numberOfBlocksLeftToSend;
		
		// MPLogPrintfSlow("seed = %08lx\n", seed);
		numberOfBlocksLeftToSend = OTGetRandomNumber(&seed, 0, 100);
		// MPLogPrintfSlow("numberOfBlocksLeftToSend = %ld\n", numberOfBlocksLeftToSend);
		while (err == noErr && numberOfBlocksLeftToSend > 0) {
			FillSendBuffer(sndBuffer, 256);
			err = OTMPXSnd(ep, sndBuffer, sizeof(sndBuffer), 0);
			if (err > noErr) {
				err = noErr;
			}
			if (err == noErr) {
				OTByteCount bytesNeeded;
				char *		rcvCursor;
				OTFlags 	junkFlags;
				
				bytesNeeded = 256 * sizeof(UInt32);
				rcvCursor = (char *) rcvBuffer;
				do {
					err = OTMPXRcv(ep, rcvCursor, bytesNeeded, &junkFlags);
					if (err > noErr) {
						rcvCursor += err;
						bytesNeeded -= err;
						err = noErr;
					}
				} while (err == noErr && bytesNeeded != 0);
			}
			if (err == noErr) {
				CheckReceiveBuffer(rcvBuffer, sndBuffer, 256);
				numberOfBlocksLeftToSend -= 1;
			}
		};
	}
	if (err == noErr) {
		err = OrderlyDisconnect(ep);
	}
	
	if (ep != NULL) {
		junk = OTMPXCloseProvider(ep);
		assert(junk == noErr);
	}
	OTMPXUnprepareThisTask();

	(void) OTAtomicAdd32(-1, &gPoundersRunning);

	return err;
}

static SInt32 gPounderNumber;

static void PoundEchoServer(Boolean lotsOfPounding)
{
	OSStatus 	err;
	char 		ipAddrStr[256];
	InetHost 	ipAddr;
	MPTaskID 	junkTaskID;

	err = noErr;
	printf("Enter IP address of echo server:\n");
	gets(ipAddrStr);
	if (ipAddrStr[0] == 'q') {
		err = userCanceledErr;
	}
	if (err == noErr) {
		if (ipAddrStr[0] == 0) {
			ipAddr = 0x7f000001;			// 127.0.0.1
		} else {
			err = OTInetStringToHost(ipAddrStr, &ipAddr);
		}
	}
	if (err == noErr) {
		if (lotsOfPounding) {
			UInt32 startTicks;
			UInt32 lastPrinted;
			
			startTicks = TickCount();
			lastPrinted = TickCount();
			do {
				if (TickCount() > (lastPrinted + 30)) {
					printf(".");
					fflush(stdout);
					lastPrinted = TickCount();
				}
				if (gPoundersRunning < 5) {
					gPounderNumber -= 1;
					err = MPCreateTask(EchoPounder, (void *) ipAddr, 65536, gEchoDeathQueue, (void *) gPounderNumber, NULL, kNilOptions, &junkTaskID);
					if (err == noErr) {
						(void) OTAtomicAdd32(1, &gPoundersRunning);
					}
				}
			} while ( (err == noErr) && (TickCount() < (startTicks + (60 * 60))) );
		} else {
			gPounderNumber -= 1;
			err = MPCreateTask(EchoPounder, (void *) ipAddr, 65536, gEchoDeathQueue, (void *) gPounderNumber, NULL, kNilOptions, &junkTaskID);
		}
	}
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static void RawStreamTest(void)
{
	OSStatus 		err;
	OSStatus 		junk;
	OTMPEndpointRef ep;
	UInt32 			i;
	UInt32 			data;
	strbuf 			dataBuf;

	dataBuf.buf 	= (char *) &data;
	dataBuf.len 	= sizeof(data);
	dataBuf.maxlen 	= sizeof(data);
	
	ep = OTMPOpenEndpointQInContext("QTestMod", 0, NULL, &err, NULL);
	if (err == noErr) {
		for (i = 0; i < 1000; i++) {
			data = i;
			err = OTMPPutMessage(ep, NULL, &dataBuf, 0);
			if (err != noErr) {
				break;
			}
		}
	}
	if (err == noErr) {
		for (i = 0; i < 1000; i++) {
			OTFlags flags;
			err = OTMPGetMessage(ep, NULL, &dataBuf, &flags);
			if (err == noErr) {
				assert(dataBuf.len == sizeof(data));
				assert(i == data);
			} else {
				break;
			}
		}
	}
	
	if (ep != NULL) {
		junk = OTMPCloseProvider(ep);
		assert(junk == noErr);
	}

	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static OSStatus ConnectTask(void *parameter)
{
	#pragma unused(parameter)
	OSStatus err;
	OSStatus junk;
	OTMPEndpointRef ep;

	ep = NULL;

	DebugStr("\pFoo");
		
	err = OTMPXPrepareThisTask();
	if (err == noErr) {
		ep = OTMPXOpenEndpointQInContext("tcp", 0, NULL, &err, NULL);
	}
	if (err == noErr) {
		err = OTMPXBind(ep, NULL, NULL);
	}
	if (err == noErr) {
		TCall sndCall;
		DNSAddress dnsAddr;
		
		OTMemzero(&sndCall, sizeof(sndCall));
		sndCall.addr.buf = (UInt8 *) &dnsAddr;
		sndCall.addr.len = OTInitDNSAddress(&dnsAddr, "www.apple.com:80");
		
		err = OTMPXConnect(ep, &sndCall, NULL);
	}
	if (err == noErr) {
		OTByteCount numBytes;
		
		err = OTMPXCountDataBytes(ep, &numBytes);
		MPLog2(kOTMPAPILogID, 'CNT!', (void *) err, (void *) numBytes);
	}
	
	if (ep != NULL) {
		junk = OTMPXCloseProvider(ep);
		assert(junk == noErr);
	}
	
	OTMPXUnprepareThisTask();
	return err;
}

static void DoOTMPConnectTest(void)
{
	OSStatus 	err;
	OSStatus 	junk;
	MPQueueID 	deathQueue;
	MPTaskID  	junkTask;
	OSStatus	taskStatus;
	Boolean		done;
	UInt32		lastPrinted;

	deathQueue = kInvalidID;
	err = MPCreateQueue(&deathQueue);
	if (err == noErr) {
		err = MPCreateTask(ConnectTask, NULL, 65536, deathQueue, (void *) 1, (void *) 666, kNilOptions, &junkTask);
	}
	if (err == noErr) {
		printf("Waiting for MP task to terminate.\n");

		done = false;
		lastPrinted = TickCount();
		do {
			err = MPWaitOnQueue(deathQueue, NULL, NULL, (void **) &taskStatus, kDurationImmediate);
			if (err == noErr) {
				printf("MP task completed with status %ld.\n", taskStatus);
				done = true;
			} else if (err == kMPTimeoutErr) {
				#if !TARGET_API_MAC_CARBON
					SystemTask();
				#endif
				if (TickCount() > (lastPrinted + 60)) {
					printf(".");
					fflush(stdout);
					lastPrinted = TickCount();
				}
				err = noErr;
			}
		} while ( ! done );
	}
	
	if (deathQueue != kInvalidID) {
		junk = MPDeleteQueue(deathQueue);
		assert(junk == noErr);
	}
	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

void main(void)
{
	OSStatus err;
	OTClientContextPtr junkContext;
	Boolean quitNow;
	char commandStr[256];

	// Can't use MPLogPrintfSlow because MP isn't initialised yet.
		
	printf("Hello Cruel World!\n");
	printf("OTMPTest.c\n");

	err = noErr;
	if ( ! MPLibraryIsLoaded() ) {
		err = -1;
	}
	if (err == noErr) {
		err = InitMPLog(65536);
	}
	if (err == noErr) {
		// MPLogSetMask(0xFFFFFFFF);
		MPLogSetMask(0);
		err = InitOpenTransportMPXInContext(kInitOTForApplicationMask, &junkContext);
	}
	if (err == noErr) {
		quitNow = false;
		do {
			printf("\n");
			printf("q) Quit\n");
			printf("n) Nested init test\n");
			printf("b) Basic debugging test\n");
			printf("s) Send flow control test\n");
			printf("l) Set MPLog mask\n");
			printf("w) Write MPLog\n");
			printf("f) Print OTMPSnd flow control info\n");
			printf("e) Start echo server\n");
			printf("d) Display echo death info\n");
			printf("p) Test the echo server\n");
			printf("P) Pound echo server\n");
			printf("r) Raw stream test\n");
			printf("c) OTMPConnect test\n");
			printf("Enter a command:\n");
			gets(commandStr);
			switch (commandStr[0]) {
				case 'q':
					quitNow = true;
					break;
				case 'n':
					NestedInitTest();
					break;
				case 'b':
					BasicDebuggingTests();
					break;
				case 's':
					SendFlowControlTest();
					break;
				case 'l':
					printf("Enter new log mask (in hex):\n");
					gets(commandStr);
					if (commandStr[0] != 0) {
						UInt32 newMask;
						
						sscanf(commandStr, "%x", &newMask);
						MPLogSetMask(newMask);
						printf("MPLogSetMask(0x%08lx)\n", newMask);
					}
					break;
				case 'w':
					DoWriteMPLog();
					break;
				case 'f':
					OTMPPrintSndRetryFreqDist();
					break;
				case 'e':
					StartEchoServer();
					break;
				case 'd':
					PrintEchoDeathInfo();
					break;
				case 'p':
				case 'P':
					PoundEchoServer(commandStr[0] == 'P');
					break;
				case 'r':
					RawStreamTest();
					break;
				case 'c':
					DoOTMPConnectTest();
					break;
				default:
					printf("Huh?\n");
					break;
			}
		} while (!quitNow);

		// StopEchoServer();
		
		CloseOpenTransportMPXInContext(NULL);
	}
	
	// Test that startup after shutdown works.
	
	if (err == noErr) {
		err = InitOpenTransportMPXInContext(kInitOTForApplicationMask, &junkContext);
		if (err == noErr) {
			CloseOpenTransportMPXInContext(NULL);
		}
	}
	
	printf("\n");
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %ld\n", err);
	}
	
	printf("Done.  Press command-Q to Quit.\n");
}
