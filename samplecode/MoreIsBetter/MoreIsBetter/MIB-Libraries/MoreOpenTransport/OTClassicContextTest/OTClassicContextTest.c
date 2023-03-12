/*
	File:		OTClassicContextTest.c

	Contains:	Code for testing OTClassicContext.

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

$Log: OTClassicContextTest.c,v $
Revision 1.4  2002/11/08 23:43:18         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.3  2001/11/07 15:56:42         
Tidy up headers, add CVS logs, update copyright.


         <2>    13/12/00    Quinn   Now that InitOpenTranportInContext is fixed to accept nil, pass
                                    NULL for the outClientContext parameter.
         <1>    12/10/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <Events.h>

// Standard C interfaces

#include <stdio.h>

// Our prototypes

#include "OTClassicContext.h"

/////////////////////////////////////////////////////////////////

// The following routines are sufficiently obscure that I didn't bother 
// testing them.
//
// OTStreamPipeInContext
// OTOpenProviderOnStreamInContext
// OTOpenEndpointOnStreamInContext

static OSStatus TransferTest(void)
{
	OSStatus err;
	OSStatus junk;
	EndpointRef ep;
	EndpointRef ep2;
	OTClient me;
	
	ep2 = NULL;
	ep = OTOpenEndpointInContext(OTCreateConfiguration("tcp"), 0, NULL, &err, NULL);
	if (err == noErr) {
		me = OTWhoAmIInContext(NULL);
		
		ep2 = OTTransferProviderOwnershipInContext(ep, me, &err, NULL);
		if (err == noErr) {
			ep = NULL;
		}
	}
	if (ep != NULL) {
		junk = OTCloseProvider(ep);
		assert(junk == noErr);
	}
	if (ep2 != NULL) {
		junk = OTCloseProvider(ep2);
		assert(junk == noErr);
	}
	return err;
}

static OSStatus MemTest(void)
{
	OSStatus err;
	OSStatus junk;
	EndpointRef ep;
	void *p1;
	void *p2;
	
	p1 = NULL;
	p2 = NULL;
	ep = OTOpenEndpointInContext(OTCreateConfiguration("tcp"), 0, NULL, &err, NULL);
	if (err == noErr) {
		p1 = OTAllocInContext(ep, T_BIND, T_ALL, &err, NULL);
	}
	if (err == noErr) {
		p2 = OTAllocMemInContext(100, NULL);
	}
	if (p2 != NULL) {
		OTFreeMem(p2);
	}
	if (p1 != NULL) {
		junk = OTFree(p1, T_BIND);
		assert(junk == noErr);
	}
	if (ep != NULL) {
		junk = OTCloseProvider(ep);
		assert(junk == noErr);
	}
	return err;
}

static pascal void DummyNotifier(void *contextPtr, OTEventCode code, OTResult result, void *cookie)
{
	#pragma unused(result)
	
	if (code == T_OPENCOMPLETE) {
		if (cookie == NULL) {
			cookie = (void *) -1;
		}
		*((ProviderRef *) contextPtr) = (ProviderRef) cookie;
	} else if (code == kStreamOpenEvent) {
		if (cookie == NULL) {
			cookie = (void *) -1;
		}
		*((StreamRef *) contextPtr) = (StreamRef) cookie;
	}
}

static OSStatus OpenTest(void)
{
	OSStatus err;
	OSStatus junk;
	EndpointRef ep1;
	EndpointRef ep2;
	MapperRef map1;
	MapperRef map2;
	ATSvcRef at1;
	ATSvcRef at2;
	InetSvcRef inet1;
	InetSvcRef inet2;
	ProviderRef prov1;
	ProviderRef prov2;

	ep1   = NULL;
	ep2   = NULL;
	map1  = NULL;
	map2  = NULL;
	at1   = NULL;
	at2   = NULL;
	inet1 = NULL;
	inet2 = NULL;
	prov1 = NULL;
	prov2 = NULL;
	
	ep1 = OTOpenEndpointInContext(OTCreateConfiguration("tcp"), 0, NULL, &err, NULL);
	if (err == noErr) {
		err = OTAsyncOpenEndpointInContext(OTCreateConfiguration("tcp"), 0, NULL, DummyNotifier, &ep2, NULL);
		if (err == noErr) {
			while (ep2 == NULL) {
				SystemTask();
			}
			if (ep2 == (void *) -1) {
				err = -1;
			}
		}
	}

	if (err == noErr) {
		map1 = OTOpenMapperInContext(OTCreateConfiguration("nbp"), 0, &err, NULL);
	}
	if (err == noErr) {
		err = OTAsyncOpenMapperInContext(OTCreateConfiguration("nbp"), 0, DummyNotifier, &map2, NULL);
		if (err == noErr) {
			while (map2 == NULL) {
				SystemTask();
			}
			if (map2 == (void *) -1) {
				err = -1;
			}
		}
	}

	if (err == noErr) {
		inet1 = OTOpenInternetServicesInContext(kDefaultInternetServicesPath, 0, &err, NULL);
	}
	if (err == noErr) {
		err = OTAsyncOpenInternetServicesInContext(kDefaultInternetServicesPath, 0, DummyNotifier, &inet2, NULL);
		if (err == noErr) {
			while (inet2 == NULL) {
				SystemTask();
			}
			if (inet2 == (void *) -1) {
				err = -1;
			}
		}
	}

	if (err == noErr) {
		at1 = OTOpenAppleTalkServicesInContext(kDefaultAppleTalkServicesPath, 0, &err, NULL);
	}
	if (err == noErr) {
		err = OTAsyncOpenAppleTalkServicesInContext(kDefaultAppleTalkServicesPath, 0, DummyNotifier, &at2, NULL);
		if (err == noErr) {
			while (at2 == NULL) {
				SystemTask();
			}
			if (at2 == (void *) -1) {
				err = -1;
			}
		}
	}


	if (err == noErr) {
		prov1 = OTOpenProviderInContext(OTCreateConfiguration("enet"), 0, &err, NULL);
	}
	if (err == noErr) {
		err = OTAsyncOpenProviderInContext(OTCreateConfiguration("enet"), 0, DummyNotifier, &prov2, NULL);
		if (err == noErr) {
			while (prov2 == NULL) {
				SystemTask();
			}
			if (prov2 == (void *) -1) {
				err = -1;
			}
		}
	}

	if (ep1 != NULL) {
		junk = OTCloseProvider(ep1);
		assert(junk == noErr);
	}
	if (ep2 != NULL) {
		junk = OTCloseProvider(ep2);
		assert(junk == noErr);
	}
	if (map1 != NULL) {
		junk = OTCloseProvider(map1);
		assert(junk == noErr);
	}
	if (map2 != NULL) {
		junk = OTCloseProvider(map2);
		assert(junk == noErr);
	}
	if (at1 != NULL) {
		junk = OTCloseProvider(at1);
		assert(junk == noErr);
	}
	if (at2 != NULL) {
		junk = OTCloseProvider(at2);
		assert(junk == noErr);
	}
	if (inet1 != NULL) {
		junk = OTCloseProvider(inet1);
		assert(junk == noErr);
	}
	if (inet2 != NULL) {
		junk = OTCloseProvider(inet2);
		assert(junk == noErr);
	}
	if (prov1 != NULL) {
		junk = OTCloseProvider(prov1);
		assert(junk == noErr);
	}
	if (prov2 != NULL) {
		junk = OTCloseProvider(prov2);
		assert(junk == noErr);
	}
	return err;
}

static pascal void DummyProc(void *arg)
{
	#pragma unused(arg)
}

static OSStatus TaskTest(void)
{
	OSStatus err;
	OSStatus junk;
	OTDeferredTaskRef dt;
	OTTimerTask tt;
	OTSystemTaskRef st;
	
	dt = 0;
	tt = 0;
	st = 0;
	
	err = noErr;
	dt = OTCreateDeferredTaskInContext(DummyProc, NULL, NULL);
	if (dt == 0) {
		err = -1;
	}
	if (err == noErr) {
		tt = OTCreateTimerTaskInContext(DummyProc, NULL, NULL);
	}
	if (tt == 0) {
		err = -2;
	}
	if (err == noErr) {
		st = OTCreateSystemTaskInContext(DummyProc, NULL, NULL);
	}
	if (st == 0) {
		err = -3;
	}
	
	if (dt != 0) {
		junk = OTDestroyDeferredTask(dt);
		assert(junk == noErr);
	}
	if (tt != 0) {
		OTDestroyTimerTask(tt);
	}
	if (st != 0) {
		junk = OTDestroySystemTask(st);
		assert(junk == noErr);
	}

	return err;
}

static OSStatus StreamTest(void)
{
	OSStatus err;
	OSStatus junk;
	StreamRef strm1;
	StreamRef strm2;
	StreamRef strm3;
	StreamRef strm4;
	
	strm1 = NULL;
	strm2 = NULL;
	strm3 = NULL;
	strm4 = NULL;

	err = noErr;
	if (err == noErr) {
		strm1 = OTStreamOpenInContext("sad", 0, &err, NULL);
	}
	if (err == noErr) {
		err = OTAsyncStreamOpenInContext("sad", 0, DummyNotifier, &strm2, NULL);
		if (err == noErr) {
			while (strm2 == NULL) {
				SystemTask();
			}
			if (strm2 == (void *) -1) {
				err = -1;
			}
		}
	}
	if (err == noErr) {
		strm3 = OTCreateStreamInContext(OTCreateConfiguration("tcp"), 0, &err, NULL);
	}
	if (err == noErr) {
		err = OTAsyncCreateStreamInContext(OTCreateConfiguration("tcp"), 0, DummyNotifier, &strm4, NULL);
		if (err == noErr) {
			while (strm4 == NULL) {
				SystemTask();
			}
			if (strm4 == (void *) -1) {
				err = -1;
			}
		}
	}
	
	if (strm1 != NULL) {	
		junk = OTStreamClose(strm1);
		assert(junk == noErr);
	}
	if (strm2 != NULL) {	
		junk = OTStreamClose(strm2);
		assert(junk == noErr);
	}
	if (strm3 != NULL) {	
		junk = OTStreamClose(strm3);
		assert(junk == noErr);
	}
	if (strm4 != NULL) {	
		junk = OTStreamClose(strm4);
		assert(junk == noErr);
	}
	return err;
}

extern void main(void)
{
	OSStatus err;
	OSStatus junk;
	
	err = InitOpenTransportInContext(kInitOTForApplicationMask, NULL);
	if (err == noErr) {
		err = OTRegisterAsClientInContext("\pOTClassicContextTest", DummyNotifier, NULL);
		if (err == noErr) {
			err = TransferTest();
			if (err == noErr) {
				err = MemTest();
			}
			if (err == noErr) {
				err = OpenTest();
			}
			if (err == noErr) {
				err = TaskTest();
			}
			if (err == noErr) {
				err = StreamTest();
			}
			
			junk = OTUnregisterAsClientInContext(NULL);
			assert(junk == noErr);
		}
		CloseOpenTransportInContext(NULL);
	}
	
	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}	
	printf("Done.  Press command-Q to Quit.\n");
}
