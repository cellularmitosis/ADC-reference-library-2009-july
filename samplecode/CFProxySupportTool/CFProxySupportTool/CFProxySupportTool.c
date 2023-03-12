/*
    File:       CFProxySupportTool.c

    Contains:   Shows the use of the APIs in "CFProxySupport.h".

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
                ("Apple") in consideration of your agreement to the following
                terms, and your use, installation, modification or
                redistribution of this Apple software constitutes acceptance of
                these terms.  If you do not agree with these terms, please do
                not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following
                terms, and subject to these terms, Apple grants you a personal,
                non-exclusive license, under Apple's copyrights in this
                original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or
                without modifications, in source and/or binary forms; provided
                that if you redistribute the Apple Software in its entirety and
                without modifications, you must retain this notice and the
                following text and disclaimers in all such redistributions of
                the Apple Software. Neither the name, trademarks, service marks
                or logos of Apple Inc. may be used to endorse or promote
                products derived from the Apple Software without specific prior
                written permission from Apple.  Except as expressly stated in
                this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any
                patent rights that may be infringed by your derivative works or
                by other works in which the Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis. 
                APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
                WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
                MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
                THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
                INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
                TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
                DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY
                OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
                OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
                OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF
                SUCH DAMAGE.

    Change History (most recent first):

$Log$

*/

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>

#pragma mark ***** Infrastructure

// In some cases we need to use asynchronous APIs as if they were synchronous. 
// So, we add the asynchronous API's runloop source to the runloop, and then 
// sit inside CFRunLoopRunInMode waiting for things to complete.  We use a custom 
// mode as an example of how you would do this in a real application (of course 
// blocking for a long time in a real application would be a mistake).  It's 
// easy to make the mistake of blocking in kCFRunLoopDefaultMode.  That's a 
// problem in a real application, where all sorts of other things (like 
// event loop timers) are attached to the default mode, and they end up running 
// unexpectedly.

#define kPrivateRunLoopMode CFSTR("com.apple.dts.CFProxySupportTool")

static void CFQRelease(CFTypeRef cf)
	// Trivial wrapper for CFRelease that treats NULL as a no-op.
{
	if (cf != NULL) {
		CFRelease(cf);
	}
}

static const char *EasyToUseButNotThreadSafeCStringForCFString(CFStringRef str)
	// Returns a UTF-8 C string representation of the input CFString.  This isn't 
	// thread safe, but it is very easy to use.
	//
	// WARNING: Don't do something like this:
	//
	//     printf(
	//         "%s = %s\n", 
	//         EasyToUseButNotThreadSafeCStringForCFString(key), 
	//         EasyToUseButNotThreadSafeCStringForCFString(value)
	//     );
	//
	// The second call to EasyToUseButNotThreadSafeCStringForCFString will invalidate 
	// the results of the first.
{
	Boolean			success;
	static char *	sCStr;
	size_t			cStrSize;
	
	assert(str != NULL);
	
	free(sCStr);
	
	cStrSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8);
	sCStr = malloc(cStrSize);
	assert(sCStr != NULL);
	
	success = CFStringGetCString(str, sCStr, cStrSize, kCFStringEncodingUTF8);
	assert(success);

	return sCStr;
}

static void PrintAndRemoveIfPresentProxyDetail(
	CFMutableDictionaryRef	proxyMutable, 
	CFStringRef				key, 
	CFTypeID				expectedTypeID, 
	const char *			name, 
	Boolean					printAsBullets
)
	// Check to see if the key is present in proxyMutable.  If it is, print its 
	// value (using name is the user-visible field name) and remove it from the 
	// proxy dictionary.
	//
	// Also, verify that the type of the value matches expectedTypeID.
	//
	// Finally, if printAsBullets is true, print the value as bullets rather 
	// than the actual value (which is likely to be a password).
{
	assert(proxyMutable != NULL);
	assert(key != NULL);
	assert(name != NULL);
	
	if (CFDictionaryContainsKey(proxyMutable, key)) {
		CFTypeRef		value;
		
		value = CFDictionaryGetValue(proxyMutable, key);
		assert(value != NULL);
		assert(CFGetTypeID(value) == expectedTypeID);
		
		if (printAsBullets) {
			assert(expectedTypeID == CFStringGetTypeID());
			
			fprintf(stdout, "  %s = ********\n", name);
		} else {
			CFStringRef		desc;
			
			if ( (CFGetTypeID(value) == CFStringGetTypeID()) 
			  || (CFGetTypeID(value) == CFNumberGetTypeID())
			  || (CFGetTypeID(value) == CFURLGetTypeID()) ) {
				desc = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@"), value);
			} else {
				desc = CFCopyDescription(value);
			}
			assert(desc != NULL);
			
			fprintf(stdout, "  %s = %s\n", name, EasyToUseButNotThreadSafeCStringForCFString(desc));
			
			CFQRelease(desc);
		}
	
		// Do this last, otherwise the retain count on value might drop to zero.

		CFDictionaryRemoveValue(proxyMutable, key);
	}
}

