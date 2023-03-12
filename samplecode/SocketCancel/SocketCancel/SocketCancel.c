/*
    File:       SocketCancel.c

    Contains:   A program to demonstrate how to write BSD sockets code that can be cancelled.

    Written by: DTS

    Copyright:  Copyright (c) 2005 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

    Change History (most recent first):

$Log: SocketCancel.c,v $
Revision 1.1  2005/08/02 10:31:35         
First checked in.  The only difference from version 1.0a1 is that I've changed the definition of some local variables from int to socklen_t to quieten GCC 4.0's warnings.


*/

/////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>

/////////////////////////////////////////////////////////////////
#pragma mark **** Cancellable Socket Abstraction Layer

struct CSocket {
    int 	sock;
    int 	sndCancel;
    int 	rcvCancel;
    int		maxFD;
    bool	didCancel;
};
typedef struct CSocket CSocket;

static void CSocketClose(CSocket *cSock);		// forward declaration

static int CSocketOpen(int domain, int type, int protocol, CSocket **cSockPtr)
    // Our equivalent to "socket".  Creates an CSocket object for the specified 
    // domain, type and protocol.  The result is an errno-style error.
{
    int 		err;
    CSocket *	cSock;
    
    assert( cSockPtr != NULL);
    assert(*cSockPtr == NULL);

    // Allocate the CSocket.
    
    err = 0;
    cSock = (CSocket *) calloc(1, sizeof(*cSock));
    if (cSock == NULL) {
        err = ENOMEM;
    }
    
    // Initialise it, create the data socket, set it non-blocking, and 
    // then create the socket pair we use for cancellation.  That socket 
    // pair remains blocking, because we only ever send a single byte on it.
    
    if (err == 0) {
        cSock->sock      = -1;
        cSock->sndCancel = -1;
        cSock->rcvCancel = -1;
        assert( cSock->didCancel == false );		// cleared by calloc, above
        
        cSock->sock = socket(domain, type, protocol);
        if (cSock->sock < 0) {
            err = errno;
        }
    }
    if (err == 0) {
        err = fcntl(cSock->sock, F_SETFL, O_NONBLOCK);
        if (err < 0) {
            err = errno;
        }
    }
    if (err == 0) {
        int tmpSock[2];
        
        err = socketpair(AF_UNIX, SOCK_STREAM, 0, tmpSock);
        if (err < 0) {
            err = errno;
        } else {
            cSock->sndCancel = tmpSock[0];
            cSock->rcvCancel = tmpSock[1];
        }
    }
    if (err == 0) {
        cSock->maxFD = cSock->sock;
        if (cSock->sndCancel > cSock->maxFD) {
            cSock->maxFD = cSock->sndCancel;
        }
        if (cSock->rcvCancel > cSock->maxFD) {
            cSock->maxFD = cSock->rcvCancel;
        }
    }
    
    // Clean up.

    if (err != 0) {
        CSocketClose(cSock);
        cSock = NULL;
    }
    *cSockPtr = cSock;

    assert( (err == 0) && (cSockPtr != NULL) );
    
    return err;
}

static void CSocketClose(CSocket *cSock)
    // Our equivalent to "close".  Closes an CSocket object.
{
    int junk;
    
    if (cSock != NULL) {
        if (cSock->sock != -1) {
            junk = close(cSock->sock);
            assert(junk == 0);
        }
        if (cSock->sndCancel != -1) {
            junk = close(cSock->sndCancel);
            assert(junk == 0);
        }
        if (cSock->rcvCancel != -1) {
            junk = close(cSock->rcvCancel);
            assert(junk == 0);
        }
        free(cSock);
    }
}

static bool CSocketValid(const CSocket *cSock)
    // Returns true if the CSocket object is valid.
{
    return     (cSock != NULL)
            && (cSock->sock != -1)
            && (cSock->sndCancel != -1)
            && (cSock->rcvCancel != -1)
            && (cSock->maxFD >= cSock->sock)
            && (cSock->maxFD >= cSock->sndCancel)
            && (cSock->maxFD >= cSock->rcvCancel);
}

typedef enum {
    kCSocketSelectConnect,
    kCSocketSelectRead,
    kCSocketSelectWrite
} CSocketSelectMode;

