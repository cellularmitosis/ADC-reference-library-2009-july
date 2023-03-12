/*
 
 File: GSSauthenticate.c
 
 Abstract: Simplified functions to help with GSSAPI authentications on
           server side.
 
 Version: <1.0>
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */

#include <Kerberos/Kerberos.h>
#include "GSSauthenticate.h"

OM_uint32 AuthenticateGSS( char *inToken, int inTokenLen, char **outToken, int *outTokenLen, char **inOutServiceName, 
						   char **outUserPrinc, gss_ctx_id_t *inOutGSScontext, gss_cred_id_t *inOutGSScreds )
{
	OM_uint32           minorStatus				= 0;
	OM_uint32           majorStatus				= GSS_S_DEFECTIVE_TOKEN;
	gss_buffer_desc		sendToken				= GSS_C_EMPTY_BUFFER;
	gss_buffer_desc     recvToken				= { inTokenLen, inToken };
    gss_name_t			gssClientPrincipal		= GSS_C_NO_BUFFER;
	
	if( inToken == NULL ) {
		// our default majorStatus is GSS_S_DEFECTIVE_TOKEN
		goto finished;
	}
	
	// if we don't have credentials and a service name is supplied, attempt to get credentials for that service principal
	if( *inOutGSScreds == NULL && inOutServiceName && *inOutServiceName && (*inOutServiceName)[0] != '\0' ) {
		
		if( AcquireGSSCredentials(*inOutServiceName, inOutGSScreds) != 0 ) {
			majorStatus = GSS_S_DEFECTIVE_CREDENTIAL;
			goto finished;
		}
	}
	
	// let's accept the security context, if this is a new context, it will set a new one up based on incoming token
	majorStatus = gss_accept_sec_context( &minorStatus, inOutGSScontext, *inOutGSScreds, &recvToken, 
										  GSS_C_NO_CHANNEL_BINDINGS, &gssClientPrincipal, NULL, 
										  &sendToken, NULL, NULL, NULL );
	
	if( majorStatus == GSS_S_COMPLETE ) {
		
		// if a service name was not specified for this, let's return the one that was used
		if( inOutServiceName != NULL && *inOutServiceName == NULL ) {
			
			// export the credentials that were used to make the connection (i.e., http/server@REALM, server@REALM, etc.)
			gss_name_t servicePrincipal = GSS_C_NO_NAME;
			
			// get the information from the buffer
			majorStatus = gss_inquire_context( &minorStatus, *inOutGSScontext, NULL, &servicePrincipal, NULL, NULL, 
											   NULL, NULL, NULL );
			if( majorStatus == GSS_S_COMPLETE ) {
				
				gss_buffer_desc nameToken = GSS_C_EMPTY_BUFFER;
				
				// now let's get a readable version of the service principal
				majorStatus = gss_display_name( &minorStatus, servicePrincipal, &nameToken, NULL );
				if( majorStatus == GSS_S_COMPLETE ) 
				{
					*inOutServiceName = strdup( nameToken.value );
					gss_release_buffer( &minorStatus, &nameToken );
				}
			}
		}

		// reset to complete regardless here cause we don't want to return an error
		majorStatus = GSS_S_COMPLETE;
	}
	
	// if we have a return token, be sure to return it, whether it is a continue or a 
	if( sendToken.length && sendToken.value ) {
		
		*outTokenLen = sendToken.length;
		*outToken = calloc( 1, sendToken.length );
		
		bcopy( sendToken.value, *outToken, sendToken.length );
	} else {
		
		*outTokenLen = 0;
		*outToken = NULL;
	}
	
	// release any buffer held by the sendToken
	gss_release_buffer( &minorStatus, &sendToken );
	
finished:
	
	// if we weren't successful, let's cleanup the context
	if( majorStatus != GSS_S_CONTINUE_NEEDED && majorStatus != GSS_S_COMPLETE ) {
		
		gss_delete_sec_context( &minorStatus, inOutGSScontext, GSS_C_NO_BUFFER );
		*inOutGSScontext = GSS_C_NO_CONTEXT;
	}
		
	// let's return the gssClientPrincipal in case they want to note a failure, but only
	// if it is not already set
	if( gssClientPrincipal != NULL && *outUserPrinc == NULL ) {
		gss_buffer_desc nameToken = GSS_C_EMPTY_BUFFER;
		
		OM_uint32 iStatus = gss_display_name( &minorStatus, gssClientPrincipal, &nameToken, NULL );
		if( iStatus == GSS_S_COMPLETE ) {
			
			*outUserPrinc = strdup( nameToken.value );
			gss_release_buffer( &minorStatus, &nameToken );
		}
	}
	
	return majorStatus;
}

int AcquireGSSCredentials( const char *inServiceName, gss_cred_id_t *outServiceCredentials )
{
	gss_name_t			stServerName	= GSS_C_NO_NAME;
	gss_buffer_desc		stNameBuffer;
	OM_uint32			iMajorStatus;
	OM_uint32			iMinorStatus;
	
	// Check the inServiceName and and if there is a string
	if( inServiceName == NULL || *inServiceName == '\0' ) {
		return -1;
	}
	
	// fill the stNameBuffer with the incomine service name
	stNameBuffer.value = (char *) inServiceName;
	stNameBuffer.length = strlen(inServiceName) + 1;
	
	iMajorStatus = gss_import_name( &iMinorStatus, &stNameBuffer, GSS_C_NT_HOSTBASED_SERVICE, &stServerName );
	if( iMajorStatus != GSS_S_COMPLETE ) {
		// Log here if necessary with more detail using:
		//		gss_display_status( &tempStatus, iMajorStatus, GSS_C_GSS_CODE, ... )
		//		gss_display_status( &tempStatus, iMinorStatus, GSS_C_MECH_CODE, ... )
		return -1;
	}
	
	// get credentials with the expectation that we are accepting credentials on behalf of this service
	// this will go to the keytab look for the key for this service and prepare to allow authentication
	// to the service by clients
	iMajorStatus = gss_acquire_cred( &iMinorStatus, stServerName, 0, GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
									 outServiceCredentials, NULL, NULL);
	
	// need to release the allocated stServerName
	int iTempStatus; // create a temporary variable so we don't over-write the iMinorStatus
	gss_release_name( &iTempStatus, &stServerName );
	
	// check our status and determine if we succeeded or not
	if( iMajorStatus != GSS_S_COMPLETE ) {
		// Log here if necessary with more detail using:
		//		gss_display_status( &tempStatus, iMajorStatus, GSS_C_GSS_CODE, ... )
		//		gss_display_status( &tempStatus, iMinorStatus, GSS_C_MECH_CODE, ... )
		return -1;
	}
	
	return 0;
}

