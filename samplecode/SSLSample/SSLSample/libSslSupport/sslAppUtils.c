/*
	File:		sslAppUtils.c
        
	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                        ("Apple") in consideration of your agreement to the following terms, and your
                        use, installation, modification or redistribution of this Apple software
                        constitutes acceptance of these terms.  If you do not agree with these terms,
                        please do not use, install, modify or redistribute this Apple software.

                        In consideration of your agreement to abide by the following terms, and subject
                        to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
                        copyrights in this original Apple software (the "Apple Software"), to use,
                        reproduce, modify and redistribute the Apple Software, with or without
                        modifications, in source and/or binary forms; provided that if you redistribute
                        the Apple Software in its entirety and without modifications, you must retain
                        this notice and the following text and disclaimers in all such redistributions of
                        the Apple Software.  Neither the name, trademarks, service marks or logos of
                        Apple Computer, Inc. may be used to endorse or promote products derived from the
                        Apple Software without specific prior written permission from Apple.  Except as
                        expressly stated in this notice, no other rights or licenses, express or implied,
                        are granted by Apple herein, including but not limited to any patent rights that
                        may be infringed by your derivative works or by other works in which the Apple
                        Software may be incorporated.

                        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                        COMBINATION WITH YOUR PRODUCTS.

                        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
                11/4/02		1.0d1

*/


#include "sslAppUtils.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/MacErrors.h>

char *sslGetCipherSuiteString(SSLCipherSuite cs)
{
	static char noSuite[40];
	
	switch(cs) {
		case SSL_NULL_WITH_NULL_NULL:
			return "SSL_NULL_WITH_NULL_NULL";
		case SSL_RSA_WITH_NULL_MD5:
			return "SSL_RSA_WITH_NULL_MD5";
		case SSL_RSA_WITH_NULL_SHA:
			return "SSL_RSA_WITH_NULL_SHA";
		case SSL_RSA_EXPORT_WITH_RC4_40_MD5:
			return "SSL_RSA_EXPORT_WITH_RC4_40_MD5";
		case SSL_RSA_WITH_RC4_128_MD5:
			return "SSL_RSA_WITH_RC4_128_MD5";
		case SSL_RSA_WITH_RC4_128_SHA:
			return "SSL_RSA_WITH_RC4_128_SHA";
		case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5:
			return "SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5";
		case SSL_RSA_WITH_IDEA_CBC_SHA:
			return "SSL_RSA_WITH_IDEA_CBC_SHA";
		case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA:
			return "SSL_RSA_EXPORT_WITH_DES40_CBC_SHA";
		case SSL_RSA_WITH_DES_CBC_SHA:
			return "SSL_RSA_WITH_DES_CBC_SHA";
		case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
			return "SSL_RSA_WITH_3DES_EDE_CBC_SHA";
		case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA:
			return "SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA";
		case SSL_DH_DSS_WITH_DES_CBC_SHA:
			return "SSL_DH_DSS_WITH_DES_CBC_SHA";
		case SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA:
			return "SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA";
		case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA:
			return "SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA";
		case SSL_DH_RSA_WITH_DES_CBC_SHA:
			return "SSL_DH_RSA_WITH_DES_CBC_SHA";
		case SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA:
			return "SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA";
		case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA:
			return "SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA";
		case SSL_DHE_DSS_WITH_DES_CBC_SHA:
			return "SSL_DHE_DSS_WITH_DES_CBC_SHA";
		case SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
			return "SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA";
		case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA:
			return "SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA";
		case SSL_DHE_RSA_WITH_DES_CBC_SHA:
			return "SSL_DHE_RSA_WITH_DES_CBC_SHA";
		case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
			return "SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA";
		case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5:
			return "SSL_DH_anon_EXPORT_WITH_RC4_40_MD5";
		case SSL_DH_anon_WITH_RC4_128_MD5:
			return "SSL_DH_anon_WITH_RC4_128_MD5";
		case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA:
			return "SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA";
		case SSL_DH_anon_WITH_DES_CBC_SHA:
			return "SSL_DH_anon_WITH_DES_CBC_SHA";
		case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
			return "SSL_DH_anon_WITH_3DES_EDE_CBC_SHA";
		case SSL_FORTEZZA_DMS_WITH_NULL_SHA:
			return "SSL_FORTEZZA_DMS_WITH_NULL_SHA";
		case SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA:
			return "SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA";
		case SSL_RSA_WITH_RC2_CBC_MD5:
			return "SSL_RSA_WITH_RC2_CBC_MD5";
		case SSL_RSA_WITH_IDEA_CBC_MD5:
			return "SSL_RSA_WITH_IDEA_CBC_MD5";
		case SSL_RSA_WITH_DES_CBC_MD5:
			return "SSL_RSA_WITH_DES_CBC_MD5";
		case SSL_RSA_WITH_3DES_EDE_CBC_MD5:
			return "SSL_RSA_WITH_3DES_EDE_CBC_MD5";
		case SSL_NO_SUCH_CIPHERSUITE:
			return "SSL_NO_SUCH_CIPHERSUITE";
		default:
			sprintf(noSuite, "Unknown (%d)", (unsigned)cs);
			return noSuite;	
	}
}

