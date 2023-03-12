//
//
//	File:		<eGmp.c>
//
//	Abstract:	Calculate e using the GMP library for MP floating point arithmetic. 
//
//	Version: <1.0 >
//
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by
//				Apple Inc. ( "Apple" ) in consideration of your agreement to the
//				following terms, and your use, installation, modification or
//				redistribution of this Apple software constitutes acceptance of these
//				terms.  If you do not agree with these terms, please do not use,
//				install, modify or redistribute this Apple software.
//
//				In consideration of your agreement to abide by the following terms, and
//				subject to these terms, Apple grants you a personal, non - exclusive
//				license, under Apple's copyrights in this original Apple software ( the
//				"Apple Software" ), to use, reproduce, modify and redistribute the Apple
//				Software, with or without modifications, in source and / or binary forms;
//				provided that if you redistribute the Apple Software in its entirety and
//				without modifications, you must retain this notice and the following
//				text and disclaimers in all such redistributions of the Apple Software.
//				Neither the name, trademarks, service marks or logos of Apple Inc.
//				may be used to endorse or promote products derived from the Apple
//				Software without specific prior written permission from Apple.  Except
//				as expressly stated in this notice, no other rights or licenses, express
//				or implied, are granted by Apple herein, including but not limited to
//				any patent rights that may be infringed by your derivative works or by
//				other works in which the Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//				MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//				THE IMPLIED WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS
//				FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//				OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//				OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//				SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//				INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//				MODIFICATION AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//				AND WHETHER UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ),
//				STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//				POSSIBILITY OF SUCH DAMAGE.
//
//	Copyright ( C ) 2007 Apple Inc. All Rights Reserved.
//
// ****************************************************
#pragma mark -
#pragma mark * includes & imports *
// ----------------------------------------------------

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <gmp.h>
#include <unistd.h>
#include <time.h>
#include <mach/mach_time.h> 

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static void usage(char **argv);
static double currentTime(void);
static void calculateE( unsigned bits, mpf_t e);

// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

int main(int argc, char **argv)
{
	unsigned bits;
	double start, end;
	mpf_t e;
	
	if(argc < 2) {
		usage(argv);
	}
	bits = atoi(argv[1]);
	if(bits == 0) {
		usage(argv);
	}
	
	printf("Calculating e...");
	fflush(stdout);
	
	start = currentTime();
	
	mpf_set_default_prec(bits);
	mpf_init(e);
	
	calculateE(bits, e);
	
	end = currentTime();
	
	printf("\ne = ");
	mpf_out_str(stdout, 10, 0, e);
	printf("\n");
	printf("total elapsed time : %.2f seconds\n", end - start);
	return 0;
}

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

static void usage(char **argv)
{
	printf("usage: %s bits\n", argv[0]);
	exit(1);
}

static double currentTime(void) 
{ 
	static double scale = 0.0; 
	if (0.0 == scale) { 
		mach_timebase_info_data_t info; 
		mach_timebase_info(&info); 
		scale = info.numer / info.denom * 1e-9; 
	} 
	return mach_absolute_time() * scale; 
} 

/* 
 * Evaluate the series 1/0! + 1/1! + 1/2! + 1/3! + 1/4! ...
 * On input, 'bits' is the desired precision in bits.
 * On output, e' contains the calculated value.
 */
static void calculateE(
	unsigned bits,
	mpf_t e)
{
	mpf_t lastE;
	mpf_t invFact;					/* 1/2!, 1/3!, 1/4!, etc. */
	unsigned term;					/* 2, 3, 4... */
	
	/* initial conditions, including the first two terms */
	mpf_init_set_ui(lastE, 0);
	mpf_set_ui(e, 2);				
	mpf_init_set_ui(invFact, 1);	/* 1/1! */
	term = 2;
	
	for(;;) {
		/* invFact /= (term) */
		mpf_div_ui(invFact, invFact, term);
		/* e += 1 / (term!) */
		mpf_add(e, e, invFact);
		
		/* if e == lastE, within the requested precision, we're done */
		if(mpf_eq(e, lastE, bits)) {
			break;
		}
		mpf_set(lastE, e);
		term++;
	}
	/* free memory associated with mpf_t's we allocated */
	mpf_clear(lastE);
	mpf_clear(invFact);
}
