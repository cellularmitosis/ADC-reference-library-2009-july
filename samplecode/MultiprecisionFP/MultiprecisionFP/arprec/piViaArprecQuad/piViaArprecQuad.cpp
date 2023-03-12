//
//
//	File:		<piViaArprecQuad.cpp>
//
//	Abstract:	Calculate pi via the Gauss-Legendre algorithm, using the ARPREC Quadrature package. 
//
//				This is a very basic example of the use of all three Quadrature classes in the 
//				ARPREC package written by David Bailey et. al. at Lawrence Berkeley Labs. The 
//				Quadrature classes provide three different algorithms by which a definite integral 
//				of an arbitrary single-variable function f(x) is calculated.
//
//				This example calculates the value of pi by integrating the function
//
//					y = square root of (1 - x**2)
//
//				...over 0 <= x <= 1; this describes a quarter circle with radius 1.0. Thus
//				pi is 4.0 times that integral. 
//
//				This is an extremely inefficient way to calculate pi, but it serves as a
//				straightforward example of the use of the three Quadrature classes. 
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
#include "quad-erf.h"
#include "quad-gs.h"
#include "quad-ts.h"
#include "util.h"

// ****************************************************
#pragma mark - 
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------

// Denote one of three quadrature algorithms
typedef enum 
{
    QT_Gaussian,
    QT_ErrFunc,
    QT_TanhSinh
} QuadType;

// Defaults
#define LEVEL_DEF				10
#define LEVEL_GAUSS_DEF			9
#define TABLE_SIZE_DEF			13000
#define TABLE_SIZE_LARGE		20000
#define PRIMARY_PRECISION_DEF	400
#define SECOND_PRECISION_DEF	800
#define DEBUG_LEVEL_DEF			2

typedef mp_real (QuadFcn)(const mp_real &x);

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static void usage(char **argv);
static double currentTime(void); 
static mp_real circleFunc(const mp_real &x);
static void doIntegrate(	QuadFcn quadFcn, int x1, int x2, QuadType quadType, 
							int primaryPrecision, int secondaryPrecision, mp_real &result );
// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

int main(int argc, char **argv)
{
	double start, end;
	
    QuadType quadType = QT_Gaussian;
    int levels = 0;
    int primaryPrecision = PRIMARY_PRECISION_DEF;
    int secondaryPrecision = SECOND_PRECISION_DEF;
    
    // gather command line options
    extern char *optarg;
    int arg;
    while ((arg = getopt(argc, argv, "getp:h")) != -1) {
		switch (arg) {
			case 'g':
				quadType = QT_Gaussian;
				break;
			case 'e':
				quadType = QT_ErrFunc;
				break;
			case 't':
				quadType = QT_TanhSinh;
				break;
			case 'p':
				primaryPrecision = atoi(optarg);
				secondaryPrecision = primaryPrecision * 2;
				break;
			default:
			case 'h':
				usage(argv);
		}
    }
    if(optind != argc) {
	    usage(argv);
    }
	
	start = currentTime();
	
    mp::mp_init(secondaryPrecision);
    mp::mpsetoutputprec(primaryPrecision);
	
	mp_real pi;
	doIntegrate(circleFunc, 0, 1,
				quadType, primaryPrecision, secondaryPrecision,
				pi);
	
	end = currentTime();
	
	std::cout << "\npi = " << (pi * 4.0) << "\n";
	std::cout << "total elapsed time : " << (end - start) << " seconds\n";
	return 0;
}

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

static void usage(char **argv)
{
    printf("Usage: %s [options]\n", argv[0]);
    printf("Options:\n");
    printf("  -g           -- Gaussian quadrature (default)\n");
    printf("  -e           -- Error-function quadrature\n");
    printf("  -t           -- tanh-sinh quadrature\n");
    printf("  -p prec      -- primary precision; default %d\n", PRIMARY_PRECISION_DEF);
	exit(1);
}

// obtain high-precision real time as a double
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
// A function to integrate.
//
typedef mp_real (QuadFcn)(const mp_real &x);

//
// The function we integrate, a pointer to which is passed to the 
// ArprecIntegrate::integrate() function. 
//
// Given an x coordinate, 0 <= x <= 1.0, the function returns the 
// associated y coordinate on a circle with radius 1. 
//
static mp_real circleFunc(const mp_real &x)
{
    return sqrt(1.0 - (x * x));
}

// The core integration routine. 
static void doIntegrate(
	// specify function and endpoints
	QuadFcn			quadFcn,
	int				x1,
	int				x2,
	
	// quadrature parameters
	QuadType		quadType,			// QT_Gaussian, etc.
	int				primaryPrecision,
	int				secondaryPrecision,
	
	// result written here
	mp_real			&result)
{
    ArprecIntegrate<mp_real> *integrator;
	int tableSize = TABLE_SIZE_DEF;
	
	if(primaryPrecision > 800) {
		tableSize = TABLE_SIZE_LARGE;
	}
	switch(quadType) {
		case QT_Gaussian:
			integrator = new QuadGS(LEVEL_GAUSS_DEF, tableSize, 
				-primaryPrecision, -primaryPrecision,
				DEBUG_LEVEL_DEF);
			break;
		case QT_ErrFunc:
			integrator = new QuadErf(LEVEL_DEF, tableSize, 
				-primaryPrecision, -secondaryPrecision,
				DEBUG_LEVEL_DEF);
			break;
		case QT_TanhSinh:
			integrator = new QuadTS(LEVEL_DEF, tableSize, 
				-primaryPrecision, -secondaryPrecision,
				DEBUG_LEVEL_DEF);
			break;
	}
	
    int nwords1 = integrator->getPrecWd1();
    int nwords2 = integrator->getPrecWd2();

	// Create endpoints with precision getPrecWd2()
    mp::mpsetprecwords(nwords2);
    mp_real x1Real((double)x1);
	mp_real x2Real((double)x2);
	
	// Create result with precision getPrecWd1()
    mp::mpsetprecwords(nwords1);
    mp_real calcVal(0.0);
	
    calcVal = integrator->integrate(quadFcn, x1Real, x2Real);
	result = calcVal;
}

