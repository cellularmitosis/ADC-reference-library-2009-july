/*
	File:		MoreBacktraceTest.c

	Contains:	A simple program to test MoreBacktrace.

	Written by:	DTS

	Copyright:	Copyright © 2003 by Apple Computer, Inc., All Rights Reserved.

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

*/

/////////////////////////////////////////////////////////////////

#include "MoreSetup.h"

// System prototypes

// Include <mach/mach.h> for the Mach-O build.  Put it inside
// extern "C" guards for the C++ build because the Mach header 
// files don't have them.  Put it before main includes because 
// Carbon includes Mach via I/O Kit via CG.

#if TARGET_RT_MAC_MACHO
	#if defined(__cplusplus)
		extern "C" {
	#endif

	#include <mach/mach.h>

	#if defined(__cplusplus)
		}
	#endif
#endif

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <MacTypes.h>
	#include <IBCarbonRuntime.h>
	#include <Debugging.h>
	#include <Gestalt.h>
	#include <TextEdit.h>
#endif

#if TARGET_RT_MAC_MACHO
	#include <unistd.h>
	#include <signal.h>
#endif

#include <stdlib.h>

// MIB Prototypes

#include "MoreOSUtils.h"
#include "MoreProcesses.h"
#include "MoreCFQ.h"
#include "MoreControls.h"
#include "MoreBacktrace.h"
#include "MoreAddrToSym.h"

#if TARGET_RT_MAC_MACHO
	#include "MoreUNIX.h"
#endif

/////////////////////////////////////////////////////////////////

