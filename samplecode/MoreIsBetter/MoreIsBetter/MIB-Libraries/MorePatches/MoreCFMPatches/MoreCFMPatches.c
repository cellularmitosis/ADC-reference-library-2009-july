/*
	File:		MoreCFMPatches.c

	Contains:	Implementation of CFM patching technology.

	Written by: Quinn

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

$Log: MoreCFMPatches.c,v $
Revision 1.6  2002/11/08 23:54:47         
Move compile time environment check to header. Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2001/11/07 15:51:40         
Tidy up headers, add CVS logs, update copyright.


         <4>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <3>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <2>    24/11/00    Quinn   Complain if compiled for Carbon.
         <1>     16/3/99    Quinn   First checked in.
*/

////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreCFMPatches.h"

// Standard Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <CodeFragments.h>
	#include <OSUtils.h>
#endif

// ANSI C Interfaces

#include <stddef.h>

// MIB Interfaces

#include "MoreOSUtils.h"

////////////////////////////////////////////////////////////////
#pragma mark ----- Diagrams -----

/*
	Introduction
	------------
	
			IMPORTANT:
			Before looking at this code, you need to read and under the
			following:

				a) "Mac OS Runtime Architectures" -- This Inside Macintosh-like
				    book is available from <http://www.apple.com/developer/>.
				    You should concentrate on the sections describing CFM 
				    architectures, especially if terms like "TVector" make your
				    eyes glaze over.

				b) "MoreCFMPatches.h" -- The comments in the interface file for 
				   this module contain information about its goal, as well as
				   some important invariants for TVectors.

	The basic design feature of this module is the "patch island".  Each
	patch island contains a list of patches that have been attached to 
	a particular TVector, along with evil PowerPC assembly language that
	glues the TVector to the patches.  The assembly language is particularly
	evil because the patching mechanism must maintain the TVector invariants
	described in "MoreCFMPatches.h".  It is described in very great detail
	in the "MoreCFMPatching.s" file.
	
	A key detail of the assembly language is that the code contains two entry
	points.  When a client calls the patched TVector, it actually branches to
	"patchIslandEntry", which is responsible for routing the patch through the 
	patch chain.  When a patch wishes to call through to the next patch in the
	chain, it calls gMoreCFMPatchesCallThrough, which calls the "callThrough"
	entry point of the assembly language to do the appropriate skank.
	Both entry points actually share a significant proportion of their code.
	
	A patch island contains a fixed size header which is mostly
	PowerPC instructions (there's also a signature which we use to identify
	the patch island format), followed by an unbounded array of "patch records".
	Each patch record describes one particular patch on the TVector,
	except the last patch record, which describes the original TVector itself.

	Patches in the patch record are executed in order; when a client calls
	the original TVector, the first patch record is executed.  It has the
	choice of calling through to the next patch record or simply returning
	to the caller.  Under this architecture, all patches are 'surround'
	patches.  There is no specific provision for 'head' or 'tail' patches,
	although each is a degenerate case of the surround patch.
	
	The assembly code in the patch island must be able to reference the
	patch records in the patch island.  This is particularly tricky because
	PowerPC has no "PC-relative" addressing modes.  Instead, we load up
	the address of the patch records using immediate instructions.  When
	we construct the patch island, we modify these instructions "on the fly"
	to contain the right pointer.  This allows us to store an arbitrary
	32-bit variable inside the patch island itself, which can be accessed from
	both C and assembly.  This 32-bit variable is known as the patch island
	"payload".

	As you can imagine, there is a very incestuous relationship between
	the C and assembly language code in this module.  If you make any
	changes to the C data structures PatchIsland and PatchRecord, you will
	have to make corresponding changes to the assembly language.  Similarly,
	if you make any changes to the assembly language, even adding or
	removing an instruction, you will have to change the C data structures.
	
			IMPORTANT:
			If you make any significant changes to the way this code
			works, you *MUST* change the signature in the patch island.
			Otherwise other developers who use this code will start
			assuming that your patches are the same format as their
			patches, with tragic results.

	The patch island concept was designed by various folks in Apple's
	tool group during the Copland effort.  It was explained to me in
	minute detail by Alan Lillich (thanks Alan!).  However, this code
	is entirely my work and all errors are therefore mine.
	
	
	Patch Island Structure in Memory
	--------------------------------

	This diagram shows how a patch island might look in memory.
	[Low memory is shown at the top of the page in typically confusing
	computer geek style.]  The patch starts with the patch island
	assembly language code, which is followed by an unbounded array
	of patch records.
	
	Each patch record contains a pointer to the code that actually
	implements the patch and a pointer to the TVector for that code.
	The distinction is important.  Specifically, the address of the
	original TVector is necessary so that a) we can update all
	the patched TVectors when we move the patch island, and b) we
	can unpatch a patch TVector's code pointer when the remove
	the patch from the chain.
		
					+-------------------+
					| Patch Island code |
					|					|
					|					|
					|					|
					+-------------------+
		patches[0]	| patchCode			|	<- most recent patch
					| patchTVector		|
					+-------------------+
		patches[1]	| patchCode			|
					| patchTVector		|
					+-------------------+
					.					.
					.					.
					.					.
					+-------------------+
		patches[N]	| patchCode			|	<- original implementation
					| patchTVector		|
					+-------------------+

	Before and After Patching
	-------------------------
	
	The following diagram shows the state of the machine before
	MyPatch is applied to NavLoad.  Each TVector contains a pointer
	to its own code and TOC.  
				
		NavLoad ->	+--------------------+
					| NavLoad code ->	 |
					| NavLoad TOC  ->	 |
					+--------------------+
		
		MyPatch ->	+--------------------+
					| MyPatch code ->	 |
					| MyPatch TOC  ->	 |
					+--------------------+
	
	The following diagram shows what happens after you execute the
	code:
	
		err = MorePatchTVector(NavLoad, MyPatch);
	
	Note that the code pointers for *both* TVectors have been changed
	to point to patchIslandEntry.  From now on, if either of these
	TVectors is called, the patch will execute.  In addition, if
	MyPatch calls gMoreCFMPatchesCallThrough, it will enter the patch
	island at callThrough, which routes the call to the next patch in
	the patch island.
	
	This organisation generalises to more than one patch.  Trust me (-:
	
		NavLoad ->	+--------------------+
					| patchIslandEntry ->|
					| NavLoad TOC  ->	 |
					+--------------------+
		
		MyPatch ->	+--------------------+
					| patchIslandEntry ->|
					| MyPatch TOC  ->	 |
					+--------------------+

	 PatchIsland -> +--------------------+
					| callThrough		 |
patchIslandEntry -> | patchIslandEntry	 | (loads r11 with address of patches[0])
					| patchIslandCommon	 |
					+--------------------+
					| MyPatch code ->	 | patches[0]
					| MyPatch ->		 | (ie address of the TVector)
					+--------------------+
					| NavLoad code ->	 | patches[1]
					| NavLoad ->		 | (ie address of the TVector)
					+--------------------+

	Stack Diagrams
	--------------

	The key to this design is the use of a "system reserved" word
	in the PowerPC stack frame.  This word is 16 bytes above the
	stack pointer and is officially designated as reserved for system
	use.  This sample serves to document the Apple official use of this
	word.
	
	The two diagrams below show how a stack frame is built when
	a patch is in place.  The caller calls the NavLoad TVector,
	which actually runs the patchIslandEntry code in our patch island.
	This code gets the address of the first patch record from
	the patch island's payload.  It stores that address in the
	*caller's* frame, and then proceeds to call through to MyPatch.
	When MyPatch calls gMoreCFMPatchesCallThrough (which is routed
	through to the callThrough entry point of the patch island),
	the assembly language grabs the pointer to the current patch
	record from the caller's stack (that's the original caller, not
	MyPatch -- it can find it by looking up one stack frame, hence
	the restriction that you can only call gMoreCFMPatchesCallThrough
	from the mainline of your patch), increments it to point to
	the next patch record, stores it in the current frame (ie
	MyPatch's frame) and calls through to the next patch.
	
	Eventually this process reaches the last patch record in the
	patch chain, which contains the code pointer for the original
	implementation.  At this point, the most recent patch record 
	pointer stored in a frame points to the last patch record in
	the chain.  This is cool though, because the original implementation
	is never going to call gMoreCFMPatchesCallThrough, and hence
	we never increment the current patch record pointer off the end
	of the patch chain.
	
		Caller's stack frame before calling NavLoad.
			
					| Caller			 |	 ^
					|					 |	 |
					|					 |	 |
					|					 |	 | link to previous frame
			 sp ->	|					 | --+
					+--------------------+

		MyPatch's stack frame
		
					| Caller			 |	 ^
					|					 |	 |
		 sp^+16 ->	| &patches[0]		 |	 |
					|					 |	 | link to previous frame
			sp^ ->	|					 | --+
					+--------------------+	 |
					| MyPatch			 |	 |
					|					 |	 |
					|					 |	 |
					|					 |	 | link to previous frame
			 sp ->	|					 | --+
					+--------------------+
	
		So MyPatch can figure out what the next patch to call
		is by looking up one stack frame to the Caller's stack
		frame and then extracting the pointer to the current
		PatchRecord from offset 16 into the frame and then
		incrementing that pointer by kPatchRecordSize.
		
		NavLoad's stack frame
		
					| Caller			 |	 ^
					|					 |	 |
		sp^^+16 ->	| &patches[0]		 |	 |
					|					 |	 | link to previous frame
		   sp^^ ->	|					 | --+
					+--------------------+	 |
					| MyPatch			 |	 |
					|					 |	 |
		 sp^+16 ->	| &patches[1]		 |	 |
					|					 |	 | link to previous frame
			sp^ ->	|					 | --+
					+--------------------+	 |
					| NavLoad			 |	 |
					|					 |	 |
					|					 |	 |
					|					 |	 | link to previous frame
			 sp ->	|					 | --+
					+--------------------+

		If NavLoad attempted to call through to the next patch,
		it would fail because we've reached the end of the patch
		array.	However, NavLoad is the original (non-patched)
		routine, so it shouldn't try to call through.

	Why?
	----
	
	So, why do we use this convoluted approach?  Well, the most
	obvious answer is "because it works".  However, I doubt this
	will satisfy you.
	
	Patching CFM is extremely tricky.  The biggest problem is one
	of data storage.  CFM fragments can be multiply instantiated,
	with each fragment having its own data section, and hence its
	own TVectors.  Each instance of the TVector's may have a
	different patch chain.  [For example, all instances may have
	some global patches but an application might apply its own
	per-context patches.]  So you can't just store the information
	about the patches in a global variable.  It has to be stored
	per-TVector.
	
	Which implies that the TVector has to somehow reference this
	storage.  I'm not sure whether you've noticed, but TVectors
	don't have a lot of extra space for system expansion.  In fact,
	the only field that's guaranteed to exist is the first field,
	ie the pointer to the code.  So we have to use that field
	to point to our patching code, and construct the patching code
	so that it can find the information about what patches to
	apply to this particular TVector, preferably without an expensive
	table lookup.
	
	The above pretty much dictates the use of PC-relative storage,
	which in turn implies that we duplicate some patch code
	for each patch.  [Actually, we could share more of the patch
	code than we do, but the complexity of doing this seems
	to outweigh the memory savings.]  The rest of the design falls
	out from there.

*/

