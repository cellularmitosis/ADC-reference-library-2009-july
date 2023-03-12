//
//
//	File:		<eArprec.cpp>
//
//	Abstract:	Calculate e, using a variety of algorithms, using the ARPREC C++ library 
//				for MP floating point arithmetic. 
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
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------

typedef enum {
	AlgLimit,
	AlgSeries,
	AlgNewton
} WhichAlg;

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static void usage(char **argv);
static double currentTime(void);
static void eViaSeries( mp_real eps, mp_real &e);
static void eViaLimit( mp_real eps, mp_real &e);
static void eViaNewtonRaphson( mp_real eps, mp_real &e);
																		   
// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

int main(int argc, char **argv)
{
	unsigned digits;
	double start, end;
	extern char *optarg;
	extern int optind;
	int arg;
	WhichAlg alg = AlgSeries;
	const char *method = "Series";
	bool needConsts = false;
	
	if (argc < 2) {
		usage(argv);
	}
	while ((arg = getopt(argc, argv, "lrh")) != -1) {
		switch (arg) {
			case 'l':
				alg = AlgLimit;
				method = "Limit";
				needConsts = 1;		// need to precalculate pi, etc.
				break;
			case 'r':
				alg = AlgNewton;
				method = "Newton-Raphson";
				needConsts = 1;		// need to precalculate pi, etc.
				break;
			default:
				usage(argv);
		}
	}
	if (optind != (argc - 1)) {
		usage(argv);
	}
	
	digits = atoi(argv[optind]);
	if (digits == 0) {
		usage(argv);
	}
	
	printf("Calculating e to %d digits using %s...", digits, method);
	fflush(stdout);
	
	start = currentTime();
	
	// Note the series algorithm does not need consts like pi precalculated
	mp::mp_init(digits + 2, needConsts);
	mp::mpsetoutputprec(digits);
	
	mp_real e;
    mp_real eps = pow(mp_real(10.0), -((int)digits));
	
	switch(alg) {
		case AlgLimit:
			eViaLimit(eps, e);
			break;
		case AlgSeries:
			eViaSeries(eps, e);
			break;
		case AlgNewton:
			eViaNewtonRaphson(eps, e);
			break;
	}
	
	end = currentTime();
	
	std::cout << "\ne = " << e << "\n";
	std::cout << "Algorithm          : " << method << "\n";
	std::cout << "total elapsed time : " << (end - start) << " seconds\n";
	return 0;
}

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

static void usage(char **argv)
{
	printf("usage: %s [options] digits\n", argv[0]);
	printf("Options:\n");
	printf("  -l          -- use limit algorithm; default is series\n");
	printf("  -r          -- use Newton/Raphson algorithm; default is series\n");
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

//
// Evaluate e as the series 1/0! + 1/1! + 1/2! + 1/3! ....
// On input, 'eps' is the epsilon indicating the requested precision. 
// On output, e' contains the calculated value.
//
static void eViaSeries(
	mp_real eps,		// epsilon indicating requested precision
	mp_real &e)			// RETURNED
{
	mp_real lastE(0.0);
	
	// starting condition includes the first two terms
	mp_real invFact(1.0);		// 1/2!, 1/3!, etc.
	e = 2.0;
	int term = 2;
	
	for(;;) {
		invFact /= term;
		e += invFact;
		term++;
		
		// if e == lastE, within the requested precision, we're done
		if (abs(e - lastE) < eps) {
			break;
		}
		lastE = e;
	}
}

//
// Evaluate e as the limit, as n-->infinity, of 
// (1 + (1/n)) ^ n
//
static void eViaLimit(
	mp_real eps,		// epsilon indicating requested precision
	mp_real &e)			// RETURNED */
{
	mp_real lastE(0.0);
	
	// start at n=4, grow by x32 each loop
	mp_real n(4.0);
	mp_real oneOverN(0.25);
	
	for(;;) {
		mp_real b(oneOverN + 1.0);
		e = pow(b, n);
		
		// if e == lastE, within the requested precision, we're done
		if (abs(e - lastE) < eps) {
			break;
		}
		lastE = e;
		n *= 32.0;
		oneOverN /= 32.0;
	}
}

//
// Calculate e via Newton-Raphson method, which requires an MP log function. 
//
// By definition:
//
//		log(e) = 1.0
//
// Or, 
//		log(e) - 1.0 = 0;
//
// So we seek to find the zero of this function: 
//
//		f(x) = log(e) - 1.0
//
// The derivative of this function is 
//
//		f'(x) = 1 / x
//
// So for each iteration of Newton-Raphson, we do:
//
//		x[n+1] = x[n] - (f(x[n]) / f'(x[n]))
//			   = x[n] - (log(x[n]) - 1) / (1 / x[n])
//			   = x[n] - (x[n] * (log(x[n]) - 1))
//
static void eViaNewtonRaphson(
	mp_real eps,		// epsilon indicating requested precision
	mp_real &e)			// RETURNED
{
	mp_real lastE(2.7);		// initial condition 
	mp_real tmp;
	
	for(;;) {
		tmp = log(lastE) - 1.0;
		tmp *= lastE;
		e = lastE - tmp;
		
		// if e == lastE, within the requested precision, we're done
		if (abs(e - lastE) < eps) {
			break;
		}
		lastE = e;
	}
}

