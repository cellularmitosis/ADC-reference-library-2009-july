/*
 *  EthernetSocketStuff.h
 *  BSDLLCTest
 *
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
 *
 */
#ifndef __ETHERNETSOCKETSTUFF__

#include <Carbon/Carbon.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/dlil.h>
#include <net/ndrv.h>
#include <net/ethernet.h>
#include <pthread.h>
#include "GetEthernetAddrSample.h"

#define BSDLLCTEST_PATH	"/tmp/BSDLLCTest\0"
#define kBSDTestFileDescriptorsKey CFSTR("com.apple.dts.BSDLLCTest.FD")

// defines for setting and reading the data buffer
// there appears to be a problem such that if you specify the packet size to be 1500 bytes, the
// recvfrom call will not receive the packet.  the actual size of the packet does not matter. 
// reference radar bug

#define DATASIZE	1500	
#define DATAOFFSET	1000	// first byte in data buffer where we place valid data
#define CONTROLCODE	0x03

enum {
    kIOInProgress = 1
};

enum {
        kReceiveTest = 1,
    	kSendTest = 2,
	    kCancelTest = 3
};

enum {
    kThreadActive = 0x08000000,
    kQuitThread =	0x10000000
};

struct PacketBuffer {
	UInt32				rawModeOffset;
	UInt32				i;
	UInt32				lastFlowErrPacketNum;
	UInt8				data[DATASIZE+32];
};

typedef struct PacketBuffer PacketBuffer;
typedef struct PacketBuffer *PacketBufPtr;

struct EnetTestData	{
    pthread_t		pthread;
	MACAddress		srcaddr;
	MACAddress		destaddr;
	int				fd;
	MPQueueID		termQueue;
	MPTaskID		bsdTaskID;
	UInt16			sap;
	UInt8			snap[5];
	UInt8			filler;
	UInt32			numPacketsToSend;
	UInt32			numPacketsSent;
	UInt32			packetsRead;
	UInt32			packetsInOrder;
	UInt32			packetsOutOfOrder;
	UInt32			lastPacketNum;
	UInt32			sendErrors;
	UInt32			readErrors;
	UInt32			sendRcvState;
	UInt32			flag;
	UInt32			verMacOS;
	time_t			tbegin;
	time_t			tend;
	time_t			tdiff;
	PacketBufPtr	pkt;
	struct sockaddr	saddr;
	Boolean			destAddrIsMCast;
    Boolean			isFirstTime;
};

typedef struct EnetTestData EnetTestData;
typedef struct EnetTestData *EnetTestDataPtr;

extern OSStatus DoEnetTest(EnetTestData *enetdata);

#endif	//__ETHERNETSOCKETSTUFF__