////////////////////////////////////////////////////////////////
#pragma mark ---- Patch Island Data Structures -----

// We align this with mac68k alignment because a) all the structures
// are padded such that mac68k is optimal for PowerPC as well, and
// b) we need to guarantee a *specific* alignment in memory, so we might
// as well choose a well supported one.

#pragma options align=mac68k

// A PatchRecord is used to hold details about a patch
// in the patch island.	 The details include a pointer
// to the code of the routine and a pointer to the routine's
// transition vector, along with a creator (for debugging
// and patch management) and a refcon (for the patch owner's
// use).

struct PatchRecord {
	void	*patchCode;
	TVector *patchTVector;
	OSType	patchCreator;
	void	*patchRefcon;
};
typedef struct PatchRecord PatchRecord;

// The PatchIsland structure represents the (assembly language) code of
// the patch and an unbounded array of PatchRecords that presents the
// patches applied (the last entry represents the original routine before
// patching).
//
// For details on this structure, see "MoreCFMPatches.s", which describes
// the actual assembly language.  You shouldn't change this structure
// unless you also change that code.  If you change either, you should
// also change the kPatchIslandSignature, which allows us to identify
// our specific patching technology.

struct PatchIsland {
	UInt32 callThrough[4];
	OSType signature;
	UInt16 patchIslandEntry[4];
	UInt32 patchIslandCommon[6];
	PatchRecord patches[1];
};
typedef struct PatchIsland PatchIsland;

