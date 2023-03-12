#	File:		MoreCFMPatches.s
#
#	Contains:	Assembly language stuff for the CFM patching library.
#
#	Written by:	Quinn
#
#	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.
#
#	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
#				("Apple") in consideration of your agreement to the following terms, and your
#				use, installation, modification or redistribution of this Apple software
#				constitutes acceptance of these terms.  If you do not agree with these terms,
#				please do not use, install, modify or redistribute this Apple software.
#
#				In consideration of your agreement to abide by the following terms, and subject
#				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
#				copyrights in this original Apple software (the "Apple Software"), to use,
#				reproduce, modify and redistribute the Apple Software, with or without
#				modifications, in source and/or binary forms; provided that if you redistribute
#				the Apple Software in its entirety and without modifications, you must retain
#				this notice and the following text and disclaimers in all such redistributions of
#				the Apple Software.  Neither the name, trademarks, service marks or logos of
#				Apple Computer, Inc. may be used to endorse or promote products derived from the
#				Apple Software without specific prior written permission from Apple.  Except as
#				expressly stated in this notice, no other rights or licenses, express or implied,
#				are granted by Apple herein, including but not limited to any patent rights that
#				may be infringed by your derivative works or by other works in which the Apple
#				Software may be incorporated.
#
#				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
#				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
#				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
#				COMBINATION WITH YOUR PRODUCTS.
#
#				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
#				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
#				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
#				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
#				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
#				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	Change History (most recent first):
#
# $Log: MoreCFMPatches.s,v $
# Revision 1.3  2001/11/07 15:51:49         
# Tidy up headers, add CVS logs, update copyright.
#
#
#        <2>     16/3/99    Quinn   Fix checkin comment.
#        <1>     16/3/99    Quinn   First checked in.
#

# IMPORTANT:
# Each instruction is commented with its offset from the beginning
# of the routine, and hence the offset into the C PatchIsland
# data structure.  Rather than comment line-by-line, I've included
# comments at the end of the file.  These comments reference the
# instructions by their offset.

# Constants shared with "MoreCFMPatches.c".  If you change them
# here, you must also change them there.

kSystemReservedFrameOffset	equ		16
kPatchIslandSignature 		equ		'Nat!'
kPatchRecordSize			equ		16

# TVector for the MorePatchCallThrough routine.

		csect	MoreCFMPatchesCallThrough{DS}
		export	MoreCFMPatchesCallThrough{DS}
		dc.l	.MoreCFMPatchesCallThrough{PR}
		dc.l	toc{tc0}

# Patch island code

		csect	.MoreCFMPatchesCallThrough{PR}
		export	.MoreCFMPatchesCallThrough{PR}

# Local labels correspond to field names in the C PatchIsland data structure.

callThrough:
		lwz			r11,0(sp)								# 0
		lwz			r11,kSystemReservedFrameOffset(r11)		# 4
		addi		r11,r11,kPatchRecordSize				# 8
		b			PatchIslandCommon						# 12
signature
		dc.l		kPatchIslandSignature					# 16
patchIslandEntry:
		lis			r11,0									# 20
		ori			r11,r11,0								# 24
patchIslandCommon:
		lwz			r0,0(r11)								# 28
		lwz			r12,4(r11)								# 32
		mtctr		r0										# 36
		stw			r11,kSystemReservedFrameOffset(sp)		# 40
		lwz			RTOC,4(r12)								# 44
		bctr												# 48
															# 52
		toc

		end

# Stuff after the "end" directive is ignored by the assembler.

How and Why
-----------

To understand this code you must understand three key things about
how CFM uses TVectors, as described in the "MoreCFMPatches.h" header
file:

//      a) the first word must point to the code,
//      b) the caller must load the second word into R2, and
//      c) the caller must load R12 with a pointer to the TVector itself.
//         [This allows the callee to extract any extra information
//          it cares to store in the TVector.]

This patch island must preserve these invariants.

There are two basic threads of execution.  When the patched routine
is first called, we enter at patchIslandEntry.  The next two
instructions (offsets 20 and 24) comprise the patch island payload.
[The term "payload" is explained in "MoreCFMPatches.c".]  As written
here, they load R11 with a constant 0.  However, when we build
a patch island, we modify these instruction to load R11 with a
pointer to the first patch record in the patch island (which is
actually at offset 52, although this code doesn't know that).  Once
we have R11 set to the 'current' patch record, we fall through
to patchIslandCommon (offset 28), which does the bulk of the work.

	Note:
	PowerPC calling conventions on the Mac OS say that R11
	is free for uses such as these.  See "Mac OS Runtime
	Architectures" <http://www.apple.com/developer/> for
	details.

The second entry point is callThrough, which is called when a
patch calls gMoreCFMPatchesCallThrough.  The goal of this
entry point is to call through to the next patch record in the
patch chain.  To that end, it must load R11 with the address
of that patch record and then 'fall' through to patchIslandCommon
(offset 28), which does the bulk of the work.

callThrough gets the appropriate patch record from the stack frame
of the callee (not the patch routine that's calling
gMoreCFMPatchesCallThrough, but the person who called the patch).
The instruction at offset 0 loads R11 with the callee's 'frame' pointer,
ie the value of SP that the patch routine's preamble saved there.
The next instruction (offset 4) gets the current patch record
pointer into R11 by extracting it from the system reserved frame
offset in the callee's stack frame.  Next (offset 8), callThrough
increments that pointer by the size of the patch record, to shuffle 
along to the next patch record in the chain.  Finally (offset 12),
it branches to patchIslandCommon (offset 28).

patchIslandCommon is the guts behind the patch island.  The input
parameter is R11, a pointer to the patch record we wish to invoke.
[As explained in "MoreCFMPatches.c", the last patch record actually
describes the original patched routine, but the logic works in
that case as well.]  patchIslandCommon starts (offset 28) by
loading R0 with the patch code pointer from the patch record
pointed to by R11.  [In C this is the patchCode field of the
PatchRecord type.]  It then (offset 32) loads R12 with the TVector
pointer of the patch record.  [In C this is the patchTVector field
of the PatchRecord type.]  Remember, the Mac OS runtime requires
that, when a routine is entered, R12 must always be a pointer
to its TVector, so that the routine can extract extra information
from the TVector.  Then (offset 36) we move R0 (the code pointer)
into the counter register, in preparation for a branch to it (offset 48).
But before that (offset 40), we store the R11 into the current frame,
which allows callThrough to extract it from this frame if the patch
calls gMoreCFMPatchesCallThrough.  Finally (offset 44), we load RTOC with
the second word of the TVector (this is also required by the runtime
architecture) and then branch (offset 48) to the patch code, whose
address is in the counter register (put there by the instruction at
offset 36).

Neat huh?
