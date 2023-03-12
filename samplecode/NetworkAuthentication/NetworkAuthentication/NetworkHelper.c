/*
 
 File: NetworkHelper.c
 
 Abstract: Helper code for demonstration applications only
 
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

/*
 *  This is helper code that is not guaranteed to be complete or sufficient for a normal
 *  application.  This is purely for use in the example application.  It does not deal
 *  with network byte order.  Assumes packets are sent as is, with no interpretation.
 *
 */

#include "NetworkHelper.h"

#include <unistd.h>
#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static int ReadData( int iSocket, char *outData, int inBytesToRead );
static int WriteData( int iSocket, char *inData, int inBytesToWrite );

int BindSocket( int inPort )
{
	struct sockaddr_in	stSocketAddr;
	int					returnSocket;
	int					iFlag			= 1;                 
	
	// create a socket for our use
	returnSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( returnSocket < 0) {
		printf( "Error:  Unable to create socket\n" );
		return -1;
	}
	
	// let's set it so the socket can be re-used right away for purposes of testing
	setsockopt( returnSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&iFlag, sizeof(iFlag) );
	
	// set the socket address information
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_port = htons( inPort );
	stSocketAddr.sin_addr.s_addr = INADDR_ANY;	
	
	// now bind to the socket
	if( bind(returnSocket, (struct sockaddr *) &stSocketAddr, sizeof(stSocketAddr)) < 0) {
		printf( "Error:  Unable to socket to port %d, must be in use\n", inPort );
		close( returnSocket ); // close out the socket
		return -1;
	}
	
	// now listen on the socket
	if( listen(returnSocket, 5) < 0 ) {
		printf( "Error:  Unable to listen on socket" );
		close( returnSocket ); // close out the socket
		return -1;
	} 
	
	return returnSocket;
}

int ConnectTo( char *inHostname, int inPort )
{
	struct sockaddr_in	stAddress;
	struct hostent		*pHostEntry;
	int					iReturnSocket;
    
	pHostEntry = gethostbyname( inHostname );
	if( pHostEntry == NULL ) {
		printf( "Unknown host: %s\n", inHostname );
		return -1;
	}
	
	stAddress.sin_family = pHostEntry->h_addrtype;
	bcopy( pHostEntry->h_addr, (char *)&stAddress.sin_addr, sizeof(stAddress.sin_addr) );
	stAddress.sin_port = htons( inPort );
	
	iReturnSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( iReturnSocket > 0 ) {
		if( connect(iReturnSocket, (struct sockaddr *)&stAddress, sizeof(stAddress)) < 0) {
			close( iReturnSocket );
			iReturnSocket = -1;
		}
	} else {
		iReturnSocket = -1;
	}
	
	return iReturnSocket;
}

int ReadToken( int iSocket, char **outData )
{
	// first read the length of the data
	long	lLength		= 0;
	int		iBytesRead	= ReadData( iSocket, (char *) &lLength, sizeof(lLength) );
	
	// if we read a length, let's now get the data
	if( iBytesRead == sizeof(lLength) ) {
		
		// allocate memory based on the length we received, within reason
		if( lLength < 65536 ) {
			*outData = (char *) calloc( lLength, sizeof(char) );
			lLength = ReadData( iSocket, *outData, lLength );
		} else {
			lLength = -1;
		}
	} else {
		// network error?
		printf( "Error reading length byte from token, value was %d\n", iBytesRead );
		lLength = -1;
	}
	
	return lLength;
}

int SendToken( int iSocket, char *inData, long inBytesToWrite )
{
	// default to error state
	int	iReturnValue = -1;
	
	// let's write out the length byte first..
	if( WriteData( iSocket, (char *)&inBytesToWrite, sizeof(inBytesToWrite) ) == sizeof(inBytesToWrite) ) {
		
		// if we succeeded, let's write out the data
		iReturnValue = WriteData( iSocket, inData, inBytesToWrite );
	}
	
	return iReturnValue;
}

#pragma mark -
#pragma mark Internal Support routines

int ReadData( int iSocket, char *outData, int inBytesToRead )
{
	fd_set			readFDS;
	int				iBytesRead		= 0;
	struct timeval	waitTime;
	
	waitTime.tv_sec = 30;
	waitTime.tv_usec = 0;
	
	while( inBytesToRead > 0 ) {

		FD_ZERO( &readFDS );
		FD_SET( iSocket, &readFDS );

		if( select(FD_SETSIZE, &readFDS, NULL, NULL, &waitTime ) <= 0 ||
			FD_ISSET(iSocket, &readFDS) == 0 ) {
			
			break;
		}
		
		int iBytes = recv( iSocket, &outData[iBytesRead], inBytesToRead, 0 );
		
		if( iBytes < 0 ) {
			
			// did we get interrupted, if so continue
			if( errno == EINTR )
				continue;
			
			// otherwise errored
			break;
		} else if (iBytes == 0) {
			break;
		}
		
		iBytesRead += iBytes;
		inBytesToRead -= iBytes;
	}
	
	return iBytesRead;
}

int WriteData( int iSocket, char *inData, int inBytesToWrite )
{
	int		iWroteBytes	= 0;
	
	while( inBytesToWrite > 0 ) {
		
		int iWrote = send( iSocket, &inData[iWroteBytes], inBytesToWrite, 0 );
		if( iWrote < 0 ) {
			if( errno == EINTR )
				continue;
			break;
		} else if( iWrote == 0 ) {
			break;
		}
		inBytesToWrite -= iWrote;
		iWroteBytes += iWrote;
	}

	return iWroteBytes;
}