static void PrintAndRemoveIfPresentProxyDetails(CFMutableDictionaryRef proxyMutable)
	// Print all of the well known properties in a proxy dictionary, 
	// removing them from the dictionary if they're present.
{
	PrintAndRemoveIfPresentProxyDetail(proxyMutable, kCFProxyHostNameKey,			  CFStringGetTypeID(), "host",     false);
	PrintAndRemoveIfPresentProxyDetail(proxyMutable, kCFProxyPortNumberKey,			  CFNumberGetTypeID(), "port",     false);
	PrintAndRemoveIfPresentProxyDetail(proxyMutable, kCFProxyUsernameKey,			  CFStringGetTypeID(), "username", false);
	PrintAndRemoveIfPresentProxyDetail(proxyMutable, kCFProxyPasswordKey,			  CFStringGetTypeID(), "password", true);
	PrintAndRemoveIfPresentProxyDetail(proxyMutable, kCFProxyAutoConfigurationURLKey, CFURLGetTypeID(),    "url",	   false);
}

static void PrintProxy(CFIndex proxyIndex, CFDictionaryRef proxy)
	// Print a single proxy dictionary.
{
	CFMutableDictionaryRef	proxyMutable;
	CFStringRef				proxyType;
	const char *			proxyTypeCStr;
	Boolean					printUnexpectedMessage;
	
	printUnexpectedMessage = true;
	
	// Create a mutable copy.  We work with this copy.  As we process keys we remove 
	// them from the mutable copy.  When we get to the end we check whether we removed 
	// all of the keys.  If not, that's weird and we tell the user.
	
	proxyMutable = CFDictionaryCreateMutableCopy(NULL, CFDictionaryGetCount(proxy), proxy);
	assert(proxyMutable != NULL);
	
	// Get the type of the proxy and dispatch off that.
	
	proxyType = (CFStringRef) CFDictionaryGetValue(proxyMutable, kCFProxyTypeKey);
	assert(proxyType != NULL);
	assert(CFGetTypeID(proxyType) == CFStringGetTypeID());
	
	if (CFEqual(proxyType, kCFProxyTypeNone)) {
		proxyTypeCStr = "no proxy";
	} else if (CFEqual(proxyType, kCFProxyTypeHTTP)) {
		proxyTypeCStr = "HTTP proxy";
	} else if (CFEqual(proxyType, kCFProxyTypeHTTPS)) {
		proxyTypeCStr = "HTTPS proxy";
	} else if (CFEqual(proxyType, kCFProxyTypeSOCKS)) {
		proxyTypeCStr = "SOCKS proxy";
	} else if (CFEqual(proxyType, kCFProxyTypeFTP)) {
		proxyTypeCStr = "FTP proxy";
	} else if (CFEqual(proxyType, kCFProxyTypeAutoConfigurationURL)) {
		proxyTypeCStr = "PAC proxy";
	} else {
		proxyTypeCStr = "unknown proxy";
		printUnexpectedMessage = true;
	}
	fprintf(
		stdout, 
		"%d %s (%s)\n", 
		(int) proxyIndex, 
		proxyTypeCStr, 
		EasyToUseButNotThreadSafeCStringForCFString(proxyType)
	);
	PrintAndRemoveIfPresentProxyDetails(proxyMutable);

	// Do this last, otherwise the retain count on proxyType might drop to zero.

	CFDictionaryRemoveValue(proxyMutable, kCFProxyTypeKey);
	
	// Check to see if any keys got missed.
	
	if (CFDictionaryGetCount(proxyMutable) != 0) {
		if ( printUnexpectedMessage ) {
			fprintf(stderr, "proxy dictionary contains unexpected keys\n");
		}
		CFShow(proxyMutable);
	}
	
	CFQRelease(proxyMutable);
}

static void PrintProxies(CFArrayRef proxies)
	// Print a proxies array.
{
	CFIndex		proxyCount;
	CFIndex		proxyIndex;

	assert(proxies != NULL);
	// CFShow(proxies);
		
	proxyCount = CFArrayGetCount(proxies);
	
	for (proxyIndex = 0; proxyIndex < proxyCount; proxyIndex++) {
		CFDictionaryRef		thisProxy;
		
		thisProxy = (CFDictionaryRef) CFArrayGetValueAtIndex(proxies, proxyIndex);
		assert(CFGetTypeID(thisProxy) == CFDictionaryGetTypeID());

		PrintProxy(proxyIndex, thisProxy);
	}
}

