!!ARBvp1.0

ATTRIB 	vPos  		= vertex.position;
ATTRIB 	vCol  		= vertex.color;
ATTRIB 	vTexcoord	= vertex.texcoord;

# Input parameter from application
PARAM	ABCD		= program.env[0];
PARAM	EQ0123		= program.env[1];
PARAM	EQ4567		= program.env[2];

# Constants
PARAM 	negOne			= {-1.00000000000000,   -1.00000000000000,     -1.00000000000000,       -1.00000000000000};
PARAM 	oneTenth		= { 0.10000000000000,    0.10000000000000,  	0.10000000000000,        0.10000000000000};
PARAM 	oneQuarter		= { 0.25000000000000,    0.25000000000000,  	0.25000000000000,        0.25000000000000};
PARAM 	oneHalf			= { 0.50000000000000,    0.50000000000000,  	0.50000000000000,        0.50000000000000};
PARAM 	threeQuarter	= { 0.75000000000000,    0.75000000000000,  	0.75000000000000,        0.75000000000000};

PARAM 	fact_1		= {1.00000000000000,	-1.00000000000000,	1.000000000000000,	-1.000000000000000};
PARAM 	fact_2		= {2.00000000000000,	-2.00000000000000,	5.000000000000e-1,	-5.000000000000e-1};
PARAM 	fact_3		= {6.00000000000000,	-6.00000000000000,	1.666666666667e-1,	-1.666666666667e-1};
PARAM 	fact_4		= {2.400000000000e1,	-2.400000000000e1,	4.166666666667e-2,	-4.166666666667e-2};
PARAM 	fact_5		= {1.200000000000e2,	-1.200000000000e2,	8.333333333333e-3,	-8.333333333333e-3};
PARAM 	fact_6		= {7.200000000000e2,	-7.200000000000e2,	1.388888888889e-3,	-1.388888888889e-3};
PARAM 	fact_7		= {5.040000000000e3,	-5.040000000000e3,	1.984126984127e-4,	-1.984126984127e-4};
PARAM 	fact_8		= {4.032000000000e4,	-4.032000000000e4,	2.480158730159e-5,	-2.480158730159e-5};
PARAM 	fact_9	 	= {3.628800000000e5,	-3.628800000000e5,	2.755731922399e-6,	-2.755731922399e-6};
PARAM 	fact_10		= {3.628800000000e6,	-3.628800000000e6,	2.755731922399e-7,	-2.755731922399e-7};
PARAM 	fact_11	 	= {3.991680000000e7,	-3.991680000000e7,	2.505210838544e-8,	-2.505210838544e-8};
PARAM 	fact_12		= {4.790016000000e8,	-4.790016000000e8,	2.087675698787e-9,	-2.087675698787e-9};
PARAM 	fact_13		= {6.227020800000e9,	-6.227020800000e9,	1.605904383682e-10,	-1.605904383682e-10};
PARAM 	fact_14		= {8.717829120000e10,	-8.717829120000e10,	1.147074559773e-11,	-1.147074559773e-11};
PARAM 	fact_15		= {1.307674368000e12,	-1.307674368000e12,	7.647163731820e-13,	-7.647163731820e-13};
PARAM 	fact_16		= {2.092278988800e13,	-2.092278988800e13,	4.779477332387e-14,	-4.779477332387e-14};
PARAM 	fact_17		= {3.556874280960e14,	-3.556874280960e14,	2.811457254346e-15,	-2.811457254346e-15};
PARAM 	fact_18		= {6.402373705728e15,	-6.402373705728e15,	1.561920696859e-16,	-1.561920696859e-16};
PARAM 	fact_19		= {1.216451004088e17,	-1.216451004088e17,	8.220635246624e-18,	-8.220635246624e-18};
PARAM 	fact_20		= {2.432902008177e18,	-2.432902008177e18,	4.110317623312e-19,	-4.110317623312e-19};
PARAM 	fact_21		= {5.109094217171e19,	-5.109094217171e19,	1.957294106339e-20,	-1.957294106339e-20};
PARAM 	fact_22		= {1.124000727778e21,	-1.124000727778e21,	8.896791392451e-22,	-8.896791392451e-22};

# Bug fixes
PARAM	select			= { 1.00000000000000,	 0.00000000000000,	0.00000000000000,        0.00000000000000};
PARAM 	threeQuarterAlpha	= { 0.00000000000000,    0.00000000000000,  	0.00000000000000,        0.75000000000000};
		 

