/*
	File:		main.c

	Abstract:	Tool which will remove inherited ACEs from a file system object
				(demonstrating File Manager ACL support).

	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/

#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <sys/acl.h>


// this routine uses the acl_t API to run through the ACL and remove any
// entry that is marked as inherited.  It will set aclChanged to true
// if any entries are actually removed.
static int RemoveInheritedEntriesFromACL(acl_t theACL, Boolean *aclChanged, Boolean *aclNowEmpty)
{
	int	result = 0, getEntryResult;
	acl_entry_t entry;
	acl_flagset_t	flags;
	
	// loop through all the ACEs, starting at the beginning
	getEntryResult = acl_get_entry(theACL, ACL_FIRST_ENTRY, &entry); 
	for (getEntryResult = acl_get_entry(theACL, ACL_FIRST_ENTRY, &entry); getEntryResult == 0; getEntryResult = acl_get_entry(theACL, ACL_NEXT_ENTRY, &entry))
	{
		// get the flagset and check for the ACL_ENTRY_INHERITED flag - if present delete the entry
		if (acl_get_flagset_np(entry, &flags) == 0)
		{
			if (acl_get_flag_np(flags, ACL_ENTRY_INHERITED) != 0) 
			{
				result = acl_delete_entry(theACL, entry);
				if (result == 0)
					*aclChanged = true;
				else
					break;
			}
		}
	}

	// if anything was removed check to see if the acl is empty
	if ((aclChanged) && (result == 0))
	{
		// if we can get the first entry then there is at least one left...
		*aclNowEmpty = (acl_get_entry(theACL, ACL_FIRST_ENTRY, &entry) == 0) ? false : true;
	}
	
	return result;
}

static OSStatus RemoveInheritedEntriesFromObject(const FSRef *ref, Boolean *aclChanged)
{
	FSCatalogInfo info;
	OSStatus err;
	
	// nothing has changed yet
	*aclChanged = false;
	
	// get an FSFileSecurityRef for the object
	err = FSGetCatalogInfo(ref, kFSCatInfoFSFileSecurityRef, &info, NULL, NULL, NULL);
	if (err == noErr)
	{
		// we have a valid FSFileSecurityRef or NULL.
		// NULL will be returned if the item has no extened secutiry info (which will
		// always be the case for a volume with extended security disabled).
		FSFileSecurityRef fileSecRef = ((FSPermissionInfo *) info.permissions)->fileSec;
		if (fileSecRef != NULL)
		{
			acl_t	theACL;
			// get a copy of the access control list
			err = FSFileSecurityCopyAccessControlList(fileSecRef, &theACL);
			if (err == noErr)
			{
				Boolean aclNowEmpty;
				int result = RemoveInheritedEntriesFromACL(theACL, aclChanged, &aclNowEmpty);
				if (result == 0)
				{
					// now the inherited entries are gone, so we need to update the FSSecurityRef and then
					// set it back on the file system object.  The FSFileSecuritySetAccessControlList call
					// is necessary since we operated on a copy of the access control list
					if (*aclChanged)
					{
						if (!aclNowEmpty)
							err = FSFileSecuritySetAccessControlList(fileSecRef, theACL);
						else	// if the acl is empty then remove it (all entries were inherited)
							err = FSFileSecuritySetAccessControlList(fileSecRef, kFSFileSecurityRemoveACL);
							
						if (err == noErr)
						{
							err = FSSetCatalogInfo(ref, kFSCatInfoFSFileSecurityRef, &info);
							if (err != noErr)
								printf("FSSetCatalogInfo returned %ld\n", err);
						}
						else
							printf("FSFileSecuritySetAccessControlList returned %ld\n", err);
					}
					else // if nothing changed then no need to set it back
						err = noErr;

				}
				else
				{
					printf("RemoveInheritedEntriesFromACL returned %d\n", result);
					err = ioErr; // return something other than noErr
				}
				// free our copy of the acl
				acl_free(theACL);
			}
			else if (err == errFSPropertyNotValid) // errFSPropertyNotValid means no acl, so no entries need to be removed
				err = noErr;
			else
				printf("FSFileSecurityCopyAccessControlList returned %ld\n", err);

			// need to release the fileSecRef
			CFRelease(fileSecRef);
		}
		else	// no info, so no inherited entries to remove
			err = noErr;
	}
	else
		printf("FSGetCatalogInfo returned %ld\n", err);
		
	return err;
}

int main (int argc, const char * argv[]) {

	if (argc == 2)
	{
		FSRef	ref;
		OSStatus err;
		
		err = FSPathMakeRef((UInt8 *)argv[1], &ref, NULL);
		if (err == noErr)
		{
			Boolean aclChanged;
			
			err = RemoveInheritedEntriesFromObject(&ref, &aclChanged);
			if (err == noErr)
			{
				if (aclChanged)
					printf("Inherited entries removed from %s\n", argv[1]);
				else
					printf("No inherited entried found on %s\n", argv[1]);
			}
			else
				printf("RemoveInheritedEntries returned %ld for %s\n", err, argv[1]);
		}
		else
			printf("FSPathMakeRef returned %ld for %s [%ld]\n", err, argv[1]);
	}		
	else
		printf("usage: %s path\nThis tool will remove any inherited ACEs from the item specified by path\n", argv[0]);
    return 0;
}