#pragma mark ***** Commands

static OSStatus ProxiesForURL(const char *urlStr)
	// Implements the "proxiesForURL" command.
{
	OSStatus		err;
	CFURLRef		url;
	CFDictionaryRef proxySettings;
	CFArrayRef		proxies;
	
	url = NULL;
	proxySettings = NULL;
	proxies = NULL;
	
	// Create a URL from the argument C string.
	
	err = noErr;
	url = CFURLCreateWithBytes(NULL, (const UInt8 *) urlStr, strlen(urlStr), kCFStringEncodingUTF8, NULL);
	if (url == NULL) {
		err = coreFoundationUnknownErr;
	}
	
	// Get the default proxies dictionary from CF.
	
	if (err == noErr) {
		proxySettings = SCDynamicStoreCopyProxies(NULL);
		if (proxySettings == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Call CFNetworkCopyProxiesForURL and print the results.
	
	if (err == noErr) {
		proxies = CFNetworkCopyProxiesForURL(url, proxySettings);
		if (proxies == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		PrintProxies(proxies);
	}
	
	// Clean up.
	
	CFQRelease(proxies);
	CFQRelease(proxySettings);
	CFQRelease(url);
	
	return err;
}

static OSStatus ProxiesForURLUsingScript(const char *urlStr, const char *scriptPath)
	// Implements the "proxiesForURLUsingScript" command.
{
	OSStatus	err;
	Boolean		success;
	CFURLRef	url;
	CFURLRef	scriptURL;
	CFDataRef	scriptData;
	CFStringRef	scriptStr;
	CFArrayRef	proxies;
	SInt32		readErr;
	
	url = NULL;
	scriptURL = NULL;
	scriptData = NULL;
	scriptStr = NULL;
	proxies = NULL;

	// Create CFURLs from the input parameters.
	
	err = noErr;
	url = CFURLCreateWithBytes(NULL, (const UInt8 *) urlStr, strlen(urlStr), kCFStringEncodingUTF8, NULL);
	if (url == NULL) {
		err = coreFoundationUnknownErr;
	}
	if (err == noErr) {
		scriptURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) scriptPath, strlen(scriptPath), false);
		if (scriptURL == NULL) {
			err = coreFoundationUnknownErr;
		}
	}

	// Read the contents of the script file.
	
	if (err == noErr) {
		success = CFURLCreateDataAndPropertiesFromResource(NULL, scriptURL, &scriptData, NULL, NULL, &readErr);
		if ( ! success ) {
			err = readErr;
			assert(err != noErr);
		}
	}
	if (err == noErr) {
		scriptStr = CFStringCreateWithBytes(NULL, CFDataGetBytePtr(scriptData), CFDataGetLength(scriptData), kCFStringEncodingUTF8, true);
		if (scriptStr == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Run the script and print the results.
	
	if (err == noErr) {
		// Work around <rdar://problem/5530166>.  This dummy call to 
		// CFNetworkCopyProxiesForURL initialise some state within CFNetwork 
		// that is required by CFNetworkCopyProxiesForAutoConfigurationScript.
		
		(void) CFNetworkCopyProxiesForURL(url, NULL);
		
		proxies = CFNetworkCopyProxiesForAutoConfigurationScript(scriptStr, url);
		if (proxies == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		PrintProxies(proxies);
	}
	
	// Clean up.
	
	CFQRelease(url);
	CFQRelease(scriptURL);
	CFQRelease(scriptData);
	CFQRelease(scriptStr);
	CFQRelease(proxies);

	return err;
}

static void ResultCallback(void * client, CFArrayRef proxies, CFErrorRef error)
	// Callback for CFNetworkExecuteProxyAutoConfigurationURL.  client is a 
	// pointer to a CFTypeRef.  This stashes either error or proxies in that 
	// location.
{
	CFTypeRef *		resultPtr;
	
	assert( (proxies != NULL) == (error == NULL) );
	
	resultPtr = (CFTypeRef *) client;
	assert( resultPtr != NULL);
	assert(*resultPtr == NULL);
	
	if (error != NULL) {
		*resultPtr = CFRetain(error);
	} else {
		*resultPtr = CFRetain(proxies);
	}
	CFRunLoopStop(CFRunLoopGetCurrent());
}

static OSStatus ProxiesForURLUsingScriptURL(const char *urlStr, const char *scriptURLStr)
	// Implements the "proxiesForURLUsingScriptURL" command.
{
	OSStatus			err;
	CFURLRef			url;
	CFURLRef			scriptURL;
	CFArrayRef			proxies;
	CFRunLoopSourceRef	rls;
	CFTypeRef			result;
	
	url = NULL;
	scriptURL = NULL;
	proxies = NULL;
	rls = NULL;
	result = NULL;

	// Create CFURLs from the input parameters.
	
	err = noErr;
	url = CFURLCreateWithBytes(NULL, (const UInt8 *) urlStr, strlen(urlStr), kCFStringEncodingUTF8, NULL);
	if (url == NULL) {
		err = coreFoundationUnknownErr;
	}
	if (err == noErr) {
		scriptURL = CFURLCreateWithBytes(NULL, (const UInt8 *) scriptURLStr, strlen(scriptURLStr), kCFStringEncodingUTF8, NULL);
		if (scriptURL == NULL) {
			err = coreFoundationUnknownErr;
		}
	}

	// Run the script and print the results.
	// 
	// Note: CFNetworkExecuteProxyAutoConfigurationURL is an async 
	// routine, but we just treat it as a synchronous routine by attaching 
	// the returned runloop source to our runloop and then running the runloop.  
	// A real application would return to runloop and pick things up in the 
	// CFNetworkExecuteProxyAutoConfigurationURL results callback.
	
	if (err == noErr) {
		CFStreamClientContext	context = { 0, &result, NULL, NULL, NULL };
		
		// Work around <rdar://problem/5530166>.  This dummy call to 
		// CFNetworkCopyProxiesForURL initialise some state within CFNetwork 
		// that is required by CFNetworkCopyProxiesForAutoConfigurationScript.
		
		(void) CFNetworkCopyProxiesForURL(url, NULL);
		
		rls = CFNetworkExecuteProxyAutoConfigurationURL(scriptURL, url, ResultCallback, &context);
		if (rls == NULL) {
			err = coreFoundationUnknownErr;
		}
        
        // Despite the fact that CFNetworkExecuteProxyAutoConfigurationURL has 
        // neither a "Create" nor a "Copy" in the name, we are required to 
        // release the return CFRunLoopSourceRef <rdar://problem/5533931>.
	}
	if (err == noErr) {
		CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kPrivateRunLoopMode);
		
		CFRunLoopRunInMode(kPrivateRunLoopMode, 1.0e10, false);
		
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kPrivateRunLoopMode);
		
		// Once the runloop returns, we should have either an error result or a 
		// proxies array result.  Do the appropriate thing with that result.
		
		assert(result != NULL);
		
		if ( CFGetTypeID(result) == CFErrorGetTypeID() ) {
			if ( CFEqual(CFErrorGetDomain( (CFErrorRef) result ), kCFErrorDomainOSStatus) ) {
				err = CFErrorGetCode( (CFErrorRef) result );
			} else {
				err = coreFoundationUnknownErr;
			}
		} else if ( CFGetTypeID(result) == CFArrayGetTypeID() ) {
			PrintProxies(result);
		} else {
			assert(false);
			err = kernelTimeoutErr;
		}
	}
	
	// Clean up.
	
	CFQRelease(result);
	CFQRelease(rls);
	CFQRelease(proxies);
	CFQRelease(scriptURL);
	CFQRelease(url);

	return err;
}

// IMPORTANT
// The next few routines (specifically, HTTPGetAndPrint, 
// InitSockAddrFromHostCFStringAndPort, HTTPGetAndPrintFromURL, and 
// HTTPGetAndPrintFromProxy) represent a bone-headed implementation of HTTP using 
// BSD APIs.  This code is present purely to show how you can use CFProxySupport 
// to glue non-CFNetwork based networking code to the system-specified 
// proxies.  It is not meant to be a good example of BSD programming. 
//
// For example, this code:
//
// o does not support IPv6
//
// o uses an ancient interface to the DNS (<x-man-page://3/gethostbyname> 
//   when it should be using <x-man-page://3/getaddrinfo>)
//
// o does not connect to all of the addresses returned by the DNS; it 
//   just tries the first one then gives up
//
// o does not handle short writes (see comments in HTTPGetAndPrint)
//
// In summary, don't copy and paste this part of the sapmle code into a real 
// product.

static OSStatus HTTPGetAndPrint(const char *resource, const struct sockaddr_in *addr)
    // This is a bone-headed implementation of HTTP using straight BSD sockets.  
    // It connects to the server specified by addr, requests the specified 
    // resource, and prints the result to stdout.
{
    int                 err;
    char *              httpRequest;
    int                 fd;
    int                 junk;
    ssize_t             bytesWritten;
    ssize_t             bytesRead;
    ssize_t             bufIndex;
    char                buffer[1024];
    
    httpRequest = NULL;
    
    // Create the HTTP request string.  Note that we specify HTTP/1.0 because 
    // we /so/ aren't prepared to handle HTTP/1.0 (-:
    
    (void) asprintf(&httpRequest, "GET %s HTTP/1.0\r\n\r\n", resource);
    
    // Create and connect the socket.

    err = 0;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        err = errno;
    }
    if (err == 0) {
        err = connect(fd, (const struct sockaddr *) addr, addr->sin_len);
        if (err < 0) {
            err = errno;
        }
    }

    // Send the request.  Note that we don't handle short writes (that is, 
    // write returning a positive number less than strlen(httpRequest).  
    // In practice this never happens for us (because strlen(httpRequest) is 
    // much less than the size of the socket buffer), but a robust 
    // implementation must handle this.
    //
    // Similarly, we don't handle EINTR because this write should never block.
    
    if (err == 0) {
        bytesWritten = write(fd, httpRequest, strlen(httpRequest));
        if (bytesWritten < 0) {
            err = errno;
        } else {
            assert(bytesWritten == (ssize_t) strlen(httpRequest));
        }
    }
    
    // Read and print the response.  We completely ignore text encodings here.
    // If the server responds with ISO Latin1, as many do, and we print it to 
    // Terminal (which is expecting UTF-8), we're going to print gibberish.
    
    if (err == 0) {
        do {
            bytesRead = read(fd, buffer, sizeof(buffer));
            if (bytesRead < 0) {
                err = errno;
                if (err == EINTR) {
                    err = 0;
                }
            } else if (bytesRead > 0) {
                for (bufIndex = 0; bufIndex < bytesRead; bufIndex++) {
                    if (buffer[bufIndex] != '\r') {
                        (void) fputc(buffer[bufIndex], stdout);
                    }
                }
            }
        } while ( (err == 0) && (bytesRead != 0) );
    }
    
    // Clean up.
    
    if (fd >= 0) {
        junk = close(fd);
        assert(junk == 0);
    }
    free(httpRequest);
    
    // Convert errno-style error to OSStatus-style error per Q&A 1499.
    //
    // <http://developer.apple.com/qa/qa2006/qa1499.html>
    
    return (err == 0) ? noErr : ((OSStatus) (100000 + err));
}

static OSStatus InitSockAddrFromHostCFStringAndPort(
    CFStringRef             host, 
    uint16_t                port, 
    struct sockaddr_in *    addrPtr
)
    // Set up a (struct sockaddr_in) by resolving the specified host to an 
    // IP address.
{
    OSStatus                err;
    Boolean                 success;
    char                    hostStr[NI_MAXHOST];
    const struct hostent *  hostEnt;

    // Get the host string as ASCII (not UTF-8 -- the DNS uses ASCII!).
    
    err = noErr;
    success = CFStringGetCString(host, hostStr, sizeof(hostStr), kCFStringEncodingASCII);
    if ( ! success ) {
        err = coreFoundationUnknownErr;
    }

    // Resolve it.  This should be using <x-man-page://3/getaddrinfo>, but 
    // gethostbyname is simpler and we're not going to handle IPv6 anyway.
    
    if (err == noErr) {
        hostEnt = gethostbyname(hostStr);;
        if (hostEnt == NULL) {

            // Convert errno-style error to OSStatus-style error per Q&A 1499.
            //
            // <http://developer.apple.com/qa/qa2006/qa1499.html>

            err = (OSStatus) (100000 + ENOENT);
        }
    }
    
    // Fill out the (struct sockaddr_in).  Note that this uses just the first 
    // IP address for the host.  A real implementation would have to try 
    // each address in turn.
    
    if (err == noErr) {
        memset(addrPtr, 0, sizeof(*addrPtr));
        addrPtr->sin_len = sizeof(*addrPtr);
        addrPtr->sin_family = AF_INET;
        addrPtr->sin_port = htons(port);
        addrPtr->sin_addr.s_addr = *((in_addr_t *)hostEnt->h_addr);
    }
    
    return err;
}

static OSStatus HTTPGetAndPrintFromURL(CFURLRef url)
    // Use direct HTTP to get and print the resource at the specifier URL.
{   
    OSStatus            err;
    CFStringRef         host;
    SInt32              port;
    struct sockaddr_in  hostAddr;
    CFRange             pathRange;
    
    host = NULL;
    
    // Create a (struct sockaddr_in) from the URL.

    port = CFURLGetPortNumber(url);
    if (port == -1) {
        port = 80;  // If there's no port, default to port 80.
    }
    err = noErr;
    host = CFURLCopyHostName(url);
    if (host == NULL) {
        err = coreFoundationUnknownErr;
    }
    if (err == noErr) {
        err = InitSockAddrFromHostCFStringAndPort(host, (uint16_t) port, &hostAddr);
    }

    // Do the HTTP request.  Note that this code is complicated, somewhat, 
    // by the fact that we want to send the path component (and everything after 
    // it) to the server.  So, we get the URL as bytes and then find the path 
    // component and use its /start/ as the 'the rest of the URL'.  Also, 
    // if path component is empty, we just send a "/".
    
    if (err == noErr) {
        CFIndex             bufSize;
        char *              buf;
		const char *		path;

        bufSize = CFURLGetBytes(url, NULL, 0) + 1;
        buf = malloc(bufSize);
        assert(buf != NULL);
        
        (void) CFURLGetBytes(url, (UInt8 *) buf, bufSize - 1);
        buf[bufSize - 1] = 0;
        
        pathRange = CFURLGetByteRangeForComponent(url, kCFURLComponentPath, NULL);
		if (pathRange.length == 0) {
			path = "/";
		} else {
			path = &buf[pathRange.location];
		}
        
        err = HTTPGetAndPrint(path, &hostAddr);
        
        free(buf);
    }

    CFQRelease(host);

    return noErr;
}

static OSStatus HTTPGetAndPrintFromProxy(CFURLRef url, CFDictionaryRef thisProxy)
    // Use an HTTP proxy to get and print the resource at the specifier URL.
{
    OSStatus            err;
    Boolean             success;
    CFStringRef         proxyHost;
    CFNumberRef         proxyPortNum;
    SInt32              proxyPort;
    struct sockaddr_in  proxyAddr;

    // Create a (struct sockaddr_in) from the proxy host and port.

    err = noErr;
    proxyHost = CFDictionaryGetValue(thisProxy, kCFProxyHostNameKey);
    if (proxyHost == NULL) {
        err = coreFoundationUnknownErr;
    } else {
        assert(CFGetTypeID(proxyHost) == CFStringGetTypeID());
    }
    if (err == noErr) {
        proxyPortNum = CFDictionaryGetValue(thisProxy, kCFProxyPortNumberKey);
        if (proxyPortNum == NULL) {
            proxyPort = 8080;  // If there's no port, default to port 8080.
        } else {
            assert(CFGetTypeID(proxyPortNum) == CFNumberGetTypeID());
            
            success = CFNumberGetValue(proxyPortNum, kCFNumberSInt32Type, &proxyPort);
            if ( ! success ) {
                err = coreFoundationUnknownErr;
            }
        }
    }
    if (err == noErr) {
        err = InitSockAddrFromHostCFStringAndPort(proxyHost, (uint16_t) proxyPort, &proxyAddr);
    }
    
    // Do the HTTP request to the proxy.  As the proxy expects the entire URL, 
    // this is much easier than the HTTPGetAndPrintFromURL case.
    
    if (err == noErr) {
        CFIndex             bufSize;
        char *              buf;

        bufSize = CFURLGetBytes(url, NULL, 0) + 1;
        buf = malloc(bufSize);
        assert(buf != NULL);
        
        (void) CFURLGetBytes(url, (UInt8 *) buf, bufSize - 1);
        buf[bufSize - 1] = 0;

        err = HTTPGetAndPrint(buf, &proxyAddr);
        
        free(buf);
    }

    return err;
}

static OSStatus CreateProxyListWithExpandedPACProxies(
    CFURLRef        url, 
    CFArrayRef      proxies, 
    CFArrayRef *    expandedProxiesPtr
)
    // proxies is the array of proxies for the URL specified by url.  Create 
    // a new array of proxies, where any PAC-based proxy is replaced by 
    // the results of running the PAC script.
{
    OSStatus			err;
	CFMutableArrayRef	expandedProxies;
	CFIndex				proxyCount;
	CFIndex				proxyIndex;
	
    assert(proxies != NULL);
    assert( expandedProxiesPtr != NULL);
    assert(*expandedProxiesPtr == NULL);

	expandedProxies = NULL;
	
    // Start with an empty results array.
    
    err = noErr;
	expandedProxies = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (expandedProxies == NULL) {
		err = coreFoundationUnknownErr;
	}
    
    // For each incoming proxy, if it's not a PAC-based proxy, just add the proxy 
    // to the results array.  If it /is/ a PAC-based proxy, run the PAC script 
    // and append its results to the array.
    
	if (err == noErr) {
		proxyCount = CFArrayGetCount(proxies);
		
		for (proxyIndex = 0; proxyIndex < proxyCount; proxyIndex++) {
			CFDictionaryRef		thisProxy;
			CFStringRef			proxyType;
		
			thisProxy = (CFDictionaryRef) CFArrayGetValueAtIndex(proxies, proxyIndex);
			assert(thisProxy != NULL);
			assert(CFGetTypeID(thisProxy) == CFDictionaryGetTypeID());
			
			proxyType = (CFStringRef) CFDictionaryGetValue(thisProxy, kCFProxyTypeKey);
			assert(proxyType != NULL);
			assert(CFGetTypeID(proxyType) == CFStringGetTypeID());
			
			if ( ! CFEqual(proxyType, kCFProxyTypeAutoConfigurationURL) ) {
				// If it's not a PAC proxy, just copy it across.
				
				CFArrayAppendValue(expandedProxies, thisProxy);
			} else {
				CFRunLoopSourceRef		rls;
				CFURLRef				scriptURL;
				CFTypeRef				result;
				CFStreamClientContext	context = { 0, &result, NULL, NULL, NULL };
				
				// If it is a PAC proxy, expand it and copy the results across.
				
				result = NULL;
				
				scriptURL = CFDictionaryGetValue(thisProxy, kCFProxyAutoConfigurationURLKey);
				assert(scriptURL != NULL);
				assert(CFGetTypeID(scriptURL) == CFURLGetTypeID());
				
                // We don't need to work around <rdar://problem/5530166> because 
                // we know that our caller has called CFNetworkCopyProxiesForURL.
                
				rls = CFNetworkExecuteProxyAutoConfigurationURL(scriptURL, url, ResultCallback, &context);
				if (rls == NULL) {
					err = coreFoundationUnknownErr;
				}

                // Despite the fact that CFNetworkExecuteProxyAutoConfigurationURL has 
                // neither a "Create" nor a "Copy" in the name, we are required to 
                // release the return CFRunLoopSourceRef <rdar://problem/5533931>.

				if (err == noErr) {
					CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kPrivateRunLoopMode);
					
					CFRunLoopRunInMode(kPrivateRunLoopMode, 1.0e10, false);
					
					CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kPrivateRunLoopMode);
					
					// Once the runloop returns, we should have either an error result or a 
					// proxies array result.  Do the appropriate thing with that result.
					
					assert(result != NULL);
					
					if ( CFGetTypeID(result) == CFErrorGetTypeID() ) {
						if ( CFEqual(CFErrorGetDomain( (CFErrorRef) result ), kCFErrorDomainOSStatus) ) {
							err = CFErrorGetCode( (CFErrorRef) result );
						} else {
							err = coreFoundationUnknownErr;
						}
					} else if ( CFGetTypeID(result) == CFArrayGetTypeID() ) {
						CFArrayAppendArray(expandedProxies, result, CFRangeMake(0, CFArrayGetCount(result)));
					} else {
						assert(false);
						err = kernelTimeoutErr;
					}
				}
				
				CFQRelease(result);
				CFQRelease(rls);
			}
		}
	}
	
	// Clean up.
	
	if (err == noErr) {
		*expandedProxiesPtr = expandedProxies;
	} else {
		CFQRelease(expandedProxies);
	}
    
    assert( (err == noErr) == (*expandedProxiesPtr != NULL) );
    
    return err;
}