enum {
	kPatchIslandHeaderSize = offsetof(PatchIsland, patches)
};

#pragma options align=reset

////////////////////////////////////////////////////////////////
#pragma mark ----- Shared with "MoreCFMPatches.s" -----

// Constants shared with "MoreCFMPatches.s".  If you change them
// here, you must also change them there.

enum {

	// When calling a patch, we store a pointer to the current
	// PatchRecord in a "reserved for system use" field of the frame
	// of the caller.
	// This allows our "call through" glue to find the next patch to
	// call (or the original routine, which is the last PatchRecord
	// in the array).  See the stack diagrams for details.
	//
	// For more information about the system reserved frame offset,
	// see the "Mac OS Runtime Architectures", available from
	// <http://www.apple.com/developer/>.
	
	kSystemReservedFrameOffset = 16,
	
	// Patch islands installed by this module contain a signature to
	// help us identify whether we have patched a TVector.	If you
	// modify this code to implement another patching mechanism that
	// is not 100% compatible with this one, you must change this
	// signature to avoid confusion.

	kPatchIslandSignature = 'Nat!',

	// This is sizeof(PatchRecord).	 We define it as a strict constant
	// so as to be in sync with the assembly language code.	 Before
	// running (see below) we also:
	//
	//	 assert(kPatchRecordSize == sizeof(PatchRecord));
		
	kPatchRecordSize = 16
};

////////////////////////////////////////////////////////////////
#pragma mark ----- Patch Island Primitives -----

// Much of the code here contains intimate details of the assembly
// code in "PatchManager.s"

// This declaration allows us to access the assembly language code
// for the patch island.  It's declared to return a long so that
// it's type compatible with ProcPtr, which is how it's exported
// to clients.  In truth, there's no way you can look at this code
// as a C function, so there's no good prototype for it.

#ifdef __cplusplus
	extern "C" {
#endif

	extern long MoreCFMPatchesCallThrough(void);

#ifdef __cplusplus
	}
#endif

// The following declarations contain information about the assembly
// language code in the patch island, specifically the patchIslandEntry
// field which is the code we modify to include the payload.

enum {
	kEntryOpcode0 = 0x3D60,
	kEntryOpcode1 = 0x616B,
	
	kEntryOpcode0Index = 0,
	kEntryHighPayloadIndex = 1,
	kEntryOpcode1Index = 2,
	kEntryLowPayloadIndex = 3
};

static UInt32 GetPatchIslandPayload(PatchIsland *patchIsland)
	// This routine gets the payload within the patch island.
	// The 32 bit instructions containing the payload are
	// each represented by two UInt16s.	 The second UInt16 in the
	// the instruction is the data portion of the instruction.
	// The first instruction (a "lis") contains the high 16 bits
	// of the payload, and the second (an "ori") contains the 
	// low 16 bits.
{
	assert(patchIsland != NULL);
	assert(patchIsland->signature == kPatchIslandSignature);
	assert(patchIsland->patchIslandEntry[kEntryOpcode0Index] == kEntryOpcode0);
	assert(patchIsland->patchIslandEntry[kEntryOpcode1Index] == kEntryOpcode1);
	
	return	 (patchIsland->patchIslandEntry[kEntryHighPayloadIndex] << 16)
		   | patchIsland->patchIslandEntry[kEntryLowPayloadIndex];
}

