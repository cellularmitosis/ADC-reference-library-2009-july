/*
	File:		sslServer.cpp
        
        Contains:	SSL viewer tool, SecureTransport / OS X version.

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


#include <Security/SecureTransport.h>
#include <libSslSupport/sslAppUtils.h>
#include <libSslSupport/ioSock.h>
#include <libSslSupport/printCert.h>
#include <libSslSupport/fileIo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define DEFAULT_GETMSG  	"GET"
#define DEFAULT_PATH		"/"
#define DEFAULT_GET_SUFFIX	"HTTP/1.0\r\n\r\n"

#define DEFAULT_HOST   	  	"www.amazon.com"
#define DEFAULT_PORT     	443

#include <Security/SecCertificate.h>

static void usage(char **argv)
{
    printf("Usage: %s [hostname|-] [path] [option ...]\n", argv[0]);
    printf("       %s hostname [path] [option ...]\n", argv[0]);
	printf("Specifying '-' for hostname, or no args, uses default of %s.\n",
		DEFAULT_HOST);
	printf("Optional path argument must start with leading '/'.\n");
    printf("Options:\n");
    printf("   e           Allow Expired Certs\n");
    printf("   r           Allow any root cert\n");
	printf("   f fileBase  Write Peer Certs to fileBase*\n");
	printf("   c           Display peer certs\n");
	printf("   d           Display received data\n");
	printf("   C=cipherSuite (e=40-bit d=DES D=40-bit DES 3=3DES 4=RC4 $=40-bit RC4 "
				"2=RC2\n");
	printf("   2           SSLv2 only (default is all)\n");
	printf("   3           SSLv3 only (default is all)\n");
	printf("   t           TLSv1 only (default is all)\n");
	printf("   o           TLSv1, SSLv3 use kSSLProtocol__X__Only\n");
	printf("   u           kSSLProtocolUnknown only (default is SSLv2, SSLv3, TLSv1)\n");
	printf("   K           Keep connected until server disconnects\n");
	printf("   n           Require closure notify message in TLSv1, SSLv3 mode (implies k)\n");
	printf("   R           Disable resumable session support\n");
	printf("   v           Verify negotiated protocol equals attempted\n");
	printf("   m=[23t]     Max protocol supported as specified; implies v\n");
	printf("   H           allow hostname spoofing\n");
	printf("   k=keychain  Contains client cert and keys. Optional.\n");
	printf("   E=keychain  Encryption-only cert and keys. Optional.\n");
	printf("   l=loopCount Perform loopCount ops (default = 1)\n");
	printf("   P=port      Default = %d\n", DEFAULT_PORT); 
	printf("   p           Pause after each loop\n");
	printf("   q           Quiet/diagnostic mode (site names and errors only)\n");
	printf("   s           Silent\n");
	printf("   V           Verbose\n");
	printf("   h           Help\n");
    exit(1);
}

/* 
 * Arguments to top-level sslPing()
 */
typedef struct {
	SSLProtocol				tryVersion;
	const char				*hostName;			// e.g., "www.amazon.com"
	unsigned short			port;
	const char				*getMsg;			// e.g., 
												//   "GET / HTTP/1.0\r\n\r\n" 
	CSSM_BOOL				allowExpired;
	CSSM_BOOL				allowAnyRoot;
	CSSM_BOOL				dumpRxData;			// display server data
	char					cipherRestrict;		// '2', 'd'. etc...; '\0' for 
												//   no restriction
	CSSM_BOOL				keepConnected;
	CSSM_BOOL				requireNotify;		// require closure notify 
												//   in V3 mode
	CSSM_BOOL				resumableEnable;
	CSSM_BOOL				allowHostnameSpoof;
	CFArrayRef				clientCerts;		// optional
	CFArrayRef				encryptClientCerts;	// optional
	CSSM_BOOL				quiet;				// minimal stdout
	CSSM_BOOL				silent;				// no stdout
	CSSM_BOOL				verbose;
	SSLProtocol				negVersion;			// RETURNED
	SSLCipherSuite			negCipher;			// RETURNED
	CFArrayRef				peerCerts;			// mallocd & RETURNED
} sslPingArgs;

