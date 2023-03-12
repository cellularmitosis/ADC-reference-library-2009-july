/*
	File:		SimplePing.h
	
	Description:	This file is a header for a  simple API to ping a remote host.
	
	Author:		Chad Jones 

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): 
        10/3/03		Chad Jones	Added comment indicating sample requires build tools 
                                        ProjectBuilder 2.1 or later
*/

//Note this sample requires Mac OS X 10.2.x or later and 
//ProjectBuilder version 2.1 or later.

#if !defined(__DTSSampleCode_SimplePing__)
#define __DTSSampleCode_SimplePing__ 1

#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

struct PingICMPPacket 
{
    struct icmp 	icmpHeader; //required icmp header
    struct timeval 	packetTimeStamp; //our optional data will be the packet send time.
};

/*****************************************************
 * LookupHostAddress
 *****************************************************
 * Purpose:  This function provides a mechanism to get a host address
 * as a internet address structure given only a string describing the
 * remote host.  The string passed will be either a host name
 * (e.g. www.apple.com) or an IP address (e.g. 17.203.23.111)
 *
 * Parameters:
 * 	HostToPing		A constant C-string.  On calling
 * LookupHostAddress this variable holds a CString describing the
 * remote host.  The string can either be a host name (e.g. www.google.com)
 * or an IP address (e.g. 17.203.23.112)
 *
 * 	HostAddress	A pointer to a pre-allocated array of sockaddr_in structure.
 * On calling LookupHostAddress this variable must be a pointer to a
 * pre-allocated sockaddr_in structure.  On successful return from LookupHostAddress
 * this variable will hold the host address expressed as an sockaddr_in structure.
 * On failed return the value of this variable is undefined
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
int LookupHostAddress(const char* HostToPing, struct sockaddr_in* HostAddress);

/*****************************************************
 * CreateSocketForCommunicationWithHost
 *****************************************************
 * Purpose:  This function sets up a socket for communicating
 * with the given remote host and also sets the parameters of 
 * that socket to our required specifications (timeout of 1 second
 * and larger receive buffer).
 *
 * Parameters:
 * 	HostAddress		A sockaddr_in structure.  On calling 
 * CreateSocketForCommunicationWithHost this variable has an address
 * of the host the socket will be established with expressed in sockaddr_in format.
 *
 * 	SocketToReturn	A pointer to a pre-allocated integer.  On successful 
 * return from CreateSocketForCommunicationWithHost this variable will hold
 * a socket descriptor used to communicate with the socket in future calls.  On
 * failed result this variable will be undefined.  Note on successful result its
 * the caller's responsibility to close the socket.
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
int CreateSocketForCommunicationWithHost(const struct sockaddr_in HostAddress, int* SocketToReturn);

/*****************************************************
 * CreateAndSendICMPPacket
 *****************************************************
 * Purpose:  This function creates an ICMP packet with the necessary information
 * inside of it to make it a ping packet.  This function also sends that ping packet to
 * the remote host.
 *
 * Parameters:
 * 	HostAddress		A sockaddr_in structure.  On calling 
 * CreateAndSendICMPPacket this variable has an address
 * of the host where the ping will be sent.
 *
 * 	SequenceNumber	A integer.  when calling CreateAndSendICMPPacket this variable 
 * will hold the sequence number of the ping packet being sent.  This information is
 * then put in the ping packet sent to the host so when they reply we know which
 * send the reply is associated with.
 *
 *	SocketConnectionToHost A socket descriptor expressed as an integer.  On calling
 * CreateAndSendICMPPacket this variable will be a valid socket which is already setup
 * for communicating with the remote host.  
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
int CreateAndSendICMPPacket(struct sockaddr_in HostAddress, int SequenceNumber, int SocketConnectionToHost);

/*****************************************************
 * WaitAndPrintICMPs
 *****************************************************
 * Purpose:  This function creates an ICMP packet with the necessary information
 * inside of it to make it a ping packet.  This function also sends that ping packet to
 * the remote host.
 *
 * Parameters:
 *	SocketConnectionToHost A socket descriptor expressed as an integer.  On calling
 * WaitAndPrintICMPs this variable will be a valid socket which is already setup
 * for communicating with the remote host.  This will be used in getting the reply
 * packets from the host.
 *
 * 	TimeoutInSeconds	A timeout expressed as an integer.  On calling 
 * WaitAndPrintICMPs this variable will have the number of seconds to wait for
 * ICMP replies.  Note even if a valid ICMP reply comes that matches what we expect
 * we will still wait for additional ICMP packets until the timeout has occurred.
 *
 * 	GotResponse		A pre-allocated integer.  when calling WaitAndPrintICMPs
 * this variable will be a pre-allocated integer.  On successful return from WaitAndPrintICMPs this
 * variable will hold a value of 1 if a ICMP packet response to our ping was received.
 * And a zero otherwise.  On failed return the value of this variable will be undefined.
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
int WaitAndPrintICMPs(int socketConnectionToHost, int TimeoutInSeconds, int ReturnimmediatelyAfterReply, int* GotResponse);

/*****************************************************
 * SimplePing
 *****************************************************
 * Purpose:  This function uses the above functions to ping a given remote host with
 * a given number of packets and with a given timeout to wait for responses on each packet.
 *
 * Parameters:
 *	HostToPing 		A C-String describing the remote host.  On calling
 * SimplePing this variable will hold a description of the host as a C-String.  The string
 * can either be a hostname (e.g. www.google.com) or an IP address (e.g. 17.203.23.111)
 *
 * 	NumberOfPacketsToSend	An integer.  On calling SimplePing this variable holds
 * the number of ping packets to send to the host before stopping.
 *
 * 	PingTimeoutInSeconds	An integer.  On calling Simple Ping this variable holds
 * the amount of time SimplePing will wait for responses from the ping before going onto the next packet.
 * Note even if a ping reply is received we will continue waiting until the timeout has occurred.
 *
 * 	*Function Result* 	A UNIX error integer return value as described in:
 *				/usr/include/sys/errno.h on a BSD system.
 * 				Note: This value will be zero on success.
 *****************************************************/
int SimplePing(const char* HostToPing, const int NumberOfPacketsToSend, const int PingTimeoutInSeconds, const int ReturnimmediatelyAfterReply);

//Checksum function stolen from original ping ;-)
int in_cksum(u_short *addr, int len);

#endif