static int CSocketSelect(CSocket *cSock, CSocketSelectMode mode, bool *dataPresent)
    // Waits for activity on the data socket associated with cSock 
    // and the socket used to signal cancellating, and sets 
    // *dataPresent if there was activity on the data socket.  Result 
    // is an errno-style error.  A value of 0 indicates that the select 
    // was successful and some data is present.  A value of ECANCELED 
    // indicates that the select unblocked with data on the cancellation 
    // socket.  Loops on EINTR, so EINTR is never returned.
    //
    // The exact sockets are controlled by mode.
    //
    // Mode						read FD set			write FD set	dataPresents tests
    // ----						-----------			------------	------------------
    // kCSocketSelectConnect	rcvCancel, sock		sock			-
    // kCSocketSelectRead		rcvCancel, sock		-				sock in read FD set
    // kCSocketSelectWrite		rcvCancel			sock			sock in write FD set
{
    int 		err;
    fd_set 		readFDs;
    fd_set 		writeFDs;
    fd_set *	dataPresentFDs;
    int			selectResult;
    
    FD_ZERO(&readFDs);
    FD_ZERO(&writeFDs);

    // We always want the cancel socket in the read FD set.
    
    FD_SET(cSock->rcvCancel, &readFDs);

    // Add other sockets to the FD sets.  Also remember which 
    // FD set we're supposed to return whether data is present in.
    
    switch (mode) {
        case kCSocketSelectConnect:
            FD_SET(cSock->sock, &readFDs);
            FD_SET(cSock->sock, &writeFDs);
            dataPresentFDs = NULL;
            break;
        case kCSocketSelectRead:
            FD_SET(cSock->sock, &readFDs);
            dataPresentFDs = &readFDs;
            break;
        case kCSocketSelectWrite:
            FD_SET(cSock->sock, &writeFDs);
            dataPresentFDs = &writeFDs;
            break;
        default:
            assert(false);
            break;
    }

    // Do the select, looping while we get EINTR errors.
    
    err = 0;
    do {
        selectResult = select(cSock->maxFD + 1, &readFDs, &writeFDs, NULL, NULL);
        
        if (selectResult < 0) {
            err = errno;
        }
    } while (err == EINTR);
    
    if (err == 0) {
		// We have an infinite timeout, so a result of 0 should be impossible, 
        // so assert that selectResult is positive.

        assert(selectResult > 0);

        // Check for cancellation first.
        
        if ( FD_ISSET(cSock->rcvCancel, &readFDs) ) {
            err = ECANCELED;
        } else {
            if ( (dataPresent != NULL) && (dataPresentFDs != NULL) ) {
                *dataPresent = ( FD_ISSET(cSock->sock, dataPresentFDs) != 0 );
            }
        }
    }

    return err;
}

static int CSocketConnect(CSocket *cSock, const struct sockaddr *name, int namelen)
    // Our equivalent to "connect".  Connects the CSocket object to the host 
    // specified by name and namelen.  The result is an errno-style error.
{
    int err;
    
    assert(CSocketValid(cSock));
    assert(name != NULL);
    
    // Start the connect.
    
	err = connect(cSock->sock, name, namelen);
    
    // Handle any error conditions.
    
    if (err < 0) {
        err = errno;
        
        // EINPROGRESS means that the connect started, and we have to wait 
        // for it to complete.  Let's do that.  Any other error is passed to 
        // the caller.
        
        if (err == EINPROGRESS) {
            bool 		connected;
            socklen_t 	len;
            
            connected = false;

            err = CSocketSelect(cSock, kCSocketSelectConnect, NULL);

            if (err == 0) {
                // Not cancelled, so must have either connected or failed.  
                // Check to see if we're connected by calling getpeername.
                
                len = 0;
                err = getpeername(cSock->sock, NULL, &len);
                if (err < 0) {
                    err = errno;
                }
                
                if (err == 0) {
                    // The connection attempt worked.
                    connected = true;
                } else if (err == ENOTCONN) {
                    int tmpErr;
                    
                    // The connection failed.  Get the error associated with 
                    // the connection attempt.
                    
                    len = sizeof(tmpErr);
                    err = getsockopt(cSock->sock, SOL_SOCKET, SO_ERROR, &tmpErr, &len);
                    if (err < 0) {
                        err = errno;
                    } else {
                        assert(tmpErr != 0);
                        err = tmpErr;
                    }
                }
            }
        }
    }
    
    return err;
}

