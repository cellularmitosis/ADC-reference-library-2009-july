/*
 
 File: DSUtility.c
 
 Abstract: Utility functions for common authentication/lookup with Directory
           Services
 
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#pragma mark Internal Prototypes

tDirStatus AppendStringToBuffer( tDataBufferPtr inBuffer, const char *inString, long inLength );

#pragma mark -
#pragma mark Directory Utility routines

tDirStatus OpenSearchNode( tDirReference inDSRef, tDirNodeReference *outNodeRef )
{
	tDataBufferPtr		pWorkingBuffer	= NULL;
	tDataListPtr		pSearchNode		= NULL;
	tDirStatus			dsStatus;
	tContextData		dsContext		= NULL;
	unsigned long		ulReturnCount	= 0;

	// verify none of the parameters are NULL, if so return an eDSNullParameter
	if( outNodeRef == NULL || inDSRef == 0 ) {
		return eDSNullParameter;
	}
	
	// allocate a buffer to hold return information, defaulting to 4k
	pWorkingBuffer = dsDataBufferAllocate( inDSRef, 4096 );
	if( pWorkingBuffer == NULL ) {
		return eMemoryAllocError;
	}
	
	// locate the name of the search node
	dsStatus = dsFindDirNodes( inDSRef, pWorkingBuffer, NULL, eDSSearchNodeName, &ulReturnCount, &dsContext );
	if( dsStatus == eDSNoErr ) {
		// pass 1 for node index since there should only be one value
		dsStatus = dsGetDirNodeName( inDSRef, pWorkingBuffer, 1, &pSearchNode );
	}
	
	// if we ended up with a context, we should release it
	if( dsContext != NULL ) {
		dsReleaseContinueData( inDSRef, dsContext );
		dsContext = NULL;
	}
	
	// release the current working buffer
	if( pWorkingBuffer != NULL ) {
		dsDataBufferDeAllocate( inDSRef, pWorkingBuffer );
		pWorkingBuffer = NULL;
	}
	
	// open search node
	if( dsStatus == eDSNoErr && pSearchNode != NULL ) {
		dsStatus = dsOpenDirNode( inDSRef, pSearchNode, outNodeRef );
	} 
	
	// deallocate the tDataListPtr item used to locate the Search node
	if( pSearchNode != NULL ) {
		dsDataListDeallocate( inDSRef, pSearchNode );
		
		// need to free pointer as dsDataListDeallocate does not free it, just the list items
		free( pSearchNode );
		pSearchNode = NULL;
	}
	
	return dsStatus;
} // OpenSearchNode

tDirStatus LocateUserRecordNameAndNode( tDirReference inDSRef, tDirNodeReference inSearchNode, const char *inUsername,
										char **outRecordName, char **outNodeName )
{
	tDirStatus		dsStatus		= eDSRecordNotFound;
	tDataListPtr	pAttribsToGet	= NULL;
	tDataListPtr	pRecTypeList	= NULL;
	tDataListPtr	pRecNameList	= NULL;
	tDataBufferPtr	pSearchBuffer	= NULL;
	unsigned long	ulRecCount		= 0;	// do not limit the number of records we are expecting
	unsigned long	ulBufferSize	= 2048;	// start with a 2k buffer for any data
	
	// ensure none of the parameters are NULL
	if( inDSRef == 0 || inSearchNode == 0 || inUsername == NULL || outRecordName == NULL || outNodeName == NULL ) {
		return eDSNullParameter;
	}
	
	// we will want the actual record name and the name of the node where the user resides
	pAttribsToGet = dsBuildListFromStrings( inDSRef, kDSNAttrRecordName, kDSNAttrMetaNodeLocation, NULL );
	if( pAttribsToGet == NULL ) {
		dsStatus = eMemoryAllocError;
		goto cleanup;
	}
	
	// build a list to search for user record
	pRecNameList = dsBuildListFromStrings( inDSRef, inUsername, NULL );
	if( pRecNameList == NULL ) {
		dsStatus = eMemoryAllocError;
		goto cleanup;
	}
	
	// build a list of record types to search, in this case users
	pRecTypeList = dsBuildListFromStrings( inDSRef, kDSStdRecordTypeUsers, NULL);
	if( pRecTypeList == NULL ) {
		dsStatus = eMemoryAllocError;
		goto cleanup;
	}
	
	// allocate a working buffer, this may be grown if we receive a eDSBufferTooSmall error
	pSearchBuffer = dsDataBufferAllocate( inDSRef, ulBufferSize );
	if( pSearchBuffer == NULL ) {
		dsStatus = eMemoryAllocError;
		goto cleanup;
	}
	
	// now search for the record using dsGetRecordList
	dsStatus = dsGetRecordList( inSearchNode, pSearchBuffer, pRecNameList, eDSExact, pRecTypeList, 
								pAttribsToGet, 0, &ulRecCount, NULL );
	
	// if there was not an error and we found only 1 record for this user
	if( dsStatus == eDSNoErr && ulRecCount == 1 ) {
		tAttributeListRef	dsAttributeListRef	= 0;
		tRecordEntryPtr		dsRecordEntryPtr	= 0;
		int					ii;

		// Get the 1st record entry from the buffer since we only expect 1 result
		dsStatus = dsGetRecordEntry( inSearchNode, pSearchBuffer, 1, &dsAttributeListRef, &dsRecordEntryPtr );
		if (dsStatus == eDSNoErr)
		{
			// loop through the attributes in the record to get the data we requested
			// all indexes with Open Directory APIs start with 1 not 0
			for (ii = 1 ; ii <= dsRecordEntryPtr->fRecordAttributeCount; ii++)
			{
				tAttributeEntryPtr		dsAttributeEntryPtr			= NULL;
				tAttributeValueEntryPtr	dsAttributeValueEntryPtr	= NULL;
				tAttributeValueListRef	dsAttributeValueListRef		= 0;
				
				// get the attribute entry from the record
				dsStatus = dsGetAttributeEntry( inSearchNode, pSearchBuffer, dsAttributeListRef, ii, 
												&dsAttributeValueListRef, &dsAttributeEntryPtr );
				
				// get the value from the attribute if we were successful at getting an entry
				if( dsStatus == eDSNoErr ) {
					dsStatus = dsGetAttributeValue( inSearchNode, pSearchBuffer, 1, 
													dsAttributeValueListRef, &dsAttributeValueEntryPtr );
				}
				
				// if we were still sucessful, see which attribute we were getting and fill in the return
				// values appropriately
				if( dsStatus == eDSNoErr ) {
					
					// always check for the specific attributes, since a plugin is not restricted from
					// returning more data than you requested
					
					// check the signature of the attribute and see if it is the metanode location
					if (strcmp(dsAttributeEntryPtr->fAttributeSignature.fBufferData, kDSNAttrMetaNodeLocation) == 0) {
						*outNodeName = (char *) calloc( dsAttributeValueEntryPtr->fAttributeValueData.fBufferSize + 1, sizeof(char));
						if( *outNodeName != NULL ) {
							strncpy( *outNodeName, dsAttributeValueEntryPtr->fAttributeValueData.fBufferData, dsAttributeValueEntryPtr->fAttributeValueData.fBufferSize);
						}
						
					// if not, see if it is the Record Name
					} else if (strcmp(dsAttributeEntryPtr->fAttributeSignature.fBufferData, kDSNAttrRecordName) == 0) {
						*outRecordName = (char *) calloc( dsAttributeValueEntryPtr->fAttributeValueData.fBufferSize + 1, sizeof(char));
						if( *outRecordName != NULL ) {
							strncpy( *outRecordName, dsAttributeValueEntryPtr->fAttributeValueData.fBufferData, 
									 dsAttributeValueEntryPtr->fAttributeValueData.fBufferSize );
						}
					}
				}

				// close any value list references that may have been opened
				if( dsAttributeValueListRef != 0 ) {
					dsCloseAttributeValueList( dsAttributeValueListRef );
					dsAttributeValueListRef = 0;
				}
				
				// free the attribute value entry if we got an entry
				if( dsAttributeValueEntryPtr != NULL ) {
					dsDeallocAttributeValueEntry( inDSRef, dsAttributeValueEntryPtr );
					dsAttributeValueEntryPtr = NULL;
				}
				
				// free the attribute entry itself as well
				if( dsAttributeEntryPtr != NULL ) {
					dsDeallocAttributeEntry( inDSRef, dsAttributeEntryPtr );
					dsAttributeEntryPtr = NULL;
				}
			}
			
			// close any reference to attribute list
			if( dsAttributeListRef != 0 ) {
				dsCloseAttributeList( dsAttributeListRef );
				dsAttributeListRef = 0;
			}
			
			// deallocate the record entry
			if( dsRecordEntryPtr != NULL ) {
				dsDeallocRecordEntry( inDSRef, dsRecordEntryPtr );
				dsRecordEntryPtr = NULL;
			}
		}
	} else if( dsStatus == eDSNoErr && ulRecCount > 1 ) {
		// if we have more than 1 user, then we shouldn't attempt to authenticate
		// we chose to return eDSAuthInvalidUserName as an error since we can't distinguish
		// the specific user to return
		dsStatus = eDSAuthInvalidUserName;
	}

cleanup:
	// if we allocated pAttribsToGet, we need to clean up
	if( pAttribsToGet != NULL ) {
		dsDataListDeallocate( inDSRef, pAttribsToGet );

		// need to free pointer as dsDataListDeallocate does not free it, just the list items
		free( pAttribsToGet );
		pAttribsToGet = NULL;
	}

	// if we allocated pRecTypeList, we need to clean up
	if( pRecTypeList != NULL ) {
		dsDataListDeallocate( inDSRef, pRecTypeList );
		
		// need to free pointer as dsDataListDeallocate does not free it, just the list items
		free( pRecTypeList );
		pRecTypeList = NULL;
	}
	
	// if we allocated pRecNameList, we need to clean up
	if( pRecNameList != NULL ) {
		dsDataListDeallocate( inDSRef, pRecNameList );
		
		// need to free pointer as dsDataListDeallocate does not free it, just the list items
		free( pRecNameList );
		pRecNameList = NULL;
	}
	
	// if we allocated pSearchBuffer, we need to clean up
	if( pSearchBuffer != NULL ) {
		dsDataBufferDeAllocate( inDSRef, pSearchBuffer );
		pSearchBuffer = NULL;
	}
	
	return dsStatus;
} // LocateUserRecordNameAndNode

#pragma mark -
#pragma mark Authentication Routines

tDirStatus DoPasswordAuth( tDirReference inDSRef, tDirNodeReference inNodeRef, const char *inAuthMethod,
						   const char *inRecordName, const char *inPassword )
{
	tDirStatus		dsStatus		= eDSAuthFailed;
	tDataNodePtr	pAuthMethod		= NULL;
	tDataBufferPtr	pAuthStepData	= NULL;
	tDataBufferPtr	pAuthRespData	= NULL;
	tContextData	pContextData	= NULL;
	
	// if any of our parameters are NULL, return a NULL parameter
	// if a password is not set for a user, an empty string should be sent for the password
	if( inDSRef == 0 || inNodeRef == 0 || inRecordName == NULL || inPassword == NULL ) {
		return eDSNullParameter;
	}
	
	// since this is password based, we can only support password-based methods
	if( strcmp(inAuthMethod, kDSStdAuthNodeNativeNoClearText) == 0 || 
		strcmp(inAuthMethod, kDSStdAuthNodeNativeClearTextOK) == 0 ||
		strcmp(inAuthMethod, kDSStdAuthClearText) == 0 ||
		strcmp(inAuthMethod, kDSStdAuthCrypt) == 0 ) {
		
		// turn the specified method into a tDataNode
		pAuthMethod = dsDataNodeAllocateString( inDSRef, inAuthMethod );
		if( pAuthMethod == NULL ) {
			dsStatus = eMemoryAllocError;
			goto cleanup;
		}

		// allocate a buffer large enough to hold all the username and password plus length bytes
		pAuthStepData = dsDataBufferAllocate( inDSRef, 4 + strlen(inRecordName) + 4 + strlen(inPassword) );
		if( pAuthStepData == NULL ) {
			dsStatus = eMemoryAllocError;
			goto cleanup;
		}
		
		// allocate a buffer for the out step data even though we don't expect anything, 
		// it is a required parameter
		pAuthRespData = dsDataBufferAllocate( inDSRef, 128 );
		if( pAuthRespData == NULL ) {
			dsStatus = eMemoryAllocError;
			goto cleanup;
		}
		
		// now place the username and password into the buffer
		AppendStringToBuffer( pAuthStepData, inRecordName, strlen(inRecordName) );
		AppendStringToBuffer( pAuthStepData, inPassword, strlen(inPassword) );
		
		// attemp the authentication
		dsStatus = dsDoDirNodeAuth( inNodeRef, pAuthMethod, 1, pAuthStepData, pAuthRespData, &pContextData );
		
	} else {
		// otherwise, return a parameter error
		dsStatus = eDSAuthParameterError;
	}
	
cleanup:
		
	// release pContextData if we had continue data
	if( pContextData != NULL ) {
		dsReleaseContinueData( inDSRef, pContextData );
		pContextData = NULL;
	}

	// deallocate memory for pAuthRespData if it was allocated
	if( pAuthRespData != NULL ) {
		dsDataNodeDeAllocate( inDSRef, pAuthRespData );
		pAuthRespData = NULL;
	}
	
	// deallocate memory for pAuthStepData if it was allocated
	if( pAuthStepData != NULL ) {
		dsDataBufferDeAllocate( inDSRef, pAuthStepData );
		pAuthStepData = NULL;
	}
	
	// deallocate memory for pAuthMethod if it was allocated
	if( pAuthMethod != NULL ) {
		dsDataNodeDeAllocate( inDSRef, pAuthMethod );
		pAuthMethod = NULL;
	}
	
	return dsStatus;
} // DoPasswordAuth


tDirStatus DoChallengeResponseAuth( tDirReference inDSRef, tDirNodeReference inNodeRef, const char *inAuthMethod,
									const char *inRecordName, const char *inChallenge, long inChallengeLen, 
									const char *inResponse, long inResponseLen )
{
	tDirStatus		dsStatus		= eDSAuthFailed;
	tDataNodePtr	pAuthMethod		= NULL;
	tDataBufferPtr	pAuthStepData	= NULL;
	tDataBufferPtr	pAuthRespData	= NULL;
	tContextData	pContextData	= NULL;
	
	// if any of our parameters are NULL, return a NULL parameter
	// if a password is not set for a user, an empty string should be sent for the password
	if( inDSRef == 0 || inNodeRef == 0 || inRecordName == NULL || inChallenge == NULL ) {
		return eDSNullParameter;
	}

	// we will only authenticate using methods which are challenge-response based
	if( strcmp(inAuthMethod, kDSStdAuthCRAM_MD5) == 0 || 
		strcmp(inAuthMethod, kDSStdAuthAPOP) == 0 ||
		strcmp(inAuthMethod, kDSStdAuthCHAP) == 0 ||
		strcmp(inAuthMethod, kDSStdAuthSMB_LM_Key) == 0 ||
		strcmp(inAuthMethod, kDSStdAuthSMB_NT_Key) == 0 ) {
		
		// turn the specified method into a tDataNode
		pAuthMethod = dsDataNodeAllocateString( inDSRef, inAuthMethod );
		if( pAuthMethod == NULL ) {
			dsStatus = eMemoryAllocError;
			goto cleanup;
		}
		
		// allocate a buffer large enough to hold all the username and password plus length bytes
		pAuthStepData = dsDataBufferAllocate( inDSRef, 4 + strlen(inRecordName) + 4 + inChallengeLen + 4 + inResponseLen );
		if( pAuthStepData == NULL ) {
			dsStatus = eMemoryAllocError;
			goto cleanup;
		}
		
		// allocate a buffer for the out step data even though we don't expect anything, 
		// it is a required parameter
		pAuthRespData = dsDataBufferAllocate( inDSRef, 128 );
		if( pAuthRespData == NULL ) {
			dsStatus = eMemoryAllocError;
			goto cleanup;
		}
		
		// now place the username and password into the buffer
		AppendStringToBuffer( pAuthStepData, inRecordName, strlen(inRecordName) );
		AppendStringToBuffer( pAuthStepData, inChallenge, inChallengeLen );
		AppendStringToBuffer( pAuthStepData, inResponse, inResponseLen );

		// attempt the authentication
		dsStatus = dsDoDirNodeAuth( inNodeRef, pAuthMethod, 1, pAuthStepData, pAuthRespData, &pContextData );
		
	} else {
		// otherwise, return a parameter error
		dsStatus = eDSAuthParameterError;
	}
	
cleanup:
	
	// release pContextData if we had continue data
	if( pContextData != NULL ) {
		dsReleaseContinueData( inDSRef, pContextData );
		pContextData = NULL;
	}
	
	// deallocate memory for pAuthRespData if it was allocated
	if( pAuthRespData != NULL ) {
		dsDataNodeDeAllocate( inDSRef, pAuthRespData );
		pAuthRespData = NULL;
	}
	
	// deallocate memory for pAuthStepData if it was allocated
	if( pAuthStepData != NULL ) {
		dsDataBufferDeAllocate( inDSRef, pAuthStepData );
		pAuthStepData = NULL;
	}
	
	// deallocate memory for pAuthMethod if it was allocated
	if( pAuthMethod != NULL ) {
		dsDataNodeDeAllocate( inDSRef, pAuthMethod );
		pAuthMethod = NULL;
	}
	
	return dsStatus;
} // DoChallengeResponseAuth

#pragma mark -
#pragma mark Support Functions

tDirStatus AppendStringToBuffer( tDataBufferPtr inBuffer, const char *inString, long inLength )
{
	tDirStatus	dsStatus	= eDSBufferTooSmall;
	
	// ensure neither of our parameters are NULL
	if( inString == NULL || inBuffer == NULL ) {
		return eDSNullParameter;
	}
	
	// check to see if we have enough room in the buffer for the string and the 4 byte length
	if( inBuffer->fBufferSize >= (inBuffer->fBufferLength + 4 + inLength) ) {
		
		char	*pBufferEnd = inBuffer->fBufferData + inBuffer->fBufferLength;
		
		// prepend the data with the length of the string
		bcopy( &inLength, pBufferEnd, sizeof(long) );
		pBufferEnd += sizeof( long );
		
		// now add the string to the buffer
		bcopy( inString, pBufferEnd, inLength );
		
		// increase the buffer accordingly
		inBuffer->fBufferLength += 4 + inLength;
		
		// set successful error status
		dsStatus = eDSNoErr;
	}
	
	return dsStatus;
} // AppendStringToBuffer

