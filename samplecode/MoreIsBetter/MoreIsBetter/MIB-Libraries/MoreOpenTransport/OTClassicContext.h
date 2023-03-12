/*
	File:		OTClassicContext.h

	Contains:	Extra "InContext" OT routines for non-Carbon code.

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

$Log: OTClassicContext.h,v $
Revision 1.2  2001/11/07 15:56:37         
Tidy up headers, add CVS logs, update copyright.


         <1>    12/10/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <OpenTransport.h>
#include <OpenTransportProviders.h>
#include <OpenTransportProtocol.h>

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// This interface file declares all of the "InContext" routines, ie 
// routines that create assets that Open Transport needs to track. 
// Some routines already have prototypes in the various Open Transport
// interface files, and thus are only mentioned here in a comment.
// Some other routines do not have "InContext" variants in the 
// Open Transport headers (primarily because the "InContext" effort 
// was part of Carbon, and these routines are available to Carbon 
// clients), and thus their prototypes are declared here.

// InitOpenTransportInContext (OpenTransport.h)
// CloseOpenTransportInContext  (OpenTransport.h)

EXTERN_API( OSStatus )
OTRegisterAsClientInContext     (OTClientName           name,
                                 OTNotifyUPP            proc,
                                 OTClientContextPtr     clientContext);

EXTERN_API( OSStatus )
OTUnregisterAsClientInContext   (OTClientContextPtr     clientContext);

EXTERN_API( ProviderRef )
OTTransferProviderOwnershipInContext(
                                 ProviderRef            ref,
                                 OTClient               prevOwner,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( OTClient )
OTWhoAmIInContext               (OTClientContextPtr     clientContext);

// OTAllocInContext (OpenTransport.h)
// OTAllocMemInContext (OpenTransport.h)

// OTOpenEndpointInContext (OpenTransport.h)
// OTAsyncOpenEndpointInContext (OpenTransport.h)

// OTOpenMapperInContext (OpenTransport.h)
// OTAsyncOpenMapperInContext (OpenTransport.h)

// OTCloseProvider (OpenTransport.h)

// OTCreateDeferredTaskInContext (OpenTransport.h)
// OTCreateTimerTaskInContext (OpenTransportProtocol.h)

EXTERN_API( OTSystemTaskRef )
OTCreateSystemTaskInContext     (OTProcessUPP           proc,
                                 void *                 arg,
                                 OTClientContextPtr     clientContext);

// OTOpenInternetServicesInContext (OpenTransportProviders.h)
// OTAsyncOpenInternetServicesInContext (OpenTransportProviders.h)

// OTOpenAppleTalkServicesInContext (OpenTransportProviders.h)
// OTAsyncOpenAppleTalkServicesInContext (OpenTransportProviders.h)

EXTERN_API( ProviderRef )
OTOpenProviderInContext         (OTConfigurationRef     cfig,
                                 OTOpenFlags            flags,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( OSStatus )
OTAsyncOpenProviderInContext    (OTConfigurationRef     cfig,
                                 OTOpenFlags            flags,
                                 OTNotifyUPP            proc,
                                 void *                 contextPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( StreamRef )
OTStreamOpenInContext           (const char *           name,
                                 OTOpenFlags            oFlags,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( OSStatus )
OTAsyncStreamOpenInContext      (const char *           name,
                                 OTOpenFlags            oFlags,
                                 OTNotifyUPP            proc,
                                 void *                 contextPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( StreamRef )
OTCreateStreamInContext         (OTConfigurationRef     cfig,
                                 OTOpenFlags            oFlags,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( OSStatus )
OTAsyncCreateStreamInContext    (OTConfigurationRef     cfig,
                                 OTOpenFlags            oFlags,
                                 OTNotifyUPP            proc,
                                 void *                 contextPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( OTResult )
OTStreamPipeInContext           (StreamRef              streamsToPipe[],
                                 OTClientContextPtr     clientContext);

EXTERN_API( ProviderRef )
OTOpenProviderOnStreamInContext (StreamRef              strm,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);

EXTERN_API( EndpointRef )
OTOpenEndpointOnStreamInContext (StreamRef              strm,
                                 OSStatus *             errPtr,
                                 OTClientContextPtr     clientContext);

#ifdef __cplusplus
}
#endif