static int CSocketRead(CSocket *cSock, void *buf, size_t nbytes, size_t *bytesReadPtr)
    // Our equivalent to "read".  Reads nbytes of data from the CSocket object 
    // to the buffer specified by buf and nbytes.  The result is an errno-style 
    // error.  Returns EPIPE if end of file is encountered.
    //
    // IMPORTANT: Unlike standard UNIX read, this routine will block until 
    // either the entire buffer is full or an end of file is reached. 
    // If you get an EPIPE error, *bytesReadPtr indicates how much data 
    // is valid in the buffer.
{
    int				err;
    size_t			bytesLeft;
    char *			cursor;
    ssize_t			bytesThisTime;
    bool			dataPresent;
    
    assert(CSocketValid(cSock));
    assert(buf != NULL);
    
    err = 0;
    bytesLeft = nbytes;
    cursor = (char *) buf;
    
    while ( (err == 0) && (bytesLeft != 0) ) {
        bytesThisTime = read(cSock->sock, buf, nbytes);
        if (bytesThisTime > 0) {
            cursor    += bytesThisTime;
            bytesLeft -= bytesThisTime;
        } else if (bytesThisTime == 0) {
            err = EPIPE;
        } else {
            assert(bytesThisTime == -1);
            
			err = errno;
			assert(err != 0);

            // We don't need to handle EINTR because read never blocks, 
            // and thus can never be interrupted.
            
            if (err == EAGAIN) {
                err = CSocketSelect(cSock, kCSocketSelectRead, &dataPresent);
                
                assert( (err != 0) || dataPresent );
            }
        }
    }
    
    // Clean up.
    
    if (bytesReadPtr != NULL) {
		*bytesReadPtr = nbytes - bytesLeft;        
    }
    return err;
}

static int CSocketWrite(CSocket *cSock, const void *buf, size_t nbytes, size_t *bytesWrittenPtr)
    // Our equivalent to "write".  Writes data from the buffer specified by buf 
    // and nbytes to the CSocket object.  The result is an errno-style error. 
    // All data is written, unless the operation fails with an error.
    // On failure, only some of the bytes may have been written.  If you 
    // want to find out how many, pass non-NULL to bytesWrittenPtr.  If not, 
    // pass NULL to bytesWrittenPtr.
{
    int				err;
    size_t 			bytesLeft;
    const char *	cursor;
    ssize_t			bytesThisTime;
    bool			spaceAvailable;
    
    assert(CSocketValid(cSock));
    assert(buf != NULL);

	err = 0;
	bytesLeft = nbytes;
	cursor = (const char *) buf;
    
	while ( (err == 0) && (bytesLeft != 0) ) {
		bytesThisTime = write(cSock->sock, cursor, bytesLeft);
		if (bytesThisTime > 0) {
			cursor    += bytesThisTime;
			bytesLeft -= bytesThisTime;
		} else if (bytesThisTime == 0) {
			assert(false);
			err = EPIPE;
		} else {
			assert(bytesThisTime == -1);
			
			err = errno;
			assert(err != 0);
            
            // We don't need to handle EINTR because write never blocks, 
            // and thus can never be interrupted.

			if (err == EAGAIN) {
                err = CSocketSelect(cSock, kCSocketSelectWrite, &spaceAvailable);

                assert( (err != 0) || spaceAvailable );
			}
		}
	}

    // Clean up.
    
	if (bytesWrittenPtr != NULL) {
		*bytesWrittenPtr = nbytes - bytesLeft;
	}
	return err;
}

