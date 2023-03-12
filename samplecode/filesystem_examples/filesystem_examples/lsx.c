/*
	File:		lsx.c

	Abstract:	Lists the extended attribute keys and values of the specified file or directory.

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


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/xattr.h>


void
printEAs(char *path)
{
	ssize_t nsize, lsize;
	ssize_t asize = 0;
	char *attrnamebuf;
	char attrdatabuf[4096];

	/* get the size of our EA names and allocate some space */
	nsize = listxattr(path, NULL, 0, 0);
	if (NULL == (attrnamebuf = (char *)malloc(nsize))) {
		fprintf(stderr, "malloc() failed, path=%s nsize=%ld\n",
		    path, nsize);
		return;
	}

	/* get the list of EA names */
	lsize = listxattr(path, attrnamebuf, nsize, 0);

	if (lsize >= 0) {
		char *ptr;
		int spaces;

		printf("%s:\n", path);

		/* iterate through the EAs */
		for(ptr=attrnamebuf; ptr < attrnamebuf + lsize; ) {

			spaces = 32;
			attrdatabuf[0] = '\0';

			/* get the size of the current EA */
			asize = getxattr(path, ptr, NULL, 0, 0, 0);
			if (asize < 0) {
				fprintf(stderr,
				    "getxattr returned: %ld (%s)\n",
				    asize, strerror(errno));
				break;
			}

			if (asize < sizeof(attrdatabuf)) {
				ssize_t asize2;

				asize2 = getxattr(path, ptr, attrdatabuf,
				    sizeof(attrdatabuf), 0, 0);
				if (asize2 < 0) {
					fprintf(stderr,
					    "getxattr returned: %ld (%s)\n",
					    asize2, strerror(errno));
				} else {
					attrdatabuf[asize2] = '\0';
				}
			}

			/* an attempt to pretty print manually */
			spaces -= strlen(ptr);
			printf("  %7d  %s", (int)asize, ptr);
			ptr += strlen(ptr) + 1;

			if (asize > 1) {
				int i;
				for (i=0; i < spaces; ++i)
					printf(" ");
				printf(" %s\n", attrdatabuf);
			} else {
				printf("\n");
			}
    		}

	} else {
		fprintf(stderr, "listxattr() returned: %ld (%s) on %s\n",
		    lsize, strerror(errno), path);
	}

	free(attrnamebuf);
	return;
}



int
main(int argc, char *argv[])
{
	int i;

	if (argc < 2 ) {
		printf("usage: %s file1 [file2 ...]\n",
		    argv[0]);
		exit(1);
	}

	for (i=1 ; i<argc ; i++)
		printEAs(argv[i]);

	exit(0);
}