/* 
 * Given a SSLProtocolVersion - typically from SSLGetProtocolVersion -
 * return a string representation.
 */
char *sslGetProtocolVersionString(SSLProtocol prot)
{
	static char noProt[20];
	
	switch(prot) {
		case kSSLProtocolUnknown:
			return "kSSLProtocolUnknown";
		case kSSLProtocol2:
			return "kSSLProtocol2";
		case kSSLProtocol3:
			return "kSSLProtocol3";
		case kSSLProtocol3Only:
			return "kSSLProtocol3Only";
		case kTLSProtocol1:
			return "kTLSProtocol1";
		case kTLSProtocol1Only:
			return "kTLSProtocol1Only";
		default:
			sprintf(noProt, "Unknown (%d)", (unsigned)prot);
			return noProt;	
	}
}

/* 
 * Return string representation of SecureTransport-related OSStatus.
 */
char *sslGetSSLErrString(OSStatus err)
{
	static char noErrStr[20];
	
	switch(err) {
		case noErr:
			return "noErr";
		case memFullErr:
			return "memFullErr";
		case paramErr:
			return "paramErr";
		case unimpErr:
			return "unimpErr";
		case errSSLProtocol:
			return "errSSLProtocol";
		case errSSLNegotiation:
			return "errSSLNegotiation";
		case errSSLFatalAlert:
			return "errSSLFatalAlert";
		case errSSLWouldBlock:
			return "errSSLWouldBlock";
		case ioErr:
			return "ioErr";
		case errSSLSessionNotFound:
			return "errSSLSessionNotFound";
		case errSSLClosedGraceful:
			return "errSSLClosedGraceful";
		case errSSLClosedAbort:
			return "errSSLClosedAbort";
   		case errSSLXCertChainInvalid:
			return "errSSLXCertChainInvalid";
		case errSSLBadCert:
			return "errSSLBadCert"; 
		case errSSLCrypto:
			return "errSSLCrypto";
		case errSSLInternal:
			return "errSSLInternal";
		case errSSLModuleAttach:
			return "errSSLModuleAttach";
		case errSSLUnknownRootCert:
			return "errSSLUnknownRootCert";
		case errSSLNoRootCert:
			return "errSSLNoRootCert";
		case errSSLCertExpired:
			return "errSSLCertExpired";
		case errSSLCertNotYetValid:
			return "errSSLCertNotYetValid";
		case badReqErr:
			return "badReqErr";
		case errSSLClosedNoNotify:
			return "errSSLClosedNoNotify";
		case errSSLBufferOverflow:
			return "errSSLBufferOverflow";
		case errSSLBadCipherSuite:
			return "errSSLBadCipherSuite";
		default:
			sprintf(noErrStr, "Unknown (%d)", (unsigned)err);
			return noErrStr;	
	}
}

