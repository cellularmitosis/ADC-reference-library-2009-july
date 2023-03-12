/*
	File:		sslServer.cpp
        
        Contains:	Trivial SSL server example, SecureTransport / OS X version.

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
#include <sys/param.h>

#include <Security/Security.h>

#define AUTHENTICATE_ENABLE		0	/* until this is available in TOT */

/*
 * Defaults, overridable by user. 
 */
#define SERVER_MESSAGE  "HTTP/1.0 200 OK\015\012\015\012" \
	"<HTML><HEAD><TITLE>SecureTransport Test Server</TITLE></HEAD>" \
	"<BODY><H2>Secure connection established.</H2>" \
	"Message from the 'sslServer' sample application.\015\012</BODY>" \
	"</HTML>\015\012"

/* For ease of debugging, pick a non-privileged port */
#define DEFAULT_PORT     1200
// #define DEFAULT_PORT     443

#define DEFAULT_HOST	"localhost"

#define DEFAULT_KC		"certkc"

/*
 * IE (5.0 on OS9, 5.1.3 on OS X) have an interesting way of handling unrecognized
 * server certs. Upon receipt of the server cert msg with an unrecognized cert,
 * IE immediately closes the connection, asks the user of they want to proceed
 * after the usual dire warning, and retries the whole op from scratch if 
 * so authorized. Unfortunately this is often seen at this end as a broken 
 * pipe when writing either the server cert or the server hello done msg which follows
 * it. We really have to handle the sigpipe so the lower-level I/O code 
 * (in ioSock.c) can see the error on the write and cleanup. If we don't handle
 * the signal our process dies.
 */
#define IGNORE_SIGPIPE	1
#if 	IGNORE_SIGPIPE
#include <signal.h>

void sigpipe(int sig) 
{ 
	fflush(stdin);
	printf("***SIGPIPE***\n");
}
#endif

static void usage(char **argv)
{
    printf("Usage: %s [option ...]\n", argv[0]);
    printf("Options:\n");
	printf("   H=hostname  Set host name for Cert name match; defulat is %s\n",
		DEFAULT_HOST);
	printf("   P=port      Port to listen on; default is %d\n", DEFAULT_PORT);
	printf("   k=keychain  Contains server cert and keys. Default is default KC.\n");
	printf("   E=keychain  Encryption-only cert and keys. Optional, no default.\n");
    printf("   e           Allow Expired Certs\n");
    printf("   r           Allow any root cert\n");
	printf("   f=fileBase  Write Peer Certs to fileBase*\n");
	printf("   c           Display peer certs\n");
	printf("   d           Display received data\n");
	printf("   C=cipherSuite (e=40-bit d=DES D=40-bit DES 3=3DES 4=RC4 $=40-bit RC4 "
				"2=RC2\n");
	printf("   2           SSLv2 only (default is best fit)\n");
	printf("   3           SSLv3 only (default is best fit)\n");
	printf("   t           TLSv1 only (default is best fit)\n");
	printf("   o           TLSv1, SSLv3 use kSSLProtocol__X__Only\n");
	printf("   R           Disable resumable session support\n");
	printf("   a=[nat]     Authentication: n=never; a=always; t=try\n");
	printf("   p           Pause after each phase\n");
	printf("   l           Loop, performing multiple transactions\n");
	printf("   q           Quiet/diagnostic mode (site names and errors only)\n");
	printf("   h           Help\n");
    exit(1);
}

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

/* snag a copy of current connection's peer certs so we can 
 * examine them later after the connection is closed */
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

/* print reply received from server */
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
static char *bogusArgv[2] = {"sslServer", ""};

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

static void doPause(const char *prompt) {	
	if(prompt) {
		printf("%s. ", prompt);
	}
	fpurge(stdin);
	printf("Continue (n/anything)? ");
	char c = getchar();
	if(c == 'n') {
		exit(0);
	}
}


/*
 * Perform one SSL diagnostic server-side session. Returns nonzero on error. 
 * Normally no output to stdout except initial "waiting for connection" message, 
 * unless there is a really screwed up error (i.e., something not directly related 
 * to the SSL connection). 
 */