/* print a '.' every few seconds to keep UI alive while connecting */
static time_t lastTime = (time_t)0;
#define TIME_INTERVAL		3

static void outputDot()
{
	time_t thisTime = time(0);
	
	if((thisTime - lastTime) >= TIME_INTERVAL) {
		printf("."); fflush(stdout);
		lastTime = thisTime;
	}
}

/* 
 * Snag a copy of current connection's peer certs so we can 
 * examine them later after the connection is closed.
 * SecureTransport actually does the create and retain for us. 
 */
static OSStatus copyPeerCerts(
	SSLContext 	*ctx,
	CFArrayRef	*peerCerts)		// mallocd & RETURNED
{
	OSStatus ortn = SSLGetPeerCertificates(ctx, peerCerts);
	if(ortn) {
		printf("***Error obtaining peer certs: %s\n",
			sslGetSSLErrString(ortn));
	}
	return ortn;
}

/* free the cert array obtained via SSLGetPeerCertificates() */
static void	freePeerCerts(
	CFArrayRef			peerCerts)
{
	CFIndex numCerts;
	SecCertificateRef certData;
	CFIndex i;
	
	if(peerCerts == NULL) {
		return;
	}
	numCerts = CFArrayGetCount(peerCerts);
	for(i=0; i<numCerts; i++) {
		certData = (SecCertificateRef)CFArrayGetValueAtIndex(peerCerts, i);
		CFRelease(certData);
	}
	CFRelease(peerCerts);
}	

/* print reply received from server, safely */
static void dumpAscii(
	uint8 *rcvBuf, 
	uint32 len)
{
	char *cp = (char *)rcvBuf;
	uint32 i;
	char c;
	
	for(i=0; i<len; i++) {
		c = *cp++;
		if(c == '\0') {
			break;
		}
		switch(c) {
			case '\n':
				printf("\\n");
				break;
			case '\r':
				printf("\\r");
				break;
			default:
				if(isprint(c) && (c != '\n')) {
					printf("%c", c);
				}
				else {
					printf("<%02X>", ((unsigned)c) & 0xff);
				}
			break;
		}

	}
	printf("\n");
}

/*
 * Lists of SSLCipherSuites used in setCipherRestrictions. Note that the 
 * SecureTransport library does not implement all of these; we only specify
 * the ones it claims to support.
 */
static SSLCipherSuite suites40[] = {
	SSL_RSA_EXPORT_WITH_RC4_40_MD5,
	SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5,
	SSL_RSA_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DH_anon_EXPORT_WITH_RC4_40_MD5,
	SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA,
	SSL_NO_SUCH_CIPHERSUITE
};
static SSLCipherSuite suitesDES[] = {
	SSL_RSA_WITH_DES_CBC_SHA,
	SSL_DH_DSS_WITH_DES_CBC_SHA,
	SSL_DH_RSA_WITH_DES_CBC_SHA,
	SSL_DHE_DSS_WITH_DES_CBC_SHA,
	SSL_DHE_RSA_WITH_DES_CBC_SHA,
	SSL_DH_anon_WITH_DES_CBC_SHA,
	SSL_RSA_WITH_DES_CBC_MD5,
	SSL_NO_SUCH_CIPHERSUITE
};
static SSLCipherSuite suitesDES40[] = {
	SSL_RSA_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA,
	SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA,
	SSL_NO_SUCH_CIPHERSUITE
};
static SSLCipherSuite suites3DES[] = {
	SSL_RSA_WITH_3DES_EDE_CBC_SHA,
	SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA,
	SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA,
	SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA,
	SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA,
	SSL_DH_anon_WITH_3DES_EDE_CBC_SHA,
	SSL_RSA_WITH_3DES_EDE_CBC_MD5,
	SSL_NO_SUCH_CIPHERSUITE
};
static SSLCipherSuite suitesRC4[] = {
	SSL_RSA_WITH_RC4_128_MD5,
	SSL_RSA_WITH_RC4_128_SHA,
	SSL_DH_anon_WITH_RC4_128_MD5,
	SSL_NO_SUCH_CIPHERSUITE
};
static SSLCipherSuite suitesRC4_40[] = {
	SSL_RSA_EXPORT_WITH_RC4_40_MD5,
	SSL_DH_anon_EXPORT_WITH_RC4_40_MD5,
	SSL_NO_SUCH_CIPHERSUITE
};
static SSLCipherSuite suitesRC2[] = {
	SSL_RSA_WITH_RC2_CBC_MD5,
	SSL_NO_SUCH_CIPHERSUITE
};

