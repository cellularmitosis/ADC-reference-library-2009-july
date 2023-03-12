/*

File:mulmod64asm_2.S

Abstract: Shows how to leverage the 64-bit G5 processor to perform the operation (a * b)%c, where a, b, and c are 64-bit quantities and a "reciprocal" of c has been precomputed. This sample is the implementation described in the paper "Special applications of 64-bit arithmetic: Acceleration on the Apple G5."

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

/* uint64_t modmul(uint64_t A, uint64_t B, uint64_t V, uint64_t RHI, uint64_t RLO ); */

/* Arguments */
#define MOD r3
#define STACK r1
#define A r3
#define B r4
#define V r5
#define RHI r6
#define RLO r7

/* Work registers */
#define ABHI r8
#define ABLO r9
#define ABHI_RHI_HI r3
#define ABHI_RHI_LO r4
#define ABHI_RLO_HI r10
#define ABHI_RLO_LO r11
#define ABLO_RHI_HI r12
#define ABLO_RHI_LO r0
#define ABLO_RLO_HI r2
#define DHI	r11
#define DLO r7
#define DVHI r6
#define DVLO r2
#define DHVLO r12
#define MHI r3
#define MLO r4
#define ZERO r10

	.globl _modmul
	.globl modmul
	.align 5
_modmul:
modmul:
/* Compute AB */
	mulld ABLO, A, B
	mulhdu ABHI, A, B

	/* A, B no longer necessary */
	

/* Compute AB % V via reciprocal multiplication */
	
	/* Now perform the 2x2 multiply AB*R, then drop the low 128-bits */
	/* d = (ab*r)>>s */
	mulhdu ABLO_RLO_HI, ABLO, RLO
	mulld ABLO_RHI_LO, ABLO, RHI
/* - */
	mulhdu ABLO_RHI_HI, ABLO, RHI
	mulld ABHI_RLO_LO, ABHI, RLO
	mulhdu ABHI_RLO_HI, ABHI, RLO
	mulld ABHI_RHI_LO, ABHI, RHI
/* - */
	mulhdu ABHI_RHI_HI, ABHI, RHI
	addc DLO, ABLO_RLO_HI, ABHI_RLO_LO 	/* Cracked */
	addze DHI, ABHI_RLO_HI 
/* - */
	addc DLO, DLO, ABLO_RHI_LO 			/* Cracked */
	addze DHI, ABHI_RLO_HI
		/* DLO no longer needed */
		/* <--- Need an instruction here to fill a dispatch group */
/* - */
	addc DLO, DHI, ABLO_RHI_HI			/* Cracked */
		/* DHI no longer needed */
	addze DHI, ABHI_RHI_HI
	xor ZERO, ZERO, ZERO

/* - */
	addc DLO, DLO, ABHI_RHI_LO			/* Cracked */
	addze DHI, DHI
	
	/* compute d*V, 2x1 multiply */
	/* Two digit product because A*B - 1 <= d*V <= A*B */
	/* m = AB - DV */
	/* Note that M can be > 2^{64}-1 because d can be AB/V - 1 */
	/* We do not care though because M, V <= 2^{64}-1 ==> AB - DV <= 2^{65}-2*/
	mulld DHVLO, DHI, V
/* - */
	mulld DVLO, DLO, V
	mulhdu DVHI, DLO, V
	/* (mulhdu DHVHI, DHI, V) == 0, so skip it */
	subfc MLO, DVLO, ABLO				/* Cracked */
/* - */
	add DVHI, DVHI, DHVLO
	subfe MHI, DVHI, ABHI
	
	/* if (m > V) { m -= V, div--; } */
	/* m = AB % V or m = (AB % V) + V */
	/* Subtract V from M */
	subfc MLO, V, MLO					/* Cracked */
/* - */
	subfe MHI, ZERO, MHI
	
	/* Make MHI a mask */
	sradi MHI, MHI, 63
	and V, V, MHI
	/* Add back MASK & V to MLO */
	add MOD, V, MLO
	
	/* MOD now contains A * B % V */
	
#if 0
	/* If MHI == -1, d was right. If MHI == 0, d was one too low */
	nor MHI, MHI, MHI
	addc DLO, DLO, MHI
	addze DHI, DHI
	
	/* Store D somewhere... */
#endif	
	blr
