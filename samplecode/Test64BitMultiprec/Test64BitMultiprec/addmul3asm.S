/*

File:addmul3asm.S

Abstract: This sample illustrates an optimized 64-bit multiply accumulate 
routine for the G5, as described in the paper "Special applications of 
64-bit arithmetic: Acceleration on the Apple G5." The quantity A*Y+B is 
computed. A and B are arrays of 64-bit words and Y is a 64-bit integer. 
This implementation requires that the size argument is a multiple of 4.

Version: <1.0>

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006 Apple Computer, Inc., All Rights Reserved

*/ 
#if !defined(__ppc64__)
#error "64-bit asm required!"
#endif 

/* uint64 addmul(uint64 *B, uint64 *A, uint64 Y, uint64 words); */

#define CARRY r0
#define STACK r1
#define B_PTR r3
#define A_PTR r4
#define Y r5
#define SIZE r6
#define A0 r6
#define A1 r7
#define B0 r8
#define B1 r9
#define C0 r10
#define C1 r11
#define D0 r12
#define D1 r13

	.globl _addmul
	.align 5
_addmul:
	srdi SIZE, SIZE, 2	/* divide word by 4 */
	ld A1, 0(A_PTR)		/* load first word of A[] */
	std r13, -8(STACK)	/* spill to stack */
	li CARRY, 0		/* carry in starts as 0 */
	ld B1, 8(A_PTR)		/* load second word of A[] */
	subi SIZE, SIZE, 1	/* the last group of four words is special */
	ld C1, 16(A_PTR)
	ld D1, 24(A_PTR)	/* load 2nd and 3rd word of A[] */
	mtctr SIZE		/* set number of iterations */
	.align32 4, 0x60000000
.loop:
	mulld A0, A1, Y
	mulhdu A1, A1, Y
	mulld B0, B1, Y
	mulhdu B1, B1, Y
	mulld C0, C1, Y
	mulhdu C1, C1, Y
	mulld D0, D1, Y
	mulhdu D1, D1, Y	/* multiply each A[] word by Y */

	addc CARRY, CARRY, A0
	adde A1, A1, B0
	adde B1, B1, C0
	adde C1, C1, D0
	addze D1, D1		/* sum the halves of the four products */

	ld A0, 0(B_PTR)
	ld B0, 8(B_PTR)
	ld C0, 16(B_PTR)
	ld D0, 24(B_PTR)	/* load corresponding words of B[] */

	addc A0, A0, CARRY
	adde B0, B0, A1
	adde C0, C0, B1
	adde D0, D0, C1
	addze CARRY, D1		/* add them to the sum, saving carry out */

	ld A1, 32(A_PTR)
	ld B1, 40(A_PTR)
	ld C1, 48(A_PTR)
	ld D1, 56(A_PTR)	/* load the next 4 words of A[] */

	std A0, 0(B_PTR)
	std B0, 8(B_PTR)
	std C0, 16(B_PTR)
	std D0, 24(B_PTR)	/* store completed words of B[] */

	addi A_PTR, A_PTR, 32
	addi B_PTR, B_PTR, 32
	bdnz .loop		/* increment pointers and loop */

	mulld A0, A1, Y
	mulhdu A1, A1, Y
	mulld B0, B1, Y
	mulhdu B1, B1, Y
	mulld C0, C1, Y
	mulhdu C1, C1, Y
	mulld D0, D1, Y
	mulhdu D1, D1, Y

	addc CARRY, CARRY, A0
	adde A1, A1, B0
	adde B1, B1, C0
	adde C1, C1, D0
	addze D1, D1

	ld A0, 0(B_PTR)
	ld B0, 8(B_PTR)
	ld C0, 16(B_PTR)
	ld D0, 24(B_PTR)

	addc A0, A0, CARRY
	adde B0, B0, A1
	adde C0, C0, B1
	adde D0, D0, C1
	addze CARRY, D1

	std A0, 0(B_PTR)
	std B0, 8(B_PTR)
	std C0, 16(B_PTR)
	std D0, 24(B_PTR)
	ld r13, -8(STACK)
	mr r3, CARRY
	blr
