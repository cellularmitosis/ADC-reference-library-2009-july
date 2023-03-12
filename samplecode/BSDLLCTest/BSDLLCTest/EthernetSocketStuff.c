/*
 *  EthernetSocketStuff.c
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

#include "BSDLLCTestCommon.h"
#include "MoreUNIX.h"
#include "MoreSecurity.h"
#include "MoreCFQ.h"
#include "EthernetSocketStuff.h"
#include <sys/un.h>
#include <sys/uio.h>
#include <fcntl.h>

static PacketBuffer		gPacket;
static Boolean			gDone;
static UInt32			gCounter;

int DoAddProtocolMatch(int fd, int ptype, u_int32_t prot_family, 
                        u_char ssap, u_char dsap, u_char cntrl_code, u_char *snapAddr);
int DoAddDelMulticast(int fd, unsigned char *mcAddr, Boolean addFlag);
OSStatus DoBSDLLCWriteTest(EnetTestData *enetdata);
OSStatus DoCancelBSDLLCReadTest(EnetTestData *enetdata);
OSStatus DoBSDLLCReadTest(EnetTestData *enetdata);
int	DoSendPacket(EnetTestData *enetdata, const void *vptr, size_t n);

/*******************************************************************************
** DoAddProtocolMatch - uses the setsocketopt call to specify the protocols
**  to be for the raw socket to watch for.  
    input parameters
    fd is the socket file descriptor
    ptype is the protocol type - NDRV_DEMUXTYPE_ETHERTYPE, NDRV_DEMUXTYPE_SAP or
                                NDRV_DEMUXTYPE_SNAP
    if ptype is NDRV_DEMUXTYPE_ETHERTYPE, then ntype is the native protocol type - e.g. 0x806
    if ptype is NDRV_DEMUXTYPE_SAP then 
    ssap and dsap are the source and destination SAP values to watch for
    and cntrl_code is the control code value - typically 3
    if ptype is NDRV_DEMUXTYPE_SNAP, then 
    *snapAddr is a 5 byte array with the SNAP addr info with the assumption that the 
    source and destination SAP are 0xAA and the control code is 0x03
********************************************************************************/

int DoAddProtocolMatch(int fd, int ptype, u_int32_t prot_family, 
                        u_char ssap, u_char dsap, u_char cntrl_code, u_char *snapAddr)
{
    struct ndrv_protocol_desc	desc;
    struct ndrv_demux_desc	demux_desc[1];
    int				result;
	int 			i;
    
    bzero(&desc, sizeof(desc));  			// zero out the strucuture
    bzero(&demux_desc, sizeof(demux_desc));		// zero out the strucuture

    desc.version = NDRV_PROTOCOL_DESC_VERS;
    desc.protocol_family = prot_family;
    desc.demux_count = (u_int32_t)1;
    desc.demux_list = (struct ndrv_demux_desc*)&demux_desc;
    
    switch (demux_desc[0].type = ptype)
    {
        case NDRV_DEMUXTYPE_ETHERTYPE:
			fprintf(stderr, "adding ethertype protocol 0x%X\n", demux_desc[0].data.ether_type);
            demux_desc[0].length = sizeof(demux_desc[0].data.ether_type);
            demux_desc[0].data.ether_type = sizeof(demux_desc[0].data.ether_type);
            break;
        
        case NDRV_DEMUXTYPE_SAP:
			fprintf(stderr, "adding SAP protocol 0x%X\n", dsap);
            demux_desc[0].length = sizeof(demux_desc[0].data.sap);
            demux_desc[0].data.sap[0] = dsap;
            demux_desc[0].data.sap[1] = ssap;
            demux_desc[0].data.sap[2] = cntrl_code;
            break;
        
        case NDRV_DEMUXTYPE_SNAP:
			fprintf(stderr, "adding SNAP protocol ");
			for (i = 0; i < 5; i++)
			{
				fprintf(stderr, "%X ", snapAddr[i]);
			}
			fprintf(stderr, "\n");
            demux_desc[0].length = sizeof(demux_desc[0].data.snap);
            demux_desc[0].data.snap[0] = (u_int8_t)snapAddr[0];
            demux_desc[0].data.snap[1] = (u_int8_t)snapAddr[1];
            demux_desc[0].data.snap[2] = (u_int8_t)snapAddr[2];
            demux_desc[0].data.snap[3] = (u_int8_t)snapAddr[3];
            demux_desc[0].data.snap[4] = (u_int8_t)snapAddr[4];
            break;
        
        default:
            return -1;	// return -1 as an error if an unknown protocol type was passed in
            break;
    }
    
    result = setsockopt(fd, SOL_NDRVPROTO, NDRV_SETDMXSPEC, (caddr_t)&desc, sizeof(desc));
	if (result)
	fprintf(stderr, "error on setsockopt %d\n", result);
    return result;
}