static void SetPatchIslandPayload(PatchIsland *patchIsland,
								  UInt32 newPayload)
	// This routine sets the payload of a patch island.	 The storage
	// for the payload is described in the routine above.  The only
	// tricky part is that it has to call MakeDataExecutable to ensure
	// that the code modification "sticks".
	//
	// It's important that this flush the code cache for the entire
	// patch island header, not just the two instructions that it modifies.
	// This is because other routines rely on this to flush the entire
	// header.
{
	assert(patchIsland != NULL);
	assert(patchIsland->signature == kPatchIslandSignature);
	assert(patchIsland->patchIslandEntry[kEntryOpcode0Index] == kEntryOpcode0);
	assert(patchIsland->patchIslandEntry[kEntryOpcode1Index] == kEntryOpcode1);

	patchIsland->patchIslandEntry[kEntryHighPayloadIndex] =
												(newPayload >> 16);
	patchIsland->patchIslandEntry[kEntryLowPayloadIndex] =
												 newPayload;
	MakeDataExecutable(patchIsland, kPatchIslandHeaderSize);
}

static Boolean ValidPatchIsland(PatchIsland *patchIsland)
	// This routine returns true if patchIsland looks
	// like a valid patch island.  In the non-debug build,
	// it's used by HasTVectorBeenPatched to determine whether
	// there's a patch island already in place for a particular
	// TVector.  In debug builds, it's used in assertions everywhere.
	//
	// The specific checks include:
	//
	// o does the patch island contain our signature
	// o does the patch island start with our two instructions,
	//   ie "lis" and "ori"
	// o do those instructions create a constant that points
	//	 to the patches field of the PatchIsland structure.
	//
	// This routine could be more robust.  Specifically, it could
	// check that all the codePointers of all the patchTVectors of
	// all the PatchRecords point to this patch island.  However,
	// that's more work than I'm prepared to do right now.
{
	return (patchIsland != NULL)
		&& (patchIsland->signature == kPatchIslandSignature) 
		&& (patchIsland->patchIslandEntry[kEntryOpcode0Index] == kEntryOpcode0)
		&& (patchIsland->patchIslandEntry[kEntryOpcode1Index] == kEntryOpcode1)
		&& GetPatchIslandPayload(patchIsland) == (UInt32) &patchIsland->patches[0];
}

static PatchIsland *GetPatchIslandFromTVector(TVector *tVector)
	// If tVector points to a TVector that has been patched,
	// this routine returns a pointed to the patch island by
	// simply subtracting a constant.
{
	assert(tVector != NULL);

	return (PatchIsland *) ( (char *)(tVector->codePointer) - 
							 offsetof(PatchIsland, patchIslandEntry)
						   );
}

static Boolean HasTVectorBeenPatched(TVector *tVector)
	// This routine determines whether tVector has been patched by
	// us.	It does this working out where the patch island would be
	// (if there was one) and then looking for various specific features
	// of a patch island, namely those checked by ValidPatchIsland.
	//
	// Later on in the code, we use this routine to decide whether we
	// should create a new patch island for the TVector or simply
	// add ourselves to the front of the existing patch island.
{
	Boolean result;
	PatchIsland *potentialPatchIsland;

	result = false;
	if (tVector != NULL) {
		potentialPatchIsland = GetPatchIslandFromTVector(tVector);
	
		result = ValidPatchIsland(potentialPatchIsland);
	}
	return result;	
}

static OSStatus GetPatchIslandPatchCount(PatchIsland *patchIsland, ItemCount *patchCount)
	// This routine calculates the number of PatchRecords in a patch
	// island (including the last PatchRecord that represents the
	// original routine that was patched).	It determines this using the
	// the size of the pointer block that contains the patch island.
	//
	// I decided against storing the count of the number of patches
	// in the patch island because there's a reliable way to get
	// the size of Memory Manager pointer blocks and adding a count
	// would just be adding redundant information that I'd have to keep
	// in sync.
{
	OSStatus err;
	Size patchIslandSize;
	
	assert(ValidPatchIsland(patchIsland));
	assert(patchCount	!= NULL);

	patchIslandSize = GetPtrSize( (Ptr) patchIsland );
	err = MemError();
	if (err == noErr) {
		*patchCount = ( patchIslandSize - kPatchIslandHeaderSize ) / sizeof(PatchRecord);
		assert(*patchCount * sizeof(PatchRecord) + kPatchIslandHeaderSize == patchIslandSize);
	}
	return err;
}

static OSStatus FindPatchInPatchIsland(PatchIsland *patchIsland, TVector *tVector, ItemCount *patchIndex)
	// This routine determines whether a TVector has been patched with
	// this a patch island.	 It does this by searching through the list
	// of PatchRecords in the patch island, looking for the TVector.
	// If it finds it, it returns the index to the PatchRecord in
	// patchIndex.
{
	OSStatus  err;
	ItemCount i;
	ItemCount patchCount;
	
	assert(ValidPatchIsland(patchIsland));
	assert(tVector		!= NULL);
	assert(patchIndex	!= NULL);

	err = GetPatchIslandPatchCount(patchIsland, &patchCount);
	if (err == noErr) {
		err = kPatchNotFoundInPatchIslandErr;
		for (i = 0; i < patchCount; i++) {
			if ( patchIsland->patches[i].patchTVector == tVector ) {
				*patchIndex = i;
				err = noErr;
				break;
			}
		}
	}
	
	return err;
}