/*
 * Given an SSLContextRef and an array of SSLCipherSuites, terminated by
 * SSL_NO_SUCH_CIPHERSUITE, select those SSLCipherSuites which the library
 * supports and do a SSLSetEnabledCiphers() specifying those. 
 */
static OSStatus setEnabledCiphers(
	SSLContextRef ctx,
	const SSLCipherSuite *ciphers)
{
	UInt32 numSupported;
	OSStatus ortn;
	SSLCipherSuite *supported = NULL;
	SSLCipherSuite *enabled = NULL;
	unsigned enabledDex = 0;	// index into enabled
	unsigned supportedDex = 0;	// index into supported
	unsigned inDex = 0;			// index into ciphers
	
	/* first get all the supported ciphers */
	ortn = SSLGetNumberSupportedCiphers(ctx, &numSupported);
	if(ortn) {
		printSslErrStr("SSLGetNumberSupportedCiphers", ortn);
		return ortn;
	}
	supported = (SSLCipherSuite *)malloc(numSupported * sizeof(SSLCipherSuite));
	ortn = SSLGetSupportedCiphers(ctx, supported, &numSupported);
	if(ortn) {
		printSslErrStr("SSLGetSupportedCiphers", ortn);
		return ortn;
	}
	
	/* 
	 * Malloc an array we'll use for SSLGetEnabledCiphers - this will  be
	 * bigger than the number of suites we actually specify 
	 */
	enabled = (SSLCipherSuite *)malloc(numSupported * sizeof(SSLCipherSuite));
	
	/* 
	 * For each valid suite in ciphers, see if it's in the list of 
	 * supported ciphers. If it is, add it to the list of ciphers to be
	 * enabled. 
	 */
	for(inDex=0; ciphers[inDex] != SSL_NO_SUCH_CIPHERSUITE; inDex++) {
		for(supportedDex=0; supportedDex<numSupported; supportedDex++) {
			if(ciphers[inDex] == supported[supportedDex]) {
				enabled[enabledDex++] = ciphers[inDex];
				break;
			}
		}
	}
	
	/* send it on down. */
	ortn = SSLSetEnabledCiphers(ctx, enabled, enabledDex);
	if(ortn) {
		printSslErrStr("SSLSetEnabledCiphers", ortn);
	}
	free(enabled);
	free(supported);
	return ortn;
}

/* bogus argv array for setCipherRestrictions */
static char *bogusArgv[2] = {"sslViewer", ""};

/*
 * Specify a restricted set of cipherspecs.
 */
static OSStatus setCipherRestrictions(
	SSLContextRef ctx,
	char cipherRestrict)
{
	OSStatus ortn;
	
	if(cipherRestrict == '\0') {
		return noErr;		// actually should not have been called 
	}
	switch(cipherRestrict) {
		case 'e':
			ortn = setEnabledCiphers(ctx, suites40);
			break;
		case 'd':
			ortn = setEnabledCiphers(ctx, suitesDES);
			break;
		case 'D':
			ortn = setEnabledCiphers(ctx, suitesDES40);
			break;
		case '3':
			ortn = setEnabledCiphers(ctx, suites3DES);
			break;
		case '4':
			ortn = setEnabledCiphers(ctx, suitesRC4);
			break;
		case '$':
			ortn = setEnabledCiphers(ctx, suitesRC4_40);
			break;
		case '2':
			ortn = setEnabledCiphers(ctx, suitesRC2);
			break;
		default:
			printf("***bad cipherSpec***\n");
			usage(bogusArgv);
	}
	return ortn;
}

/*
 * Perform one SSL diagnostic session. Returns nonzero on error. Normally no
 * output to stdout except initial "connecting to" message, unless there 
 * is a really screwed up error (i.e., something not directly related 
 * to the SSL connection). 
 */
#define RCV_BUF_SIZE		256