/*******************************************************************************
** DoAddDelMulticast - used to add or delete a multicast address when receiving
** packets.
********************************************************************************/

int DoAddDelMulticast(int fd, unsigned char *mcAddr, Boolean addFlag)
{
    struct sockaddr_dl	dl;
    int			result;

    bzero(&dl, sizeof(dl));
    dl.sdl_len = sizeof(dl);
    dl.sdl_family = AF_LINK;
    dl.sdl_type = IFT_ETHER;
    dl.sdl_nlen = 0;
    dl.sdl_alen = sizeof(struct ether_addr);
    bcopy(mcAddr, dl.sdl_data, sizeof(struct ether_addr));

    // enable multicast reception
    if (addFlag)
    {
        result = setsockopt(fd, SOL_NDRVPROTO, NDRV_ADDMULTICAST, &dl, sizeof(dl));
        if (result < 0) 
        {
            fprintf(stderr, "setsockopt(NDRV_ADDMULTICAST) failed: %s\n", strerror(errno));
        }
    }
    else
    {
        result = setsockopt(fd, SOL_NDRVPROTO, NDRV_DELMULTICAST, &dl, sizeof(dl));
        if (result < 0) 
        {
            fprintf(stderr, "setsockopt(NDRV_DELMULTICAST) failed: %s\n", strerror(errno));
        }
    }
    

    return result;
}

OSStatus DoBSDLLCWriteTest(EnetTestData *enetdata)
{
    OSStatus	err = noErr;
    int			result;
    UInt32		i, datasize;
    
	enetdata->numPacketsSent = 0;
    memset(&gPacket, 0, sizeof(gPacket));		// zero out the global packet buffer structure
        // since we are writing, we don't have to add a protocol to listen for

	gPacket.rawModeOffset = 17;
		// check is we are doing SNAP
	if (enetdata->sap == 0xAA)
		gPacket.rawModeOffset += 5;


        // set up the first 18 bytes past the control byte for non SNAP LLC endpoint or
        // past the SNAP header for a SNAP endpoint, so that we can recognize it 
    strcpy((char*)&gPacket.data[gPacket.rawModeOffset], "begin data section\0");

        // set up some specific bytes in the data buffer that begins at the same point
        // relative to the LLC or SNAP header
    gPacket.data[DATAOFFSET     + gPacket.rawModeOffset] = 0;
    gPacket.data[DATAOFFSET + 1 + gPacket.rawModeOffset] = 0;
    strcpy((char*)&gPacket.data[DATAOFFSET+2 + gPacket.rawModeOffset], "end of data section\0");
    
	// set up for a rawmode send data call.  Fill in the enet header
	// set the destination address
	for (i = 0; i < sizeof(MACAddress); i++)
		gPacket.data[i] = enetdata->destaddr[i];
    
	// set the source address
	for (i = 0; i < sizeof(MACAddress); i++)
		gPacket.data[i+6] = enetdata->srcaddr[i];

	// set the packet len field
	// due to a bug in Mac OS X 10.1.x, if you set the data size field of the Ethernet header to 1500,
	// the packet will not be received. Detect if Mac OS 10.1.x is present and limit the data size to 1499
	datasize = DATASIZE;
	if (datasize >= 1500)
	 {
		 if (enetdata->verMacOS <= 0x1015)
		 {
			 datasize = 1499;
		 }
		 else
			 datasize = 1500;
	 }
    gPacket.data[12] = datasize >> 8;
    gPacket.data[13] = datasize & 0xFF;

	// set the dsap, ssap, and control byte fields.
    gPacket.data[14] = enetdata->sap;	// set DSAP
    gPacket.data[15] = enetdata->sap;	// set SSAP
    gPacket.data[16] = 0x03;			// set control byte
    
    if (enetdata->sap == 0xAA)
    {
        // set up the SNAP Addr
        for (i = 0; i < 5; i++)
            gPacket.data[i+17] = enetdata->snap[i];
    }

    enetdata->tbegin = clock ();
    
    gDone = false;
    gPacket.i = 0;
        
    while (!gDone)
    {
		// send the packet and make sure to include the length of the ethernet header
        if ((result = DoSendPacket(enetdata, &gPacket.data, datasize + ETHER_HDR_LEN)) == (datasize + ETHER_HDR_LEN))
        {
            err = noErr;
			enetdata->numPacketsSent++;
        }
        else
        {
            fprintf(stderr, "\n error occurred sending data, only %d bytes sent", result);
        }
                                
    }	// end while loop sending data

    enetdata->tend = clock();
	return err;
}