static void SyncPatchedTVectors(PatchIsland *patchIsland, ItemCount patchCount)
	// This routine sychronises a patch island with all TVectors that
	// make up the patches.	 It's used when we add or remove a patch
	// to/from a patch island.	In this case, the patch island moves
	// in memory.  So we have to go through the list of PatchRecords
	// in the island, making sure that all the codePointer fields in
	// all the transition vectors they point to point to the new
	// patch island's entry point.
	//
	// We do this with interrupts disabled because I've been
	// unable to prove to my satisfaction that everything works OK
	// with interrupts enabled, even if you did rework the code
	// to modify the TVectors in revese.  It's better to be safe
	// than sorry.
	//
	// ••• Disabling interrupts in not going to be enough if the TVector
	// is being used by MP threads. •••
{
	UInt16 oldLevel;
	ItemCount i;

	assert(ValidPatchIsland(patchIsland));
	assert(patchCount	>= 1);

	oldLevel = SetInterruptMask(7);
	
	for (i = 0; i < patchCount; i++) {
		patchIsland->patches[i].patchTVector->codePointer = &patchIsland->patchIslandEntry[0];
	}

	(void) SetInterruptMask(oldLevel);
}

////////////////////////////////////////////////////////////////
#pragma mark ----- Memory Allocators -----

// Patch islands must be allocated in memory with the following
// characteristics:
//
// 1. shared between all contexts that can access the TVector,
// 2. persistent until the last context that can access the TVector
//	  goes away,
// 3. held resident (if the TVector can be called when paging is unsafe).
//
// At the moment, the system heap fulfills all these requirements.
// If you change the memory allocator to some other scheme, make sure it
// still fulfills these requirements.

static OSStatus NewPatchIsland(ItemCount patchCount, PatchIsland **patchIsland)
{
	OSStatus err;
	
	assert(patchCount	>= 1);
	assert(patchIsland != NULL);

	err = noErr;
	*patchIsland = (PatchIsland *) NewPtrSys( kPatchIslandHeaderSize + patchCount * sizeof(PatchRecord) );
	if (*patchIsland == NULL) {
		err = MemError();
		assert(err != noErr);
	}
	return err;
}

static void DisposePatchIsland(PatchIsland *patchIsland)
{
	assert(patchIsland != NULL);

	DisposePtr( (Ptr) patchIsland );
	assert(MemError() == noErr);
}

////////////////////////////////////////////////////////////////
#pragma mark ----- Patch Creation -----

static OSStatus CreateNewPatchIsland(TVector *tVectorToPatch)
	// This routine creates a new patch island and applies it into
	// tVectorToPatch.	The patch island contains one PatchRecord,
	// which represents just the original routine that was patched.
{
	OSStatus err;
	PatchIsland *newPatchIsland;
	
	assert(tVectorToPatch != NULL);

	assert( ! HasTVectorBeenPatched(tVectorToPatch) );

	// Create the memory for the patch island in the system heap.
		
	err = NewPatchIsland(1, &newPatchIsland);
	
	// Fill out the patch island, first by cloning the standard patch
	// island code, then by setting the payload (which also flushes
	// the caches), then by filling out the first PatchRecord.
	
	if (err == noErr) {
	
		// OK, I'll admit, the first parameter to this BlockMoveData is pretty
		// scary.  Basically MoreCFMPatchesCallThrough is a procedure pointer, ie
		// a pointer to a TVector.	But we don't want to copy the TVector,
		// we actually want to copy the code associated with it.  So we
		// cast it to a TVector, extract the codePointer field and copy from
		// that.
		
		BlockMoveData(	((TVector *) MoreCFMPatchesCallThrough)->codePointer, 
						newPatchIsland, 
						sizeof(PatchIsland));
		
		SetPatchIslandPayload(newPatchIsland, (UInt32) &newPatchIsland->patches[0]);
		newPatchIsland->patches[0].patchCode	= tVectorToPatch->codePointer;
		newPatchIsland->patches[0].patchTVector = tVectorToPatch;
		newPatchIsland->patches[0].patchCreator = 'last';
		newPatchIsland->patches[0].patchRefcon	= NULL;

		// The last thing we do is smash the code pointer for
		// tVectorToPatch.	Because we do this with a single store, we
		// don't need to disable interrupts.
		
		tVectorToPatch->codePointer = &newPatchIsland->patchIslandEntry[0];
	}
	
	return err;
}

