/*

File:addmulv2.c

Abstract: This sample illustrates the use of vecLib's implementation of
a 512-bit multiprecision multiply. This source file is referenced
in the paper "Special applications of 64-bit arithmetic: Acceleration on
the Apple G5."

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
//****************************************************
#pragma mark * complation directives *

#define kNumPasses	50
#define kNumCalls	1000

//****************************************************
#pragma mark -
#pragma mark * includes & imports *
//----------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vecLib/vBigNum.h>
#include <mach/mach_time.h>

//#include "hires.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
//----------------------------------------------------

//#define NUM_WORDS (512/32)
//****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
//----------------------------------------------------

static double CurrentTime(void);

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *
//----------------------------------------------------
/*****************************************************
*
* Routine:	main ( argc, argv ) 
*
* Purpose:	main program entry point
*
* Inputs:	argc	 - the number of elements in the argv array
*			argv	 - an array of pointers to the parameters to this application
*
* Returns:	int		 - error code ( 0 == no error ) 
*
*/
int main(int argc, char *argv[]) 
{
	vU512 a, b;
	vU1024 prod;
	double start, stop, diff, min, max;
	int i, j;
	
	// initialize A & B arrays with random values

 	srand48(1111111111);              
	a.s.MSW = lrand48(); b.s.MSW = lrand48();
	a.s.d2 = lrand48(); b.s.d2 = lrand48();
	a.s.d3 = lrand48(); b.s.d3 = lrand48();
	a.s.d4 = lrand48(); b.s.d4 = lrand48();
	a.s.d5 = lrand48(); b.s.d5 = lrand48();
	a.s.d6 = lrand48(); b.s.d6 = lrand48();
	a.s.d7 = lrand48(); b.s.d7 = lrand48();
	a.s.d8 = lrand48(); b.s.d8 = lrand48();
	a.s.d9 = lrand48(); b.s.d9 = lrand48();
	a.s.d10 = lrand48(); b.s.d10 = lrand48();
	a.s.d11 = lrand48(); b.s.d11 = lrand48();
	a.s.d12 = lrand48(); b.s.d12 = lrand48();
	a.s.d13 = lrand48(); b.s.d13 = lrand48();
	a.s.d14 = lrand48(); b.s.d14 = lrand48();
	a.s.LSW = lrand48(); b.s.LSW = lrand48();

	min = (uint64_t)(-1);
	max = (uint64_t)(0);

	for (i = 0; i < kNumPasses; i++)
	{
		start = CurrentTime();
		for (j = 0; j < kNumCalls; j++)
		{
			vU512FullMultiply(&a, &b, &prod);
		}
		stop = CurrentTime();
		diff = stop - start;
		if (diff < min)
		{
			min = diff;
		}
		if (diff > max)
		{
			max = diff;
		}
	}

	printf("%lf seconds per multiply\n", min);
	printf("%lf multiplys per seconds\n", 1.0 / min);
	
//	printf("%lf seconds per multiply (max)\n", max);
//	printf("delta: %f.\n", max / min );

	return 0;
}	// main

//****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
//----------------------------------------------------

//
// Returns the current time in seconds
// 
static double CurrentTime(void)
{
    static double scale = 0.0;
	
    if (0.0 == scale) {
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        scale = info.numer / info.denom * 1e-9;
    }
	
    return mach_absolute_time() * scale;
}	// CurrentTime