int DoSendPacket(EnetTestData *enetdata, const void *vptr, size_t n)
{
    int		nleft = 0;
    int		nwritten = 0;
    const char	*ptr;
            
    if (gDone == false)
    {
        ptr = vptr;
        nleft = n;
        while (nleft > 0)
        {
 //           nwritten = write(fd, ptr, nleft);
            nwritten = sendto(enetdata->fd, ptr, nleft, 0, &(enetdata->saddr), sizeof(enetdata->saddr));
            if (nwritten <= 0)
            {
                if (errno == EINTR)
                    nwritten = 0;	// try calling write once more
                else
                {
                    fprintf (stderr, "\n error writing data, errno - %d", errno);
                    enetdata->sendErrors++;
                    if (enetdata->sendErrors > 10)
                        gDone = true;
                    return nwritten;
                }
            }
            nleft -= nwritten;
            ptr += nwritten;
            
        }
    }
    
    gPacket.i++;
                                    
    if (gPacket.i >= enetdata->numPacketsToSend)
            gDone = true;
    else
    {
                    // increment the counter in the packet
            gPacket.data[DATAOFFSET+0+gPacket.rawModeOffset] = gPacket.i >> 8;
            gPacket.data[DATAOFFSET+1+gPacket.rawModeOffset] = gPacket.i;
                    
    }				
    return n;
}

static void* CallBSDRead (void *data)
{
    EnetTestData *enetdata = (EnetTestData*)data;
    int		offset, lcounter;
    int		addrlen;
    int		nread;
    char	buff[1530];
    OSStatus 	err = noErr;
    
    while ((enetdata->flag & kQuitThread) == 0)
    {
        addrlen = sizeof(enetdata->saddr);
		// issue a blocking read call
        nread = recvfrom(enetdata->fd, buff, sizeof(buff), 0, &(enetdata->saddr), &addrlen);
//        nread = recvfrom(enetdata->fd, buff, sizeof(buff), 0, NULL, NULL);
        if (nread < 0)
        {
             fprintf (stderr, "error on recvfrom - %d\n", nread);
             enetdata->readErrors++;
        }
        else
        {
            enetdata->packetsRead++;
                // if rawmode is on then we want to account for
                // the additional 17 bytes which will be at the
                // beginning of the packet.
            offset = 17;
            if (enetdata->sap == 0xAA)
            {
                offset += 5;		// account for the SNAP header
            }
                                    
            lcounter = buff[DATAOFFSET + offset + 0] << 8;
            lcounter |= buff[DATAOFFSET + offset + 1];
				// the lastPacketNum value (zero based) is displayed in the statistics 
				// so add 1 to the value so
				// that it appears the same as the packetsRead value which is 1 based
			enetdata->lastPacketNum = lcounter + 1;
			
			if (lcounter == 0)	// check if we are receiving a new sequence of test packets
								// the following assumes that we always receive the first packet of a series
								// which is not necessarily the way things are going to be.
			{
				enetdata->packetsInOrder = 0;
				enetdata->packetsOutOfOrder = 0;
			}
			else if (lcounter == gCounter)
				enetdata->packetsInOrder++;
			else
				enetdata->packetsOutOfOrder++;
			
			gCounter = lcounter + 1;	// prepare gCounter for next incoming packet to compare
        }
    }
    
    return (void*)err;
}

