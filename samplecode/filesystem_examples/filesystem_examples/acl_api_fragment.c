/*
	File:		acl_api_fragment.c

	Abstract:	Demonstrate the creation of a file 'foo' in the current directory with 
				"read only" permissions using Access Control List functionality. Note that once 
				the ace is set up you can call acl_set_file() repeditively to set those 
				permissions on many files.
				
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

/*
 * % ./acl_api_fragment ; ls -le foo
 * result=3
 * ---------- + 1 emura  staff  0 Apr 27 22:59 foo
 * 0: user:emura allow read,readattr,readextattr,readsecurity
*/

#include <err.h>
#include <strings.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <membership.h>
#include <sys/acl.h>


#define ROPERMS ( ACL_READ_DATA | ACL_READ_SECURITY |\
                  ACL_READ_ATTRIBUTES | ACL_READ_EXTATTRIBUTES )


int
acl_readonly_example(uuid_t *uuid)
{
	int fd;
	acl_t	acl;
	acl_entry_t ace;
	acl_permset_t perms;
	filesec_t fsec;

	/* initialize our ACL */
	if (NULL == (acl = acl_init(32)))
		err(1, "acl_init()");

	/*
	 * create an ACE
	 *
	 * acl_create_entry_np() has a position capability via the
	 * 'entry_index' argument (ACL_FIRST_ENTRY or ACL_LAST_ENTRY)
	 */
	if (0 != acl_create_entry(&acl, &ace))
		err(1, "acl_create_entry()");

	/* allow or deny */
	if (0 != acl_set_tag_type(ace, ACL_EXTENDED_ALLOW))
		err(1, "acl_set_tag_type()");

	/* associate this with our uuid */
	if (0 != acl_set_qualifier(ace, uuid))
		err(1, "acl_set_qualifier()");

	/* grant "read only" permissions */
	if (0 != acl_get_permset(ace, &perms))
		err(1, "acl_get_permset()");

	if (0 != acl_clear_perms(perms))
		err(1, "acl_clear_perms()");

	if (0 != acl_add_perm(perms, ROPERMS))
		err(1, "acl_add_perm()");

	if (0 != acl_set_permset(ace, perms))
		err(1, "acl_set_permset()");


	/* create a file security object */
	fsec = filesec_init();

	/* add the ACL to the security descriptor */
	filesec_set_property(fsec, FILESEC_ACL, &acl);
	acl_free(acl);

	/* turn off all other permissions on the file */
	filesec_set_property(fsec, FILESEC_MODE, 0);

	/* create a file using our ACL */
	fd = openx_np("foo", O_CREAT|O_EXCL|O_RDWR, fsec);

	/* clean up */
	filesec_free(fsec);
	if (-1 != fd )
		close(fd);

	return(fd);
}


int
main(void)
{
	int result;
	long retval;
	uuid_t *uuid=NULL;

	/* check to see if ACLs are supported in the current directory*/
	if (-1 == (retval = pathconf(".", _PC_EXTENDED_SECURITY_NP))) {
		err(1, "pathconf()");
	} else {
		if(0 == retval) {
			fprintf(stderr,
				"ACLs not supported here (retval=%ld)\n",
				retval);
			exit(1);
		}
	}


	if (NULL == (uuid = (uuid_t *)calloc(1,sizeof(uuid_t))))
		err(1, "unable to allocate a uuid");

	if (0 != mbr_uid_to_uuid(getuid(), *uuid)) {
		perror("mbr_uid_to_uuid()");
		free(uuid);
		exit(1);
	}

	result = acl_readonly_example(uuid);
	free(uuid);

	printf("result=%d\n", result);
	return(result);
}