static int CSocketCancel(CSocket *cSock)
    // Cancels a blocked CSocket operation.  Once you call this, 
    // all subsequent CSocketConnect, CSocketRead, and CSocketWrite
    // operations that try to block will return ECANCELED.
{
    int	err;
    ssize_t bytesWritten;
    static const char kCancelMessage = 0;
    
    assert(CSocketValid(cSock));
    
    err = 0;
    if ( ! cSock->didCancel ) {
		bytesWritten = write(cSock->sndCancel, &kCancelMessage, sizeof(kCancelMessage));
		if (bytesWritten < 0) {
			err = errno;
	
			// I don't bother handling EINTR here because writing a single 
			// byte will never block, and thus can't be interrupted.
	
			assert( err != EINTR);
		} else {
			cSock->didCancel = true;
		}
	}
    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark **** Driver Program

static void SIGINTHandler(int sigNum)
{
    char nl;
    
    assert(sigNum == SIGINT);
    
    // Do nothing.  The signal simply breaks us out of the "pause" call.
    
    // Print a newline character to prevent subsequent messages showing 
    // up on the same line as the ^C.
    
    nl = '\n';
    (void) write(STDIN_FILENO, &nl, sizeof(nl));
}

static int InstallSIGINTHandler(void)
    // Installs a dummy SIGINT handler which prevents us 
    // from quitting when we get a SIGINT.
{
    int					err;
	struct sigaction 	act;
    
    act.sa_handler = SIGINTHandler;
    (void) sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    
	err = sigaction(SIGINT, &act, NULL);
    if (err < 0) {
        err = errno;
    }
    
    return err;
}

// ConnectionRecord is the parameter we pass to ConnectThreadEntry.
// It tells ConnectThreadEntry where to connect to, and what to do 
// once it connects.

struct ConnectionRecord {
    pthread_t			thread;
    CSocket *			sock;
    struct sockaddr *	connectAddr;
    bool				readData;
};
typedef struct ConnectionRecord ConnectionRecord;

// We need a mighty big buffer in order for the write test to fill 
// up the kernel socket buffer and start to block.

static char gJunkBuf[256 * 1024];

static void *ConnectThreadEntry(void *param)
    // A separate thread is used for the connect and I/O operations, 
    // so we can test cancellation.  This is its entry point.
{
    int 				err;
    ConnectionRecord *	cr;
    
	cr = (ConnectionRecord *) param;
    
    // First connect to the host specified by the connection record.
    
	fprintf(stderr, "Connect thread is connecting.\n"); 
    
    err = CSocketConnect(cr->sock, cr->connectAddr, cr->connectAddr->sa_len);
    
    // After the connect, do a read or write.
    
    if (err == 0) {
        if (cr->readData) {
            size_t bytesRead;
            
            fprintf(stderr, "Connect thread is reading.\n"); 
            
            err = CSocketRead(cr->sock, gJunkBuf, sizeof(gJunkBuf), &bytesRead);
            
            if (err == EPIPE) {
                fprintf(stderr, "Looks like the server threw us off.\n"); 
            }
        } else {
            fprintf(stderr, "Connect thread is writing.\n"); 

            err = CSocketWrite(cr->sock, gJunkBuf, sizeof(gJunkBuf), NULL);
        }
    }
    
	fprintf(stderr, "Connect thread done, err = %d\n", err);
    
    return (void *) err;
}

static int DoConnectReadTest(const char *hostName)
    // Do a test that involves connecting to the HTTP service 
    // on hostName and trying to read data from it.  HTTP servers 
    // don't spontaneously generate data (you have to send them a 
    // request first), so this will always block indefinitely 
    // (or until the server thinks we're trying to mount a denial 
    // of service attack, and closes the connection).
{
    int 				err;
    int 				junk;
    struct addrinfo *	allAddrs;
    struct addrinfo *	thisAddr;
    struct addrinfo 	hints;
    ConnectionRecord	crStore;
    ConnectionRecord *	cr;
    
    fprintf(stderr, "Test connect and read to %s\n", hostName);
    
    allAddrs = NULL;

    memset(&crStore, 0, sizeof(crStore));
    cr = &crStore;
    
    cr->readData = true;
    
    // Resolve hostName and put a copy into the connection record.
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;					// Should be PF_UNSPEC, but this code can't handle connecting to multiple addresses yet.
    hints.ai_socktype = SOCK_STREAM;

	fprintf(stderr, "Main thread is looking up name.\n"); 

	err = getaddrinfo(hostName, "http", &hints, &allAddrs);

    if (err == 0) {
        thisAddr = allAddrs;			// right now we only use the first IPv4 address
        
        cr->connectAddr = malloc(thisAddr->ai_addrlen);
        if (cr->connectAddr == NULL) {
            err = ENOMEM;
        } else {
            (void *) memcpy(cr->connectAddr, thisAddr->ai_addr, thisAddr->ai_addrlen);
        }
    }
    
    // Open the socket and create the connection thread.
    
    if (err == 0) {        
        err = CSocketOpen(thisAddr->ai_family, thisAddr->ai_socktype, thisAddr->ai_protocol, &cr->sock);
    }
    if (err == 0) {
        err = pthread_create(&cr->thread, NULL, ConnectThreadEntry, cr);
    }
    
    // Install a NOP SIGINT handler and then wait for a signal.
    // After getting the signal, cancel the thread and wait 
    // for it to complete.
    
    if (err == 0) {
        err = InstallSIGINTHandler();
    }
    if (err == 0) {
        void *threadResult;
        
        fprintf(stderr, "SIGINT (typically ^C) to cancel\n");
        
        (void) pause();

        junk = CSocketCancel(cr->sock);
        assert(junk == 0);
        
        err = pthread_join(cr->thread, &threadResult);
        if (err == 0) {
            fprintf(stderr, "threadResult = %d\n", (int) threadResult);
        }
    }

    // Clean up.
    
    if (cr->sock != NULL) {
        CSocketClose(cr->sock);
    }
    free(cr->connectAddr);
    if (allAddrs != NULL) {
        freeaddrinfo(allAddrs);
    }
    
    return err;
}

static int DoWriteTest(void)
    // Do a test which involve opening a listening socket on the local 
    // machine and then starting a thread that connects to the socket 
    // and writes a whole bunch of data to it, eventually blocking.
    // Then we let the user cancel the blocked write.  This code 
    // doesn't even bother trying to be protocol independent.
{
    int					err;
    int					junk;
    int					listener;
    int					accepted;
    ConnectionRecord	crStore;
    ConnectionRecord *	cr;
    struct sockaddr_in 	bindAddr;

    fprintf(stderr, "Test write to localhost\n");

    listener = -1;
    accepted = -1;
    
    memset(&crStore, 0, sizeof(crStore));
    cr = &crStore;
    
    cr->readData = false;

    // Open a listening socket on localhost on port 10666.
    
    err = 0;
	listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        err = errno;
    }
    if (err == 0) {
        bindAddr.sin_len = sizeof(bindAddr);
        bindAddr.sin_family = AF_INET;
        bindAddr.sin_port = htons(10666);
        bindAddr.sin_addr.s_addr = INADDR_ANY;
        
        err = bind(listener, (struct sockaddr *) &bindAddr, sizeof(bindAddr));
        if (err < 0) {
            err = errno;
        }
    }
    if (err == 0) {	
        err = listen(listener, 10);
        if (err < 0) {
            err = errno;
        }
    }
    
    // Start a thread to connect to that socket and write a lot of data.
    
    if (err == 0) {
        cr->connectAddr = malloc(sizeof(bindAddr));
        if (cr->connectAddr == NULL) {
            err = ENOMEM;
        } else {
            (void *) memcpy(cr->connectAddr, &bindAddr, sizeof(bindAddr));
        }
    }
    if (err == 0) {        
        err = CSocketOpen(AF_INET, SOCK_STREAM, 0, &cr->sock);
    }
    if (err == 0) {
        err = pthread_create(&cr->thread, NULL, ConnectThreadEntry, cr);
    }
    
    // Accept the incoming connection from the pthread and then specifically 
    // don't read the data that's coming from the thread.  The thread will 
    // eventually block when the socket buffer fills up.
    
    if (err == 0) {
        socklen_t addrlen;
        
        addrlen = 0;
        accepted = accept(listener, NULL, &addrlen);
        if (accepted < 0) {
            err = errno;
        }
    }
    
    // Install a NOP SIGINT handler and then wait for a signal.
    // After getting the signal, cancel the thread and wait 
    // for it to complete.
    
    if (err == 0) {
        err = InstallSIGINTHandler();
    }
    if (err == 0) {
        void *threadResult;
        
        fprintf(stderr, "SIGINT (typically ^C) to cancel\n");
        
        (void) pause();

        junk = CSocketCancel(cr->sock);
        assert(junk == 0);
        
        err = pthread_join(cr->thread, &threadResult);
        if (err == 0) {
            fprintf(stderr, "threadResult = %d\n", (int) threadResult);
        }
    }

    // Clean up.
    
    if (cr->sock != NULL) {
        CSocketClose(cr->sock);
    }
    free(cr->connectAddr);
    if (listener != -1) {
        junk = close(listener);
        assert(junk == 0);
    }
    if (accepted != -1) {
        junk = close(accepted);
        assert(junk == 0);
    }
        
    return err;
}

static const char *gProgramName;

static void PrintUsage(const char *command)
{
    fprintf(stderr, "%s: Usage: BootstrapPortDump [ host ]\n", gProgramName);
}

int main (int argc, const char * argv[])
{
    int 				err;
    const char *		hostName;

    // Set gProgramName to the last path component of argv[0]
    
    gProgramName = strrchr(argv[0], '/');
    if (gProgramName == NULL) {
        gProgramName = argv[0];
    } else {
        gProgramName += 1;
    }

    // Parse our arguments.

    err = 0;
    switch (argc) {
        case 1:
            hostName = NULL;
            break;
        case 2:
            hostName = argv[1];
            break;
        default:
            PrintUsage(argv[0]);
            err = EINVAL;
            break;
    }

    // Do one of two different tests.
    
    if (err == 0) {
        if (hostName == NULL) {
            err = DoWriteTest();
        } else {
            err = DoConnectReadTest(hostName);
        }
    }
    
    if (err == 0) {
        fprintf(stderr, "Success!\n");
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Failed with error %d.\n", err);
        return EXIT_FAILURE;
    }
}