#
#   SurfaceVertexProgram.vsh
#
#  Created by Michael Larson on Tue Mar 11 2003.
#  Copyright (c) 2003 Apple Computer. All rights reserved.
#
# 
# Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
#   ("Apple") in consideration of your agreement to the following terms, and your
#   use, installation, modification or redistribution of this Apple software
#   constitutes acceptance of these terms.  If you do not agree with these terms,
#   please do not use, install, modify or redistribute this Apple software.
#
#   In consideration of your agreement to abide by the following terms, and subject
#   to these terms, Apple grants you a personal, non-exclusive license, under Apple's
#   copyrights in this original Apple software (the "Apple Software"), to use,
#   reproduce, modify and redistribute the Apple Software, with or without
#   modifications, in source and/or binary forms; provided that if you redistribute
#   the Apple Software in its entirety and without modifications, you must retain
#   this notice and the following text and disclaimers in all such redistributions of
#   the Apple Software.  Neither the name, trademarks, service marks or logos of
#   Apple Computer, Inc. may be used to endorse or promote products derived from the
#   Apple Software without specific prior written permission from Apple.  Except as
#   expressly stated in this notice, no other rights or licenses, express or implied,
#   are granted by Apple herein, including but not limited to any patent rights that
#   may be infringed by your derivative works or by other works in which the Apple
#   Software may be incorporated.
#
#   The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
#   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
#   WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#   PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
#   COMBINATION WITH YOUR PRODUCTS.
#
#   IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
#   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
#   OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
#   (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
#   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

# Tempory variables
TEMP 	tPos, aPos;
TEMP	temp0, temp1;

# surface is described by
#   x = sin(u);
#   y = sin(v);
#   z = sin((dd - aa * u - bb * v) / cc);

MUL	temp0, ABCD.xxxx, vPos.xxxx;	# aa * u
SUB	temp0, ABCD.wwww, temp0.xxxx;	# dd - (aa * u)
MUL	temp1, ABCD.yyyy, vPos.yyyy;	# bb * v
SUB	temp0, temp0.xxxx, temp1.xxxx;	# dd - (aa * u) - (bb * v)
RCP	temp1, ABCD.z;
MUL	temp0, temp0, temp1;
SWZ	temp0, temp0, 0,0,z,0;
SWZ	aPos,  vPos,  x,y,0,1;
ADD	aPos, aPos, temp0;		# aPos = u, v, ((dd - (aa * u) - (bb * v)) / cc)

# sin(x) = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 x^7 + 1/362880 * x^9......
MUL	temp0, aPos, aPos;			# temp0 = xyz^2

MUL	temp1, temp0, aPos;  			# temp1 = xyz^3		n=1
MAD	tPos, temp1, fact_3.wwww,  aPos;	# temp0 = x - 1/6 * x^3

MUL	temp1, temp1, temp0;  			# temp1 = xyz^5		n=2
MAD	tPos, temp1, fact_5.zzzz,  tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5

MUL	temp1, temp1, temp0;  			# temp1 = xyz^7		n=3
MAD	tPos, temp1, fact_7.wwww,  tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7

MUL	temp1, temp1, temp0;  			# temp1 = xyz^9		n=4
MAD	tPos, temp1, fact_9.zzzz,  tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9

MUL	temp1, temp1, temp0;  			# temp1 = xyz^11	n=5
MAD	tPos, temp1, fact_11.wwww, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9....

MUL	temp1, temp1, temp0;  			# temp1 = xyz^13	n=6
MAD	tPos, temp1, fact_13.zzzz, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9....

MUL	temp1, temp1, temp0;  			# temp1 = xyz^15	n=7
MAD	tPos, temp1, fact_15.wwww, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9....

MUL	temp1, temp1, temp0;  			# temp1 = xyz^17	n=8
MAD	tPos, temp1, fact_17.zzzz, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9.......

MUL	temp1, temp1, temp0;  			# temp1 = xyz^19	n=9
MAD	tPos, temp1, fact_19.wwww, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9.......

MUL	temp1, temp1, temp0;  			# temp1 = xyz^21	n=10
MAD	tPos, temp1, fact_21.zzzz, tPos;	# temp0 = x - 1/6 * x^3 + 1/120 * x^5 - 1/5040 * x^7 + 1/362880 * x^9....... = sin(x)

# Set the w to 1
SWZ	temp0, tPos, x,y,z,1;

# Transform the vertex by the modelview/projection matrix
DP4 result.position.x, state.matrix.mvp.row[0], temp0;
DP4 result.position.y, state.matrix.mvp.row[1], temp0;
DP4 result.position.z, state.matrix.mvp.row[2], temp0;
DP4 result.position.w, state.matrix.mvp.row[3], temp0;

# Set Texture coordinate
MUL temp0, EQ0123, select;
ADD temp0, temp0, vTexcoord;
MOV result.texcoord, temp0;

#SWZ result.color, threeQuarter, 1,1,1,w;
MOV result.color, threeQuarterAlpha;

END
