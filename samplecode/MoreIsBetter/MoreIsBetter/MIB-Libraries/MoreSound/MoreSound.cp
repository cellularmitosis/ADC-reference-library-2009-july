/*
	File:		MoreSound.cp

	Contains:	helper functions for simple use of Sound Manager

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998, Apple Computer, Inc. All Rights Reserved.

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

$Log: MoreSound.cp,v $
Revision 1.6  2002/11/09 00:11:26         
Convert nil to NULL. Convert MoreAssertQ to assert. Convert MoreAssert to MoreAssertPCG.

Revision 1.5  2001/11/07 15:55:28         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Get rid of wacky Finder label.
         <3>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <1>    10/23/98    PCG     initial check-in
*/

#include "MoreSetup.h"

#include "MoreSound.h"

pascal OSErr NewDigitizedSoundChannel (UInt16 qLength, SndChannelPtr *result)
{
	OSErr err = noErr;

	*result = (SndChannelPtr) ::NewPtr ((sizeof (**result) - sizeof ((**result).queue)) + (sizeof ((**result).queue[0]) * qLength));

	if (!*result)
		err = ::MemError ( );
	else
	{
		(**result).qLength = qLength;
		err = ::SndNewChannel (result,0,0,NULL);
		if (err) ::DisposePtr (Ptr (*result));
	}

	return err;
}

pascal OSErr DisposeSoundChannel (SndChannelPtr chan)
{
	OSErr err = ::SndDisposeChannel (chan,true);

	if (!err)
	{
		::DisposePtr (Ptr (chan));
		assert(MemError ( ) == noErr);
	}

	return err;
}

pascal OSErr PlaySoundListHandle (SndChannelPtr chan, SndListHandle hand, Boolean asynch)
{
	OSErr err = noErr;

	if (!MoreAssertPCG (chan && hand && *hand))
		err = paramErr;
	else
	{

#if MORE_DEBUG

		if (asynch)
		{
			SInt8 hState = ::HGetState (Handle (hand));
			if (MoreAssertPCG (!(err = MemError ( ))))
			{
				assert(hState & 0x80); // is it locked?
			}
		}

#endif

		err = ::SndPlay (chan,hand,asynch);
	}

	return err;
}

pascal OSErr IsSoundChannelPlaying (SndChannelPtr chan, Boolean *isPlaying)
{
	OSErr err = noErr;

	if (!MoreAssertPCG (chan))
		err = paramErr;
	else
	{
		SCStatus status;

		if (!(err = ::SndChannelStatus (chan,sizeof(status),&status)))
			*isPlaying = status.scChannelBusy && !(status.scChannelPaused);
	}

	return err;
}