#if TARGET_API_MAC_CARBON

	static UInt8  gOutputFont[] = "\pMonaco";
	static SInt16 gOutputSize = 10;

	enum {
		kFrameCount = 70
	};
	
	static ControlRef gOutputText = NULL;
	static ControlRef gOutputUser = NULL;			// NULL indicates that gOutputText is in use
	static ControlRef gPIDText = NULL;

	static void DoAbout(void)
		// Displays the about box.
	{
		SInt16 junkHit;
		
		(void) StandardAlert(kAlertPlainAlert, "\pMoreBacktraceTest", "\pA simple program to test MoreBacktrace.\r\rDTS\r\r© 2003 Apple Computer, Inc.", NULL, &junkHit);
	}
	
	static void DisplayError(OSStatus errNum)
	{
		OSStatus	junk;
		Str255 		errStr;
		SInt16 		junkHit;
		
		if ( (errNum != noErr) && (errNum != userCanceledErr) ) {
			NumToString(errNum, errStr);

			junk = StandardAlert(kAlertStopAlert, "\pError.", errStr, NULL, &junkHit);
			assert(junk == noErr);
		}
	}

	static ControlUserPaneDrawUPP gOutputUserPaneDrawUPP;		// -> OutputUserPaneDrawProc

	static pascal void OutputUserPaneDrawProc(ControlRef control, SInt16 part)
		// The draw procedure for gOutputUser.  This only runs on traditional 
		// Mac OS.  It calls TETextBox to draw the contents of the user pane 
		// control.  The text is stored in a CFStringRef in the control's 
		// refCon.
	{
		OSStatus	err;
		CFStringRef controlStr;
		CFIndex		unicodeLength;
		Handle		textH;
		CFIndex		textHSize;
		CFIndex		numberConverted;
		#pragma unused(part)
		
		// Get the text, calculate the number of bytes need to convert it to 
		// MacRoman, and allocate a handle of that size.  Note that hard coding 
		// MacRoman is OK in this case because, on traditional Mac OS, text 
		// encoding is related to font choice, and we hard code Monaco, which is 
		// a MacRoman font.  Further note that hard coding a font is a really 
		// bad idea in a production application.
		
		controlStr = (CFStringRef) GetControlReference(control);
		assert( (controlStr != NULL) && (CFGetTypeID(controlStr) == CFStringGetTypeID()) );
		
		unicodeLength = CFStringGetLength(controlStr);
		textHSize = CFStringGetMaximumSizeForEncoding(unicodeLength, kCFStringEncodingMacRoman);
		
		textH = NewHandle(textHSize);
		err = MemError();
		
		// Get the MacRoman bytes out of the CFString into the handle.  Resize 
		// the handle to encompass just the bytes that we got.
		
		if (err == noErr) {
			HLock(textH);
			assert(MemError() == noErr);
			
			numberConverted = CFStringGetBytes(controlStr, CFRangeMake(0, unicodeLength), 
											   kCFStringEncodingMacRoman, 0, false, 
											   (UInt8 *) *textH, textHSize, &textHSize);

			HUnlock(textH);
			assert(MemError() == noErr);

			if ( numberConverted == unicodeLength ) {
				SetHandleSize(textH, textHSize);
				assert(MemError() == noErr);
			} else {
				err = coreFoundationUnknownErr;
			}
		}
		
		// Draw the handle of text into the control's bounding box using 
		// TETextBox.
		
		if (err == noErr) {
			GrafPtr oldPort;
			SInt16	monacoFontNum;
			Rect	bounds;
			
			HLock(textH);
			
			GetPort(&oldPort);
			SetPort(GetWindowPort(GetControlOwner(control)));
			
			(void) GetControlBounds(control, &bounds);
			
			GetFNum(gOutputFont, &monacoFontNum);
			
			EraseRect(&bounds);
			
			TextFont(monacoFontNum);
			TextFace(0);
			TextMode(srcOr);
			TextSize(gOutputSize);
			
			TETextBox(*textH, textHSize, &bounds, teJustLeft);
			
			SetPort(oldPort);
		}

		// Clean up.
		
		if (textH != NULL) {
			DisposeHandle(textH);
			assert(MemError() == noErr);
		}
		
		assert(err == noErr);
	}

	static OSStatus SetOutputText(CFStringRef newText)
		// Put newText into the output control.  This is either 
		// an edit text control (gOutputText) or a user pane 
		// control (gOutputUser) depending on the system version. 
		// We use an edit text control on Mac OS X because it's 
		// a) easy, b) handles multiple lines of text, and 
		// c) allows the user to copy the results.  We use an 
		// user pane control on traditional Mac OS because the 
		// edit text control can't handle more than 255 
		// characters (at least when we set its contents using 
		// SetTextControlTextCompat).
	{
		OSStatus err;
		
		assert(newText != NULL);
		
		if (gOutputUser == NULL) {
			err = SetTextControlTextCompat(gOutputText, false, newText);
			Draw1Control(gOutputText);
		} else {
			CFStringRef oldText;
			
			// Get the old text and release it.
			
			oldText = (CFStringRef) GetControlReference(gOutputUser);
			assert( (oldText == NULL) || (CFGetTypeID(oldText) == CFStringGetTypeID()) );
			CFQRelease(oldText);
			
			// Retain the new text and install it.
			
			CFRetain(newText);
			SetControlReference(gOutputUser, (SInt32) newText);

			Draw1Control(gOutputUser);
			
			err = noErr;
		}
		return err;
	}

	static void ClearOutput(void)
		// Clear the output text.
	{
		OSStatus junk;
		
		junk = SetOutputText(CFSTR(""));
		assert(junk == noErr);
	}

	static OSStatus OutputFrames(const MoreBTPPCFrame *frameArray, ItemCount frameCount, Boolean lookupSymbolNames)
		// Output a textual description of frameCount frames from frameArray 
		// into the output text control.  If lookupSymbolNames is true, 
		// we look up the symbol names of the PCs of each of the frames 
		// (currently on Mach-O builds only).
	{
		OSStatus			err;
		CFMutableStringRef 	result;
		ItemCount			frameIndex;
		ItemCount			row;
		ItemCount			col;
		MoreAToSSymInfo *	symbols;

		result = NULL;
		
		// Create an array of NULL CFStringRefs to hold the symbol pointers.
		
		err = noErr;
		symbols = (MoreAToSSymInfo *) malloc(frameCount * sizeof(*symbols));
		if (symbols == NULL) {
			err = memFullErr;
		}
		if (err == noErr) {
			MoreAToSCreate(frameCount, symbols);
		}
		
		// If we've been asked to look up the symbols, do so.
		
		if (err == noErr && lookupSymbolNames) {
			MoreAToSAddr *addresses;
			
			addresses = (MoreAToSAddr *) malloc(frameCount * sizeof(*addresses));
			if (addresses == NULL) {
				err = memFullErr;
			}
			if (err == noErr) {	
				for (frameIndex = 0; frameIndex < frameCount; frameIndex++) {
					if (frameArray[frameIndex].flags & kMoreBTPCBadMask) {
						addresses[frameIndex] = NULL;
					} else {
						addresses[frameIndex] = (MoreAToSAddr) frameArray[frameIndex].pc;
					}
				}
			}
			
			if (err == noErr) {
				#if TARGET_RT_MAC_MACHO
					err = MoreAToSCopySymbolNamesUsingDyld(frameCount, addresses, symbols);
				#else
					// do nothing
				#endif
			}
			
			free(addresses);
		}
		
		// Create the output string, starting with an empty string and then 
		// appending an entry for each frame.
		
		if (err == noErr) {
			result = CFStringCreateMutable(NULL, 0);
			err = CFQError(result);
		}
		if (err == noErr) {
			for (row = 0; row < (kFrameCount / 2); row++) {
				for (col = 0; col < 2; col++) {
					frameIndex = col * (kFrameCount / 2) + row;
					
					if ( frameIndex < frameCount ) {
						CFStringRef	thisSymbol;
						enum {
							kStringWidth = 23
						};
						
						CFStringAppendFormat(result, NULL, CFSTR("%2d %c%c%c %08x %08x "), 
								 frameIndex,
								 (frameArray[frameIndex].flags & kMoreBTFrameBadMask)      ? 'F' : 'f',
								 (frameArray[frameIndex].flags & kMoreBTPCBadMask)         ? 'P' : 'p',
								 (frameArray[frameIndex].flags & kMoreBTSignalHandlerMask) ? 'S' : 's',
								 frameArray[frameIndex].sp, 
								 frameArray[frameIndex].pc);
											 
						thisSymbol = symbols[frameIndex].symbolName;
						
						if (thisSymbol != NULL) {
							CFStringRef tmpStr;
							CFStringRef tmpStr2;
							CFIndex strLen;

							tmpStr = NULL;
							tmpStr2 = NULL;
							
							strLen = CFStringGetLength(thisSymbol);
							
							if ( strLen > kStringWidth ) {
								tmpStr = CFStringCreateWithSubstring(NULL, thisSymbol, CFRangeMake(0, kStringWidth));
								tmpStr2 = CFSTR("");
								CFRetain(tmpStr2);
							} else {
								tmpStr = thisSymbol;
								CFRetain(tmpStr);
								tmpStr2 = CFStringCreateWithFormat(NULL, NULL, CFSTR("%*.*s"), kStringWidth - strLen, kStringWidth - strLen, "");
							}
							CFStringAppendFormat(result, NULL, CFSTR("%@+%04x%@    "), 
												 tmpStr,
												 symbols[frameIndex].symbolOffset,
												 tmpStr2
												);
							CFQRelease(tmpStr);
							CFQRelease(tmpStr2);
						} else {
							CFStringAppendFormat(result, NULL, CFSTR("%*.*s"), 
												 kStringWidth + 1 + 4 + 4,
												 kStringWidth + 1 + 4 + 4,
												 ""
												);
						}
					}
				}
				CFStringAppendFormat(result, NULL, CFSTR("\r"));
			}
		}
		
		// Put the string into the control.
		
		if (err == noErr) {
			err = SetOutputText(result);
		}
		
		// Clean up.

		if (symbols != NULL) {
			MoreAToSDestroy(frameCount, symbols);
		}
		free(symbols);
		CFQRelease(result);
				
		return err;
	}

	static void TestMoreBacktracePPCCarbonSelf(void)
	{
		OSStatus		err;
		MoreBTPPCFrame 	frames[kFrameCount];
		ItemCount		frameCount;
		ItemCount		validFrames;

		ClearOutput();
		
		frameCount = sizeof(frames) / sizeof(*frames);
		err = MoreBacktracePPCCarbonSelf(0, 0, frames, frameCount, &validFrames);
		if (err == noErr) {
			if (validFrames > frameCount) {
				validFrames = frameCount;
			}
			err = OutputFrames(frames, validFrames, true);
		}
		DisplayError(err);
	}

	#if TARGET_RT_MAC_MACHO

		static void TestMoreBacktracePPCMachSelf(void)
		{
			OSStatus		err;
			MoreBTPPCFrame 	frames[kFrameCount];
			ItemCount		frameCount;
			ItemCount		validFrames;

			ClearOutput();
			
			frameCount = sizeof(frames) / sizeof(*frames);
			err = MoreBacktracePPCMachSelf(0, 0, frames, frameCount, &validFrames);
			if (err == noErr) {
				if (validFrames > frameCount) {
					validFrames = frameCount;
				}
				err = OutputFrames(frames, validFrames, true);
			}
			DisplayError(err);
		}

		static void TestMoreBacktracePPCMachThread(void)
		{
			OSStatus		err;
			OSStatus		junk;
			MoreBTPPCFrame 	frames[kFrameCount];
			ItemCount		frameCount;
			ItemCount		validFrames;
			CFStringRef		pidStr;
			Str255			pidPStr;
			long			pidLong;
			pid_t			pid;
			task_t			targetTask;
			Boolean			didSuspend;
			thread_array_t	threadList;
			mach_msg_type_number_t	threadCount;

			ClearOutput();

			targetTask = MACH_PORT_NULL;
			pidStr = NULL;
			threadList = NULL;
			
			didSuspend = false;
			
			// Get the PID of the process to sample from the 
			// gPIDText edit text control and convert that to 
			// the Mach task control port.
			
			err = CopyTextControlTextCompat(gPIDText, false, &pidStr);
			if (err == noErr) {
				err = CFQErrorBoolean( CFStringGetPascalString(pidStr, pidPStr, sizeof(pidPStr), kCFStringEncodingASCII) );
			}
			if (err == noErr) {
				StringToNum(pidPStr, &pidLong);
				pid = (pid_t) pidLong;
			}
			if (err == noErr) {
				err = task_for_pid(mach_task_self(), pid, &targetTask);
			}
			if (err == noErr) {
				if (targetTask == mach_task_self()) {
					err = -1;			// this won't go well
				}
			}
			
			// Suspend the task while we sample it.  Otherwise the 
			// list of threads might change.
			
			if (err == noErr) {
				err = task_suspend(targetTask);
				didSuspend = (err == noErr);
			}
			if (err == noErr) {
				err = task_threads(targetTask, &threadList, &threadCount);
			}
			if (err == noErr) {
				// A task without any threads makes no sense.
				
				assert(threadCount > 0);
				
				// We always sample the first thread.  This has no real 
				// significance because Mach doesn't guarantee to return 
				// the threads in any particular order.  In a real tool 
				// you'd iterate over all of the threads and sample each, 
				// but I have no way of displaying the results in my 
				// test application.
				
				frameCount = sizeof(frames) / sizeof(*frames);
				err = MoreBacktracePPCMachThread(targetTask, threadList[0], 0, 0, frames, frameCount, &validFrames);
			}
			
			// Resume the task as quickly as possibly after the backtrace.
			
			if (didSuspend) {
				junk = task_resume(targetTask);
				assert(junk == noErr);
			}
			
			if (err == noErr) {
				if (validFrames > frameCount) {
					validFrames = frameCount;
				}
				err = OutputFrames(frames, validFrames, false);
			}
			DisplayError(err);
			
			// Clean up.
			
			CFQRelease(pidStr);
			junk = mach_port_deallocate(mach_task_self(), targetTask); 
			assert(junk == 0);
			if (threadList != NULL) {
				mach_msg_type_number_t thisThread;
				
				for (thisThread = 0; thisThread < threadCount; thisThread++) {
					junk = mach_port_deallocate(mach_task_self(), threadList[thisThread]);
					assert(junk == noErr);
				}
				junk = vm_deallocate(mach_task_self(), (vm_address_t) threadList, threadCount * sizeof(*threadList));
				assert(junk == noErr);
			}
		}
		
		static void MySIGUSR1Handler(int signal)
			// Handle SIGUSR1.  Note that we call many functions 
			// here that aren't "signal safe".  However, we know 
			// that this signal isn't happening asynchronously, 
			// we're sending it to ourselves via a call to "kill", 
			// so most of the unsafeness is moot.  Still, if this 
			// wasn't a just test program I'd figure out a better way.
		{
			assert(signal == SIGUSR1);
			
			TestMoreBacktracePPCMachSelf();
		}

		static OSStatus TestSignalBacktraceNested(void)
			// Send a SIGUSR1 to ourselves.  Ideally I'd like to send 
			// this to our specific thread using pthread_kill, but that 
			// function doesn't existing on 10.1.  So, instead we rely 
			// on the behaviour documented in DTS Q&A 1184, which 
			// indicates that the main thread will catch the signal.
			// Of course, the Q&A says that you shouldn't rely on this 
			// behaviour, but hey, this is only a test program.
			//
			// <http://developer.apple.com/qa/qa2001/qa1184.html>
		{
			OSStatus err;
			
		    err = kill(getpid(), SIGUSR1);
		    err = MoreUNIXErrno(err);
		    return err;
		}

		static void TestSignalBacktrace(void)
		{
		    OSStatus 			err;
		    OSStatus 			junk;
		    struct sigaction 	oldSig;
		    struct sigaction 	newSig;

			// Install MySIGUSR1Handler as the SIGUSR1 handler.
			
		    newSig.sa_handler = &MySIGUSR1Handler;
		    sigemptyset(&newSig.sa_mask);
		    newSig.sa_flags   = SA_RESTART;
		    err = sigaction(SIGUSR1, &newSig, &oldSig);
		    err = MoreUNIXErrno(err);
		    
		    if (err == 0) {
		    
		    	// Call a routine that sends a signal to us.
		    	
		        err = TestSignalBacktraceNested();
		        
		        // Restore the old signal handler.
		        
		        junk = sigaction(SIGUSR1, &oldSig, NULL);
		        assert(junk == noErr);
		    }

			// Complain if we got an error.
			
			DisplayError(err);
		}

	#endif

	// Code to sample the current PC and SP.  I copied this directly 
	// out of "MoreBacktrace.c".  In an ideal world I'd only have one 
	// copy of this, but I don't want to export it from MoreBackTrace 
	// because it's so ugly.  So, for the moment, we have two copies.
	//
	// See the code "MoreBacktrace.c" for comments about how this works.
	//
	// Gosh, it'd be nice if GCC and CodeWarrior could agree about 
	// inline assembler.
	
	#if defined(__MWERKS__)

        #define MoreBTPPCGetStackPointer(result)						\
                asm {	addi	result,sp,0		}

		asm static const void * MoreBTPPCGetProgramCounter(void)
		{
			mflr		r3
			blr
		}
	
	#elif defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )

        #define MoreBTPPCGetStackPointer(result)						\
                __asm__ volatile("mr		%0,r1" : "=r" (result));

        static const void * MoreBTPPCGetProgramCounter(void)
		{
            const void * result;
            __asm__ volatile("mflr		%0" : "=r" (result));
            return result;
		}

	#else
		#error MoreBacktrace: What compiler are you using?
	#endif
	
	static Boolean	 				gDeadThreadReady = false;
	static MPQueueID 				gReadyQueue;
	static const void *	volatile 	gDeadThreadSP;
	static const void *	volatile 	gDeadThreadPC;
	
	static void DeadThreadNested4(void)
		// The victim routine itself.  It stores its PC and SP 
		// in globals, notifies gReadyQueue so that the main 
		// thread can continue, and then blocks indefinitely on 
		// waitForeverQ.
	{
		OSStatus 				err;
		MPQueueID 				waitForeverQ;
		register const void *	mySP;
		const void *			myPC;
		
		MoreBTPPCGetStackPointer(mySP);
		myPC = MoreBTPPCGetProgramCounter();
		
		gDeadThreadSP = mySP;
		gDeadThreadPC = myPC;
		
		err = MPCreateQueue(&waitForeverQ);
		if (err == noErr) {
			err = MPNotifyQueue(gReadyQueue, NULL, NULL, NULL);
		}
		if (err == noErr) {
			err = MPWaitOnQueue(waitForeverQ, NULL, NULL, NULL, kDurationForever);
		}
		assert(err == noErr);
	}

	static void DeadThreadNested3(void)
		// A few nested procedures so you get an interesting trace.
	{
		DeadThreadNested4();
	}
	
	static void DeadThreadNested2(void)
		// A few nested procedures so you get an interesting trace.
	{
		DeadThreadNested3();
	}
	
	static void DeadThreadNested1(void)
		// A few nested procedures so you get an interesting trace.
	{
		DeadThreadNested2();
	}
	
	static OSStatus DeadThread(void *param)
		// The thread entry point for the victim thread.
	{
		#pragma unused(param)
		DeadThreadNested1();
		return noErr;
	}

	static OSStatus StartDeadThread(void)
		// Start a victim thread whose stack we intend to smash 
		// in order to test our handling of bad pointers in a 
		// stack crawl.  When the thread starts up it initialises 
		// gDeadThreadSP and gDeadThreadPC, signals gReadyQueue, 
		// and then blocks forever.  We wait for the signal on 
		// gReadyQueue so that we know that gDeadThreadSP and 
		// gDeadThreadPC are set up before we return to our caller.
	{
		OSStatus 	err;

		err = noErr;
		if ( ! MPLibraryIsLoaded() ) {
			err = unimpErr;
		}
		if (err == noErr) {
			err = MPCreateQueue(&gReadyQueue);
		}
		if (err == noErr) {
			err = MPCreateTask(DeadThread, NULL, 0, kInvalidID, NULL, NULL, kNilOptions, NULL);
		}
		if (err == noErr) {
			err = MPWaitOnQueue(gReadyQueue, NULL, NULL, NULL, kDurationForever);
		}
		
		gDeadThreadReady = (err == noErr);
		
		return err;
	}
	
	static void TestStackSmash(void)
	{
		OSStatus				err;
		MoreBTPPCFrame 			frames[kFrameCount];
		ItemCount				frameCount;
		ItemCount				validFrames;
		static const void *		sBusErrorAddress = NULL;
		
		ClearOutput();
		
		if (sBusErrorAddress == NULL) {
			if ( MoreRunningOnMacOSX() ) {

				// I don't want to use 0 for the Mac OS X bus error value, because 
				// it's a little obvious.  Instead I use a value that's in the bottom 
				// page (ie is less than 0x1000) and is easily recognisable.
				
				sBusErrorAddress = (const void *) 0x0FEC;
				
			} else {
			
				// On traditional Mac OS we don't want to use 0 because it's mapped, 
				// so we used the blessed bus error value.
				
				sBusErrorAddress = (const void *) kBlessedBusErrorBait;
			}
		}
		
		// Start a thread that just blocks indefinitely.  This is the 
		// thread whose stack we're going to smash.
		
		err = noErr;
		if ( ! gDeadThreadReady ) {
			err = StartDeadThread();
		}
		assert( (err != noErr) || gDeadThreadReady );
		
		if (err == noErr) {
			int				i;
			const void **	thisFrame;
			
			frameCount = sizeof(frames) / sizeof(*frames);
			
			// We smash the stack in three different ways.
			//
			// 1. We smash the current PC address pointer.
			
			gDeadThreadPC = (const void *) sBusErrorAddress;
			
			// 2. We go down 3 frames on the stack and smash the next 
			//    frame pointer.
			
			thisFrame = (const void **) gDeadThreadSP;
			for (i = 0; i < 3; i++) {
				thisFrame = (const void **) *thisFrame;
			}
			*thisFrame = sBusErrorAddress;
			
			// 3. We go down 2 frames on the stack and smash the 
			//    return address.

			thisFrame = (const void **) gDeadThreadSP;
			for (i = 0; i < 2; i++) {
				thisFrame = (const void **) *thisFrame;
			}
			*((const void **)((const char *) thisFrame + 8)) = sBusErrorAddress;
			
			// Now let's do the backtrace and see what happens.
			
			err = MoreBacktracePPCCarbon((MoreBTPPCAddr) gDeadThreadPC, (MoreBTPPCAddr) gDeadThreadSP, 0, 0, frames, frameCount, &validFrames);
			if (err == noErr) {
				if (validFrames > frameCount) {
					validFrames = frameCount;
				}
				err = OutputFrames(frames, validFrames, true);
			}
		}

		DisplayError(err);
	}

	static EventHandlerUPP gApplicationEventHandlerUPP;		// -> ApplicationEventHandler

	static const EventTypeSpec kApplicationEvents[] = { {kEventClassCommand, kEventCommandProcess} };

	static pascal OSStatus ApplicationEventHandler(EventHandlerCallRef inHandlerCallRef, 
												   EventRef inEvent, void *inUserData)
		// Dispatches HICommands to their implementations.
	{
		OSStatus 	err;
		HICommand 	command;
		Boolean		displayMachAlert;
		#pragma unused(inHandlerCallRef)
		#pragma unused(inUserData)
		
		displayMachAlert = false;
		
		assert( GetEventClass(inEvent) == kEventClassCommand  );
		assert( GetEventKind(inEvent)  == kEventCommandProcess);
		
		err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command);
		if (err == noErr) {
			switch (command.commandID) {
				case kHICommandAbout:
					DoAbout();
					break;
				case 'btCS':
					TestMoreBacktracePPCCarbonSelf();
					break;
				case 'btMS':
					#if TARGET_RT_MAC_MACHO
						TestMoreBacktracePPCMachSelf();
					#else
						displayMachAlert = true;
					#endif
					break;
				case 'btMT':
					#if TARGET_RT_MAC_MACHO
						TestMoreBacktracePPCMachThread();
					#else
						displayMachAlert = true;
					#endif
					break;
				case 'btST':
					#if TARGET_RT_MAC_MACHO
						TestSignalBacktrace();
					#else
						displayMachAlert = true;
					#endif
					break;
				case 'btSS':
					TestStackSmash();
					break;
				default:
					err = eventNotHandledErr;
					break;
			}
		}
		
		if (displayMachAlert) {
			SInt16 			junkHit;
			(void) StandardAlert(kAlertPlainAlert, "\pCan't test Mach in CFM build.", "\p", NULL, &junkHit);
		}
		
		return err;
	}

	int main(int argc, char* argv[])
	{
	    OSStatus		err;
	    OSStatus		junk;
	    UInt32			attr;
	    IBNibRef 		nibRef;
	    WindowRef 		window;
	    #pragma unused(argc)
	    #pragma unused(argv)
	    
	    // DebugStr("\pmain");
	    
	    nibRef = NULL;
	    
	    // Create menu bar from NIB.
	    
		if ( (Gestalt(gestaltMenuMgrAttr, (SInt32 *) &attr) == noErr) && (attr & (1 << gestaltMenuMgrAquaLayoutBit)) ) {
		    err = CreateNibReference(CFSTR("main"), &nibRef);			// Mac OS X menus 
		} else {
		    err = CreateNibReference(CFSTR("main9"), &nibRef);			// traditional Mac OS menu bar
		}
	    if (err == noErr) {
		    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	    }
	    if (nibRef != NULL) {
		    DisposeNibReference(nibRef);
		    nibRef = NULL;
		}

		// Create main window from NIB.
		
		if (err == noErr) {
		    err = CreateNibReference(CFSTR("MainWindow"), &nibRef);
		}
	    if (err == noErr) {
		    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
		}
	    if (nibRef != NULL) {
		    DisposeNibReference(nibRef);
		    nibRef = NULL;
		}

		// Install our HICommand handler.
		
		if (err == noErr) {
			gApplicationEventHandlerUPP = NewEventHandlerUPP(ApplicationEventHandler);
			assert(gApplicationEventHandlerUPP != NULL);

			err = InstallApplicationEventHandler(gApplicationEventHandlerUPP, 
												 GetEventTypeCount(kApplicationEvents), 
												 kApplicationEvents, NULL, NULL);
		}

		// Get the output static text control and set it to Monaco 9.
		
		if (err == noErr) {
			err = GetControlByIDQ(window, 'ETXT', 0, &gOutputText);
		}
		if (err == noErr) {
			err = GetControlByIDQ(window, 'UTXT', 0, &gOutputUser);
		}
		if (err == noErr) {
			if ( MoreRunningOnMacOSX() ) {
				ControlFontStyleRec styleRec;
				SInt16 monacoFontNum;

				HideControl(gOutputUser);
				gOutputUser = NULL;
				
				GetFNum(gOutputFont, &monacoFontNum);
				
				styleRec.flags = kControlUseFontMask | kControlUseSizeMask;
				styleRec.font = monacoFontNum;
				styleRec.size = gOutputSize;

				err = SetControlData(gOutputText, kControlEntireControl, kControlStaticTextStyleTag,
									 sizeof(styleRec), &styleRec);
			} else {
				HideControl(gOutputText);
				err = SetOutputText(CFSTR(""));
				if (err == noErr) {

					gOutputUserPaneDrawUPP = NewControlUserPaneDrawUPP(OutputUserPaneDrawProc);
					assert(gOutputUserPaneDrawUPP != NULL);
					
					err = SetControlData(gOutputUser, kControlEntireControl, kControlUserPaneDrawProcTag,
										 sizeof(gOutputUserPaneDrawUPP), &gOutputUserPaneDrawUPP);
				}
			}
		}

		// Get the PID edit text field and initialise it to the Finder's PID.
		
		if (err == noErr) {
			err = GetControlByIDQ(window, 'PIDT', 0, &gPIDText);
		}
		if (err == noErr) {
			#if TARGET_RT_MAC_MACHO
				ProcessSerialNumber psnFinder;
				pid_t				pidFinder;
				CFStringRef			pidFinderStr;
				#pragma unused(junk)
				
				pidFinderStr = NULL;
				
				err = MoreProcFindProcessByCreator(kSignatureFinder, &psnFinder);
				if (err == noErr) {
					err = GetProcessPID(&psnFinder, &pidFinder);
				}
				if (err == noErr) {
					pidFinderStr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"), (long) pidFinder);
					err = CFQError(pidFinderStr);
				}
				if (err == noErr) {
					err = SetTextControlTextCompat(gPIDText, false, pidFinderStr);
				}
				
				CFQRelease(pidFinderStr);
				
				// I don't want to refuse to start up just because this code failed, 
				// so swallow any error.
				
				assert(err == noErr);
				err = noErr;
			#else
				junk = DeactivateControl(gPIDText);
				assert(junk == noErr);
			#endif
		}

	    if (err == noErr) {
		    // The window was created hidden so show it.
	    
	    	ShowWindow( window );
	    
		    // Call the event loop

		    RunApplicationEventLoop();
		}

		return (err != noErr);
	}

#else

    // At the moment only the Carbon targets actually run.  The InterfaceLib 
    // target is only present to check for compilation errors in the 
    // MoreBacktrace module.

	int main(int argc, char* argv[])
	{
		#pragma unused(argc)
		#pragma unused(argv)
		return 1;
	}

#endif		// TARGET_API_MAC_CARBON
