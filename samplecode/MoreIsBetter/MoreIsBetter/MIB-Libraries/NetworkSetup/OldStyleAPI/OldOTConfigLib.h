/*
	File:		OldOTConfigLib.h

	Contains:	Old interface to the OT TCP/IP and AppleTalk configurators.

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: OldOTConfigLib.h,v $
Revision 1.4  2001/11/07 15:56:35         
Tidy up headers, add CVS logs, update copyright.


         <3>     21/9/01    Quinn   Get rid of wacky Finder label.
         <2>     5/11/98    Quinn   Fix headers.
         <1>     5/11/98    Quinn   First checked in.
*/

#ifndef __OLDOTCONFIGLIB__
#define __OLDOTCONFIGLIB__

/* Consequences of changing configuration */

enum Consequence
{
	kNoAnswer		= -1,	/* Couldn't get in touch with the protocol (probably not loaded => benign)	*/
	kBenignChange	=  0,	/* Change won't disturb aything												*/
	kKillsServices	=  1,	/* Change will interrupt connections currently established					*/
	kMustReboot		=  2	/* Change requires a reboot to take effect									*/
};

#ifdef __cplusplus
extern "C" {
#endif

extern SInt32 TCPCheckChangeConfigurationConsequences(SInt16 resFileRefNum, SInt16 configResID);
	/* Check with TCP/IP what would be the consequences of changing the current 					*/
	/* config to the specified one.																	*/

extern OSErr TCPChangeConfiguration(SInt16 resFileRefNum, SInt16 configResID);
	/* Makes the TCP/IP configuration the active configuration and notifies							*/
	/* the protocol of this change.																	*/
	/* The protocol will react as announced by TCPCheckChangeConfigurationConsequences				*/

extern SInt32 ATCheckChangeConfigurationConsequences(SInt16 ref, SInt16 config);
	/* Check with AppleTalk what would be the consequences of changing the current 					*/
	/* config to the specified one.																	*/

extern OSErr ATChangeConfiguration(SInt16 ref, SInt16 config);
	/* Makes the AppleTalk configuration the active configuration and notifies						*/
	/* the protocol of this change.																	*/
	/* The protocol will react as announced by TCPCheckChangeConfigurationConsequences				*/

#ifdef __cplusplus
}
#endif

#endif