static OSStatus sslPing(
	sslPingArgs *pargs)
{
    PeerSpec            peerId;
	otSocket			sock = NULL;
    OSStatus            ortn;
    SSLContextRef       ctx = NULL;
    UInt32              length;
	UInt32				actLen;
    uint8               rcvBuf[RCV_BUF_SIZE];
	
    pargs->negVersion = kSSLProtocolUnknown;
    pargs->negCipher = SSL_NULL_WITH_NULL_NULL;
    pargs->peerCerts = NULL;
    
	/* first make sure requested server is there */
	ortn = MakeServerConnection(pargs->hostName, pargs->port, 
		&sock, &peerId);
    if(ortn) {
    	printf("MakeServerConnection returned %d; aborting\n", (int)ortn);
    	return ortn;
    }
	if(pargs->verbose) {
		printf("...connected to server; starting SecureTransport\n");
	}
	
	/* 
	 * Set up a SecureTransport session.
	 * First the standard calls.
	 */
	ortn = SSLNewContext(false, &ctx);
	if(ortn) {
		printSslErrStr("SSLNewContext", ortn);
		goto cleanup;
	} 
	ortn = SSLSetIOFuncs(ctx, SocketRead, SocketWrite);
	if(ortn) {
		printSslErrStr("SSLSetIOFuncs", ortn);
		goto cleanup;
	} 
	ortn = SSLSetProtocolVersion(ctx, pargs->tryVersion);
	if(ortn) {
		printSslErrStr("SSLSetProtocolVersion", ortn);
		goto cleanup;
	} 
	ortn = SSLSetConnection(ctx, sock);
	if(ortn) {
		printSslErrStr("SSLSetConnection", ortn);
		goto cleanup;
	}
	if(!pargs->allowHostnameSpoof) {
		/* if this isn't set, it isn't checked by APpleX509TP */
		ortn = SSLSetPeerDomainName(ctx, pargs->hostName, 
			strlen(pargs->hostName) + 1);
		if(ortn) {
			printSslErrStr("SSLSetPeerDomainName", ortn);
			goto cleanup;
		}
	}
	
	/* 
	 * SecureTransport options.
	 */ 
	if(pargs->resumableEnable) {
		const void *rtnId = NULL;
		size_t rtnIdLen = 0;
		
		ortn = SSLSetPeerID(ctx, &peerId, sizeof(PeerSpec));
		if(ortn) {
			printSslErrStr("SSLSetPeerID", ortn);
			goto cleanup;
		}
		/* quick test of the get fcn */
		ortn = SSLGetPeerID(ctx, &rtnId, &rtnIdLen);
		if(ortn) {
			printSslErrStr("SSLGetPeerID", ortn);
			goto cleanup;
		}
		if((rtnId == NULL) || (rtnIdLen != sizeof(PeerSpec))) {
			printf("***SSLGetPeerID screwup\n");
		}
		else if(memcmp(&peerId, rtnId, rtnIdLen) != 0) {
			printf("***SSLGetPeerID data mismatch\n");
		}
	}
	if(pargs->allowExpired) {
		ortn = SSLSetAllowsExpiredCerts(ctx, true);
		if(ortn) {
			printSslErrStr("SSLSetAllowExpiredCerts", ortn);
			goto cleanup;
		}
	}
	if(pargs->allowAnyRoot) {
		ortn = SSLSetAllowsAnyRoot(ctx, true);
		if(ortn) {
			printSslErrStr("SSLSetAllowAnyRoot", ortn);
			goto cleanup;
		}
	}
	if(pargs->cipherRestrict != '\0') {
		ortn = setCipherRestrictions(ctx, pargs->cipherRestrict);
		if(ortn) {
			goto cleanup;
		}
	}
	if(pargs->clientCerts) {
		ortn = SSLSetCertificate(ctx, pargs->clientCerts);
		if(ortn) {
			printSslErrStr("SSLSetCertificate", ortn);
			goto cleanup;
		}
	}
	if(pargs->encryptClientCerts) {
		ortn = SSLSetEncryptionCertificate(ctx, pargs->encryptClientCerts);
		if(ortn) {
			printSslErrStr("SSLSetEncryptionCertificate", ortn);
			goto cleanup;
		}
	}
	
	/*** end options ***/
	
	if(pargs->verbose) {
		printf("...starting SSL handshake\n");
	}
	
    do
    {   ortn = SSLHandshake(ctx);
	    if((ortn == errSSLWouldBlock) && !pargs->silent) {
	    	/* keep UI responsive */ 
	    	outputDot();
	    }
    } while (ortn == errSSLWouldBlock);
	
	/* this works even if handshake failed due to cert chain invalid */
	copyPeerCerts(ctx, &pargs->peerCerts);

	SSLGetNegotiatedCipher(ctx, &pargs->negCipher);
	SSLGetNegotiatedProtocolVersion(ctx, &pargs->negVersion);
	
    if(ortn) {
		if(!pargs->silent) {
			printf("\n");
		}
    	goto cleanup;
    }

	if(pargs->verbose) {
		printf("...SSL handshake complete\n");
	}
	length = strlen(pargs->getMsg);
	ortn = SSLWrite(ctx, pargs->getMsg, length, &actLen);

	/* 
	 * Try to snag RCV_BUF_SIZE bytes. Exit if (!keepConnected and we get any data
	 * at all), or (keepConnected and err != (none, wouldBlock)).
	 */
    while (1) {   
		actLen = 0;
		if(pargs->dumpRxData) {
			size_t avail = 0;
			
			ortn = SSLGetBufferedReadSize(ctx, &avail);
			if(ortn) {
				printf("***SSLGetBufferedReadSize error\n");
				break;
			}
			if(avail != 0) {
				printf("\n%d bytes available: ", (int)avail);
			}
		}
        ortn = SSLRead(ctx, rcvBuf, RCV_BUF_SIZE, &actLen);
        if((actLen == 0) && !pargs->silent) {
        	outputDot();
        }
        if (ortn == errSSLWouldBlock) {
			/* for this loop, these are identical */
            ortn = noErr;
        }
		if((actLen > 0) && pargs->dumpRxData) {
			dumpAscii(rcvBuf, actLen);
		}
		if(ortn != noErr) {
			/* connection closed by server or by error */
			break;
		}
		if(!pargs->keepConnected && (actLen > 0)) {
        	/* good enough, we connected */
        	break;
        }
    }
	if(!pargs->silent) {
		printf("\n");
	}
	
    /* convert normal "shutdown" into zero err rtn */
	if(ortn == errSSLClosedGraceful) {
		ortn = noErr;
	}
	if((ortn == errSSLClosedNoNotify) && !pargs->requireNotify) {
		/* relaxed disconnect rules */
		ortn = noErr;
	}
    if (ortn == noErr) {
        ortn = SSLClose(ctx);
	}
cleanup:
	if(sock) {
		endpointShutdown(sock);
	}
	if(ctx) {
	    SSLDisposeContext(ctx);  
	}    
	return ortn;
}