OSStatus DoCancelBSDLLCReadTest(EnetTestData *enetdata)
{
	OSStatus	err = noErr;

	// set the flag so that the MPTask will quit
	if (enetdata->flag & kThreadActive)
	{
		enetdata->flag |= kQuitThread;
		// if the task is still active then call terminateTask
		err = pthread_cancel(enetdata->pthread);
		if (err != noErr)
		{
			fprintf(stderr, "pthread_cancel returned error %ld\n", err);
		}
		
		close(enetdata->fd);
		enetdata->fd = 0;

	}

	return err;
}
				
OSStatus DoBSDLLCReadTest(EnetTestData *enetdata)
{
    int		result = noErr;
    
    gCounter = 0;
    gDone = false;

    result = pthread_create(&(enetdata->pthread), NULL, CallBSDRead, enetdata);

    if (result == kOTNoError)
    {
        fprintf (stderr, "pthread_create worked\n");
        enetdata->flag |= kThreadActive;
    }
    else
    {
        fprintf (stderr, "error on pthread_create - %d\n", result);
        enetdata->flag &= (-1 ^ kThreadActive);
    }
    
	return result;
}

/*
	GetPFNDRVSocket: entry function for obtaining a PF_NDRV socket.
	Parameters:
		socket - pointer to an int where the PF_NDRV socket is to be returned. 
	Result: noErr if the socket was opened by the PFNDRVTool, otherwise an error
	is returned and socket is set to -1;
	
	Uses the MoreAuthSample code to call the tooll, which is included in this sample
	For the complete sample, go to
	<http://developer.apple.com/samplecode/Sample_Code/Security/MoreAuthSample.htm>
	
*/
OSStatus GetPFNDRVSocket(int *socket)
{
    CFURLRef 			tool;
    OSStatus			err;
    AuthorizationRef	auth;
    CFDictionaryRef 	request;
    CFDictionaryRef 	response;
    CFStringRef 		key;
    CFStringRef 		value;

	*socket = -1;
    tool     = NULL;
    request  = NULL;
    response = NULL;
    auth 	 = NULL;
    // Create an Authorization Services environment.  Normally your
    // application would do this as it begins so that it can pre-authorize.
    // However, I don't pre-authorized because a) the pre-authorize flag
    // does nothing in current versions of Mac OS X [2907852], and b) doing
    // the pre-authorize triggers two authentication dialogs the first time
    // you run the application, which is never what you want.

    err = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);
            // Find our helper tool, possibly restoring it from the template.
    if (err == noErr)
    {
        err = MoreSecCopyHelperToolURLAndCheckBundled(CFBundleGetMainBundle(), CFSTR("GetPFNDRVSocketToolTemplate"), kApplicationSupportFolderType, CFSTR("BSDLLCTest"), CFSTR("GetPFNDRVSocketTool"), &tool);
    }


    // Create the request dictionary.
    if (err == noErr)
    {
		// pass in the command name
        key   = kBSDLLCTestCommandNameKey;
        value = kBSDLLCTestGetPFNDRVSocket;
        request = CFDictionaryCreate(NULL, (const void **) &key, (const void **) &value, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        err = CFQError(request);
    }
    
    // Go go gadget helper tool!
    // open the ethernet socket
    // to do this we call our tool which will run with the SUID bit set so that we
    // can open a PF_NDRV socket. On success, the socket is returned for us to use.
    if (err == noErr)
    {
        err = MoreSecExecuteRequestInHelperTool(tool, auth, request, &response);
    }

	// Extract information from the response.

    if (err == noErr)
    {
//		CFShow(response);

        err = MoreSecGetErrorFromResponse(response);
        if (err == noErr)
        {
			// things worked - get the PFNDRV socket

			CFArrayRef 	descArray;
			CFIndex		descIndex;
			CFIndex		descCount;

			// we are looking for a returned file descriptor, so specify the
			// kMoreSecFileDescriptorsKey which is set in the tool and 
			// is handled in a special manner by
			// the MoreAuthSample framework.
			descArray = (CFArrayRef) CFDictionaryGetValue(response, kMoreSecFileDescriptorsKey);
			assert(descArray != NULL);
			assert( CFGetTypeID(descArray) == CFArrayGetTypeID() );
			
			descCount = CFArrayGetCount(descArray);
			assert (descCount == 1); 	// expect only a single response
			for (descIndex = 0; descIndex < descCount; descIndex++)
			{
				CFNumberRef thisDescNum;
				
				thisDescNum = (CFNumberRef) CFArrayGetValueAtIndex(descArray, descIndex);
				assert( (thisDescNum != NULL) && (CFGetTypeID(thisDescNum) == CFNumberGetTypeID()) );

				// Normally it's bad to include function calls that have side effects 
				// within an "assert", but in this case the assert is guaranteed 
				// to be in effect because we're inside a MORE_DEBUG block.
				
				assert( CFNumberGetValue(thisDescNum, kCFNumberIntType, socket) );
				assert(*socket >= 0);
				fprintf(stderr, "returned socket is %d\n", *socket);
			}

        }
    }

    return err;
}

