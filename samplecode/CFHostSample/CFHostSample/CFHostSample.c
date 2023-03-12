/*
    File:  CFHostSample.c
    
    Contains:  Sample code which shows how to use the CFHost API to do asynchronous
    DNS name lookups and to determine host reachability.
     
    Copyright:  © Copyright 2004 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
            1.0 (4/23/2004)
*/


#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include <netinet/in.h>     // INET6_ADDRSTRLEN
#include <arpa/nameser.h>   // NS_MAXDNAME
#include <netdb.h>          // getaddrinfo, struct addrinfo, AI_NUMERICHOST
#include <unistd.h>         // getopt



static void
MyPrintAddressArray(CFArrayRef addresses, CFStringRef hostName)
{
    struct sockaddr  *addr;
    char             ipAddress[INET6_ADDRSTRLEN];
    CFIndex          index, count;
    char             *name;
    CFIndex          nameLength;
    Boolean          success;
    int              err;
    
    assert(addresses != NULL);
    assert(hostName != NULL);

    /* CFStringGetMaximumSizeForEncoding determines max bytes a string of specified length will take up if encoded. */
    nameLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(hostName), kCFStringEncodingASCII);
    name = malloc(nameLength + 1);
    assert(name != NULL);
    
    success = CFStringGetCString(hostName, name, nameLength + 1, kCFStringEncodingASCII);
    assert(success);
    
    count = CFArrayGetCount(addresses);
    for (index = 0; index < count; index++) {
        addr = (struct sockaddr *)CFDataGetBytePtr(CFArrayGetValueAtIndex(addresses, index));
        assert(addr != NULL);
        
        /* getnameinfo coverts an IPv4 or IPv6 address into a text string. */
        err = getnameinfo(addr, addr->sa_len, ipAddress, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
        if (err == 0) {
            printf("%s -> %s\n", name, ipAddress);
        } else {
            printf("getnameinfo returned %d\n", err);
        }
    }
    free(name);
}



static void
MyPrintNameArray(CFArrayRef hostNames, CFStringRef addressString)
{
    char         *name;
    CFIndex      nameLength;
    CFIndex      index, count;
    CFStringRef  hostName;
    char         *ipAddress;
    CFIndex      addressLength;
    Boolean      success;
    
    assert(hostNames != NULL);
    assert(addressString != NULL);

    /* CFStringGetMaximumSizeForEncoding determines max bytes a string of specified length will take up if encoded. */
    nameLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(addressString), kCFStringEncodingASCII);
    ipAddress = malloc(addressLength + 1);
    assert(ipAddress != NULL);
    
    success = CFStringGetCString(addressString, ipAddress, addressLength + 1, kCFStringEncodingASCII);
    assert(success);
    
    count = CFArrayGetCount(hostNames);
    for (index = 0; index < count; index++) {
        hostName = CFArrayGetValueAtIndex(hostNames, index);
        assert((hostName != NULL) && (CFGetTypeID(hostName) == CFStringGetTypeID()));
        
        /* CFStringGetMaximumSizeForEncoding determines max bytes a string of specified length will take up if encoded. */
        nameLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(hostName), kCFStringEncodingASCII);
        name = malloc(nameLength + 1);
        assert(name != NULL);
        
        success = CFStringGetCString(hostName, name, nameLength + 1, kCFStringEncodingASCII);
        assert(success);
                
        fprintf(stderr, "%s -> %s\n", ipAddress, name);
        free(name);
    }
    free(ipAddress);
}



static void
MyPrintReachability(CFDataRef data, CFStringRef nameOrAddress)
{
    SCNetworkConnectionFlags  *flags;
    CFIndex                   length;
    char                      *input;
    Boolean                   success;
    
    assert(data != NULL);
    assert(nameOrAddress != NULL);

    /* CFStringGetMaximumSizeForEncoding determines max bytes a string of specified length will take up if encoded. */
    length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(nameOrAddress), kCFStringEncodingASCII);
    input = malloc(length + 1);
    assert(input != NULL);
    
    success = CFStringGetCString(nameOrAddress, input, length + 1, kCFStringEncodingASCII);
    assert(success);
    
    flags = (SCNetworkConnectionFlags *)CFDataGetBytePtr(data);
    assert (flags != NULL);
    
    /* If you only have a PPP interface enabled, the flags will be 0 because of a bug. <rdar://problem/3627771> */
    if (*flags == 0) fprintf(stderr, "%s -> Reachability Unknown\n", input);
    
    if (*flags & kSCNetworkFlagsTransientConnection)  fprintf(stderr, "%s -> Transient Connection\n",  input);
    if (*flags & kSCNetworkFlagsReachable)            fprintf(stderr, "%s -> Reachable\n",             input);
    if (*flags & kSCNetworkFlagsConnectionRequired)   fprintf(stderr, "%s -> Connection Required\n",   input);
    if (*flags & kSCNetworkFlagsConnectionAutomatic)  fprintf(stderr, "%s -> Connection Automatic\n",  input);
    if (*flags & kSCNetworkFlagsInterventionRequired) fprintf(stderr, "%s -> Intervention Required\n", input);
    if (*flags & kSCNetworkFlagsIsLocalAddress)       fprintf(stderr, "%s -> Is Local Address\n",      input);
    if (*flags & kSCNetworkFlagsIsDirect)             fprintf(stderr, "%s -> Is Direct\n",             input);
    
    free(input);
}