#define RCV_BUF_SIZE		256

static OSStatus sslServe(
	otSocket				listenSock,
	SSLProtocol				tryVersion,
	const char				*hostName,			// e.g., "www.amazon.com"
	CFArrayRef				serverCerts,		// required
	CFArrayRef				encryptServerCerts,	// optional
	CSSM_BOOL				allowExpired,
	CSSM_BOOL				allowAnyRoot,
	char					cipherRestrict,		// '2', 'd'. etc...'\0' for no
												//   restriction
	SSLAuthenticate			authenticate,
	CSSM_BOOL				resumableEnable,
	CSSM_BOOL				silent,				// no stdout
	CSSM_BOOL				pause,
	SSLProtocol				*negVersion,		// RETURNED
	SSLCipherSuite			*negCipher,			// RETURNED
	CFArrayRef				*peerCerts)			// mallocd & RETURNED
{
	otSocket			acceptSock;
    PeerSpec            peerId;
    OSStatus            ortn;
    SSLContextRef       ctx = NULL;
    UInt32              length;
    uint8               rcvBuf[RCV_BUF_SIZE];
	char *outMsg = SERVER_MESSAGE;
	
    *negVersion = kSSLProtocolUnknown;
    *negCipher = SSL_NULL_WITH_NULL_NULL;
    *peerCerts = NULL;
    
	#if IGNORE_SIGPIPE
	signal(SIGPIPE, sigpipe);
	#endif
	
	/* first wait for a connection */
	if(!silent) {
		printf("Waiting for client connection...");
		fflush(stdout);
	}
	ortn = AcceptClientConnection(listenSock, &acceptSock, &peerId);
    if(ortn) {
    	printf("AcceptClientConnection returned %d; aborting\n", ortn);
    	return ortn;
    }

	/* 
	 * Set up a SecureTransport session.
	 * First the standard calls.
	 */
	ortn = SSLNewContext(true, &ctx);
	if(ortn) {
		printSslErrStr("SSLNewContext", ortn);
		goto cleanup;
	} 
	ortn = SSLSetIOFuncs(ctx, SocketRead, SocketWrite);
	if(ortn) {
		printSslErrStr("SSLSetIOFuncs", ortn);
		goto cleanup;
	} 
	ortn = SSLSetProtocolVersion(ctx, tryVersion);
	if(ortn) {
		printSslErrStr("SSLSetProtocolVersion", ortn);
		goto cleanup;
	} 
	ortn = SSLSetConnection(ctx, acceptSock);
	if(ortn) {
		printSslErrStr("SSLSetConnection", ortn);
		goto cleanup;
	}
	ortn = SSLSetPeerDomainName(ctx, hostName, strlen(hostName) + 1);
	if(ortn) {
		printSslErrStr("SSLSetPeerDomainName", ortn);
		goto cleanup;
	}
	
	/* have to do these options befor setting server certs */
	if(allowExpired) {
		ortn = SSLSetAllowsExpiredCerts(ctx, true);
		if(ortn) {
			printSslErrStr("SSLSetAllowExpiredCerts", ortn);
			goto cleanup;
		}
	}
	if(allowAnyRoot) {
		ortn = SSLSetAllowsAnyRoot(ctx, true);
		if(ortn) {
			printSslErrStr("SSLSetAllowAnyRoot", ortn);
			goto cleanup;
		}
	}

	ortn = SSLSetCertificate(ctx, serverCerts);
	if(ortn) {
		printSslErrStr("SSLSetCertificate", ortn);
		goto cleanup;
	}
	if(encryptServerCerts) {
		ortn = SSLSetEncryptionCertificate(ctx, encryptServerCerts);
		if(ortn) {
			printSslErrStr("SSLSetEncryptionCertificate", ortn);
			goto cleanup;
		}
	}
	
	/* 
	 * SecureTransport options.
	 */ 
	if(resumableEnable) {
		ortn = SSLSetPeerID(ctx, &peerId, sizeof(PeerSpec));
		if(ortn) {
			printSslErrStr("SSLSetPeerID", ortn);
			goto cleanup;
		}
	}
	if(cipherRestrict != '\0') {
		ortn = setCipherRestrictions(ctx, cipherRestrict);
		if(ortn) {
			goto cleanup;
		}
	}
	#if AUTHENTICATE_ENABLE
	if(authenticate != kNeverAuthenticate) {
		ortn = SSLSetClientSideAuthenticate(ctx, authenticate);
		if(ortn) {
			printSslErrStr("SSLSetClientSideAuthenticate", ortn);
			goto cleanup;
		}
	}
	#endif
	if(pause) {
		doPause("SSLContext initialized");
	}
	
	/* Perform SSL/TLS handshake */
    do
    {   ortn = SSLHandshake(ctx);
	    if((ortn == errSSLWouldBlock) && !silent) {
	    	/* keep UI responsive */ 
	    	outputDot();
	    }
    } while (ortn == errSSLWouldBlock);
	
	/* this works even if handshake failed due to cert chain invalid */
	copyPeerCerts(ctx, peerCerts);

	SSLGetNegotiatedCipher(ctx, negCipher);
	SSLGetNegotiatedProtocolVersion(ctx, negVersion);
	
	if(!silent) {
		printf("\n");
	}
    if(ortn) {
    	goto cleanup;
    }
	if(pause) {
		doPause("SSLContext handshake complete");
	}

	/* wait for one complete line or user says they've had enough */
	while(ortn == noErr) {
	    length = sizeof(rcvBuf);
	    ortn = SSLRead(ctx, rcvBuf, length, &length);
	    if(length == 0) {
	    	/* keep UI responsive */ 
	    	outputDot();
	    }
	    else {
	    	/* print what we have */
	    	printf("client request: ");
	    	dumpAscii(rcvBuf, length);
	    }
	    if(pause) {
	    	/* allow user to bail */
	    	char resp;
	    	
			fpurge(stdin);
	    	printf("\nMore client request (y/anything): ");
	    	resp = getchar();
	    	if(resp != 'y') {
	    		break;
	    	}
	    }
	    
	    /* poor person's line completion scan */
	    for(unsigned i=0; i<length; i++) {
	    	if((rcvBuf[i] == '\n') || (rcvBuf[i] == '\r')) {
	    		/* a labelled break would be nice here.... */
	    		goto serverResp;
	    	}
	    }
	    if (ortn == errSSLWouldBlock) {
	        ortn = noErr;
	    }
	}
	
serverResp:
	if(pause) {
		doPause("Client GET msg received");
	}

	/* send out canned response */
	length = strlen(outMsg);
 	ortn = SSLWrite(ctx, outMsg, length, &length);
 	if(ortn) {
 		printSslErrStr("SSLWrite", ortn);
 	}
	if(pause) {
		doPause("Server response sent");
	}
    if (ortn == noErr) {
        ortn = SSLClose(ctx);
	}
cleanup:
	if(acceptSock) {
		endpointShutdown(acceptSock);
	}
	if(ctx) {
	    SSLDisposeContext(ctx);  
	}    
	/* FIXME - dispose of serverCerts */
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
			printf("***SecCertificateGetData returned %d\n", ortn);
			continue;
		}
		printf("\n================== Peer Cert %d ===================\n\n", i);
		printCert(certData.Data, certData.Length, verbose);
		printf("\n=============== End of Peer Cert %d ===============\n", i);
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
	printf("   Number of peer certs : %d\n", numPeerCerts);
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

