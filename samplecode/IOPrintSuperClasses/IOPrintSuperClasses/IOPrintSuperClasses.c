/*
    File:       IOPrintSuperClasses.c

    Contains:   A tool to print the super classes of an I/O Kit Class.

    Written by: DTS

    Copyright:  Copyright (c) 2005 by Apple Computer, Inc., All Rights Reserved.

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

$Log: IOPrintSuperClasses.c,v $
Revision 1.1  2005/08/03 13:37:47         
Update for version 1.1.  Substantial changes to adopt IOObjectCopySuperclassForClass.


*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>

// The declaration of IOObjectCopySuperclassForClass in "IOKitLib.h" is missing an 
// availability macro <rdar://problem/4203126>, which means that it's not automatically 
// marked as a weak import.  I've worked around this by declaring the routine myself, 
// with the correct availability macro, before including the system header.

CFStringRef
IOObjectCopySuperclassForClass(CFStringRef classname)
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

#include <IOKit/IOKitLib.h>

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Compatibility Approach, pre-10.4

static kern_return_t CopyClassDictionary(CFDictionaryRef *classDict)
    // Extract the dictionary of known classes from the 
    // "IOKitDiagnostics" property.
{
    kern_return_t       err;
    kern_return_t       junk;
    io_registry_entry_t root;
    CFDictionaryRef     diagDict;
    
    assert( classDict != NULL);
    assert(*classDict == NULL);
    
    diagDict = NULL;
    
    err = 0;
    root = IORegistryGetRootEntry(kIOMasterPortDefault);
    if (root == MACH_PORT_NULL) {
        err = -1;
    }

    if (err == 0) {
        diagDict = IORegistryEntryCreateCFProperty(root, CFSTR("IOKitDiagnostics"), NULL, 0);
        if (diagDict == NULL) {
            err = -2;
        }
    }
    
    if (err == 0) {
        *classDict = CFDictionaryGetValue(diagDict, CFSTR("Classes"));
        if (*classDict == NULL) {
            err = -3;
        } else {
            CFRetain(*classDict);
        }
    }

    if (root != MACH_PORT_NULL) {
        junk = IOObjectRelease(root);
        assert(junk == 0);
    }
    if (diagDict != NULL) {
        CFRelease(diagDict);
    }
    
    assert( (err == 0) == (*classDict != NULL) );
    
    return err;
}

struct PrintIfSuperClassContext {
    int             magic;          // kPrintIfSuperClassContextMagic
    io_service_t    service;
    const char *    className;
};
typedef struct PrintIfSuperClassContext PrintIfSuperClassContext;

static const int kPrintIfSuperClassContextMagic = 'mgik';

static void PrintIfSuperClass(const void *key, const void *value, void *contextUntyped)
    // CFDictionaryApplyFunction callback.  key is a CFString that
    // names one of the known classes on the system.  value is 
    // ignored.  context is a pointer to the function's parameters
    // (PrintIfSuperClassContext),  Determine if context->service, 
    // an object we found in the registry, "conforms to" the class 
    // named by key.
{
    PrintIfSuperClassContext *  context;
    CFStringRef                 thisClassStr;
    io_name_t                   thisClass;
    Boolean                     success;
    #pragma unused(value)
    
    assert(key != NULL);
    assert(contextUntyped != NULL);
    
    context = (PrintIfSuperClassContext *) contextUntyped;
    assert(context->magic == kPrintIfSuperClassContextMagic);
    
    thisClassStr = key;
    assert( CFGetTypeID(thisClassStr) == CFStringGetTypeID() );
    
    success = CFStringGetCString(thisClassStr, thisClass, sizeof(thisClass), kCFStringEncodingUTF8);
    assert(success);
    
    // The strcmp prevents us from claiming that an object is 
    // is a member of its own class, which is true but not interesting.
    
    if ( success && (strcmp(thisClass, context->className) != 0) ) {
        if ( IOObjectConformsTo(context->service, thisClass) ) {
            fprintf(stderr, "  %s\n", thisClass);
        }
    }
}

enum {
    kItemNotFound = 12345678
};

static kern_return_t FindInstanceOfClass(const char *className, io_object_t *foundObject)
    // He we find an instance of the specified class in the 
    // registry.  We don't use IOServiceGetMatchingService because 
    // that would find instances of subclasses of className.  Instead 
    // we iterate the entire registry, get the class of each 
    // object, and compare the strings directly.
{
    kern_return_t err;
    kern_return_t junk;
    io_iterator_t iter;
    io_object_t thisEntry;

    assert( foundObject != NULL);
    assert(*foundObject == MACH_PORT_NULL);
    
    iter = MACH_PORT_NULL;
    
    err = IORegistryCreateIterator(kIOMasterPortDefault, kIOServicePlane, kIORegistryIterateRecursively, &iter);
    if (err == 0) {
        boolean_t found;
        
        found = false;
        do {
            thisEntry = IOIteratorNext(iter);
            if (thisEntry != MACH_PORT_NULL) {
                io_name_t thisClassName;
                
                err = IOObjectGetClass(thisEntry, thisClassName);
                if (err == 0) {
                    found = (strcmp(thisClassName, className) == 0);
                }

                if (0) {
                    io_name_t name;
                    
                    junk = IORegistryEntryGetName(thisEntry, name);
                    assert(junk == 0);
                    fprintf(stderr, "name = %s, class = %s\n", name, thisClassName);
                }                

                if ( found ) {
                    *foundObject = thisEntry;
                } else {
                    junk = IOObjectRelease(thisEntry);
                    assert(junk == 0);
                }
            }
        } while (err == 0 && thisEntry != MACH_PORT_NULL && !found);
        
        if ( err == 0 && ! found) {
            err = kItemNotFound;
        }
    }
    
    if (iter != MACH_PORT_NULL) {
        junk = IOObjectRelease(iter);
        assert(junk == 0);
    }
    
    assert( (err == 0) == (*foundObject != MACH_PORT_NULL) );
    
    return err;
}

static void FindInstanceOfClassAndPrintSuperClasses(CFDictionaryRef classDict, const char *className)
    // This routine finds an instance of the specified class in the 
    // registry (className) and then goes through all of the known 
    // classes (classDict) to see which ones that instance 
    // "conforms to", ie which ones the instance's class is a 
    // subclass of.
{
    kern_return_t   err;
    kern_return_t   junk;
    io_service_t    foundService;
    
    assert(classDict != NULL);
    assert(className != NULL);
    
    foundService = MACH_PORT_NULL;
    
    err = FindInstanceOfClass(className, &foundService);
    if (err == 0) {
        fprintf(stderr, "Looking for superclasses of %s:\n", className);
    } else if (err == kItemNotFound) {
        fprintf(stderr, "No instances of '%s', can't look for superclasses.\n", className);
    }
    if (err == 0) {
        PrintIfSuperClassContext context;
        
        context.magic     = kPrintIfSuperClassContextMagic;
        context.service   = foundService;
        context.className = className;
        CFDictionaryApplyFunction(classDict, PrintIfSuperClass, &context);
    }

    if (foundService != MACH_PORT_NULL) {
        junk = IOObjectRelease(foundService);
        assert(junk == 0);
    }

    if (err != 0 && err != kItemNotFound) {
        fprintf(stderr, "Error 0x%08x finding superclasses of %s.\n", err, className);
    }   
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Direct Approach, 10.4 and later

static void PrintSuperClassesDirectly(const char *className)
    // This routine finds the superclasses of className directly, by 
    // calling IOObjectCopySuperclassForClass.
{
    CFStringRef     classStr;
    CFStringRef     classBundleStr;
    
    // Create a CFString for className
    
    classStr = CFStringCreateWithCString(NULL, className, kCFStringEncodingUTF8);
    assert(classStr != NULL);
    
    // Check that the class actually exists.  I do this because 
    // IOObjectCopySuperclassForClass can return NULL if the class has 
    // no super class *or* if the class is unknown.  I want to be 
    // able to distinguish between these, so I call 
    // IOObjectCopyBundleIdentifierForClass.  This should only return 
    // NULL if the class doesn't exist.  Once I know that the class 
    // exists, I can be assured that, if IOObjectCopySuperclassForClass 
    // returns NULL, it's because the class has no superclass.
    
    classBundleStr = IOObjectCopyBundleIdentifierForClass(classStr);
    if (classBundleStr == NULL) {
        fprintf(stderr, "Unknown class '%s'.\n", className);
    } else {
        CFStringRef     superClassStr;

        // Loop, printing each successive superclasses of the class.
        
        fprintf(stderr, "Superclasses of %s:\n", className);
    
        do {
            superClassStr = IOObjectCopySuperclassForClass(classStr);
            if (superClassStr == NULL) {
                break;
            } else {
                Boolean success;
                CFIndex superClassBufSize;
                char *  superClassBuf;
                
                // Convert the class CFString to a C string so that we can print it.
                
                superClassBufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(superClassStr), kCFStringEncodingUTF8) + 1;
                superClassBuf = malloc(superClassBufSize);
                assert(superClassBuf != NULL);
                
                success = CFStringGetCString(superClassStr, superClassBuf, superClassBufSize, kCFStringEncodingUTF8);
                assert(success);
                
                fprintf(stderr, "  %s\n", superClassBuf);
                
                free(superClassBuf);
                
                CFRelease(classStr);
                classStr = superClassStr;
            }
        } while (true);
        
        // We don't have to clean up superClassStr because it's always 
        // NULL by the time we leave the above loop.
        
        assert(superClassStr == NULL);
    }
    
    if (classBundleStr != NULL) {
        CFRelease(classBundleStr);
    }
    if (classStr != NULL) {
        CFRelease(classStr);
    }
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Tool Wrapper

static void PrintUsage(const char *argv0)
{
    const char *    progName;
    
    progName = strrchr(argv0, '/');
    if (progName == NULL) {
        progName = argv0;
    } else {
        progName += 1;
    }
    fprintf(stderr, "usage: [options] %s className...\n", progName);
    fprintf(stderr, "       -o  Always use old algorithm\n");
}

int main(int argc, char *argv[])
    // Print all of the super-classes of a given I/O Kit class using 
    // two different implementations depending on whether 
    // IOObjectCopySuperclassForClass (introduced in 10.4) is present.
{
    int     retVal;
    int     ch;
    bool    useOldAlgorithm;
    int     argIndex;

    // Parse options.
    
    useOldAlgorithm = false;
    
    retVal = EXIT_SUCCESS;
    do {
        ch = getopt(argc, argv, "o");
        if (ch != -1) {
            switch (ch) {
                case 'o':
                    useOldAlgorithm = true;
                    break;
                case '?':
                default:
                    PrintUsage(argv[0]);
                    retVal = EXIT_FAILURE;
                    break;
            }
        }
    } while (ch != -1);

    // Complain if we have no proper arguments.
    
    if ( (retVal == EXIT_SUCCESS) && (optind == argc) ) {
        PrintUsage(argv[0]);
        retVal = EXIT_FAILURE;
    }
    
    // Process each argument in turn.
    
    if (retVal == EXIT_SUCCESS) {
        if ( (IOObjectCopySuperclassForClass != NULL) && ! useOldAlgorithm ) {
            for (argIndex = optind; argIndex < argc; argIndex++) {
                PrintSuperClassesDirectly(argv[argIndex]);
                if (argIndex != (argc - 1)) {
                    fprintf(stderr, "\n");
                }
            }
            retVal = EXIT_SUCCESS;
        } else {
            kern_return_t   err;
            CFDictionaryRef classDict;

            classDict = NULL;

            // Extract a dictionary of all of the  classes from the IOKitDiagnostics 
            // property.  If there are multiple classes on the command line, we use 
            // this multiple times, so we get it once at the beginning.

            err = CopyClassDictionary(&classDict);
            if (err == 0) {
                for (argIndex = optind; argIndex < argc; argIndex++) {
                
                    // Do the real work in FindInstanceOfClassAndPrintSuperClasses.
                    // The basic algorithm is, for every class specified on the 
                    // command line, find an instance of that class in the IORegistry 
                    // and then go through every known class and see which ones 
                    // the instance "conforms to".
                    
                    FindInstanceOfClassAndPrintSuperClasses(classDict, argv[argIndex]);
                    if (argIndex != (argc - 1)) {
                        fprintf(stderr, "\n");
                    }
                }
            }

            if (classDict != NULL) {
                CFRelease(classDict);
            }
            
            if (err == 0) {
                retVal = EXIT_SUCCESS;
            } else {
                fprintf(stderr, "Failed with error 0x%08x.\n", err);
                retVal = EXIT_FAILURE;
            }
        }
    }

    return retVal;
}