static OSStatus AddPatchToPatchIsland(PatchIsland *existingPatchIsland,
									  TVector *patchTVectorToAdd,
									  OSType  creator,
									  void	  *refcon)
	// This routine adds a new patch to the front of the list of
	// patches in existingPatchIsland.	The basic strategy is to
	// create a new, bigger, pointer block in the system heap, and
	// then move all the stuff out of the old patch island into the
	// new one, and then switch the patched TVector's to point to
	// the new patch island.
{
	OSStatus err;
	ItemCount existingPatchCount;
	PatchIsland *newPatchIsland;

	assert(ValidPatchIsland(existingPatchIsland));
	assert(patchTVectorToAdd	!= NULL);
	
	// Create a new, bigger patch island in the system heap.
	
	err = GetPatchIslandPatchCount(existingPatchIsland, &existingPatchCount);
	if (err == noErr) {
		err = NewPatchIsland(existingPatchCount + 1, &newPatchIsland);
	}
	
	// Fill out the new patch island with stuff from the old one.  First
	// copy across the code and re-setup the payload (which also flushes
	// the code cache).	 Then copy across all the PatchRecords from the
	// old patch island.  Then fill out the extra PatchRecord (ie
	// patches[0]).	 Then switch all the patched TVectors to point
	// to the new patch island.
	
	if (err == noErr) {
		
		// The order here is important.	 SetPatchIslandPayload flushes
		// the code cache, so we must do it after we're done modifying
		// the code.
		
		BlockMoveData(existingPatchIsland, newPatchIsland, kPatchIslandHeaderSize);
		SetPatchIslandPayload(newPatchIsland, (UInt32) &newPatchIsland->patches[0]);
		
		BlockMoveData(&existingPatchIsland->patches[0], &newPatchIsland->patches[1], existingPatchCount * sizeof(PatchRecord));

		newPatchIsland->patches[0].patchCode	= patchTVectorToAdd->codePointer;
		newPatchIsland->patches[0].patchTVector = patchTVectorToAdd;
		newPatchIsland->patches[0].patchCreator = creator;
		newPatchIsland->patches[0].patchRefcon  = refcon;
		
		SyncPatchedTVectors(newPatchIsland, existingPatchCount + 1);

		assert(ValidPatchIsland(newPatchIsland));

		// Now that every is switched over, dispose of the old
		// patch island.
		
		DisposePatchIsland(existingPatchIsland);
	}
	
	return err;
}

////////////////////////////////////////////////////////////////
#pragma mark ----- Patch Destruction -----

static void DestroyPatchIsland(PatchIsland *patchIsland)
	// This routine destroys a patch island by removing the last patch
	// and disposing of the memory for the patch island.  It
	// assumes that the patch island doesn't contain any real patches,
	// ie there's only one PatchRecord in the patch island, which
	// is the original routine.
{
	ItemCount patchCount;

	assert(ValidPatchIsland(patchIsland));
	
	assert( (GetPatchIslandPatchCount(patchIsland, &patchCount) == noErr) && (patchCount == 1) );

	// Restore the original TVector's code pointer.	 Because we do this 
	// with a single store, we don't need to disable interrupts.

	patchIsland->patches[0].patchTVector->codePointer = patchIsland->patches[0].patchCode;

	// Dispose of the patch island itself.

	DisposePatchIsland(patchIsland);
}

static OSStatus RemovePatchFromPatchIsland(PatchIsland *existingPatchIsland, ItemCount patchIndex)
	// This routine removes the patch (specified by patchIndex, an index
	// into the patches array of PatchRecords) from the patch island.
	// It does this by creating a new patch island without the patch
	// and switching the patch to use the new patch island.
{
	OSStatus err;
	ItemCount existingPatchCount;
	PatchIsland *newPatchIsland;
	
	assert(ValidPatchIsland(existingPatchIsland));
	assert(patchIndex >= 0);

	// Create a new, smaller patch island in the system heap.

	err = GetPatchIslandPatchCount(existingPatchIsland, &existingPatchCount);
	if (err == noErr) {
		assert(patchIndex < existingPatchCount);
		err = NewPatchIsland(existingPatchCount - 1, &newPatchIsland);
	}

	// Fill out the new patch island with stuff from the old one.  First
	// copy across the code and re-setup the payload (which also flushes
	// the code cache).	 Then copy across all the PatchRecords from the
	// old patch island.  Then switch all the patched TVectors to point
	// to the new patch island.	 Finally, unpatch the recently removed
	// patch's TVector and destroy the existing patch island.
	
	// You might think I could use SetPtrSize here, but I can't.  This is
	// because the patched TVector might be being called at interrupt time,
	// so I have to atomically swap one valid patch island for another.
	// I do this by disabling interrupts for the minimum amount of time,
	// just inside SyncPatchedVectors.  I don't want to be calling
	// SetPtrSize with interrupts disabled.

	if (err == noErr) {

		// The order here is important.	 SetPatchIslandPayload flushes
		// the code cache, so we must do it after we're done modifying
		// the code.

		BlockMoveData(existingPatchIsland, newPatchIsland, kPatchIslandHeaderSize);
		SetPatchIslandPayload(newPatchIsland, (UInt32) &newPatchIsland->patches[0]);
		
		// Copy across the PatchRecords, first the ones below patchIndex,
		// then the ones above it.
		
		BlockMoveData(&existingPatchIsland->patches[0], &newPatchIsland->patches[0], patchIndex * sizeof(PatchRecord));
		BlockMoveData(&existingPatchIsland->patches[patchIndex + 1], &newPatchIsland->patches[patchIndex], (existingPatchCount - (patchIndex + 1)) * sizeof(PatchRecord));

		// The order here is also important.  Make sure we completely
		// switch over to using newPatchIsland before restoring
		// the codePointer for the removed patch.

		SyncPatchedTVectors(newPatchIsland, existingPatchCount - 1);

		assert(ValidPatchIsland(existingPatchIsland));
		
		existingPatchIsland->patches[patchIndex].patchTVector->codePointer = existingPatchIsland->patches[patchIndex].patchCode;
		DisposePatchIsland(existingPatchIsland);
	}
	
	return err;
}

