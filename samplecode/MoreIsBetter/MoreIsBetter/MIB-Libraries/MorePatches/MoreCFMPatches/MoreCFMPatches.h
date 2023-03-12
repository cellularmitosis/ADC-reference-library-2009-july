/*
	File:		MoreCFMPatches.h

	Contains:	Interface to CFM patching technology.

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

$Log: MoreCFMPatches.h,v $
Revision 1.4  2002/11/08 23:54:08         
Move compile time environment check to header. Renumber error codes to fit in with new allocation policy in "MoreErrors.h".

Revision 1.3  2001/11/07 15:51:46         
Tidy up headers, add CVS logs, update copyright.


         <2>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <1>     16/3/99    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

#if !TARGET_CPU_PPC || !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON
	// We need to compile this code as CFM-PPC.  Why?  Because
	// we're statically linking in some PowerPC assembly language
	// and that only works when generating CFM-PPC.  We could have
	// an alternate technique to allow classic 68K code to patch
	// CFM entry points, but that seems like a too much work right now.
	// Also, I want to use CFM to get the MoreCFMPatchesLib to
	// allow us to be overridden by a system library.
	//
	// Also, we don’t support Carbon development because you can’t be 
	// guaranteed that the run-time architecture for Carbon is CFM. 

	#error "MoreCFMPatches.c" will only compile for CFM-PPC for InterfaceLib.
#endif

// Standard Mac OS interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
#endif

/////////////////////////////////////////////////////////////////
// Using MoreCFMPatches

// This CFM patching technology works by monkeying with transition
// vectors, henceforth known as TVectors, in non-obvious ways.
// To use it succesfully, you must know something about TVectors.
// For complete information, I suggest the book "Mac OS Runtime
// Architectures", available from <http://www.apple.com/developer/>.
// However, the following points are especially important:
//
// 1. On CFM architectures, all exported functions are called
//    by means of a TVector.  The exporter exports a TVector
//    in its data section, the importer imports the address of
//    the TVector as a pointer in its data section.  When
//    the importer calls the function, it does so by calling
//    through this TVector pointer.
//
// 2. Likewise, all routine pointers are not actually pointers
//    to code, but pointers to TVectors.
//
// 3. TVectors are typically two words long, the first word being
//    a pointer to the code and the second word being a pointer to
//    the callee's TOC (ie somewhere in the callee's data section)
//    However this is not  a requirement.  The only requirements are:
//      a) the first word must point to the code,
//      b) the caller must load the second word into R2, and
//      c) the caller must load R12 with a pointer to the TVector itself.
//         [This allows the callee to extract any extra information
//          it cares to store in the TVector.]
//    This patching technology maintains all these invariants.
//    Beware of cheap imitations!
//
// Using this library is very easy.  Just follow the 5 easy
// steps:
//
//   i. Write your patch routine to have exactly the same
//      calling conventions as the routine you are going to patch.
//
//        extern pascal void OriginalRoutine(char *param);
//
//        extern pascal void MyPatch(char *param)
//        {
//            [...do stuff...]
//        }
//
//  ii. Call MorePatchTVector to add your patch to the list
//      of patches on the original routine.  Remember, a C
//      function pointer is a pointer to a transition vector,
//      so you can just cast a pointer to the original routine
//      to a TVector.  Neat huh?
//
//        err = MorePatchTVector((TVector *) OriginalRoutine,
//                               (TVector *) MyPatch,
//                               kMyCreator,
//                               NULL);
//
// iii. Don't forget to include a useful creator code so that
//      you (and Apple) can manage your patches.
//
//  iv. If you need to call through to the original routine,
//      just cast gMoreCFMPatchesCallThrough to the appropriate
//      C routine pointer type and call it.
//
//        typedef pascal void (*OriginalRoutineProc)(char *param);
//
//        extern pascal void MyPatch(char *param)
//        {
//            [...do 'head' patch stuff...]
//            ((OriginalRoutineProc) gMoreCFMPatchesCallThrough)(param);
//            [...do 'tail' patch stuff...]
//        }
//
//      You use the same routine pointer for all patches in all
//      patch chains.  The clever run-time implementation figures
//      out what to call when.  But remember:
//
//      a. you must call through from the mainline of your
//         patch routine, not from a routine that your patch
//         routine has called, and
//
//      b. for gMoreCFMPatchesCallThrough to work properly, the fragment
//         containing "MoreCFMPatches.c" must remain in memory as long
//         as the patch code that might actually call it.  Don't
//         try to call gMoreCFMPatchesCallThrough after unloading
//         "MoreCFMPatches.c".
//
//   v. Link with the MoreCFMPatchesLib.stub library.  You must
//		weak link with this stub library.  Doing so allows future
//		system software to override the behaviour of this patching
//		module without any special hackery.  The module is designed
//		such that, if the weak link succeeds, all routines in this
//		module will call through their equivalent routines in the
//		library.
//	
//			IMPORTANT:
//			*NEVER* build MoreCFMPatches into a CFM shared library and
//			install it in the Extensions folder.  This module is meant
//			to be linked into your application as C source code.
//			Linking with a stub library is done to allow some future
//			system software version of this code to override the
//			behaviour of this code in a compatible fashion.
//		
//  vi. Finally, and this is the big one, you must remember that
//      TVectors are exported out of a fragment's *data* section.
//      Fragments may use a shared data section, or a per-context
//      data section.  Fragments with a shared data section only
//      have a single set of TVectors and are easy to patch.  However,
//      per-context fragments export a different set of TVectors
//      for each context and, if you want your patch to have global
//      effect, you must patch the TVectors in each context.  Urgh.
//      Furthermore, there's no easy way to determine whether a
//      fragment is per-context or globally shared.  You just have
//      to "DumpPEF" it and check out the flag.  And there's no
//      guarantee that a particular fragment will not switch from
//      being one to the other.
//
//		Beyond that, you should be warned that patching a TVector
//		with a TVector modifies both TVectors.  So if you're
//		patching a per-context library, you must patch it with
//		a TVector from a per-context library.
//
//      Hey, you knew the job was dangerous when you took it.

/////////////////////////////////////////////////////////////////
// Error Codes

enum {
	kVectorNotPatchedByUsErr = 5320,
		// This error can be returned by various routines when
		// you attempt to manipulate patches on a TVector that
		// either a) wasn't patched by MorePatchTVector (or compatible
		// routine), or b) has been over-patched by an incompatible
		// TVector patching technology.
		
	kPatchNotFoundInPatchIslandErr = 5321,
		// This error can be returned by MoreUnpatchTVector when
		// you attempt to unpatch a TVector that was patched with
		// a compatible TVector patching techology but whose
		// patchTVectorToRemove wasn't found in the list of patches.
		// If you successfully called MorePatchTVector, this typically
		// implies either a) you're not passing in the same TVector
		// in patchTVectorToRemove as you passed in to the
		// patchTVectorToAdd parameter of MorePatchTVector, or b)
		// someone overpatched your original patch with an incompatible
		// patching technology and then someone else overpatched that
		// with a compatible patching technology.  Urgh.
	
	kTVectorAlreadyPatchedByYouErr = 5322,
		// This error is return by MorePatchTVector when the TVector
		// being patched has already been patched with the supplied
		// patch TVector.
	
	kPatchNotFoundErr = 5323,
	
	kPatchInfoOverrunErr = 5324
};

/////////////////////////////////////////////////////////////////
// TVector Structure

// TVector represents the CFM "transition vector".  See
// "Mac OS Runtime Architectures" for more details about
// transition vectors.

// The pragma align isn't strictly speaking necessary but the
// TVector structure is graven in stone, never to change, so
// let's play by the rules and declare it that way.

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

struct TVector {
	void *codePointer;
	// ... extra fields are compiler specific
};
typedef struct TVector TVector, *TVectorPtr;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Interface to Patching Code

extern ProcPtr  gMoreCFMPatchesCallThrough;
	// This routine pointer is used to call through to the
	// previous routine in the patch chain.  Just cast it
	// to the appropriate C function pointer type and call it.

extern pascal OSStatus MorePatchTVector(TVector *tVectorToPatch,
						    TVector *patchTVectorToAdd,
						    OSType  creator,
							void    *refcon);
	// Patch the routine whose TVector is tVectorToPatch with
	// a routine whose TVector is patchTVectorToAdd.  creator
	// and refcon are extra data associated with the patch
	// that is not intepreted by this module.  You should
	// pass a registered creator code into creator and you
	// can pass whatever you like into refcon.
	//
	// The creator helps to identify which software has patched
	// which routines, which in turns helps you (and Apple) to
	// debug weird patching conflicts.
	//
	// patchTVectorToAdd must point to a TVector for a routine
	// whose prototype is *exactly* the same as the prototype
	// of the routine whose TVector is tVectorToPatch.
	//
	// If you intend to patch a routine globally, you must call
	// this for every instance of the TVector tVectorToPatch
	// that's created.  See above for details.
	//
	// The only likely error codes include kTVectorAlreadyPatchedByYouErr
	// (tVectorToPatch is already patched by tPatchTVectorToAdd) and
	// Memory Manager errors, specifically memFullErr.

extern pascal OSStatus MoreUnpatchTVector(TVector *tVectorToUnpatch,
							TVector *patchTVectorToRemove);
	// Remove the routine whose TVector is patchTVectorToRemove
	// from the list of patches on tVectorToUnpatch.  The likely
	// error codes include kVectorNotPatchedByUsErr and
	// kPatchNotFoundInPatchIslandErr, both of which are explained
	// above.

/////////////////////////////////////////////////////////////////
// Debugging/Sanity Checking Routines

extern pascal OSStatus MoreCountPatches(TVector *tVector, ItemCount *count);
	// Counts the number of patches associated with the supplied TVector.
	// Returns kVectorNotPatchedByUsErr if the TVector hasn't been
	// patched by us.
	
//	infoKind							   bufferSize

enum {
	kPatchInfoCreator = 'crea',			// sizeof(OSType)
	kPatchInfoTVector = 'tvec',			// sizeof(TVector *)
	kPatchInfoRefcon  = 'refc'			// sizeof(void *)
};

extern pascal OSStatus MoreGetIndexedPatchInfo(TVector *tVector, ItemCount index,
							OSType infoKind, void *buffer, ByteCount bufferSize);
	// Returns information about the index'th patch applied
	// to the TVector.  index is one based, and must be in the range
	// 1 through the value returned by MoreCountPatches.  infoKind
	// specifies the type of information you're interested in,
	// and must be one of the OSTypes in the enumeration above.
	// buffer must point to a buffer where the information is
	// placed.  bufferSize must be the size of the buffer pointed
	// to by buffer.
	//
	// The routine returns kPatchNotFoundErr if index is out
	// of bounds.
	//
	// The routine returns kPatchInfoOverrunErr if the buffer
	// is too small to accept all the information, but it
	// also fills in the first bufferSize bytes of the buffer.
	//
	// Note for the future: If the information is variable
	// length (a string perhaps), the returned information
	// must include a description of the true length, otherwise
	// there's no way for the client to know how much of
	// the buffer is valid if the client supplied a bigger
	// buffer than was necessary.
	
#ifdef __cplusplus
}
#endif
