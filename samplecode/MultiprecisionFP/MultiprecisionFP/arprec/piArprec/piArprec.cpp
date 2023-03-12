//
//
//	File:		<piArprec.cpp>
//
//	Abstract:	Calculate pi via the Gauss-Legendre algorithm, using the ARPREC C++ library 
//				for MP floating point arithmetic. 
//				See <http://en.wikipedia.org/wiki/Gauss-Legendre_algorithm> for a good
//				discussion of this algorithm.
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
#include <arprec/mp_real.h>
#include <unistd.h>
#include <mach/mach_time.h> 

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static void usage(char **argv);
static double currentTime(void);
static void calculatePi( unsigned digits, mp_real &pi);

// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

int main(int argc, char **argv)
{
	unsigned digits;
	double start, end;
	
	if(argc < 2) {
		usage(argv);
	}
	digits = atoi(argv[1]);
	if(digits == 0) {
		usage(argv);
	}
	
	printf("Calculating pi to %d digits...", digits);
	fflush(stdout);
	
	start = currentTime();
	
	mp::mp_init(digits + 2, 0);
	mp::mpsetoutputprec(digits);
	
	mp_real pi;
	
	calculatePi(digits, pi);
	
	end = currentTime();
	
	std::cout << "total elapsed time : " << (end - start) << " seconds\n";
	std::cout << "\npi = " << pi << "\n";
	return 0;
}

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

static void usage(char **argv)
{
	printf("usage: %s digits\n", argv[0]);
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
 * The core Gauss-Legendre routine. 
 * On input, 'digits' is the desired precision in digits.
 * On output, 'pi' contains the calculated value.
 */
static void calculatePi(
	unsigned digits,
	mp_real &pi)
{
	mp_real lastPi(0.0);
	mp_real scratch;

	/* variables per the formal Gauss-Legendre formulae */
	mp_real a(1.0);
	mp_real b;
	mp_real t(0.25);
	mp_real x;
	mp_real y;
	double p = 1.0;
	
	/* epsilon, to detect when we've hit the desired precision */
    mp_real eps = pow(mp_real(10.0), -((int)digits));

	/* initial conditions */
	b = 1.0 / sqrt(mp_real(2.0));			/* b := 1 / sqrt(2) */
	
	for(;;) {
		x = (a + b) / 2.0;
		y = sqrt(a * b);
		
		/* t := t - p * (a-x)**2 */
		t = t - (p * sqr(a - x));
		
		a = x;
		b = y;
		p *= 2.0;
		
		/* pi := ((a + b)**2) / 4t */
		pi = sqr(a + b) / (4 * t);

		/* if pi == lastPi, within the requested precision, we're done */
		if( abs(lastPi - pi) < eps) {
			break;
		}
		lastPi = pi;
	}
}