////////////////////////////////////////////////////////////////
#pragma mark ----- Static Implementation -----

extern ProcPtr gMoreCFMPatchesCallThrough = MoreCFMPatchesCallThrough;
	// I could have chosen to export MoreCFMPatchesCallThrough as
	// a routine rather than a ProcPtr -- it would work just as
	// well.  However, I chose this approach because it seems
	// clearer to me from a client perspective.  Casting a ProcPtr
	// to the appropriate C routine type avoids the confusion
	// of the actual prototype of MoreCFMPatchesCallThrough.
	//
	// Also, I may provide a future mechanism to create a call
	// through ProcPtr from the patch island, and thereby avoid
	// the requirement that the MoreCFMPatches remain resident
	// while patches might call MoreCFMPatchesCallThrough.

static pascal OSStatus MorePatchTVectorStatic(TVector *tVectorToPatch,
							  TVector *patchTVectorToAdd,
							  OSType  creator,
							  void	  *refcon)
	// See comment in interface part.
{
	OSStatus err;
	PatchIsland *patchIsland;
	ItemCount junkPatchIndex;

	assert(kPatchIslandHeaderSize == 52);
	assert(kPatchRecordSize == sizeof(PatchRecord));

	assert(tVectorToPatch	  != NULL);
	assert(patchTVectorToAdd != NULL);

	// Note that, in the most common case (ie the TVector hasn't
	// been patched already), this code is pretty inefficient.  It
	// basically patches the TVector once with a single entry
	// patch island which just contains the original routine, then
	// replaces that adds the new patch to the patch island.  It
	// may would been more efficient to create a two entry patch
	// island in that case.  But I decided that simplicity was
	// more important that efficiency, especially in this kind
	// of code.
		
	err = noErr;
	if ( ! HasTVectorBeenPatched(tVectorToPatch) ) {
		err = CreateNewPatchIsland(tVectorToPatch);
	}
	if (err == noErr) {
		patchIsland = GetPatchIslandFromTVector(tVectorToPatch);
		err = FindPatchInPatchIsland(patchIsland, patchTVectorToAdd, &junkPatchIndex);
		if (err == noErr) {
			err = kTVectorAlreadyPatchedByYouErr;
		} else if (err == kPatchNotFoundInPatchIslandErr) {
			err = AddPatchToPatchIsland(patchIsland, patchTVectorToAdd,
										creator, refcon);
		} else {
			// Unexpected result from FindPatchInPatchIsland.  Need
			// to reanalyse this result checking to see whether it still
			// makes sense.
			assert(false);
		}
	}
	return err;
}

static pascal OSStatus MoreUnpatchTVectorStatic(TVector *tVectorToUnpatch, TVector *patchTVectorToRemove)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus junk;
	PatchIsland *patchIsland;
	ItemCount patchIndex;
	ItemCount patchCount;

	assert(tVectorToUnpatch	 != NULL);
	assert(patchTVectorToRemove != NULL);

	// Like MorePatchTVector, this could be done more efficiently
	// by recognising that a removing a patch from a two entry
	// patch island is equivalent to disposing the patch island
	// itself.  But again I chose the simplest approach.

	if ( HasTVectorBeenPatched(tVectorToUnpatch) ) {
		patchIsland = GetPatchIslandFromTVector(tVectorToUnpatch);
		err = FindPatchInPatchIsland(patchIsland, patchTVectorToRemove, &patchIndex);
		if (err == noErr) {
			err = RemovePatchFromPatchIsland(patchIsland, patchIndex);
		}
		if (err == noErr) {

			// As RemovePatchFromPatchIsland changes tVectorToUnpatch->codePointer
			// from which patchIsland is derived, we have to get patchIsland
			// again.

			patchIsland = GetPatchIslandFromTVector(tVectorToUnpatch);

			junk = GetPatchIslandPatchCount(patchIsland, &patchCount);
			assert(junk == noErr);
			if (junk == noErr && patchCount == 1) {
				DestroyPatchIsland(patchIsland);
			}
		}
	} else {
		err = kVectorNotPatchedByUsErr;
	}
	return err;
}

static pascal OSStatus MoreCountPatchesStatic(TVector *tVector, ItemCount *count)
	// See interface comment for MoreCountPatches.
{
	OSStatus err;
	PatchIsland *patchIsland;
	
	assert(tVector != NULL);
	assert(count   != NULL);
	
	if ( HasTVectorBeenPatched(tVector) ) {
		patchIsland = GetPatchIslandFromTVector(tVector);
		err = GetPatchIslandPatchCount(patchIsland, count);
		if (err == noErr) {

			// Decrement count by one because GetPatchIslandPatchCount
			// returns the number of patch records in the patch island,
			// which includes the last patch record, which represents
			// the original routine.  Clients of this routine won't expect
			// to see this patch record because they didn't apply the
			// patch so there's no reason for it to be returned to them.
			// It's existance is an implementation detail.
			
			// Also, assert that count is 2 or more.  This is
			// because the patch island must always contain: 1) a
			// least one patch record applied by a client, and
			// 2) the last patch record that represents the original
			// routine.  If the patch island contained only
			// one patch, we would have destroyed it in 
			// MoreUnpatchTVector.
			
			assert(*count > 1);
			*count -= 1;
		}
	} else {
		err = kVectorNotPatchedByUsErr;
	}

	return err;
}

