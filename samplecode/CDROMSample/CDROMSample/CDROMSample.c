/*
	File:			CDROMSample.c
	
	Description:	This sample demonstrates how to use IOKitLib to find CD-ROM media mounted on the
					system. It also shows how to open, read raw sectors from, and close the drive.
                
	Copyright:		© Copyright 2000-2005 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:		IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
        
            <5>		08/17/05		Updated to produce a universal binary. Use kIOMasterPortDefault
									instead of older IOMasterPort function.
			<4>		10/17/02		Changed to use updated headers and I/O Registry
									keys. This sample now requires Mac OS X 10.1 or later.
            <3>		11/21/01		Changed to use raw disk node (/dev/rdisk*)
            <2>	 	02/08/01		Updated for Mac OS X 10.0.
            <1>	 	08/16/00		New sample.
        
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/storage/IOMediaBSDClient.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IOCDTypes.h>
#include <CoreFoundation/CoreFoundation.h>

#ifndef IO_OBJECT_NULL
// This macro is defined in <IOKit/IOTypes.h> starting with Mac OS X 10.4.
#define	IO_OBJECT_NULL ((io_object_t) 0)
#endif

static kern_return_t FindEjectableCDMedia(io_iterator_t *mediaIterator);
static kern_return_t GetBSDPath(io_iterator_t mediaIterator, char *bsdPath, CFIndex maxPathSize);
static int OpenDrive(const char *bsdPath);
static Boolean ReadSector(int fileDescriptor);
static void CloseDrive(int fileDescriptor);

// Returns an iterator across all CD media (class IOCDMedia). Caller is responsible for releasing
// the iterator when iteration is complete.
kern_return_t FindEjectableCDMedia(io_iterator_t *mediaIterator)
{
    kern_return_t			kernResult; 
    CFMutableDictionaryRef	classesToMatch;
        
/*! @function IOServiceMatching
    @abstract Create a matching dictionary that specifies an IOService class match.
    @discussion A very common matching criteria for IOService is based on its class. IOServiceMatching will create a matching dictionary that specifies any IOService of a class, or its subclasses. The class is specified by C-string name.
    @param name The class name, as a const C-string. Class matching is successful on IOService's of this class or any subclass.
    @result The matching dictionary created, is returned on success, or zero on failure. The dictionary is commonly passed to IOServiceGetMatchingServices or IOServiceAddNotification which will consume a reference, otherwise it should be released with CFRelease by the caller. */

    // CD media are instances of class kIOCDMediaClass
    classesToMatch = IOServiceMatching(kIOCDMediaClass); 
    if (classesToMatch == NULL) {
        printf("IOServiceMatching returned a NULL dictionary.\n");
    }
    else {
/*!
	@function CFDictionarySetValue
	Sets the value of the key in the dictionary.
	@param theDict The dictionary to which the value is to be set. If this
		parameter is not a valid mutable CFDictionary, the behavior is
		undefined. If the dictionary is a fixed-capacity dictionary and
		it is full before this operation, and the key does not exist in
		the dictionary, the behavior is undefined.
	@param key The key of the value to set into the dictionary. If a key 
		which matches this key is already present in the dictionary, only
		the value is changed ("add if absent, replace if present"). If
		no key matches the given key, the key-value pair is added to the
		dictionary. If added, the key is retained by the dictionary,
		using the retain callback provided
		when the dictionary was created. If the key is not of the sort
		expected by the key retain callback, the behavior is undefined.
	@param value The value to add to or replace into the dictionary. The value
		is retained by the dictionary using the retain callback provided
		when the dictionary was created, and the previous value if any is
		released. If the value is not of the sort expected by the
		retain or release callbacks, the behavior is undefined.
*/

		CFDictionarySetValue(classesToMatch, CFSTR(kIOMediaEjectableKey), kCFBooleanTrue); 
        // Each IOMedia object has a property with key kIOMediaEjectableKey which is true if the
        // media is indeed ejectable. So add this property to the CFDictionary we're matching on. 
    }
    
    /*! @function IOServiceGetMatchingServices
        @abstract Look up registered IOService objects that match a matching dictionary.
        @discussion This is the preferred method of finding IOService objects currently registered by IOKit. IOServiceAddNotification can also supply this information and install a notification of new IOServices. The matching information used in the matching dictionary may vary depending on the class of service being looked up.
        @param masterPort The master port obtained from IOMasterPort().
        @param matching A CF dictionary containing matching information, of which one reference is consumed by this function. IOKitLib can contruct matching dictionaries for common criteria with helper functions such as IOServiceMatching, IOOpenFirmwarePathMatching.
        @param existing An iterator handle is returned on success, and should be released by the caller when the iteration is finished.
        @result A kern_return_t error code. */

    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, mediaIterator);    
    if (KERN_SUCCESS != kernResult) {
        printf("IOServiceGetMatchingServices returned 0x%08x\n", kernResult);
    }
    
    return kernResult;
}
    