static void showPeerCerts(
	CFArrayRef			peerCerts,
	CSSM_BOOL			verbose)
{
	CFIndex numCerts;
	SecCertificateRef certRef;
	OSStatus ortn;
	CSSM_DATA certData;
	CFIndex i;
	
	if(peerCerts == NULL) {
		return;
	}
	numCerts = CFArrayGetCount(peerCerts);
	for(i=0; i<numCerts; i++) {
		certRef = (SecCertificateRef)CFArrayGetValueAtIndex(peerCerts, i);
		ortn = SecCertificateGetData(certRef, &certData);
		if(ortn) {
			printf("***SecCertificateGetData returned %d\n", (int)ortn);
			continue;
		}
		printf("\n================== Server Cert %d ===================\n\n", i);
		printCert(certData.Data, certData.Length, verbose);
		printf("\n=============== End of Server Cert %d ===============\n", i);
	}
}

static void writePeerCerts(
	CFArrayRef			peerCerts,
	const char			*fileBase)
{
	CFIndex numCerts;
	SecCertificateRef certRef;
	OSStatus ortn;
	CSSM_DATA certData;
	CFIndex i;
	char fileName[100];
	
	if(peerCerts == NULL) {
		return;
	}
	numCerts = CFArrayGetCount(peerCerts);
	for(i=0; i<numCerts; i++) {
		sprintf(fileName, "%s%02d.cer", fileBase, i);
		certRef = (SecCertificateRef)CFArrayGetValueAtIndex(peerCerts, i);
		ortn = SecCertificateGetData(certRef, &certData);
		if(ortn) {
			printf("***SecCertificateGetData returned %d\n", ortn);
			continue;
		}
		writeFile(fileName, certData.Data, certData.Length);
	}
	printf("...wrote %d certs to fileBase %s\n", numCerts, fileBase);
}

