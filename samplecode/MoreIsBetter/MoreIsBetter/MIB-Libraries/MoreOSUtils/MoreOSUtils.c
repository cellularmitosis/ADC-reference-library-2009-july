/*
	File:		MoreOSUtils.c

	Contains:	A OS utility library.

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

$Log: MoreOSUtils.c,v $
Revision 1.10  2003/04/09 19:45:44         
Moved a bunch of low-level Gestalt routines from MoreToolbox to MoreOSUtils.  Also, made changes to avoid implicit arithmetic conversion errors.

Revision 1.9  2002/11/08 23:52:31         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.8  2001/11/07 15:54:35         
Tidy up headers, add CVS logs, update copyright.


         <7>     2/10/01    Quinn   Use CFStringGetSystemEncoding. Also, alllow for CarbonLib 1.5
                                    and later having implementation of CSCopyUser/MachineName
                                    routines.
         <6>     28/9/01    Quinn   Added MoreCSCopyUserName and MoreCSCopyMachineName.
         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <3>    24/11/00    Quinn   MakeData68KExecutable and the interrupt mask stuff aren't
                                    available in Carbon.
         <2>     16/3/99    Quinn   Rolled in InterfaceDisableLib from DTS Technote 1137.
         <1>      1/3/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreOSUtils.h"

// Mac OS interfaces

#if ! MORE_FRAMEWORK_INCLUDES
    #include <Traps.h>
	#include <Gestalt.h>
	#include <CodeFragments.h>
	#include <MixedMode.h>
	#include <OSUtils.h>
	#include <Resources.h>
	#include <Script.h>
	#include <CFURLAccess.h>
	#include <CFPropertyList.h>
#endif

// MIB Prototypes

#include "MoreInterfaceLib.h"

/////////////////////////////////////////////////////////////////////

// We require Universal Interfaces 3.2 because earlier versions
// either mess up the prototype for FlushCodeCacheRange, or don't
// define it for CFM code, or both!

#if UNIVERSAL_INTERFACES_VERSION < 0x0320
	#error MoreOSUtils.c requires Universal Interfaces 3.2 or higher.
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ***** OS-Level Gestalt Tests

static UInt32	gSystemVersion = 0;

pascal UInt32 MoreGetSystemVersion (void)
	// See comment in header.
{
	if (gSystemVersion == 0) {
		OSStatus junk;
		
		junk = Gestalt(gestaltSystemVersion, (SInt32 *) &gSystemVersion);
		assert(junk == noErr);
	}

	return gSystemVersion;
}

extern pascal Boolean MoreRunningOnMacOSX(void)
{
	return GetSystemVersion() >= 0x01000;
}

extern pascal Boolean MoreRunningOnClassic(void)
{
    UInt32 response;
    
    return (Gestalt(gestaltMacOSCompatibilityBoxAttr, 
                    (SInt32 *) &response) == noErr)
                && ((response & 
                    (1 << gestaltMacOSCompatibilityBoxPresent))
                    != 0);
}

pascal UInt32		GetSystemVersion	(void)
{
	return MoreGetSystemVersion();
}

static Boolean	gHaveTestedForPowerManager = false;
static Boolean	gHavePowerManager = false;

pascal Boolean HavePowerManager (void)
{
	OSStatus err;
	
	if ( ! gHaveTestedForPowerManager ) {
		long powerMgrResponse;
		err = Gestalt (gestaltPowerMgrAttr,&powerMgrResponse);

		if (err == gestaltUndefSelectorErr)
			err = noErr;
		else if (!err)
		{
			gHavePowerManager = ((powerMgrResponse & (1L << gestaltPMgrExists)) != 0);

		}
		gHaveTestedForPowerManager = true;
	}

	return gHavePowerManager;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ----- 68 K Code Cache Flush -----
// There’s a space between "68" and "K" because of what seems to be 
// a bug in gcc [2632078].

#if !TARGET_API_MAC_CARBON

	// Define the _vCacheFlush trap (used by FlushCodeCache), which is not included
	// in Universal Interfaces.

	enum {
		_vCacheFlush = 0xA0BD
	};

	#if TARGET_CPU_68K

		// The following routines are used to flush the cache using direct
		// 680x0 instructions.  They are only used if no OS support is available.
		//
		// Because these routines take no parameters and return no results,
		// they are equally effective for both classic 68K and CFM-68K.
		
		static void FlushCacheViaCACR(void) =
			// FlushCacheViaCACR is an inline assembly routine that flushes both the 
			// instruction and data caches by writing directly to the CACR.  Used
			// as a last resort by MakeData68KExecutable on 68020 and 68030.
		{
			0x4E7A, 0x0002,		// MOVEC	CACR,D0
			0x08C0, 0x0003,		// BSET		#3,D0
			0x4E7B, 0x0002		// MOVEC	D0,CACR
		 };

		static void FlushCacheWithCPushA(void) =
			// FlushCacheWithCPushA is another inline assembly routine that flushes caches
			// on the MC68040 using the CPushA instruction.  Used as a last resort
			// by MakeData68KExecutable on 68040.
		{
			0x4E71,				// NOP				; to clear pending writes
		 	0xF4F8				// CPUSHA	BC
		};

	#endif

	extern pascal OSStatus MakeData68KExecutable(void *address, ByteCount count)
		// See comment in interface part.
	{
		OSErr err;

		#if TARGET_CPU_PPC
			assert(GetOSTrapAddress(_HWPriv) != GetToolTrapAddress(_Unimplemented));
		#endif
		
		// Step 1.  If we have _HWPriv, try calling FlushCodeCacheRange.  If that
		// returns an error, call FlushCodeCache.  Two important assumptions:
		//
		// a) any machine that has FlushCodeCacheRange implemented will necessarily
		//    implement FlushCodeCache.
		// b) PowerPC computers always have FlushCodeCacheRange implemented, so
		//    we don't need Mixed Mode glue for FlushCodeCache because we'll never
		//    need it on a PowerPC.
		
		if ( GetOSTrapAddress(_HWPriv) != GetToolTrapAddress(_Unimplemented) ) {
			err = MoreFlushCodeCacheRange(address, count);
			#if TARGET_CPU_68K
				if (err != noErr) {
					assert(GetOSTrapAddress(_vCacheFlush) != GetToolTrapAddress(_Unimplemented));
					FlushCodeCache();
				}
			#else
				assert(err == noErr);
			#endif

		// Step 2.  If we don't have _HWPriv, look to see whether _vCacheFlush
		// (ie FlushCodeCache) is implemented.  If it is, we'll just call it.
		
		} else if ( GetOSTrapAddress(_vCacheFlush) != GetToolTrapAddress(_Unimplemented) ) {
		
			// The call to FlushCodeCache is conditionalised because 
			// Universal Interfaces does not export the call unless
			// you're generating 68K code.  *sigh*  But that's OK because
			// all PowerPC machines have _HWPriv implemented, so this
			// code won't run.
			
			#if TARGET_CPU_68K
				FlushCodeCache();
			#else
				assert(false);
			#endif
			
		// Step 3.  Finally, if neither of these traps is implemented, we're just
		// going to execute 680x0 instructions directly.  Note that we have to
		// execute different instructions based on the 680x0 variant.
		
		} else {
		
			// Obviously, this is only going to work (or indeed compile) if we're
			// generating 68K code.  That's cool, because if we're generating PowerPC
			// code, we must end up running on a PowerPC, and that always has
			// FlushCodeCacheRange so we never get here.

			#if TARGET_CPU_68K
				{
					UInt32 gestaltResponse;
				
					if (Gestalt(gestaltProcessorType, (SInt32 *) &gestaltResponse) == noErr) {
						if (gestaltResponse >= gestalt68020) {
							if (gestaltResponse <= gestalt68030) {
								FlushCacheViaCACR();
							} else {
								FlushCacheWithCPushA();
							}
						}
					}
				}
			#else
				assert(false);
			#endif
		}
		
		// Basically this routine can't fail or, more accurately,
		// there are no expected failure cases.  So we always return
		// noErr.  In fact, the only reason this routine is defined to
		// return an error code is for symmetry with MakeDataPowerPCExecutable,
		// which has to return an error code in the classic 68K case.
		
		return noErr;
	}

#endif

/////////////////////////////////////////////////////////////////////
#pragma mark ----- PowerPC Code Cache Flush -----

#if TARGET_CPU_68K && !TARGET_RT_MAC_CFM

	// This chunk of code implements the classic 68K case for MakeDataPowerPCExecutable,
	// ie we're running classic 68K code that's generating PowerPC instructions.  Boy,
	// is this a pain to implement.  We have to connect up to Interface, find the
	// appropriate symbol, build a routine descriptor for it, and then call it.
	
	enum {
		uppMakeDataExecutableProcInfo = kPascalStackBased |
				STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(void *))) |
				STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(unsigned long)))
	};

	static CFragConnectionID gInterfaceLib = NULL;
	static UniversalProcPtr  gMakeDataExecutable = NULL;
	
	typedef pascal void (*MakeDataExecutableProcPtr)(void *address, unsigned long count);
	
	static pascal OSStatus MakeDataPowerPCExecutableFromClassic(void *address, ByteCount count)
	{
		OSErr err;
		Ptr junkMain;
		Str255 junkMessage;
		CFragSymbolClass junkClass;
		ProcPtr routinePtr;
		
		assert(GetToolTrapAddress(_CodeFragmentDispatch) != GetToolTrapAddress(_Unimplemented));
		assert(GetToolTrapAddress(_MixedModeDispatch)    != GetToolTrapAddress(_Unimplemented));
		
		err = noErr;
		if (gMakeDataExecutable == NULL) {
		
			// We're not connected to InterfaceLib let, let's connect, find the MakeDataExecutable
			// symbol, and build a routine descriptor for it.
			
			err = GetSharedLibrary("\pInterfaceLib", kPowerPCCFragArch, kLoadCFrag, &gInterfaceLib, &junkMain, junkMessage);
			if (err == noErr) {
				err = FindSymbol(gInterfaceLib, "\pMakeDataExecutable", (Ptr *) &routinePtr, &junkClass);
			}
			if (err == noErr) {
				assert(junkClass == kTVectorCFragSymbol);
				gMakeDataExecutable = NewRoutineDescriptorTrap(routinePtr, uppMakeDataExecutableProcInfo, kPowerPCISA);
				if (gMakeDataExecutable == NULL) {
					err = memFullErr;
				}
			}
			
			// If any of this failed, let's shut it all down gracefully.
			
			if (err != noErr) {
				TermMoreOSUtils();
			}
		}
		
		// If we have a UPP for MakeDataExecutable, call it.

		if (gMakeDataExecutable != NULL) {	
			((MakeDataExecutableProcPtr) gMakeDataExecutable)(address, count);
		}
		return err;
	}
	
	extern pascal void TermMoreOSUtils(void)
		// See comment in interface part.
	{
		OSErr junk;
		
		if ( gMakeDataExecutable != NULL ) {
			DisposeRoutineDescriptorTrap(gMakeDataExecutable);
			gMakeDataExecutable = NULL;
		}
		if ( gInterfaceLib != NULL ) {
			junk = CloseConnection(&gInterfaceLib);
			assert(junk == noErr);
			gInterfaceLib = NULL;
		}
	}
	
#endif

extern pascal OSStatus MakeDataPowerPCExecutable(void *address, ByteCount count)
	// See comment in interface part.
{
	// Note: We don't have to worry about the CFM-68K case because
	// CFM-68K won't run on PowerPC machines, so there's no sense
	// in trying to make some data PowerPC-executable from CFM-68K code.

	#if TARGET_RT_MAC_CFM || TARGET_RT_MAC_MACHO
		#if TARGET_CPU_68K
			#pragma unused(address)
			#pragma unused(count)
			assert(false);
			return noErr;
		#else
			MakeDataExecutable(address, count);
			return noErr;
		#endif
	#else
		return MakeDataPowerPCExecutableFromClassic(address, count);
	#endif
}

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Interrupt Enable and Disable -----

#if !TARGET_API_MAC_CARBON

	#if TARGET_CPU_PPC

		// PowerPC Specific Code
		
		// On PPC, we use MixedMode to handle moving the PPC parameters
		// into the right 68K registers and back again.  This make our
		// 68K very easy to write.
		
		enum {
			kGetSRProcInfo = kRegisterBased
					| RESULT_SIZE(SIZE_CODE(sizeof(UInt16)))
					| REGISTER_RESULT_LOCATION(kRegisterD0),
			kSetSRProcInfo = kRegisterBased
					| RESULT_SIZE(0)
					| REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(UInt16)))
		};
		
		// We define the 68K as a statically initialised data structure.
		// The use of MixedMode to call these routines makes the routines
		// themselves very simple.

		static UInt16 gGetSR[] = {
			0x40c0,		// move sr,d0
			0x4e75		// rts
		};

		static UInt16 gSetSR[] = {
			0x46c0,		// move d0,sr
			0x4e75		// rts
		};

		static UInt16 GetSR(void)
			// Returns the current value of the SR, interrupt mask
			// and all!  This routine uses MixedMode to call the gGetSR data
			// structure as if it was 68K code (which it is!).
			
		{
			return (UInt16) CallUniversalProc( (UniversalProcPtr) &gGetSR, kGetSRProcInfo);
		}

		static void SetSR(UInt16 newSR)
			// Returns the value of the SR, including the interrupt mask and all
			// the flag bits.  This routine uses MixedMode to call the gGetSR data
			// structure as if it was 68K code (which it is!).
		{
			CallUniversalProc( (UniversalProcPtr) &gSetSR, kSetSRProcInfo, newSR);
		}

	#else

		// Classic 68K and CFM-68K Specific Code

		// On classic 68K (and CFM-68K) we can simply access the
		// 68K SR register using some inline procedures.
		
		static UInt16 GetSR(void) = {
			0x40c0		// move sr,d0
		};

		#pragma parameter SetSR(__D0)
		static void SetSR(UInt16 newSR) = {
			0x46c0		// move d0,sr
		};

	#endif

	extern pascal UInt16 GetInterruptMask(void)
		// See comment in header file.
	{
		return (UInt16) ((GetSR() >> 8) & 7);
	}

	extern pascal UInt16 SetInterruptMask(UInt16 newMask)
		// See comment in header file.
	{
		UInt16 currentSR;

		assert(newMask >= 0 && newMask <= 7);
			
		currentSR = GetSR();
		SetSR( (UInt16) ((currentSR & 0xF8FF) | (newMask << 8)) );
		
		return (UInt16) ((currentSR >> 8) & 7);
	}

#endif

/////////////////////////////////////////////////////////////////
#pragma mark ----- Machine and User Name -----

#if TARGET_API_MAC_CARBON

	static UInt32 gMoreCSSystemVersion = 0;
	
	enum {
		kMoreCSMacOSX10point0 = 0x01000,
		kMoreCSMacOSX10point1 = 0x01010
	};
	
	static CFStringRef CreateCFStringFromStringResource(SInt16 rsrcID)
		// This routine is called on Mac OS 9 to create a CFString 
		// from the system 'STR ' resource with ID of rsrcID.
	{
		CFStringRef result;
		SInt8  		s;
		Handle 		stringH;
		
		result = NULL;
				
		stringH = GetResource('STR ', rsrcID);
		if (stringH != NULL) {
			s = HGetState(stringH);			assert(MemError() == noErr);
			HLock(stringH);					assert(MemError() == noErr);

			// CFStringGetSystemEncoding returns the CFStringEncoding of the 
			// system as a whole.  We want the system text encoding because 
			// the user/machine name is set by the File Sharing control panel 
			// and the File Sharing control panel is localised in the system script.
			//
			// This only applies because we're reading a resource from the 
			// System file.  If we were reading it from an application resource 
			// we would have used GetApplicationTextEncoding (from "Processes.h") 
			// instead.
			
			result = CFStringCreateWithPascalString(kCFAllocatorSystemDefault,
													(StringPtr) *stringH,
													CFStringGetSystemEncoding() );
			
			HSetState(stringH, s);			assert(MemError() == noErr);
		}
		return result;
	}

	static CFStringRef ReadMachineNameFromFile(void)
		// This routine is called when the client tries to get the 
		// machine name on Mac OS X 10.0.x.  Because CSCopyMachineName returns 
		// un-useful information on those systems, w reads the name 
		// directly from the System Configuration preferences file.
	{
		CFStringRef     result;
        CFURLRef        url;
        CFDataRef       data;
        CFDictionaryRef dict;
        CFDictionaryRef systemDict;
        CFDictionaryRef system2Dict;
        
        result = NULL;
        
        url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
        									CFSTR("/var/db/SystemConfiguration/preferences.xml"), 
        									kCFURLPOSIXPathStyle,
        									false );
        if (url != NULL) {
	        if ( CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, 
	        								url, &data, NULL, NULL, NULL ) ) {
	        	assert(data != NULL);
	            dict = (CFDictionaryRef) CFPropertyListCreateFromXMLData(kCFAllocatorSystemDefault, 
	            							data, kCFPropertyListImmutable, NULL);
	            if (dict != NULL) {
	                systemDict = (CFDictionaryRef) CFDictionaryGetValue(dict, CFSTR("System") );
	                if (systemDict != NULL) {
	                    system2Dict = (CFDictionaryRef) CFDictionaryGetValue(systemDict, CFSTR("System") );
	                    if ( system2Dict != NULL ) {
	                        result = (CFStringRef) CFDictionaryGetValue(system2Dict, CFSTR("ComputerName") );
	                        if (result != NULL) {
	                            // Increment the string's reference count so that 
    	                        // releasing "dict" doesn't release the name.
	                        	CFRetain(result);
	                        }
		                    // Don't need to release system2Dict because it was "got".
	                    }
	                    // Don't need to release systemDict because it was "got".
	                }
	                CFRelease(dict);
	            }
	            CFRelease( data );
	        }
	        CFRelease( url );
		}
		
		assert( (result == NULL) || (CFGetRetainCount(result) == 1) );

		return result;
	}

	extern pascal CFStringRef MoreCSCopyUserName(Boolean useShortName)
		// See comment in header file.
	{
		OSStatus    junk;
		CFStringRef result;
		
		if ( gMoreCSSystemVersion == 0 ) {
			junk = Gestalt(gestaltSystemVersion, (SInt32 *) &gMoreCSSystemVersion);
			assert(junk == noErr);
		}
		
		if ( gMoreCSSystemVersion < kMoreCSMacOSX10point0 ) {

			// Running on traditional Mac OS.  If we have a recent version 
			// of CarbonLib (1.5 and above) that supports the routine, use 
			// that.  Otherwise fall back to the Resource Manager.

			if ( CSCopyUserName != (void *) kUnresolvedCFragSymbolAddress) {
				result = CSCopyUserName(useShortName);
			} else {
				result = CreateCFStringFromStringResource(-16096);
			}

		} else {
			
			// Call the API.  However, if we're built CFM we can't just 
			// call it directly because the routine isn't exported to 
			// CFM on Mac OS 10.0.x.  So for CFM builds we have to call 
			// through CFBundle.
			
			#if TARGET_RT_MAC_CFM
				{
					typedef CFStringRef (*CSCopyUserNameProc)(Boolean useShortName);
					CSCopyUserNameProc 	csCopyUserName;
		            CFBundleRef 		bundle;
		            
		            result = NULL;
					csCopyUserName = NULL;
					
		            bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.apple.Carbon" ) );
		            if (bundle != NULL) {
		            	csCopyUserName = (CSCopyUserNameProc) CFBundleGetFunctionPointerForName(bundle, CFSTR("CSCopyUserName") );
		            }
		            if (csCopyUserName != NULL) {
		            	result = csCopyUserName(useShortName);
		            }
		            // Both bundle and csCopyUserName got with "Get", so 
		            // no need to release.
				}
			#elif TARGET_RT_MAC_MACHO
				result = CSCopyUserName(useShortName);
			#else
				#error MoreCSCopyUserName: What runtime are you using?
			#endif
			
			// Mac OS 10.0 and 10.0.1 (which have the same gestaltSystemVersion 
			// result -- this just gets better and better) have a bug [2665708] where 
			// they fail to retain the user name and host name strings 
			// each time they return it to the client.  The upshot is that
			// the client crashes the second time it calls CSCopyUserName or
			// CSCopyMachineName.  This extra CFRetain prevents this from happening.
			//
			// Note that we don't need a similar workaround in MoreCSCopyMachineName 
			// because we never call the CSCopyMachineName on 10.0 or 10.0.1
			// because of another issue.
			
			if (result != NULL && gMoreCSSystemVersion == kMoreCSMacOSX10point0) {
				CFRetain(result);
			}
		}
		
		return result;
	}
	
	extern pascal CFStringRef MoreCSCopyMachineName(void)
		// See comment in header file.
	{
		OSStatus    junk;
		CFStringRef result;
		
		if ( gMoreCSSystemVersion == 0 ) {
			junk = Gestalt(gestaltSystemVersion, (SInt32 *) &gMoreCSSystemVersion);
			assert(junk == noErr);
		}

		if ( gMoreCSSystemVersion < kMoreCSMacOSX10point0 ) {

			// Running on traditional Mac OS.  If we have a recent version 
			// of CarbonLib (1.5 and above) that supports the routine, use 
			// that.  Otherwise fall back to the Resource Manager.
			
			if ( CSCopyMachineName != (void *) kUnresolvedCFragSymbolAddress) {
				result = CSCopyMachineName();
			} else {
				result = CreateCFStringFromStringResource(-16413);
			}

		} else if ( gMoreCSSystemVersion < kMoreCSMacOSX10point1 ) {
			
			// Running on Mac OS X 10.0.x.  Read the file directly because 
			// the API returns the wrong information (the BSD hostname rather 
			// than the computer name) [2650897].
			
			result = ReadMachineNameFromFile();
			
		} else {
		
			// Running on Mac OS X 10.1 and above.  Let's just call the API. 

			// The following assert checks that the CFM weak link worked. 
			// It's benign for the Mach-O build.
			
			assert( CSCopyMachineName != (void *) kUnresolvedCFragSymbolAddress );
		
			if ( CSCopyMachineName == (void *) kUnresolvedCFragSymbolAddress ) {
				result = NULL;
			} else {
				result = CSCopyMachineName();
			}
		}

		return result;
	}

#endif
