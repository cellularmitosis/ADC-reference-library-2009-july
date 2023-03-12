/*
	File:		kqueue_fragment.c

	Abstract:	Notification example watching for the following VNode events: EV_ADD, EV_ENABLE,
				EV_CLEAR.  When a file system object is created, deleted or renamed the kevent 
				data is displayed through stdout.

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

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>

#define MY_NOTES (NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB |\
                  NOTE_LINK | NOTE_RENAME | NOTE_REVOKE)

#define IFX(a, x) do { \
        if ((a) & (x)) { printf(#x); a &= ~(x);  if ((a)) printf(", "); }; \
        } while (0)


int
main(int argc, char *argv[])
{
        char *path;
		int nevents;
        int fd;
        int i = 1;
        struct kevent kev;
        int kq;

        if (argc != 2) {
                errx(1, "usage: %s pathname", argv[0]);
        }

        if ((kq = kqueue()) == -1) {
                errx(3, "cannot create kqueue");
        }

        path = strdup(argv[i]);

        if (-1 == (fd = open(path, O_EVTONLY)))
                err(2, "open() on %s failed", path);

        EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD|EV_CLEAR, MY_NOTES, 0, NULL);

        if (kevent(kq, &kev, 1, NULL, 0, NULL) == -1)
                err(4, "cannot add kev to queue");

  	while (1) {
		fprintf(stderr, "calling kevent\n");
                nevents = kevent(kq, NULL, 0, &kev, 1, NULL);

                if (nevents == -1)
                        err(5, "cannot wait for event");

                if (nevents > 0) {
                        printf("ident = %ld  ", kev.ident);
                        printf("data = %ld  ", kev.data);
                        printf("udata = %p  ", kev.udata);
                        printf("flags = 0x%08x\n", kev.flags);
                        printf("fflags = { ");
                        IFX(kev.fflags, NOTE_DELETE);
                        IFX(kev.fflags, NOTE_WRITE);
                        IFX(kev.fflags, NOTE_EXTEND);
                        IFX(kev.fflags, NOTE_ATTRIB);
                        IFX(kev.fflags, NOTE_LINK);
                        IFX(kev.fflags, NOTE_RENAME);
                        IFX(kev.fflags, NOTE_REVOKE);
                        printf(" }\n\n");
                }
        }

	/* xxx -- not reached */
        return(0);
}