OSStatus DoEnetTest(EnetTestData *enetdata)
{
	OSStatus			err;

    
	// check if we are trying to cancel an active receive test
	if (enetdata->sendRcvState != kCancelTest)
	{
        // need to get a privileged socket
        err = GetPFNDRVSocket(&(enetdata->fd));

        if (err)
        {
            fprintf(stderr, "Error trying to open PF_NDRV socket %d\n", (int)err);
        }

        if (err == noErr)
        {            
            enetdata->saddr.sa_len = sizeof(struct sockaddr);
            enetdata->saddr.sa_family = AF_NDRV;
            err = bind(enetdata->fd, &(enetdata->saddr), sizeof(enetdata->saddr));
            if (err != noErr)
            {
                fprintf(stderr, "\n\nError binding raw Ethernet socket!");
                fprintf(stderr, "\nerrno is %d\n", errno);
            }
        }

        if (err == noErr)
        {
            // set up a protocol match - this is needed for listening to incoming packets
            // however for enet connections which are not presently active, adding the protocol
            // will get the stack active on the connection.
            if (enetdata->sap != 0xAA)
            {
                memset(&(enetdata->snap), 0, sizeof(enetdata->snap));
                err = DoAddProtocolMatch(enetdata->fd, DLIL_DESC_SAP, NDRV_DEMUXTYPE_SAP, enetdata->sap, enetdata->sap, CONTROLCODE, (u_char*)&(enetdata->snap));
            }
            else
            {
                err = DoAddProtocolMatch(enetdata->fd, DLIL_DESC_SNAP, NDRV_DEMUXTYPE_SNAP, enetdata->sap, enetdata->sap, CONTROLCODE, (u_char *)&(enetdata->snap));
            }

            if (err != noErr)
                fprintf(stderr, "DoAddProtocolMatch failed with error %ld\n", err);
        }
    
		if (err == noErr)
		{
			if (enetdata->sendRcvState == kSendTest)
			{
#if 0
				/* For Mac OS X 10.1.x, a problem can occur if trying to open a connection
					to an Ethernet NIC which is currently inactive. Some NIC's do not
					respond to being opened immediately - while the PFNDRV socket gets
					opened, nothing is sent across the wire.
					if this is the first time we are using a non-built in connection
					then sleep for 3 seconds.
				*/
				if (enetdata->isFirstTime == TRUE)
				{
					fprintf(stderr, "sleeping 5 secs\n");
					sleep(5);
				}
#endif
				DoBSDLLCWriteTest(enetdata);
				close(enetdata->fd);
				enetdata->fd = 0;
			}
			else
			{
				// this is a listening test.  check if the designated receive address has
				// been specified as a multicast address. Actually we could programmatically
				// figure this out
                if (enetdata->destAddrIsMCast)
                {
                    err = DoAddDelMulticast(enetdata->fd, (unsigned char*) enetdata->destaddr, true);
                    if (err)
                        fprintf(stderr, "DoAddDelMulticast failed with error %ld\n", err);
                    else
                        fprintf(stderr, "DoAddDelMulticast workedn");
                }
			
				
				if (err == noErr)
				{
					DoBSDLLCReadTest(enetdata);
					// when this routine finishes, we have started a receive test which
					// is currently active.
				}
				
				if (err)
				{
					close(enetdata->fd);
					enetdata->fd = 0;
				}
			}
		}
	}
	else
	{
		// we need to cancel an active receive test
		err = DoCancelBSDLLCReadTest(enetdata);
		if (err)
			fprintf(stderr, "Error occurred canceling the read test %ld\n", err);
		else
			fprintf(stderr, "cancelled the read test OK.\n");
	}
	return err;
}