void printSslErrStr(
	const char 	*op,
	OSStatus 	err)
{
	printf("*** %s: %s\n", op, sslGetSSLErrString(err));
}

/*
 * Convert a keychain name (which may be NULL) into the CFArrayRef required
 * by SSLSetCertificate. This is a bare-bones example of this operation,
 * since it requires and assumes that there is exactly one SecIdentity
 * in the keychain - i.e., there is exactly one matching cert/private key 
 * pair. A real world server would probably search a keychain for a SecIdentity 
 * matching some specific criteria. 
 * We also skip the operation of adding additional non-signing certs from the 
 * keychain to the CFArrayRef.
 */
#define KC_DB_PATH		"Library/Keychains"		/* relative to home */

CFArrayRef getSslCerts(
	const char 		*kcName,				// may be NULL, i.e., use default
	CSSM_BOOL		encryptOnly,
	SecKeychainRef	*pKcRef)				// RETURNED
{
	char 				kcPath[MAXPATHLEN + 1];
	UInt32 				kcPathLen = MAXPATHLEN + 1;
	SecKeychainRef 		kcRef = nil;
	OSStatus			ortn;
	
	/* pick a keychain */
	if(kcName) {
		char *userHome = getenv("HOME");
	
		if(userHome == NULL) {
			/* well, this is probably not going to work */
			userHome = "";
		}
		sprintf(kcPath, "%s/%s/%s", userHome, KC_DB_PATH, kcName);
	}
	else {
		/* use default keychain */
		ortn = SecKeychainCopyDefault(&kcRef);
		if(ortn) {
			printf("SecKeychainCopyDefault returned %d; aborting.\n", 
				(int)ortn);
			return nil;
		}
		ortn = SecKeychainGetPath(kcRef, &kcPathLen, kcPath);
		if(ortn) {
			printf("SecKeychainGetPath returned %d; aborting.\n", 
				(int)ortn);
			return nil;
		}
		
		/* 
		 * OK, we have a path, we have to release the first KC ref, 
		 * then get another one by opening it 
		 */
		CFRelease(kcRef);
	}
	ortn = SecKeychainOpen(kcPath, &kcRef);
	if(ortn) {
		printf("SecKeychainOpen returned %d.\n", 
			(int)ortn);
		printf("Cannot open keychain at %s. Aborting.\n", kcPath);
		return nil;
	}
	*pKcRef = kcRef;
	
	/* search for "any" identity matching specified key use; 
	 * in this app, we expect there to be exactly one. */
	 
	SecIdentitySearchRef srchRef = nil;
	ortn = SecIdentitySearchCreate(kcRef, 
		encryptOnly ? CSSM_KEYUSE_DECRYPT : CSSM_KEYUSE_SIGN,
		&srchRef);
	if(ortn) {
		printf("SecIdentitySearchCreate returned %d.\n", (int)ortn);
		printf("Cannot find signing key in keychain at %s. Aborting.\n", 
			kcPath);
		return nil;
	}
	SecIdentityRef identity = nil;
	ortn = SecIdentitySearchCopyNext(srchRef, &identity);
	if(ortn) {
		printf("SecIdentitySearchCopyNext returned %d.\n", (int)ortn);
		printf("Cannot find signing key in keychain at %s. Aborting.\n", 
			kcPath);
		return nil;
	}
	if(CFGetTypeID(identity) != SecIdentityGetTypeID()) {
		printf("SecIdentitySearchCopyNext CFTypeID failure!\n");
		return nil;
	}

	/* 
	 * Found one. Place it in a CFArray. 
	 * TBD: snag other (non-identity) certs from keychain and add them
	 * to array as well.
	 */
	CFArrayRef ca = CFArrayCreate(NULL,
		(const void **)&identity,
		1,
		NULL);
	if(ca == nil) {
		printf("CFArrayCreate error\n");
	}
	return ca;
}