// Given an iterator across a set of CD media, return the BSD path to the
// next one. If no CD media was found the path name is set to an empty string.
kern_return_t GetBSDPath(io_iterator_t mediaIterator, char *bsdPath, CFIndex maxPathSize)
{
    io_object_t		nextMedia;
    kern_return_t	kernResult = KERN_FAILURE;
    
    *bsdPath = '\0';
    
    /*! @function IOIteratorNext
    @abstract Returns the next object in an iteration.
    @discussion This function returns the next object in an iteration, or zero if no more remain or the iterator is invalid.
    @param iterator An IOKit iterator handle.
    @result If the iterator handle is valid, the next element in the iteration is returned, otherwise zero is returned. */

    nextMedia = IOIteratorNext(mediaIterator);
    if (nextMedia) {
        CFTypeRef	bsdPathAsCFString;
        
/*! @function IORegistryEntryCreateCFProperty
    @abstract Create a CF representation of a registry entry's property.
    @discussion This function creates an instantaneous snapshot of a registry entry property, creating a CF container analogue in the caller's task. Not every object available in the kernel is represented as a CF container; currently OSDictionary, OSArray, OSSet, OSSymbol, OSString, OSData, OSNumber, OSBoolean are created as their CF counterparts. 
    @param entry The registry entry handle whose property to copy.
    @param key A CFString specifying the property name.
    @param allocator The CF allocator to use when creating the CF container.
    @param options No options are currently defined.
    @result A CF container is created and returned the caller on success. The caller should release with CFRelease. */

        bsdPathAsCFString = IORegistryEntryCreateCFProperty(nextMedia, 
															CFSTR(kIOBSDNameKey), 
															kCFAllocatorDefault, 
															0);
        if (bsdPathAsCFString) {
            size_t devPathLength;
            
            strcpy(bsdPath, _PATH_DEV);
            
            // Add "r" before the BSD node name from the I/O Registry to specify the raw disk
            // node. The raw disk nodes receive I/O requests directly and do not go through
            // the buffer cache.
            
            strcat(bsdPath, "r");
            
            devPathLength = strlen(bsdPath);
            
            if (CFStringGetCString(bsdPathAsCFString,
								   bsdPath + devPathLength,
								   maxPathSize - devPathLength, 
								   kCFStringEncodingUTF8)) {
                printf("BSD path: %s\n", bsdPath);
                kernResult = KERN_SUCCESS;
            }
            
            CFRelease(bsdPathAsCFString);
        }

/*! @function IOObjectRelease
    @abstract Releases an object handle previously returned by IOKitLib.
    @discussion All objects returned by IOKitLib should be released with this function when access to them is no longer needed. Using the object after it has been released may or may not return an error, depending on how many references the task has to the same object in the kernel.
    @param object The IOKit object to release.
    @result A kern_return_t error code. */
    
        IOObjectRelease(nextMedia);
    }
    
    return kernResult;
}

// Given the path to a CD drive, open the drive.
// Return the file descriptor associated with the device.
int OpenDrive(const char *bsdPath)
{
    int	fileDescriptor;
    
    // This open() call will fail with a permissions error if the sample has been changed to
    // look for non-removable media. This is because device nodes for fixed-media devices are
    // owned by root instead of the current console user.
	//
	// The recommended way to handle the permissions issue is described in "Performing Privileged Operations With
	// Authorization Services":
	// <http://developer.apple.com/documentation/Security/Conceptual/authorization_concepts/index.html>
	
    fileDescriptor = open(bsdPath, O_RDONLY);
    
    if (fileDescriptor == -1) {
        printf("Error opening device %s: ", bsdPath);
        perror(NULL);
    }
    
    return fileDescriptor;
}

// Given the file descriptor for a whole-media CD device, read a sector from the drive.
// Return true if successful, otherwise false.
Boolean ReadSector(int fileDescriptor)
{
    char		*buffer;
    ssize_t		numBytes;
    u_int32_t	blockSize;
    
    // This ioctl call retrieves the preferred block size for the media. It is functionally
    // equivalent to getting the value of the whole media object's "Preferred Block Size"
    // property from the IORegistry.
    if (ioctl(fileDescriptor, DKIOCGETBLOCKSIZE, &blockSize)) {
        perror("Error getting preferred block size");
        
        // Set a reasonable default if we can't get the actual preferred block size. A real
        // app would probably want to bail at this point.
        blockSize = kCDSectorSizeCDDA;
    }
    
    printf("Media has block size of %d bytes.\n", blockSize);
    
    // Allocate a buffer of the preferred block size. In a real application, performance
    // can be improved by reading as many blocks at once as you can.
    buffer = malloc(blockSize);
    
    // Do the read. Note that we use read() here, not fread(), since this is a raw device
    // node.
    numBytes = read(fileDescriptor, buffer, blockSize);
        
    // Free our buffer. Of course, a real app would do something useful with the data first.
    free(buffer);
    
    return numBytes == blockSize ? true : false;
}

// Given the file descriptor for a device, close that device.
void CloseDrive(int fileDescriptor)
{
    close(fileDescriptor);
}

int main(void)
{
    kern_return_t	kernResult; // on PowerPC this is an int (4 bytes)
/*
 *	error number layout as follows (see mach/error.h):
 *
 *	hi		 		       lo
 *	| system(6) | subsystem(12) | code(14) |
 */

    io_iterator_t	mediaIterator;
    char			bsdPath[ MAXPATHLEN ];
 
    kernResult = FindEjectableCDMedia(&mediaIterator);
    
    kernResult = GetBSDPath(mediaIterator, bsdPath, sizeof(bsdPath));
    
    // Now open the device we found, read a sector, and close the device
    if (bsdPath[0] != '\0') {
        int fileDescriptor;
        
        fileDescriptor = OpenDrive(bsdPath);
        
        if (ReadSector(fileDescriptor)) {
            printf("Sector read successfully.\n");
        }
        else {
            printf("Could not read sector.\n");
        }
            
        CloseDrive(fileDescriptor);
        printf("Device closed.\n");
    }
    else {
        printf("No ejectable CD media found.\n");
    }

    // Release the iterator.
    if (mediaIterator) {
        IOObjectRelease(mediaIterator);
		mediaIterator = IO_OBJECT_NULL; // prevent us from inadvertently using the stale iterator later on
    }
        
    return 0;
}