static void showSSLResult(
	SSLProtocol			tryVersion,
	OSStatus			err,
	SSLProtocol			negVersion,
	SSLCipherSuite		negCipher,
	CFArrayRef			peerCerts,
	CSSM_BOOL			displayPeerCerts,
	char				*fileBase)		// non-NULL: write certs to file
{
	CFIndex numPeerCerts;
	
	printf("\n");
	printf("   Attempted  SSL version : %s\n", 
		sslGetProtocolVersionString(tryVersion));
	printf("   Result                 : %s\n", sslGetSSLErrString(err));
	printf("   Negotiated SSL version : %s\n", 
		sslGetProtocolVersionString(negVersion));
	printf("   Negotiated CipherSuite : %s\n",
		sslGetCipherSuiteString(negCipher));
	if(peerCerts == NULL) {
		numPeerCerts = 0;
	}
	else {
		numPeerCerts = CFArrayGetCount(peerCerts);
	}
	printf("   Number of server certs : %d\n", numPeerCerts);
	if(numPeerCerts != 0) {
		if(displayPeerCerts) {
			showPeerCerts(peerCerts, CSSM_FALSE);
		}
		if(fileBase != NULL) {
			writePeerCerts(peerCerts, fileBase);
		}
	}
	printf("\n");
}

static int verifyProtocol(
	CSSM_BOOL	verifyProt,
	SSLProtocol	maxProtocol,
	SSLProtocol	reqProtocol,
	SSLProtocol negProtocol)
{
	if(!verifyProt) {
		return 0;
	}
	if(reqProtocol > maxProtocol) {
		/* known not to support this attempt, relax */
		reqProtocol = maxProtocol;
	}
	if(reqProtocol != negProtocol) {
		printf("***Expected protocol %s; negotiated %s\n",
			sslGetProtocolVersionString(reqProtocol),
			sslGetProtocolVersionString(negProtocol));
		return 1;
	}
	else {
		return 0;
	}
}