static void
MyCFHostCleanup(CFHostRef host)
{
    assert(host != NULL);
    
    /* CFHostUnscheduleFromRunLoop unschedules the given host from a run loop and mode
    so the client will not receive its callbacks on that loop and mode. */
    CFHostUnscheduleFromRunLoop(host, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    /* This removes the client callback association from the host object. */
    (void) CFHostSetClient(host, NULL, NULL);
    
    CFRelease(host);
}



static void
MyCFHostClientCallBack(CFHostRef host, CFHostInfoType typeInfo, const CFStreamError *error, void *info)
{
    CFStringRef  input = (CFStringRef)info;    
    CFArrayRef   array;
    CFDataRef    data;
    
    if (error->error == noErr) {
    
        switch (typeInfo) {
        
            case kCFHostAddresses:
                /* CFHostGetAddressing retrieves the known addresses from the given host. Returns a
                CFArrayRef of addresses.  Each address is a CFDataRef wrapping a struct sockaddr. */
                array = CFHostGetAddressing(host, NULL);
                MyPrintAddressArray(array, input);
                break;
            case kCFHostNames:
                /* CFHostGetNames retrieves the names/aliases from the given host.
                Returns a CFArrayRef of names for the given host. */
                array = CFHostGetNames(host, NULL);
                MyPrintNameArray(array, input);
                break;
            case kCFHostReachability:
                /* CFHostGetReachability retrieves the reachability of the given host.  Returns a
                CFDataRef which wraps the reachability flags.  The possible values of these flags
                is declared in SystemConfiguration/SCNetwork.h.  */
                data = CFHostGetReachability(host, NULL);
                MyPrintReachability(data, input);
                break;
            default:
                fprintf(stderr, "Unknown CFHostInfoType (%d)\n", typeInfo);
                break;
        }
    } else {
        fprintf(stderr, "MyCFHostClientCallBack returned (%d, %ld)\n", error->domain, error->error);
    }
    
    /* The CFHost callback only gets called once, so we cleanup now that we're done. */
    MyCFHostCleanup(host);
    
    /* Stop the run loop now that we've retrieved the host info. */
    CFRunLoopStop(CFRunLoopGetCurrent());
}



static Boolean
MyResolveNameToAddress(CFStringRef hostName)
{
    CFHostRef            host;
    CFHostClientContext  context = { 0, (void *)hostName, CFRetain, CFRelease, NULL };
    CFStreamError        error;
    Boolean              success;

    assert(hostName != NULL);

    /* Creates a new host object with the given name. */
    host = CFHostCreateWithName(kCFAllocatorDefault, hostName);
    assert(host != NULL);
    
    /* CFHostSetClient associates a client context and callback function with a CFHostRef.
    This is required for asynchronous usage.  If not set, resolve will take place synchronously. */
    success = CFHostSetClient(host, MyCFHostClientCallBack, &context);
    if (success) {

        /* CFHostScheduleWithRunLoop schedules the given host on a run loop and mode so
        the client will receive its callbacks on that loop and mode. */
        CFHostScheduleWithRunLoop(host, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

        /* CFHostStartInfoResolution performs a lookup for the given host.
        It will search for the requested information if there is no other active request. */
        success = CFHostStartInfoResolution(host, kCFHostAddresses, &error);
        if (!success) {
            fprintf(stderr, "CFHostStartInfoResolution returned (%d, %ld)\n", error.domain, error.error);
            MyCFHostCleanup(host);
        }
    } else {
        fprintf(stderr, "CFHostSetClient failed\n");
        CFRelease(host);
    }
    
    return success;
}



static Boolean
MyResolveAddressToName(CFStringRef addressString)
{
    CFHostRef            host;
    CFDataRef            address = NULL;
    char                 *ipAddress;
    CFIndex              addressLength;
    CFHostClientContext  context = { 0, (void *)addressString, CFRetain, CFRelease, NULL };
    CFStreamError        error;
    Boolean              success;
    struct addrinfo      hints;
    struct addrinfo      *result = NULL;
    int                  err;
    
    assert(addressString != NULL);

    /* CFStringGetMaximumSizeForEncoding determines max bytes a string of specified length will take up if encoded. */
    addressLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(addressString), kCFStringEncodingASCII);
    ipAddress = malloc(addressLength + 1);
    assert(ipAddress != NULL);
    
    success = CFStringGetCString(addressString, ipAddress, addressLength + 1, kCFStringEncodingASCII);
    assert(success);
        
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_NUMERICHOST;
    hints.ai_family   = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    /* getaddrinfo coverts a text string into an IPv4 or IPv6 address in network byte order. */
    err = getaddrinfo(ipAddress, NULL, &hints, &result);
    if (err == 0) {
        address = CFDataCreate(NULL, (UInt8 *)result->ai_addr, result->ai_addrlen);
        assert(address != NULL);
        freeaddrinfo(result);
            
        /* Creates a new host object with the given address. */
        host = CFHostCreateWithAddress(kCFAllocatorDefault, address);
        assert(host != NULL);
        CFRelease(address);
                  
        /* CFHostSetClient associates a client context and callback function with a CFHostRef.
        This is required for asynchronous usage.  If not set, resolve will take place synchronously. */                  
        success = CFHostSetClient(host, MyCFHostClientCallBack, &context);
        if (success) {
                    
            /* CFHostScheduleWithRunLoop schedules the given host on a run loop and mode so
            the client will receive its callbacks on that loop and mode. */         
            CFHostScheduleWithRunLoop(host, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
            
            /* CFHostStartInfoResolution performs a lookup for the given host.
            It will search for the requested information if there is no other active request. */
            success = CFHostStartInfoResolution(host, kCFHostNames, &error);
            if (!success) {
                fprintf(stderr, "CFHostStartInfoResolution returned (%d, %ld)\n", error.domain, error.error);
                MyCFHostCleanup(host);
            }
        } else {
            fprintf(stderr, "CFHostSetClient failed\n");
            CFRelease(host);
        }
    } else {
        fprintf(stderr, "\"%s\" is not a valid IP address\n", ipAddress);
        success = false;
    }
    
    free(ipAddress);

    return success;
}



static Boolean
MyCheckReachabilityByName(CFStringRef hostName)
{
    CFHostRef            host;
    CFHostClientContext  context = { 0, (void *)hostName, CFRetain, CFRelease, NULL };
    CFStreamError        error;
    Boolean              success;

    assert(hostName != NULL);

    /* Creates a new host object with the given name. */
    host = CFHostCreateWithName(kCFAllocatorDefault, hostName);
    assert(host != NULL);
        
    /* CFHostSetClient associates a client context and callback function with a CFHostRef.
    This is required for asynchronous usage.  If not set, resolve will take place synchronously. */        
    success = CFHostSetClient(host, MyCFHostClientCallBack, &context);
    if (success) {

        /* CFHostScheduleWithRunLoop schedules the given host on a run loop and mode so
        the client will receive its callbacks on that loop and mode. */
        CFHostScheduleWithRunLoop(host, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        
        /* CFHostStartInfoResolution performs a lookup for the given host.
        It will search for the requested information if there is no other active request. */
        success = CFHostStartInfoResolution(host, kCFHostReachability, &error);
        if (!success) {
            fprintf(stderr, "CFHostStartInfoResolution returned (%d, %ld)\n", error.domain, error.error);
            MyCFHostCleanup(host);
        }
    } else {
        fprintf(stderr, "CFHostSetClient failed\n");
        CFRelease(host);
    }
    
    return success;
}



static Boolean
MyCheckReachabilityByAddress(CFStringRef addressString)
{
    CFHostRef            host;
    CFDataRef            address = NULL;
    CFHostClientContext  context = { 0, (void *)addressString, CFRetain, CFRelease, NULL };
    CFStreamError        error;
    CFIndex              addressLength;
    char                 *ipAddress;
    Boolean              success;
    struct addrinfo      hints;
    struct addrinfo      *result = NULL;
    int                  err;
    
    assert(addressString != NULL);
    
    /* CFStringGetMaximumSizeForEncoding determines max bytes a string of specified length will take up if encoded. */
    addressLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(addressString), kCFStringEncodingASCII);
    ipAddress = malloc(addressLength + 1);
    assert(ipAddress != NULL);
    
    success = CFStringGetCString(addressString, ipAddress, addressLength + 1, kCFStringEncodingASCII);
    assert(success);
        
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_NUMERICHOST;
    hints.ai_family   = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    /* getaddrinfo coverts a text string into an IPv4 or IPv6 address in network byte order. */
    err = getaddrinfo(ipAddress, NULL, &hints, &result);
    if (err == 0) {
        address = CFDataCreate(NULL, (UInt8 *)result->ai_addr, result->ai_addrlen);
        assert(address != NULL);
        freeaddrinfo(result);
        
        /* Creates a new host object with the given address. */
        host = CFHostCreateWithAddress(kCFAllocatorDefault, address);
        assert(host != NULL);
        CFRelease(address);
        
        /* CFHostSetClient associates a client context and callback function with a CFHostRef.
        This is required for asynchronous usage.  If not set, resolve will take place synchronously. */                      
        success = CFHostSetClient(host, MyCFHostClientCallBack, &context);
        if (success) {
        
            /* CFHostScheduleWithRunLoop schedules the given host on a run loop and mode so
            the client will receive its callbacks on that loop and mode. */            
            CFHostScheduleWithRunLoop(host, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

            /* CFHostStartInfoResolution performs a lookup for the given host.
            It will search for the requested information if there is no other active request. */
            success = CFHostStartInfoResolution(host, kCFHostReachability, &error);
            if (!success) {
                fprintf(stderr, "CFHostStartInfoResolution returned (%d, %ld)\n", error.domain, error.error);
                MyCFHostCleanup(host);
            }
        } else {
            fprintf(stderr, "CFHostSetClient failed\n");
            CFRelease(host);
        }
    } else {
        fprintf(stderr, "\"%s\" is not a valid IP address\n", ipAddress);
        success = false;
    }
    
    free(ipAddress);
    
    return success;
}



static void
MyPrintUsage(const char *executablePath)
{
    const char *programName;

    programName = strrchr(executablePath, '/');
    if (programName == NULL) programName = executablePath;
    else programName += 1;
        
    fprintf(stderr, "usage: %s -h www.example.com\n", programName);
    fprintf(stderr, "       %s -a 10.0.1.2\n",        programName);
    fprintf(stderr, "       %s -r www.example.com\n", programName);
    fprintf(stderr, "       %s -s 10.0.1.2\n",        programName);
}



int
main (int argc, const char * argv[])
{    
    CFStringRef  inputString;
    Boolean      success;
    int          operation;
    
    if (argc < 3) { MyPrintUsage(argv[0]); return 0; }

    operation = getopt(argc, (char * const *)argv, "hars");
    if (operation == -1) { MyPrintUsage(argv[0]); return 0; }
    
    inputString = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingASCII);
    assert(inputString != NULL);
            
    switch (operation) {
        case 'h':
            /* MyResolveNameToAddress retrieves the IP addresses for a hostname. */
            success = MyResolveNameToAddress(inputString);
            if (!success) fprintf(stderr, "MyResolveNameToAddress failed\n");
            break;
        case 'a':
            /* MyResolveAddressToName retrieves the hostname for an IP address.  You currently
            can't use this function with IPv6 link-local addresses. <rdar://problem/3621096> */
            success = MyResolveAddressToName(inputString);
            if (!success) fprintf(stderr, "MyResolveAddressToName failed\n");
            break;
        case 'r':
             /* MyCheckReachabilityByName checks the reachability of a hostname. */
            success = MyCheckReachabilityByName(inputString);
            if (!success) fprintf(stderr, "MyCheckReachabilityByName failed\n");
            break;
        case 's':
            /* MyCheckReachabilityByAddress checks the reachability of an IP address.
            Unfortunately, the callback never gets called because of a bug. <rdar://problem/3612320> */
            success = MyCheckReachabilityByAddress(inputString);
            if (!success) fprintf(stderr, "MyCheckReachabilityByAddress failed\n");
            break;
    }
    
    CFRelease(inputString);

    /* Start the run loop to receive asynchronous callbacks via MyCFHostClientCallBack. */
    if (success) CFRunLoopRun();
    
    return 0;
}