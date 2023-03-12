/*
 
 File: DSUtility.h
 
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

#include <DirectoryService/DirectoryService.h>

/*!
    @function	OpenSearchNode
    @abstract   Will locate and open the search node
    @discussion Will attempt to locate and open a tiDirNodeReference to the Search node
    @param      inDSRef Existing tDirReference from dsOpenDirService
	@param		outNodeRef A pointer to a tDirNodeReference that will be filled with valid reference if possible
	@result     Will return eDSNoErr if successful, otherwise any error that may have occurred
*/
tDirStatus OpenSearchNode( tDirReference inDSRef, tDirNodeReference *outNodeRef );

/*!
    @function	LocateUserRecordNameAndNode
    @abstract   Locate the user requested
    @discussion Will locate the user requested, returning an error if more than 1 record is found.
				Expecting only 1 record on server process for a name is appropriate.  Client processes
				interacting may not want to limit to 1 user, but return a list of users to pick from.
	@param      inDSRef Existing tDirReference from dsOpenDirService
	@param		inUsername The username to be located in the Directory
	@param		ouRecordName The official record name for the user to be used for authentications, etc.
	@param		outNodeRef A reference to the Node that contains the user record
    @result     Will return eDSNoErr if successful, otherwise any error that may have occurred
*/
tDirStatus LocateUserRecordNameAndNode( tDirReference inDSRef, tDirNodeReference inSearchNode, const char *inUsername, 
										char **outRecordName, char **outNodeName );

/*!
    @function	DoPasswordAuth
    @abstract   Will take a record name and cleartext password to authenticate a user
    @discussion Authenticates a recordname with the supplied password.  This does not mean the password
				will be sent in the clear, it just means the password is cleartext-based.
	@param      inDSRef Existing tDirReference from dsOpenDirService
	@param		inNodeRef The node found with LocateUserRecordNameAndNode for the user to be authenticated
	@param		inAuthMethod The method being used (e.g., kDSStdAuthNodeNativeNoClearText)
	@param		inRecordName The record name returned by LocateUserRecordNameAndNode for the specific user
	@param		inPassword Is a cleartext password supplied by the user
	@result     Will return eDSNoErr if successful, otherwise any error that may have occurred
*/
tDirStatus DoPasswordAuth( tDirReference inDSRef, tDirNodeReference inNodeRef, const char *inAuthMethod,
						   const char *inRecordName, const char *inPassword );

/*!
    @function	DoChallengeResponseAuth
    @abstract   Will handle standard challenge response methods
    @discussion Authenticates a record
	@param      inDSRef Existing tDirReference from dsOpenDirService
	@param		inNodeRef The node found with LocateUserRecordNameAndNode for the user to be authenticated
	@param		inAuthMethod The method being used (e.g., kDSStdAuthNodeNativeNoClearText)
	@param		inRecordName The record name returned by LocateUserRecordNameAndNode for the specific user
	@param		inChallenge Is the challenge used to calculate the response
	@param		inChallengeLen Is the length of the challenge used to calculate the response
	@param		inResponse Is the response calculated from the challenge and user's password
	@param		inResponseLen Is the length of the response
	@result     Will return eDSNoErr if successful, otherwise any error that may have occurred
*/
tDirStatus DoChallengeResponseAuth( tDirReference inDSRef, tDirNodeReference inNodeRef, const char *inAuthMethod,
									const char *inRecordName, const char *inChallenge, long inChallengeLen, 
									const char *inResponse, long inResponseLen );