static OSStatus ProxyAwareGetURL(const char *urlStr)
	// Implements the "proxyAwareGetURL" command.
{
	OSStatus            err;
	CFURLRef            url;
	CFDictionaryRef     proxySettings;
	CFArrayRef          proxies;
    CFArrayRef          proxiesToTry;
	
	url           = NULL;
	proxySettings = NULL;
	proxies       = NULL;
    proxiesToTry  = NULL;
	
	// Create a URL from the argument C string, and verify that it's an plain ol' 
    // HTTP request.
	
	err = noErr;
	url = CFURLCreateWithBytes(NULL, (const UInt8 *) urlStr, strlen(urlStr), kCFStringEncodingUTF8, NULL);
	if (url == NULL) {
		err = coreFoundationUnknownErr;
	}
    if (err == noErr) {
        CFStringRef     scheme;
        
        scheme = CFURLCopyScheme(url);
        assert(scheme != NULL);
        
        if ( CFStringCompare(scheme, CFSTR("HTTP"), kCFCompareCaseInsensitive) != kCFCompareEqualTo ) {
            err = unimpErr;
        }
        
        CFQRelease(scheme);
    }
	
	// Get the default proxies dictionary from CF.
	
	if (err == noErr) {
		proxySettings = SCDynamicStoreCopyProxies(NULL);
		if (proxySettings == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Call CFNetworkCopyProxiesForURL to get the proxy list.  Then expand 
    // any PAC-based proxies.
	
	if (err == noErr) {
		proxies = CFNetworkCopyProxiesForURL(url, proxySettings);
		if (proxies == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
    if (err == noErr) {
        err = CreateProxyListWithExpandedPACProxies(url, proxies, &proxiesToTry);
    }
    
    // Finally, try to do get the resource using the various proxies we have 
    // available.
    
    if (err == noErr) {
        CFIndex     proxyCount;
        CFIndex     proxyIndex;
        
        proxyCount = CFArrayGetCount(proxiesToTry);
        
        for (proxyIndex = 0; proxyIndex < proxyCount; proxyIndex++) {
            CFDictionaryRef     thisProxy;
            CFStringRef         proxyType;
            
            thisProxy = (CFDictionaryRef) CFArrayGetValueAtIndex(proxiesToTry, proxyIndex);
            assert(thisProxy != NULL);
            assert(CFGetTypeID(thisProxy) == CFDictionaryGetTypeID());
            
            proxyType = CFDictionaryGetValue(thisProxy, kCFProxyTypeKey);
            assert(proxyType != NULL);
            assert(CFGetTypeID(proxyType) == CFStringGetTypeID());
            
            if ( CFEqual(proxyType, kCFProxyTypeNone) ) {
                fprintf(stdout, "%ld Trying direct access (%s)\n", (long) proxyIndex, EasyToUseButNotThreadSafeCStringForCFString(proxyType));

                err = HTTPGetAndPrintFromURL(url);
            } else if ( CFEqual(proxyType, kCFProxyTypeHTTP) ) {
                fprintf(stdout, "%ld Trying HTTP proxy (%s)\n", (long) proxyIndex, EasyToUseButNotThreadSafeCStringForCFString(proxyType));
                
                err = HTTPGetAndPrintFromProxy(url, thisProxy);
            } else {
                fprintf(stdout, "%ld Skipping unsupported proxy (%s)\n", (long) proxyIndex, EasyToUseButNotThreadSafeCStringForCFString(proxyType));
                err = unimpErr;
            }
            
            if (err == noErr) {
                break;
            } else if (err != unimpErr) {
                fprintf(stdout, "  Failed with error %ld\n", (long) err);
            }
        }
    }

	// Clean up.
	
    CFQRelease(proxiesToTry);
	CFQRelease(proxies);
	CFQRelease(proxySettings);
	CFQRelease(url);
    
    return err;
}

#pragma mark ***** main

int main(int argc, char **argv)
{
	int			retVal;
	OSStatus	err;
	
	// Parse the arguments and dispatch to the various command routines.
	
	err = noErr;
	retVal = EXIT_SUCCESS;
	if (argc < 2) {
		retVal = EXIT_FAILURE;
	} else {
		if (strcasecmp(argv[1], "proxiesForURL") == 0) {
			if (argc != 3) {
				retVal = EXIT_FAILURE;
			} else {
				err = ProxiesForURL(argv[2]);
			}
		} else if (strcasecmp(argv[1], "proxiesForURLUsingScript") == 0) {
			if (argc != 4) {
				retVal = EXIT_FAILURE;
			} else {
				err = ProxiesForURLUsingScript(argv[2], argv[3]);
			}
		} else if (strcasecmp(argv[1], "proxiesForURLUsingScriptURL") == 0) {
			if (argc != 4) {
				retVal = EXIT_FAILURE;
			} else {
				err = ProxiesForURLUsingScriptURL(argv[2], argv[3]);
			}
		} else if (strcasecmp(argv[1], "proxyAwareGetURL") == 0) {
			if (argc != 3) {
				retVal = EXIT_FAILURE;
			} else {
				err = ProxyAwareGetURL(argv[2]);
			}
		} else {
			retVal = EXIT_FAILURE;
		}
	}
	
	// Handle any errors.
	
	if (err != noErr) {
		fprintf(stderr, "Failed with error %ld.\n", (long) err);
		retVal = EXIT_FAILURE;
	}
	if ( (retVal == EXIT_FAILURE) && (err == noErr) ) {
		fprintf(stderr, "usage: %s proxiesForURL targetURL\n", getprogname());
		fprintf(stderr, "       %*s proxiesForURLUsingScript targetURL scriptPath\n", (int) strlen(getprogname()), "");
		fprintf(stderr, "       %*s proxiesForURLUsingScriptURL targetURL scriptURL\n", (int) strlen(getprogname()), "");
		fprintf(stderr, "       %*s proxyAwareGetURL targetURL\n", (int) strlen(getprogname()), "");
	}
	
    return retVal;
}
