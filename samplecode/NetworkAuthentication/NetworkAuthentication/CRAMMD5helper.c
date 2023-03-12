/*
 
 File: CRAMMD5helper.c
 
 Abstract: Helper code for demonstration applications only
 
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */

#include "CRAMMD5helper.h"
#include <CommonCrypto/CommonDigest.h>
#include <strings.h>

void CalcMD5( const char *inChallenge, long inChallengeLen, const char *inPassword, long inPasswordLen, 
			  unsigned char *outDigest )
{
	CC_MD5_CTX		md5Context;
	unsigned char	pInnerPad[65]	= { 0, };
	unsigned char	pOuterPad[65]	= { 0, };
	int				i;
	unsigned char	pTempKey[16];

	// if key is longer than 64 bytes reset it to key=MD5(key)
	if (inPasswordLen > 64) {
		CC_MD5_CTX      tempContext;
		
		CC_MD5_Init( &tempContext );
		CC_MD5_Update( &tempContext, inPassword, inPasswordLen );
		CC_MD5_Final( pTempKey, &tempContext );
		
		inPassword = pTempKey;
		inPasswordLen = 16;
	}
	
	bzero( pInnerPad, sizeof(pInnerPad) );
	bzero( pOuterPad, sizeof(pOuterPad) );
	bcopy( inPassword, pInnerPad, inPasswordLen );
	bcopy( inPassword, pOuterPad, inPasswordLen );
	
	// XOR key with ipad and opad values
	for (i=0; i<64; i++) {
		pInnerPad[i] ^= 0x36;
		pOuterPad[i] ^= 0x5c;
	}

	// perform the inner MD5
	CC_MD5_Init( &md5Context );
	CC_MD5_Update( &md5Context, pInnerPad, 64 );
	CC_MD5_Update( &md5Context, inChallenge, inChallengeLen );
	CC_MD5_Final( outDigest, &md5Context );

	// perform the outer MD5
	CC_MD5_Init( &md5Context );
	CC_MD5_Update( &md5Context, pOuterPad, 64 );
	CC_MD5_Update( &md5Context, outDigest, 16 );
	CC_MD5_Final( outDigest, &md5Context );
}