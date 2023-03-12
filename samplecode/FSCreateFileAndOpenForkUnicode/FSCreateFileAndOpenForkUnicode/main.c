/*

File: main.c

Abstract: Tool which demonstrates the use of FSCreateFileAndOpenForkUnicode to
create a file with restricted access from the outset but still allow the creating
process a read/write path to the new item's data fork.  This sample uses access control
lists to perform its task.  The target volume needs to have extended security enabled.
FSCreateFileAndOpenForkUnicode allows for the creation of a file with restricted access
on disk while still allowing for a read/write path for the creator.  In most cases it will
be more efficient in terms of system calls than FSCreateFileUnicode followed by FSOpenFork.

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

#include <CoreServices/CoreServices.h>
#include <libgen.h>
#include <stdio.h>
#include <sys/acl.h>
#include <uuid/uuid.h>
#include <membership.h>

static OSStatus ConvertUTF8NameToHFSUniStr255(const char *inName, HFSUniStr255 *uniStr)
{
	OSStatus err = paramErr;
	CFStringRef	tempString;
	
	tempString = CFStringCreateWithCString(kCFAllocatorDefault, inName, kCFStringEncodingUTF8);
	if (tempString)
	{
		err = FSGetHFSUniStrFromString(tempString, uniStr);
		CFRelease(tempString);
	}
	
	return err;
}

// return true if the volume supports extended security, false otherwise.
// If any errors are encountered return false.
static Boolean TargetVolumeSupportsExtendedSecurity(const FSRef *ref)
{
	Boolean			supportsExtendedSecurity = false;
	OSStatus		err;
	FSCatalogInfo	info;

	err = FSGetCatalogInfo(ref, kFSCatInfoVolume, &info, NULL, NULL, NULL);
	if (err == noErr)
	{	
		HParamBlockRec		paramBlock;
		GetVolParmsInfoBuffer	volParmsInfoBuffer;

		paramBlock.ioParam.ioNamePtr = NULL;
		paramBlock.ioParam.ioVRefNum = info.volume;
		paramBlock.ioParam.ioBuffer = (Ptr)&volParmsInfoBuffer;
		paramBlock.ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);
		err = PBHGetVolParmsSync(&paramBlock);
		if ((err == noErr) && (volParmsInfoBuffer.vMVersion >= 3) && (volParmsInfoBuffer.vMExtendedAttributes & (1 << bSupportsExtendedFileSecurity)))
			supportsExtendedSecurity = true;
	}
	return supportsExtendedSecurity;
}

// create an ACL with a single ACE which allows the current user to read the object
static acl_t CreateReadOnlyForCurrentUserACL(void)
{
	acl_t	theACL = NULL;
	uuid_t	theUUID;
	int		result;
	
	result =  mbr_uid_to_uuid(geteuid(), theUUID);	// need the uuid for the ACE
	if (result == 0)
	{
		theACL = acl_init(1);	// create an empty ACL
		if (theACL)
		{
			Boolean freeACL = true;
			acl_entry_t newEntry;
			acl_permset_t newPermSet;

			result = acl_create_entry_np(&theACL, &newEntry, ACL_FIRST_ENTRY);
			if (result == 0)
			{	// allow
				result = acl_set_tag_type(newEntry, ACL_EXTENDED_ALLOW);
				if (result == 0)
				{	// the current user
					result = acl_set_qualifier(newEntry, (const void *)theUUID);
					if (result == 0)
					{
						result = acl_get_permset(newEntry, &newPermSet);
						if (result == 0)
						{	// to read data
							result = acl_add_perm(newPermSet, ACL_READ_DATA);
							if (result == 0)
							{	
								result = acl_set_permset(newEntry, newPermSet);
								if (result == 0)
									freeACL = false;	// all set up and ready to go
							}
						}
					}
				}
			}
		
			if (freeACL)
			{
				acl_free(theACL);
				theACL = NULL;
			}
		}
	}
	return theACL;
}

// This routine will create an FSFileSecurityRef which specifies a mode of 0 (no access) and
// an ACL with a single ACE which allows the current user to read the file.
static FSFileSecurityRef CreateReadOnlyACENoOtherAccess(void)
{
	FSFileSecurityRef	fileSecRef;
	
	// create a new FSFileSecurityRef
	fileSecRef = FSFileSecurityCreate(kCFAllocatorDefault);
	if (fileSecRef)
	{
		OSStatus err;
		Boolean releaseRef = true;	// set to false if we succeed at setting everything and have a valid ref to return
		
		err = FSFileSecuritySetMode(fileSecRef, 0);
		if (err == noErr)
		{
			acl_t theACL;
			
			theACL = CreateReadOnlyForCurrentUserACL();
			if (theACL)
			{
				if (FSFileSecuritySetAccessControlList(fileSecRef, theACL) == noErr)
					releaseRef = false;
				acl_free(theACL);
			}
		}
			
		if (releaseRef)
		{
			CFRelease(fileSecRef);
			fileSecRef = NULL;
		}
	}
	return fileSecRef;
}

int main (int argc, const char * argv[]) {

	if (argc == 2)
	{
		char	*dirPath, *baseName;
		
		dirPath = dirname(argv[1]);
		if (dirPath != NULL)
		{
			FSRef	ref;
			OSStatus err;
			err = FSPathMakeRef((UInt8 *)dirPath, &ref, NULL);
			if (err == noErr)
			{
				HFSUniStr255 uniStr;
				
				baseName = basename(argv[1]);
				if (baseName != NULL)
				{
					err = ConvertUTF8NameToHFSUniStr255(baseName, &uniStr);
					if (err == noErr)
					{
						// check to see if target volume supports extended security - do not continue if it does not
						if (TargetVolumeSupportsExtendedSecurity(&ref))
						{
							FSFileSecurityRef fileSecRef;
							
							fileSecRef = CreateReadOnlyACENoOtherAccess();
							if (fileSecRef)
							{
								SInt16				forkRefNum;
								FSCatalogInfo		info;
								FSPermissionInfo	*permInfo = (FSPermissionInfo *) info.permissions;
								static const char	kWriteData[] = "0123456789";
								
								permInfo->fileSec = fileSecRef;
								// This will create a file on disk with the POSIX mode set to no access (000) and an ACL with a single ACE that allows the current user to read the file.
								// The forkRefNum is an access path that allows read/write access to the file.  With these permissions in place even this process will not be able to 
								// open the file for writing once it is closed.  Other processes running as this user (or any other non-super user) will not be able to open this file
								// for writing since it was created from the outset with this user read only access.
								err = FSCreateFileAndOpenForkUnicode(&ref, uniStr.length, uniStr.unicode, kFSCatInfoFSFileSecurityRef, &info, 0, NULL, fsRdWrPerm, &forkRefNum, NULL);
								if (err == noErr)
								{
									ByteCount actualCount;
									// new file created and data fork opened with read/write access
									err = FSWriteFork(forkRefNum, fsFromStart, 0, sizeof(kWriteData), kWriteData, &actualCount);
									if (err == noErr)
									{
										char	readBuffer[sizeof(kWriteData)];
										// read it back and make sure it is what we expected
										err = FSReadFork(forkRefNum, fsFromStart, 0, sizeof(kWriteData), readBuffer, &actualCount);
										if (err == noErr)
										{
											if (memcmp(kWriteData, readBuffer, sizeof(kWriteData)) != 0)
												printf("Data read was not what was expected: %s (expected %s)\n", readBuffer, kWriteData);
										}
										else
											printf("FSReadFork returned %ld\n", err);
									}
									(void) FSCloseFork(forkRefNum);
								}
								else
									printf("FSCreateFileAndOpenForkUnicode returned %ld\n", err);
								CFRelease(fileSecRef);
							}
							else
								printf("Could not create read only ACL\n");
						}
						else
							printf("Target volume does not appear to support extended security\n");
					}
					else
						printf("ConvertUTF8NameToHFSUniStr255 returned %ld for %s\n", err, baseName);
				}
				else
					printf("Could not get basename for %s\n", argv[1]);
			}
			else
				printf("FSPathMakeRef returned %ld for %s [%ld]\n", err, argv[1]);
		}
		else
			printf("Could not get dirname for %s\n", argv[1]);
	}		
	else
		printf("usage: %s path\nThis tool will create a file at the location specified by path with mode 0 and an ACE which permits the owner to read the contents.\n", argv[0]);
    return 0;
}
