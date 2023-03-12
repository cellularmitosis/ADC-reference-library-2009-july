/*
 
 File: democlient.c
 
 Abstract: Test client to demonstrate client/server auths
 
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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "NetworkHelper.h"
#include "CRAMMD5helper.h"

void usage( void )
{
	printf( "Usage:  democlient -m method -h ipaddress -t port [-u username]\n" );
	printf( "                   [-p password] [-S service] \n" );
	printf( "        -m   method 'cleartext', 'CRAM-MD5', 'GSSAPI'\n" );
	printf( "        -h   host IP address or dns name\n" );
	printf( "        -t   port for server\n" );
	printf( "        -u   username to authenticate (except GSSAPI)\n" );
	printf( "        -p   password to authenticate (except GSSAPI)\n" );
	printf( "        -S   service principal 'host' to use (GSSAPI)\n" );
}

int main( int argc, char *argv[] )
{
	int		iPort		= 0;
	int		ch;
	char	*pService	= NULL;
	char	*pHost		= NULL;
	char	*pMethod	= NULL;
	char	*pUsername	= NULL;
	char	*pPassword	= NULL;
	int		iSocket;
	
	// parse out our arguments
	while ((ch = getopt(argc, argv, "m:h:t:u:p:S:")) != -1) {
		switch (ch) {
			case 'S':
				pService = optarg;
				break;
			case 'h':
				pHost = optarg;
				break;
			case 't':
				iPort = strtol( optarg, NULL, 10 );
				break;
			case 'm':
				pMethod = optarg;
				break;
			case 'u':
				pUsername = optarg;
				break;
			case 'p':
				pPassword = optarg;
				break;
			case '?':
			default:
				usage();
				return 1;
		}
	}
	
	// validate the arguments we need for all cases
	if( pMethod == NULL || strlen(pMethod) == 0 || pHost == NULL || strlen(pHost) == 0 ) {
		usage();
		return 1;
	}
	
	// if method is GSSAPI we don't care about certain info
	if( strcasecmp(pMethod, "GSSAPI") == 0 ) {
		
		// we should have a principal of some sort
		if( pService == NULL || strlen(pService) == 0 ) {
			usage();
			return 1;
		}
	} else {
		
		// otherwise, we should have a username / password too
		if( pUsername == NULL || strlen(pUsername) == 0 || pPassword == NULL || strlen(pPassword) == 0 ) {
			usage();
			return 1;
		}
	}
	
	// don't allow privileged ports for this test
	if( iPort < 1024 ) {
		printf( "Error:  Port value must be greater than 1024\n" );
		return 1;
	}
	
	// attempt to open a socket to the server
	iSocket = ConnectTo( pHost, iPort );
	if( iSocket > 0 ) {
		
		printf( "Opened connection to %s:%d\n", pHost, iPort );
		
		if( strcasecmp(pMethod, "GSSAPI") == 0 ) {
			
			char				pServiceName[255]	= { 0, };
			gss_ctx_id_t		gssContext			= GSS_C_NO_CONTEXT;
			gss_buffer_desc		sendToken			= GSS_C_EMPTY_BUFFER;
			gss_buffer_desc		recvToken			= GSS_C_EMPTY_BUFFER;
			gss_name_t			serviceName			= GSS_C_NO_NAME;
			OM_uint32			iMajorStatus;
			OM_uint32			iMinorStatus;
			OM_uint32			iReturnFlags		= 0;
			
			// first send the type of auth we want
			SendToken( iSocket, pMethod, strlen(pMethod) );
						
			// let's generate a service princpal string
			snprintf( pServiceName, 255, "%s@%s", pService, pHost );
			
			printf( "Attempting to use GSS name %s\n", pServiceName );
			
			// use the sendToken to import the name for simplicity
			sendToken.value = pServiceName;
			sendToken.length = strlen(pServiceName) ;
			
			// import the name so we can use it
			iMajorStatus = gss_import_name( &iMinorStatus, &sendToken,
											GSS_C_NT_HOSTBASED_SERVICE, &serviceName);
			
			if (iMajorStatus != GSS_S_COMPLETE) {
				// if we had a problem we could print out more information
				printf( "Error:  Couldn't import %s with gss_import_name\n", pServiceName );
				return -1;
			}
			
			sendToken.value = NULL;
			sendToken.length = 0;
			
			do {
				const OM_uint32 reqFlags = GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG;
				
				// gss_init_sec_context is used in succession as long as GSS_S_CONTINUE_NEEDED is returned
				iMajorStatus = gss_init_sec_context( &iMinorStatus,
													 GSS_C_NO_CREDENTIAL,
													 &gssContext,
													 serviceName,
													 GSS_C_NO_OID,
													 reqFlags,
													 0,
													 GSS_C_NO_CHANNEL_BINDINGS,
													 &recvToken,
													 NULL,
													 &sendToken,
													 &iReturnFlags,
													 NULL );
				
				// let's release any existing memory for this token
				gss_release_buffer( &iMinorStatus, &recvToken );
				
				// if we have a token to send, let's send the token until we are done
				if( sendToken.length != 0 ) {
					
					if( SendToken( iSocket, sendToken.value, sendToken.length ) < 0 ) {
						gss_release_buffer( &iMinorStatus, &sendToken );
						break;
					}
				}
				
				// let's wait for another token if we still have a continue
				if( iMajorStatus == GSS_S_CONTINUE_NEEDED ) {
					recvToken.length = ReadToken( iSocket, (char **) &recvToken.value );
				}
				
			} while( iMajorStatus == GSS_S_CONTINUE_NEEDED );
			
			// we need to clena up the serviceName now that we are done
			gss_release_name( &iMinorStatus, &serviceName );
			
			// if we had success, means credentials were valid
			if( iMajorStatus == GSS_S_COMPLETE ) {
				char	*pResponse	= NULL;

				if( ReadToken(iSocket, &pResponse) > 0 ) {
					printf( "Server response:  %s\n", pResponse );
				} else {
					printf( "Did not receive a response\n" );
				}
				
				if( pResponse != NULL ) {
					free( pResponse );
					pResponse = NULL;
				}
			} else {
				printf( "\nFailure:  Credentials refused by %s:%d\n", pHost, iPort );
			}
		} else if( strcasecmp(pMethod, "cleartext") == 0 ) {
			
			char	*pResponse	= NULL;

			// first send the type of auth we want
			SendToken( iSocket, pMethod, strlen(pMethod) );
						
			// just send the username and password with SendToken
			SendToken( iSocket, pUsername, strlen(pUsername) );
			SendToken( iSocket, pPassword, strlen(pPassword) );
			
			if( ReadToken(iSocket, &pResponse) > 0 ) {
				printf( "Server response:  %s\n", pResponse );
			} else {
				printf( "Did not receive a response\n" );
			}
			
			if( pResponse != NULL ) {
				free( pResponse );
				pResponse = NULL;
			}
			
		} else if( strcasecmp(pMethod, "CRAM-MD5") == 0 ) {
			char	*pChallenge		= NULL;
			char	*pResponse		= NULL;
			
			// first send the type of auth we want
			SendToken( iSocket, pMethod, strlen(pMethod) );
			
			// next receive the challenge from the server
			if( ReadToken( iSocket, &pChallenge) > 0 ) {
				
				// then create the response based on that challenge
				unsigned char	pHash[17]		= { 0, };
				unsigned char	pHashHex[33]	= { 0, };
				int				i;
				
				// zero out the hash buffer in advance
				bzero( pHash, 17 );
				
				// Compute keyed-md5 hash of password and challenge.
				CalcMD5( pChallenge, strlen(pChallenge), pPassword, strlen(pPassword), pHash );
				
				// Prepare a Hex string representation of the hash, grouping into 2-byte Hex values.
				bzero( pHashHex, 33 );
				
				for( i=0; i < 16; i++ ) {
					sprintf( &pHashHex[i<<1], "%02x", pHash[i]);
				}
				
				// Add the NULL terminator
				pHashHex[32] = 0; 
				
				// send the username and hash to the server
				SendToken( iSocket, pUsername, strlen(pUsername) );
				SendToken( iSocket, pHashHex, 32 );
				
				if( ReadToken(iSocket, &pResponse) > 0 ) {
					printf( "Server response:  %s\n", pResponse );
				} else {
					printf( "Did not receive a response\n" );
				}
				
				if( pResponse != NULL ) {
					free( pResponse );
					pResponse = NULL;
				}
				
			} else {
				printf( "Did not receive a challenge from server\n" );
			}
		} else {
			printf( "Unknown method type\n" );
		}
		
	} else {
		printf( "Error:  Unable to open socket to %s on port %d\n", pHost, iPort );
		return 1;
	}
	return 0;
}
