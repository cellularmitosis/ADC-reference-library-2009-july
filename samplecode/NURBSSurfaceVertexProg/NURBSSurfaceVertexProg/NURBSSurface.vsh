!!ARBvp1.0

#   NURBSurface.vsh
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

ATTRIB	uBasis		= vertex.position;
ATTRIB	vBasis		= vertex.color;
ATTRIB 	vTexcoord	= vertex.texcoord;

PARAM	negOne	= {-1.000000000000, -1.000000000000, -1.000000000000, -1.000000000000};
PARAM	zero	= {0.0000000000000, 0.0000000000000, 0.0000000000000, 1.0000000000000};
PARAM	one	= {1.0000000000000, 1.0000000000000, 1.0000000000000, 1.0000000000000};
PARAM	ptOne	= {0.1000000000000, 0.1000000000000, 0.1000000000000, 0.1000000000000};
PARAM	ptTwo	= {0.2000000000000, 0.2000000000000, 0.2000000000000, 0.2000000000000};
PARAM	ptFive	= {0.5000000000000, 0.5000000000000, 0.5000000000000, 0.5000000000000};

# Position Control grid
PARAM	P00		= program.local[0];
PARAM	P01		= program.local[1];
PARAM	P02		= program.local[2];
PARAM	P03		= program.local[3];
PARAM	P10		= program.local[4];
PARAM	P11		= program.local[5];
PARAM	P12		= program.local[6];
PARAM	P13		= program.local[7];
PARAM	P20		= program.local[8];
PARAM	P21		= program.local[9];
PARAM	P22		= program.local[10];
PARAM	P23		= program.local[11];
PARAM	P30		= program.local[12];
PARAM	P31		= program.local[13];
PARAM	P32		= program.local[14];
PARAM	P33		= program.local[15];

TEMP	P;
TEMP	M0, M1, M2, M3;

###########################################################################################
# Row 0
MUL	M0, uBasis.xxxx, vBasis.xxxx;	# Create 4 column basis funcitons from K[0]J[0..3]
MUL	M1, uBasis.yyyy, vBasis.xxxx;
MUL	M2, uBasis.zzzz, vBasis.xxxx;
MUL	M3, uBasis.wwww, vBasis.xxxx;

MUL	P, M0, P00;			# Start evaluating P from control points and basis functions
MAD	P, M1, P01, P;			# Multiply and accumulate as we go
MAD	P, M2, P02, P;
MAD	P, M3, P03, P;

# Row 1
MUL	M0, uBasis.xxxx, vBasis.yyyy;	# New row, notice change in vBasis to vBasis[1]
MUL	M1, uBasis.yyyy, vBasis.yyyy;
MUL	M2, uBasis.zzzz, vBasis.yyyy;
MUL	M3, uBasis.wwww, vBasis.yyyy;

MAD	P, M0, P10, P;
MAD	P, M1, P11, P;
MAD	P, M2, P12, P;
MAD	P, M3, P13, P;

# Row 2
MUL	M0, uBasis.xxxx, vBasis.zzzz;
MUL	M1, uBasis.yyyy, vBasis.zzzz;
MUL	M2, uBasis.zzzz, vBasis.zzzz;
MUL	M3, uBasis.wwww, vBasis.zzzz;

MAD	P, M0, P20, P;
MAD	P, M1, P21, P;
MAD	P, M2, P22, P;
MAD	P, M3, P23, P;

# Row 3
MUL	M0, uBasis.xxxx, vBasis.wwww;
MUL	M1, uBasis.yyyy, vBasis.wwww;
MUL	M2, uBasis.zzzz, vBasis.wwww;
MUL	M3, uBasis.wwww, vBasis.wwww;

MAD	P, M0, P30, P;
MAD	P, M1, P31, P;
MAD	P, M2, P32, P;
MAD	P, M3, P33, P;

#SWZ	P, P, x,y,z,1;

# Transform the vertex by the modelview/projection matrix
DP4    result.position.x, state.matrix.mvp.row[0], P;
DP4    result.position.y, state.matrix.mvp.row[1], P;
DP4    result.position.z, state.matrix.mvp.row[2], P;
DP4    result.position.w, state.matrix.mvp.row[3], P;

MOV    result.texcoord, vTexcoord;

END

