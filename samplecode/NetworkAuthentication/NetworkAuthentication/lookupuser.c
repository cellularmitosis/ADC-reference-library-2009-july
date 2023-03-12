/*
 
 File: lookupuser.c
 
 Abstract: Demonstration of looking up user with Directory Services
 
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

#include "DSUtility.h"

#include <DirectoryService/DirectoryService.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#pragma mark -
#pragma mark Routines

int main ( int argc, const char * argv[] )
{
	tDirReference		dsRef			= 0;
	tDirNodeReference	dsSearchNodeRef	= 0;
	tDirStatus			dsStatus;
	char				*pRecordName	= NULL;
	char				*pNodeName		= NULL;
	
	if( argc != 2 ) {
		printf( "Usage:  lookupuser <username>\n" );
		return 1;
	}
	
	// Key steps to locating a user:
	//		- Open Directory Service reference
	//		- Locate and open the Search Node
	//		- Locate the user's official RecordName and Directory Node based on the username provided
	
	// Open Directory Services reference
	dsStatus = dsOpenDirService( &dsRef );
	if( dsStatus == eDSNoErr ) {
		
		// use utility function in DSUtility.h to open the search node
		dsStatus = OpenSearchNode( dsRef, &dsSearchNodeRef );
		if( dsStatus == eDSNoErr ) {
			
			// use utility function in DSUtility.h to locate the user information
			dsStatus = LocateUserRecordNameAndNode( dsRef, dsSearchNodeRef, argv[1], &pRecordName, &pNodeName );
			if( dsStatus == eDSNoErr ) {
				
				// we should have values available, but let's check to be sure
				if( pNodeName != NULL && pNodeName[0] != '\0' && 
					pRecordName != NULL && pRecordName[0] != '\0' )
				{
					printf( "Located user '%s' = '%s' on node '%s'\n", argv[1], pRecordName, pNodeName );
				}
				else
				{
					printf( "Unable to locate user '%s'\n", argv[1] );
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
	
    return 0;
}
