/*
 
 File: demoserver.c
 
 Abstract: Demonstration of various authentication from a server perspective
 
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
#include <DirectoryService/DirectoryService.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "GSSauthenticate.h"
#include "NetworkHelper.h"
#include "DSUtility.h"

#pragma mark Prototypes

int		AuthCleartext( char *inUsername, char *inPassword );
int		AuthCRAMMD5( char *inUsername, char *inChallenge, char *inResponse );
int		AuthGSSAPI( int inSocket );

int		HandleNewConnection( int inSocket );
int		HandleCleartext( int inSocket );
int		HandleCRAMMD5( int inSocket );

char	*FindUserFromPrincipal( char *inPrincipal );

#pragma mark -
#pragma mark Authentication Routines

int AuthCleartext( char *inUsername, char *inPassword )
{
	tDirReference		dsRef			= 0;
	tDirNodeReference	dsSearchNodeRef	= 0;
	tDirNodeReference	dsUserNodeRef	= 0;
	tDirStatus			dsStatus;
	char				*pRecordName	= NULL;
	char				*pNodeName		= NULL;
	
	// Key steps to Authenticating a user:
	//	- First locate the user in the directory
	//	- Open Directory Service reference
	//	- Locate and open the Search Node
	//	- Locate the user's official RecordName and Directory Node based on the username provided
	//	- Then use authentication appropriate for the type of method
	
	// Open Directory Services reference
	dsStatus = dsOpenDirService( &dsRef );
	if( dsStatus == eDSNoErr ) {
		
		// use utility function in DSUtility.h to open the search node
		dsStatus = OpenSearchNode( dsRef, &dsSearchNodeRef );
		if( dsStatus == eDSNoErr ) {
			
			// use utility function in DSUtility.h to locate the user information
			dsStatus = LocateUserRecordNameAndNode( dsRef, dsSearchNodeRef, inUsername, &pRecordName, &pNodeName );
			if( dsStatus == eDSNoErr ) {
				
				// we should have values available, but let's check to be sure
				if( pNodeName != NULL && pNodeName[0] != '\0' && 
					pRecordName != NULL && pRecordName[0] != '\0' )
				{
					// need to create a tDataListPtr from the "/plugin/node" path, using "/" as the separator
					tDataListPtr dsUserNodePath = dsBuildFromPath( dsRef, pNodeName, "/" );
					
					dsStatus = dsOpenDirNode( dsRef, dsUserNodePath, &dsUserNodeRef );
					
					if( dsStatus == eDSNoErr ) {
						
						// Use our Utility routine to do the authentication
						dsStatus = DoPasswordAuth( dsRef, dsUserNodeRef, kDSStdAuthNodeNativeClearTextOK, 
												   pRecordName, inPassword );
						
						// Determine if successful.  There are cases where you may receive other errors
						// such as eDSAuthPasswordExpired.
						if( dsStatus == eDSNoErr ) {
							printf( "Successful:  Authentication successful for user '%s'\n", pRecordName );
						} else {
							printf( "Failure:  Authentication for user '%s' - %d\n", pRecordName, dsStatus );
						}
					}
					
					// free the data list as it is no longer needed
					dsDataListDeallocate( dsRef, dsUserNodePath );
					free( dsUserNodePath );
					dsUserNodePath = NULL;
				}
				
				// need to free any node name that may have been returned
				if( pNodeName != NULL ) {
					free( pNodeName );
					pNodeName = NULL;
				}
				
				// need to free any record name that may have been returned
				if( pRecordName != NULL ) {
					free( pRecordName );
					pRecordName = NULL;
				}
			}
			
			// close the search node cause we are done here
			dsCloseDirNode( dsSearchNodeRef );
			dsSearchNodeRef = 0;
			
		} else {
			printf( "Unable to locate and open the Search node\n" );
			return 1;
		}
		
		// need to close Directory Services at this point
		dsCloseDirService( dsRef );
		dsRef = 0;
	}
	
    return dsStatus;
}

int AuthCRAMMD5( char *inUsername, char *inChallenge, char *inResponse )
{
	tDirReference		dsRef			= 0;
	tDirNodeReference	dsSearchNodeRef	= 0;
	tDirNodeReference	dsUserNodeRef	= 0;
	tDirStatus			dsStatus;
	char				*pRecordName	= NULL;
	char				*pNodeName		= NULL;
	
	// Key steps to Authenticating a user:
	//	- First locate the user in the directory
	//	- Open Directory Service reference
	//	- Locate and open the Search Node
	//	- Locate the user's official RecordName and Directory Node based on the username provided
	//	- Then use authentication appropriate for the type of method
	
	// Open Directory Services reference
	dsStatus = dsOpenDirService( &dsRef );
	if( dsStatus == eDSNoErr ) {
		
		// use utility function to open the search node
		dsStatus = OpenSearchNode( dsRef, &dsSearchNodeRef );
		if( dsStatus == eDSNoErr ) {
			
			// use utility function to locate the user information
			dsStatus = LocateUserRecordNameAndNode( dsRef, dsSearchNodeRef, inUsername, &pRecordName, &pNodeName );
			if( dsStatus == eDSNoErr ) {
				
				// we should have values available, but let's check to be sure
				if( pNodeName != NULL && pNodeName[0] != '\0' && 
					pRecordName != NULL && pRecordName[0] != '\0' )
				{
					// need to create a tDataListPtr from the "/plugin/node" path, using "/" as the separator
					tDataListPtr dsUserNodePath = dsBuildFromPath( dsRef, pNodeName, "/" );
					
					// attempt to open the node provided
					dsStatus = dsOpenDirNode( dsRef, dsUserNodePath, &dsUserNodeRef );
					if( dsStatus == eDSNoErr ) {
						
						// Here is the Utility function that will do the authentication for us
						dsStatus = DoChallengeResponseAuth( dsRef, dsUserNodeRef, kDSStdAuthCRAM_MD5, pRecordName, 
															inChallenge, strlen(inChallenge), 
															inResponse, strlen(inResponse) );
						
						// Determine if successful.  There are cases where you may receive other errors
						// such as eDSAuthPasswordExpired.
						if( dsStatus == eDSNoErr ) {
							printf( "Successful:  CRAM-MD5 Authentication successful for user '%s'\n", pRecordName );
						} else {
							printf( "Failure:  CRAM-MD5 Authentication for user '%s' - %d\n", pRecordName, dsStatus );
						}
					}
					
					// free the data list as it is no longer needed
					dsDataListDeallocate( dsRef, dsUserNodePath );
					free( dsUserNodePath );
					dsUserNodePath = NULL;
				}
				
				// need to free any node name that may have been returned
				if( pNodeName != NULL ) {
					free( pNodeName );
					pNodeName = NULL;
				}
				
				// need to free any record name that may have been returned
				if( pRecordName != NULL ) {
					free( pRecordName );
					pRecordName = NULL;
				}
			}
			
			// close the search node cause we are done here
			dsCloseDirNode( dsSearchNodeRef );
			dsSearchNodeRef = 0;
			
		} else {
			printf( "Unable to locate and open the Search node\n" );
			return 1;
		}
		
		// need to close Directory Services at this point
		dsCloseDirService( dsRef );
		dsRef = 0;
	}
	
    return dsStatus;
}

int AuthGSSAPI( int inSocket )
{
	char			*pRecvToken		= NULL;
	int				iRecvTokenLen	= 0;
	char			*pSendToken		= NULL;
	int				iSendTokenLen	= 0;
	char			*pUserPrinc		= NULL;
	char			*pServiceName	= NULL;
	gss_ctx_id_t	gssContext		= GSS_C_NO_CONTEXT;
	gss_cred_id_t	gssCreds		= GSS_C_NO_CREDENTIAL;
	OM_uint32		iResult			= GSS_S_DEFECTIVE_CREDENTIAL;
	OM_uint32		iMinorStatus;

	// Key steps to authenticating a user with GSSAPI:
	//
	// GSSAPI is a request-response loop until finished.  For every token received
	// there may is a token sent back.  This loop is done until an error occurs or
	// we recieve a GSS_S_COMPLETE.  All tokens generated must be sent back to 
	// complete the process.
	
	// After successfully verifying the GSSAPI succeeded, we need to also verify
	// the user is a valid user on the system.  GSSAPI validates the user's credentials
	// not whether or not the user is allowed on this system.
	
	do {
		// read token data from the socket
		iRecvTokenLen = ReadToken( inSocket, &pRecvToken );
		
		printf( "Received a token with length of %d\n", iRecvTokenLen );
		
		if( iRecvTokenLen > 0 ) {
			// let's read and write data sending to and from the GSSauthenticate
			iResult = AuthenticateGSS( pRecvToken, iRecvTokenLen, &pSendToken, &iSendTokenLen, 
									   &pServiceName, &pUserPrinc, &gssContext, &gssCreds );
		}
		
		// if we have a token to send, let's send it to the client
		if( iSendTokenLen != 0 ) {
			
			printf( "Sending a token length of %d\n", iSendTokenLen );
			
			int iSent = SendToken(inSocket, pSendToken, iSendTokenLen);
			
			if( iSent == -1 || iSent != iSendTokenLen ) {
				// we had a problem, set failure state
				printf( "Failed to send packet.. bailing\n" );
				iResult = GSS_S_FAILURE;
			}
		}
		
		// free up our token if we have one
		if( pRecvToken != NULL ) {
			free( pRecvToken );
			pRecvToken = NULL;
		}
		
		// free up our send token if we have one
		if( pSendToken != NULL ) {
			free( pSendToken );
			pSendToken = NULL;
		}
		
	} while( iResult == GSS_S_CONTINUE_NEEDED );
	
	printf( "\n" ); // just print a line feed for visuals
	
	// if we completed successfully print out the userPrincipal if we had one
	if( iResult == GSS_S_COMPLETE && pUserPrinc != NULL && pServiceName != NULL ) {
		
		char	*pRecordName;
		
		printf( "Success:  Valid credentials provided by '%s' service '%s'\n", pUserPrinc, pServiceName );
	
// here you would compare the service to ensure it was the service you expected
//		if( strcasecmp( pServiceName, #expected service#) == 0 ) {
//				
//		}
		
		if( pRecordName = FindUserFromPrincipal( pUserPrinc ) ) {
			printf( "Success:  Found a user record for user - %s\n", pRecordName );
			
			// send a response
			SendToken( inSocket, "Accepted", sizeof("Accepted") );
			
			// need to free pRecordName cause it is returned
			free( pRecordName );
			pRecordName = NULL;
			
		} else {
			// send response
			SendToken( inSocket, "Rejected", sizeof("Rejected") );
			printf( "Failure:  No matching user record found for user\n" );
		}

	} else {
		if( pUserPrinc != NULL ) {
			printf( "Error:  Unable to authenticate user %s\n", pUserPrinc );
		}
		if( pServiceName != NULL ) {
			printf( "Error:  Unable to verify service %s\n", pServiceName );
		}
	}
	
	// clean up any context
	if( gssContext != GSS_C_NO_CONTEXT ) {
		gss_delete_sec_context( &iMinorStatus, gssContext, GSS_C_NO_BUFFER );
		gssContext = GSS_C_NO_CONTEXT;
	}
	
	// clean up any credentials
	if( gssCreds != GSS_C_NO_CREDENTIAL ) {
		gss_release_cred( &iMinorStatus, &gssCreds );
		gssCreds = GSS_C_NO_CREDENTIAL;
	}
	
	return 0;
}

#pragma mark -
#pragma mark Connection Negotiation

int HandleNewConnection( int inSocket )
{
	char	*pRecvToken		= NULL;
	int		iRecvTokenLen	= 0;
	int		iResult			= -1;
	
	// use ReadToken to get the first packet to identify the type of authentication
	iRecvTokenLen = ReadToken( inSocket, &pRecvToken );
	
	if( pRecvToken != NULL ) {

		if( strcasecmp(pRecvToken, "GSSAPI") == 0 ) {
			
			printf( "Attempt to authenticate with method GSSAPI received\n" );
			iResult = AuthGSSAPI( inSocket );
			
		} else if( strcasecmp(pRecvToken, "CRAM-MD5") == 0 ) {
			
			printf( "Attempt to authenticate with method CRAM-MD5 received\n" );
			iResult = HandleCRAMMD5( inSocket );
			
		} else if( strcasecmp(pRecvToken, "cleartext") == 0 ) {
			
			printf( "Attempt to authenticate with method cleartext received\n" );
			iResult = HandleCleartext( inSocket );
			
		} else {
			printf( "Unknown method received for authentication\n" );
		}
		
		// just print a blank line
		printf( "\n" );
		
		// free our token
		free( pRecvToken );
		pRecvToken = NULL;
	}
	
	return iResult;
}

int HandleCleartext( int inSocket )
{
	char	*pUsername		= NULL;
	char	*pPassword		= NULL;
	char	*pRecordname	= NULL;
	int		iResult			= -1;
	
	// we will be using ReadToken and WriteToken to send data back and forth for this
	// example, though not necessarily the most efficient, it is simplistic for this 
	// demonstration.
	
	// we expect 2 tokens, username then password
	
	// read the username from the network stream
	if( ReadToken(inSocket, &pUsername) > 0 ) {
		
		// read the password from the network stream
		if( ReadToken(inSocket, &pPassword) > 0 ) {
			
			iResult = AuthCleartext( pUsername, pPassword );
			
			// send a response
			if( iResult == eDSNoErr ) {
				SendToken( inSocket, "Accepted", sizeof("Accepted") );
			} else {
				SendToken( inSocket, "Rejected", sizeof("Rejected") );
			}
		}
	}
	
	// free any allocated strings
	if( pUsername != NULL ) {
		free( pUsername );
		pUsername = NULL;
	}
	
	if( pPassword != NULL ) {
		free( pPassword );
		pPassword = NULL;
	}

	if( pRecordname != NULL ) {
		free( pRecordname );
		pRecordname = NULL;
	}
	
	return iResult;
}

int HandleCRAMMD5( int inSocket )
{
	unsigned char	pChallenge[255]	= { 0, };
	char			pHostname[128]	= { 0, };
	char			*pResponse		= NULL;
	char			*pUsername		= NULL;
	struct timeval	stCurrentTime;
	int				iResult			= -1;

	// we will be using ReadToken and WriteToken to send data back and forth for this
	// example, though not necessarily the most efficient, it is simplistic for this 
	// demonstration.
	
	// Since CRAM-MD5 was requested, let's generate a challenge and send it to the client
	// using example method in RFC 1460, page 12.
	gethostname( pHostname, 127 );
	gettimeofday( &stCurrentTime, NULL ); // assume no error occurred
	snprintf( pChallenge, 255, "<%ld.%ld@%s>", (long) getpid(), stCurrentTime.tv_sec, pHostname );
	
	printf( "Sending challenge %s\n", pChallenge );
	
	// send our challenge to the client
	if( SendToken(inSocket, pChallenge, strlen(pChallenge)) > 0 ) {
		
		// now wait for username and response to return
		if( ReadToken(inSocket, &pUsername) > 0 ) {
			
			if( ReadToken(inSocket, &pResponse) > 0 ) {

				// here is where we authenticate the user using Open Directory
				iResult = AuthCRAMMD5( pUsername, pChallenge, pResponse );
				
				// send a response
				if( iResult == eDSNoErr ) {
					SendToken( inSocket, "Accepted", sizeof("Accepted") );
				} else {
					SendToken( inSocket, "Rejected", sizeof("Rejected") );
				}
			}
		}
	}
	
	// free up any strings we allocated
	if( pUsername != NULL ) {
		free( pUsername );
		pUsername = NULL;
	}

	if( pResponse != NULL ) {
		free( pResponse );
		pResponse = NULL;
	}

	return iResult;
}

#pragma mark -
#pragma mark Main Entry

// this demostration application will receive a authentication method type and some value following
// it to attempt to authenticate a user. 

int main( int argc, char *argv[] )
{
	int				iPort;
	int				iSocket;
	
	if( argc != 2 ) {
		printf( "Usage:  demoserver <port #>\n" );
		return 1;
	}
	
	if( geteuid() != 0 ) {
		printf( "Warning:  Tool must be run as EUID = 0 to use Kerberos/GSSAPI\n" );
	}
	
	// if we were relying on Kerberos for this service, we could pre-flight our 
	// service principal before we start the service, or just let GSSAPI determine 
	// at the time of connection
	//
	// we have opted for at time of connection, because we don't know what we
	// are using uses for service principal

	// let's convert the string provided for the port into a number
	iPort = strtol( argv[1], NULL, 10 );
	
	// don't allow privileged ports for this test
	if( iPort < 1024 ) {
		printf( "Error:  Port value must be greater than 1024\n" );
	}
	
	// this demonstration code will wait for an authentication identification code
	//    GSSAPI
	//    CRAM-MD5
	//    cleartext
	// Once method is determined, it will attempt to authenticate the user
	
	iSocket = BindSocket( iPort );
	if( iSocket > 0 ) {
		
		// just informative
		printf( "Listening on port %d\n", iPort );

		// go into a loop until we are killed
		while( 1 ) {

			int	iAccept;
			
			// Accept a TCP connection
			if( (iAccept = accept(iSocket, NULL, 0)) < 0) {
				printf("Error:  Couldn't accept connection");
				continue;
			}
			
			// once we have a connection, go to our request-response loop
			if( HandleNewConnection( iAccept ) == 0 ) {
				
				// if we had no error we could go do something, but in this case we will
				// just close the connection

				close( iAccept );
				iAccept = 0;
				
			} else {

				// close our socket
				close( iAccept );
				iAccept = 0;
			}
		}
		
		// close our socket
		close( iSocket );
		iSocket = 0;

	} else {
		printf( "Error:  Unable to listen on port %d\n", iPort );
		return 1;
	}

	return 0;
}

#pragma mark -
#pragma mark Support Functions

// helper routine to fine a user's official RecordName
//
char *FindUserFromPrincipal( char *inPrincipal )
{
	// now let's parse the name and see if we can find a valid user..
	tDirReference		dsRef			= 0;
	tDirNodeReference	dsSearchNodeRef	= 0;
	tDirStatus			dsStatus;
	char				*pRecordName	= NULL;
	char				*pNodeName		= NULL;
	char				*pUsername		= strdup( inPrincipal );
	char				*pAtSymbol		= strchr( pUsername, '@' );
	
	// need to parse just the name of the user, since principal is user@REALM
	if( pAtSymbol != NULL ) {
		*pAtSymbol = '\0';
	}
	
	// Open Directory Services reference
	dsStatus = dsOpenDirService( &dsRef );
	if( dsStatus == eDSNoErr ) {
		
		// use utility function in DSUtility.h to open the search node
		dsStatus = OpenSearchNode( dsRef, &dsSearchNodeRef );
		if( dsStatus == eDSNoErr ) {
			
			// use utility function in DSUtility.h to locate the user information
			dsStatus = LocateUserRecordNameAndNode( dsRef, dsSearchNodeRef, pUsername, &pRecordName, &pNodeName );
			if( dsStatus == eDSNoErr ) {
				
				// need to free any node name that may have been returned
				if( pNodeName != NULL ) {
					free( pNodeName );
					pNodeName = NULL;
				}
			}
			
			// close the search node cause we are done here
			dsCloseDirNode( dsSearchNodeRef );
			dsSearchNodeRef = 0;
			
		} else {
			printf( "Unable to locate and open the Search node to verify user\n" );
		}
		
		// need to close Directory Services at this point
		dsCloseDirService( dsRef );
		dsRef = 0;
	}
	
	if( pUsername != NULL ) {
		free( pUsername );
		pUsername = NULL;
	}
	
	return pRecordName;
}