static pascal OSStatus MoreGetIndexedPatchInfoStatic(TVector *tVector, ItemCount index,
							OSType infoKind, void *buffer, ByteCount bufferSize)
	// See interface comment for MoreGetIndexedPatchInfo.
{
	OSStatus err;
	PatchIsland *patchIsland;
	ItemCount patchCount;
	void *infoSource;
	ByteCount infoSize;
	
	assert(tVector   != NULL);
	assert(index     != 0);
	assert(buffer != NULL);

	if ( HasTVectorBeenPatched(tVector) ) {
		patchIsland = GetPatchIslandFromTVector(tVector);
		err = GetPatchIslandPatchCount(patchIsland, &patchCount);
		if (err == noErr && (index < 1 || index > patchCount)) {
			err = kPatchNotFoundErr;
		}
		if (err == noErr) {
		
			// Compensate for original routine's patch record
			// at the end of the patch island.  See comments in
			// MoreCountPatchesStatic for why we do this.
			
			assert(patchCount > 1);
			patchCount -= 1;
		
			// Compensate for the zero-based array versus the one-based index.
			
			index -= 1;
			
			switch (infoKind) {
				case kPatchInfoCreator:
					infoSource = &patchIsland->patches[index].patchCreator;
					infoSize = sizeof(OSType);
					break;
				case kPatchInfoTVector:
					infoSource = &patchIsland->patches[index].patchTVector;
					infoSize = sizeof(TVector *);
					break;
				case kPatchInfoRefcon:
					infoSource = &patchIsland->patches[index].patchRefcon;
					infoSize = sizeof(void *);
					break;
				default:
					err = paramErr;
					break;
			}
		}
		if (err == noErr) {
			if (infoSize > bufferSize) {
				
				// The buffer isn't big enough for the data,
				// let the caller know about it and copy in
				// as much data as possible.
				
				infoSize = bufferSize;
				err = kPatchInfoOverrunErr;
			}
			BlockMoveData(infoSource, buffer, infoSize);
		}
	} else {
		err = kVectorNotPatchedByUsErr;
	}

	return err;
}

////////////////////////////////////////////////////////////////
#pragma mark ----- Dynamic Stubs -----

extern pascal OSStatus MorePatchTVectorDynamic(TVector *tVectorToPatch,
						    TVector *patchTVectorToAdd,
						    OSType  creator,
							void    *refcon);
extern pascal OSStatus MoreUnpatchTVectorDynamic(TVector *tVectorToUnpatch,
							TVector *patchTVectorToRemove);
extern pascal OSStatus MoreCountPatchesDynamic(TVector *tVector, ItemCount *count);
extern pascal OSStatus MoreGetIndexedPatchInfoDynamic(TVector *tVector, ItemCount index,
							OSType infoKind, void *buffer, ByteCount bufferSize);

////////////////////////////////////////////////////////////////
#pragma mark ----- API Entry Point Dispatcher -----

extern pascal OSStatus MorePatchTVector(TVector *tVectorToPatch,
						    TVector *patchTVectorToAdd,
						    OSType  creator,
							void    *refcon)
	// See comment in interface part.
{
	if (MorePatchTVectorDynamic == (void *) kUnresolvedCFragSymbolAddress) {
		return MorePatchTVectorStatic(tVectorToPatch, patchTVectorToAdd, creator, refcon);
	} else {
		return MorePatchTVectorDynamic(tVectorToPatch, patchTVectorToAdd, creator, refcon);
	}
}

extern pascal OSStatus MoreUnpatchTVector(TVector *tVectorToUnpatch,
							TVector *patchTVectorToRemove)
	// See comment in interface part.
{
	if (MoreUnpatchTVectorDynamic == (void *) kUnresolvedCFragSymbolAddress) {
		return MoreUnpatchTVectorStatic(tVectorToUnpatch, patchTVectorToRemove);
	} else {
		return MoreUnpatchTVectorDynamic(tVectorToUnpatch, patchTVectorToRemove);
	}
}

extern pascal OSStatus MoreCountPatches(TVector *tVector, ItemCount *count)
	// See comment in interface part.
{
	if (MoreCountPatchesDynamic == (void *) kUnresolvedCFragSymbolAddress) {
		return MoreCountPatchesStatic(tVector, count);
	} else {
		return MoreCountPatchesDynamic(tVector, count);
	}
}
	
extern pascal OSStatus MoreGetIndexedPatchInfo(TVector *tVector, ItemCount index,
							OSType infoKind, void *buffer, ByteCount bufferSize)
	// See comment in interface part.
{
	if (MoreGetIndexedPatchInfoDynamic == (void *) kUnresolvedCFragSymbolAddress) {
		return MoreGetIndexedPatchInfoStatic(tVector, index, infoKind, buffer, bufferSize);
	} else {
		return MoreGetIndexedPatchInfoDynamic(tVector, index, infoKind, buffer, bufferSize);
	}
}