int main(int argc, char **argv)
{   
    OSStatus            err;
	int					arg;
	char 				*argp;
	char				getMsg[300];
	char				fullFileBase[100];
	int					ourRtn = 0;			// exit status - sum of all errors
	unsigned			loop;
	SecKeychainRef		serverKc = nil;
	SecKeychainRef		encryptKc = nil;
	sslPingArgs			pargs;
	
	/* user-spec'd parameters */
	char				*getPath = DEFAULT_PATH;
	char				*fileBase = NULL;
	CSSM_BOOL			displayCerts = CSSM_FALSE;
	CSSM_BOOL			doSslV2 = CSSM_TRUE;
	CSSM_BOOL			doSslV3 = CSSM_TRUE;
	CSSM_BOOL			doTlsV1 = CSSM_TRUE;
	CSSM_BOOL			protXOnly = CSSM_FALSE;	// kSSLProtocol3Only, kTLSProtocol1Only
	CSSM_BOOL			doProtUnknown = CSSM_FALSE;
	unsigned			loopCount = 1;
	CSSM_BOOL			doPause = CSSM_FALSE;
	CSSM_BOOL			verifyProt = CSSM_FALSE;
	SSLProtocol			maxProtocol = kTLSProtocol1;
	char				*keyChainName = NULL;
	char				*encryptKeyChainName = NULL;
	
	/* special case - one arg of "h" or "-h" */
	if( (argc == 2) && 
	    ( (strcmp(argv[1], "h") == 0) || (strcmp(argv[1], "-h") == 0))) {
		usage(argv);
	}
	
	/* set up defaults */
	memset(&pargs, 0, sizeof(sslPingArgs));
	pargs.hostName = DEFAULT_HOST;
	pargs.port = DEFAULT_PORT;
	pargs.resumableEnable = CSSM_TRUE;

	for(arg=1; arg<argc; arg++) {
		argp = argv[arg];
		if(arg == 1) {
			/* first arg, is always hostname; '-' means default */
			if(argp[0] != '-') {
				pargs.hostName = argp;
			}
			continue;
		}
		if(argp[0] == '/') {
			/* path always starts with leading slash */
			getPath = argp;
			continue;
		}
		/* options */
		switch(argp[0]) {
			case 'e':
				pargs.allowExpired = CSSM_TRUE;
				break;
			case 'r':
				pargs.allowAnyRoot = CSSM_TRUE;
				break;
			case 'd':
				pargs.dumpRxData = CSSM_TRUE;
				break;
			case 'c':
				displayCerts = CSSM_TRUE;
				break;
			case 'f':
				if(++arg == argc)  {
					/* requires another arg */
					usage(argv);
				}
				fileBase = argp;
				break;
			case 'C':
				pargs.cipherRestrict = argp[2];
				break;
			case '2':
				doSslV3 = doTlsV1 = CSSM_FALSE;
				break;
			case '3':
				doSslV2 = doTlsV1 = CSSM_FALSE;
				break;
			case 't':
				doSslV2 = doSslV3 = CSSM_FALSE;
				break;
			case 'o':
				protXOnly = CSSM_TRUE;
				break;
			case 'u':
				doSslV2 = doSslV3 = doTlsV1 = CSSM_FALSE;
				doProtUnknown = CSSM_TRUE;
				break;
			case 'K':
				pargs.keepConnected = CSSM_TRUE;
				break;
			case 'n':
				pargs.requireNotify = CSSM_TRUE;
				pargs.keepConnected = CSSM_TRUE;
				break;
			case 'R':
				pargs.resumableEnable = CSSM_FALSE;
				break;
			case 'v':
				verifyProt = CSSM_TRUE;
				break;
			case 'm':
				verifyProt = CSSM_TRUE;		// implied
				switch(argp[2]) {
					case '2':
						maxProtocol = kSSLProtocol2;
						break;
					case '3':
						maxProtocol = kSSLProtocol3;
						break;
					case 't':
						maxProtocol = kTLSProtocol1;
						break;
					default:
						usage(argv);
				}
				break;
			case 'l':
				loopCount = atoi(&argp[2]);
				if(loopCount == 0) {
					printf("***bad loopCount\n");
					usage(argv);
				}
				break;
			case 'P':
				pargs.port = atoi(&argp[2]);
				break;
			case 'H':
				pargs.allowHostnameSpoof = CSSM_TRUE;
				break;
			case 'k':
				keyChainName = &argp[2];
				break;
			case 'E':
				encryptKeyChainName = &argp[2];
				break;
			case 'p':
				doPause = CSSM_TRUE;
				break;
			case 'q':
				pargs.quiet = CSSM_TRUE;
				break;
			case 'V':
				pargs.verbose = CSSM_TRUE;
				break;
			case 's':
				pargs.silent = pargs.quiet = CSSM_TRUE;
				break;
			default:
				usage(argv);
		}
	}

	sprintf(getMsg, "%s %s %s", 
		DEFAULT_GETMSG, getPath, DEFAULT_GET_SUFFIX);
	pargs.getMsg = getMsg;
	if(fileBase) {
		sprintf(fullFileBase, "%s_v3", fileBase);
	}
			
	/* get server cert and optional encryption cert as CFArrayRef */
	if(keyChainName) {
		pargs.clientCerts = getSslCerts(keyChainName, CSSM_FALSE, &serverKc);
		if(pargs.clientCerts == nil) {
			exit(1);
		}
	}
	if(encryptKeyChainName) {
		pargs.encryptClientCerts = getSslCerts(encryptKeyChainName, CSSM_TRUE, 
				&encryptKc);
		if(pargs.encryptClientCerts == nil) {
			exit(1);
		}
	}

	for(loop=0; loop<loopCount; loop++) {
		/* 
		 * One pass for each protocol version, skipping any explicit version if
		 * an attempt at a higher version and succeeded in doing so successfully fell
		 * back.
		 */
		if(doTlsV1) {
			pargs.tryVersion = 
				protXOnly ? kTLSProtocol1Only : kTLSProtocol1;
			if(!pargs.silent) {
				printf("Connecting to host %s with TLS V1...", pargs.hostName); 
			}
			fflush(stdout);
			err = sslPing(&pargs);
			if(err) {
				ourRtn++;
			}
			if(!pargs.quiet) {
				showSSLResult(pargs.tryVersion,
					err, 
					pargs.negVersion, 
					pargs.negCipher, 
					pargs.peerCerts,
					displayCerts,
					fileBase ? fullFileBase : NULL);
			}
			freePeerCerts(pargs.peerCerts);
			if(!err) {
				/* deal with fallbacks, skipping redundant tests */
				switch(pargs.negVersion) {
					case kSSLProtocol3:
						doSslV3 = CSSM_FALSE;
						break;
					case kSSLProtocol2:
						doSslV3 = CSSM_FALSE;
						doSslV2 = CSSM_FALSE;
						break;
					default:
						break;
				}
				ourRtn += verifyProtocol(verifyProt, maxProtocol, kTLSProtocol1,
					pargs.negVersion);
			}
		}
		if(doSslV3) {
			pargs.tryVersion = protXOnly ? kSSLProtocol3Only : kSSLProtocol3;
			if(!pargs.silent) {
				printf("Connecting to host %s with SSL V3...", pargs.hostName); 
			}
			fflush(stdout);
			err = sslPing(&pargs);
			if(err) {
				ourRtn++;
			}
			if(!pargs.quiet) {
				showSSLResult(pargs.tryVersion,
					err, 
					pargs.negVersion, 
					pargs.negCipher, 
					pargs.peerCerts,
					displayCerts,
					fileBase ? fullFileBase : NULL);
			}
			freePeerCerts(pargs.peerCerts);
			if(!err) {
				/* deal with fallbacks, skipping redundant tests */
				switch(pargs.negVersion) {
					case kSSLProtocol2:
						doSslV2 = CSSM_FALSE;
						break;
					default:
						break;
				}
				ourRtn += verifyProtocol(verifyProt, maxProtocol, kSSLProtocol3,
					pargs.negVersion);
			}
		}
		
		if(doSslV2) {
			if(fileBase) {
				sprintf(fullFileBase, "%s_v2", fileBase);
			}
			if(!pargs.silent) {
				printf("Connecting to host %s with SSL V2...", pargs.hostName);
			}
			fflush(stdout);
			pargs.tryVersion = kSSLProtocol2;
			err = sslPing(&pargs);
			if(err) {
				ourRtn++;
			}
			if(!pargs.quiet) {
				showSSLResult(kSSLProtocol2,
					err, 
					pargs.negVersion, 
					pargs.negCipher, 
					pargs.peerCerts,
					displayCerts,
					fileBase ? fullFileBase : NULL);
			}
			freePeerCerts(pargs.peerCerts);	
			if(!err) {
				ourRtn += verifyProtocol(verifyProt, maxProtocol, kSSLProtocol2,
					pargs.negVersion);
			}
		}
		if(doProtUnknown) {
			if(!pargs.silent) {
				printf("Connecting to host %s with kSSLProtocolUnknown...", 
					pargs.hostName); 
			}
			fflush(stdout);
			pargs.tryVersion = kSSLProtocolUnknown;
			err = sslPing(&pargs);
			if(err) {
				ourRtn++;
			}
			if(!pargs.quiet) {
				showSSLResult(kSSLProtocolUnknown,
					err, 
					pargs.negVersion, 
					pargs.negCipher, 
					pargs.peerCerts,
					displayCerts,
					fileBase ? fullFileBase : NULL);
			}
			freePeerCerts(pargs.peerCerts);
		}
		if(doPause) {
			char resp;
			fpurge(stdin);
			printf("a to abort, c to continue: ");
			resp = getchar();
			if(resp == 'a') {
				break;
			}
		}
    }	/* main loop */
	parseCertShutdown();
	if(ourRtn) {
		printf("===%s exiting with %d %s for host %s\n", argv[0], ourRtn, 
			(ourRtn > 1) ? "errors" : "error", pargs.hostName);
	}
    return ourRtn;

}