int main(int argc, char **argv)
{   
    OSStatus            err;
	int					arg;
	char				fullFileBase[100];
	SSLProtocol			negVersion;
	SSLCipherSuite		negCipher;
	CFArrayRef			peerCerts = NULL;		
	char				*argp;
	otSocket			listenSock;
	CFArrayRef			serverCerts = nil;		// required
	CFArrayRef			encryptCerts = nil;		// optional
	SecKeychainRef		serverKc = nil;
	SecKeychainRef		encryptKc = nil;
	
	/* user-spec'd parameters */
	char				*hostName = DEFAULT_HOST;	
	unsigned short		portNum = DEFAULT_PORT;
	CSSM_BOOL			allowExpired = CSSM_FALSE;
	CSSM_BOOL			allowAnyRoot = CSSM_FALSE;
	char				*fileBase = NULL;
	CSSM_BOOL			displayRxData = CSSM_FALSE;
	CSSM_BOOL			displayCerts = CSSM_FALSE;
	char				cipherRestrict = '\0';
	SSLProtocol			attemptProt = kTLSProtocol1;
	CSSM_BOOL			protXOnly = CSSM_FALSE;	// kSSLProtocol3Only, 
												//    kTLSProtocol1Only
	CSSM_BOOL			quiet = CSSM_FALSE;
	CSSM_BOOL			resumableEnable = CSSM_TRUE;
	CSSM_BOOL			pause = CSSM_FALSE;
	char				*keyChainName = NULL;
	char				*encryptKeyChainName = NULL;
	CSSM_BOOL			loop = CSSM_FALSE;
	SSLAuthenticate		authenticate = kNeverAuthenticate;
	
	for(arg=1; arg<argc; arg++) {
		argp = argv[arg];
		switch(argp[0]) {
			case 'H':
				hostName = &argp[2];
				break;
			case 'P':
				portNum = atoi(&argp[2]);
				break;
			case 'k':
				keyChainName = &argp[2];
				break;
			case 'E':
				encryptKeyChainName = &argp[2];
				break;
			case 'e':
				allowExpired = CSSM_TRUE;
				break;
			case 'r':
				allowAnyRoot = CSSM_TRUE;
				break;
			case 'd':
				displayRxData = CSSM_TRUE;
				break;
			case 'c':
				displayCerts = CSSM_TRUE;
				break;
			case 'f':
				fileBase = &argp[2];
				break;
			case 'C':
				cipherRestrict = argp[2];
				break;
			case '2':
				attemptProt = kSSLProtocol2;
				break;
			case '3':
				attemptProt = kSSLProtocol3;
				break;
			case 't':
				attemptProt = kTLSProtocol1;
				break;
			case 'o':
				protXOnly = CSSM_TRUE;
				break;
			case 'R':
				resumableEnable = CSSM_FALSE;
				break;
			case 'a':
				if(argp[1] != '=') {
					usage(argv);
				}
				switch(argp[2]) {
					case 'a': authenticate = kAlwaysAuthenticate; break;
					case 'n': authenticate = kNeverAuthenticate; break;
					case 't': authenticate = kTryAuthenticate; break;
					default: usage(argv);
				}
				break;
			case 'p':
				pause = CSSM_TRUE;
				break;
			case 'q':
				quiet = CSSM_TRUE;
				break;
			case 'l':
				loop = CSSM_TRUE;
				break;
			default:
				usage(argv);
		}
	}

	/* get server cert and optional encryption cert as CFArrayRef */
	serverCerts = getSslCerts(keyChainName, CSSM_FALSE, &serverKc);
	if(serverCerts == nil) {
		exit(1);
	}
	if(encryptKeyChainName) {
		encryptCerts = getSslCerts(encryptKeyChainName, CSSM_TRUE, &encryptKc);
		if(encryptCerts == nil) {
			exit(1);
		}
	}
	
	/* one-time only server port setup */
	err = ListenForClients(portNum, &listenSock);
	if(err) {
    	printf("ListenForClients returned %d; aborting\n", err);
		exit(1);
	}
	do {
		err = sslServe(listenSock,
			attemptProt,
			hostName,
			serverCerts,
			encryptCerts,
			allowExpired,
			allowAnyRoot,
			cipherRestrict,
			authenticate,
			resumableEnable,
			quiet,
			pause,
			&negVersion,
			&negCipher,
			&peerCerts);
		if(!quiet) {
			showSSLResult(attemptProt,
				err, 
				negVersion, 
				negCipher, 
				peerCerts,
				displayCerts,
				fileBase ? fullFileBase : NULL);
		}
		freePeerCerts(peerCerts);
			
	} while(loop);
	
	endpointShutdown(listenSock);
	parseCertShutdown();
	if(serverKc) {
		CFRelease(serverKc);
	}
	if(encryptKc) {
		CFRelease(encryptKc);
	}
    return 0;

}


