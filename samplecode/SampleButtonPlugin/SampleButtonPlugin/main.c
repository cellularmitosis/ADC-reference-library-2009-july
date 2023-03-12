/*
	File:		main.c
	
	Description: Sample Button Plugin

	Author:	Image Capture Engineering

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
    
    	   <1>	 	6/17/03	WN			first file

*/
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#include <mach/mach.h>
#include <pthread.h>
#include <time.h>

#define THREAD 0
#define DEBUG 0

typedef CALLBACK_API_C( void , ButtonPressedCallback )(OSType 	message,
                                                       UInt32* 	locationID,
                                                       UInt64* 	guid);

//-------------------------------------------------------------------------------------- MyDevice
typedef struct MyDevice
{
    UInt32				sLocationID;
#if THREAD
    pthread_t			sUnsolEventProcThread;
#else
    EventLoopTimerRef 	sTimerRef;
#endif
	ButtonPressedCallback sButtonCallback;
} MyDevice;

// globals
MyDevice gMyDevice;

// prototypes
UInt8 PushButtonStatus();
void CancelThreads();
void *UnsolEventProcThreadEntryPoint(void *inParam);
void UnsolEventProcThreadCleanupHandler(void *inParam);

#pragma mark -

//-------------------------------------------------------------------------------------- PushButtonStatus
UInt8 PushButtonStatus()
{
    KeyMap theKeys;
    KeyMap f5Key = { 0x00000000, 0x0000000C, 0x00000000, 0x00000000 };
    
    GetKeys(theKeys);
    return (0 == memcmp(theKeys, f5Key, sizeof(KeyMap)));
}

//-------------------------------------------------------------------------------------- CancelThreads
void CancelThreads()
{
#if THREAD
    int		fStatus;
    void	*fResult;

    // unblock thread processing unsol.status notifications and cancel it
    fStatus = pthread_cancel( gMyDevice.sUnsolEventProcThread );
	fStatus = pthread_join( gMyDevice.sUnsolEventProcThread, &fResult );
#endif
}

//-------------------------------------------------------------------------------------- UnsolEventProcThreadEntryPoint
void *UnsolEventProcThreadEntryPoint(void *inParam)
{
//	printf( "UnsolEventProcThreadEntryPoint\n" );

#if THREAD
    pthread_cleanup_push( UnsolEventProcThreadCleanupHandler, NULL );
    while( 1 )
    {
		UInt32 button;
		struct timespec sleepTime;

        pthread_testcancel();	// check if the thread has been canceled
		sleepTime.tv_sec = 1;
		sleepTime.tv_nsec = 0;
		button = PushButtonStatus();
		if (button)
			gMyDevice.sButtonCallback('but1', &gMyDevice.sLocationID, nil);
		nanosleep(&sleepTime, 0);
    }
	pthread_cleanup_pop( 1 );
#else
#endif
	return NULL;
}

//-------------------------------------------------------------------------------------- UnsolEventProcThreadCleanupHandler
void UnsolEventProcThreadCleanupHandler(void *inParam)
{
//    printf( "UnsolEventProcThreadCleanupHandler\n" );
}

//-------------------------------------------------------------------------------------- TimerEvent
pascal void TimerEvent (EventLoopTimerRef inTimer,
                        void *inUserData)
{
    UInt32 button;
    button = PushButtonStatus();
    if (button)
        gMyDevice.sButtonCallback('but1', &gMyDevice.sLocationID, nil);
}

#pragma mark -

//-------------------------------------------------------------------------------------- TWAINButtonPluginStart
OSErr TWAINButtonPluginStart(UInt32* 	 locationID,
                             UInt64* 	 guid,
                             io_string_t twainPath,
                             ButtonPressedCallback callback)
{
	IOReturn err = noErr;

#if DEBUG
    printf("---------------------------------------\n");
    printf("        TWAINButtonPluginStart\n");
    printf("---------------------------------------\n");
#endif
    
	memset(&gMyDevice, 0, sizeof(gMyDevice));
	gMyDevice.sLocationID = *locationID;

    require(err == noErr, bail);
	
	gMyDevice.sButtonCallback = callback;
#if THREAD
    err = pthread_create(
				&gMyDevice.sUnsolEventProcThread,
				NULL,
				UnsolEventProcThreadEntryPoint,
				NULL
			);
#else
    err = InstallEventLoopTimer( GetCurrentEventLoop(),
                                 (kEventDurationSecond),
                                 (kEventDurationSecond),
                                 NewEventLoopTimerUPP(TimerEvent),
                                 NULL,
                                 &gMyDevice.sTimerRef);
#endif
    
bail:
#if DEBUG
    if (err)
        printf("exiting TWAINButtonPluginStart err = 0x%08x\n", err);
#endif 
    return err;
}

//-------------------------------------------------------------------------------------- TWAINButtonPluginStop
OSErr TWAINButtonPluginStop(UInt32* 	 locationID,
                            UInt64* 	 guid,
                            io_string_t twainPath)
{
#if DEBUG
    printf("---------------------------------------\n");
    printf("        TWAINButtonPluginStop\n");
    printf("---------------------------------------\n");
#endif 

#if THREAD
    CancelThreads();
#else
    RemoveEventLoopTimer(gMyDevice.sTimerRef);
#endif
    
    return noErr;
}

