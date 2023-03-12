/*
	File:		Tool.c

	Contains:	Bundle loading test tool.

	Written by:	DTS

	Copyright:	Copyright (c) 2005 by Apple Computer, Inc., All Rights Reserved.

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

$Log: Tool.c,v $
Revision 1.3  2005/07/22 15:54:14         
Support a universal binary bundle; this involves a complicated workaround for <radr://problem/4189935>.  See the routine WorkAroundRadarID4189935 for details.

Revision 1.2  2005/03/16 16:54:42         
It's not necessary to mprotect the memory we pass to NSCreateObjectFileImageFromMemory; dyld doesn't run the code from that buffer, but rather uses it as a backing store for the real running image.  Also, only pass NSLINKMODULE_OPTION_BINDNOW to NSLinkModule in the debug case.

Revision 1.1  2005/03/10 17:33:23         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Frameworks

#include <CoreServices/CoreServices.h>

// Standard C includes.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <mach/mach.h>

// Dynamic linker (dyld) stuff

#include <mach-o/arch.h>
#include <mach-o/fat.h>
#include <mach-o/dyld.h>

/////////////////////////////////////////////////////////////////

// Definitions for the bundle entry point.

#define kBundleEntryPointName "HelloCruelWorld"
typedef void (*EntryPoint)(const char *message);

/////////////////////////////////////////////////////////////////

static void DoCFBundleTest(const char *pathToBundle)
	// Load and call the bundle the easy way, via CFBundle.
{
	CFURLRef	u;
	CFBundleRef b;
	EntryPoint  f;

	u = NULL;
	b = NULL;
	
	u = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) pathToBundle, strlen(pathToBundle), true);
	if (u == NULL) {
		fprintf(stderr, "Could not create URL.\n");
	} else {
		b = CFBundleCreate(NULL, u);
		if (b == NULL) {
			fprintf(stderr, "Could not create bundle.\n");
		} else {
			f = (EntryPoint) CFBundleGetFunctionPointerForName(b, CFSTR(kBundleEntryPointName));
			if (f == NULL) {
				fprintf(stderr, "Could not get entry point.\n");
			} else {
				f("... from CFBundle");
			}
		}
	}
	
	if (b != NULL) {
		CFRelease(b);
	}
	if (u != NULL) {
		CFRelease(u);
	}
}

static int ReadFileIntoPageAlignedBuffer(const char *filePath, void **bufPtr, size_t *bufSizePtr)
	// Read a file into a page aligned buffer.  The caller is responsible 
	// for freeing the buffer with vm_deallocate (see the comment next to 
    // our call to NSDestroyObjectFileImage (below) for an explanation of why 
    // we use Mach calls directly).
{
	int				err;
	void *			buf;
	off_t			fileSize;
	size_t			bufSize;
	int				fd;
	size_t			bytesRead;
	int				junk;
	
	assert(filePath != NULL);
	assert(bufPtr != NULL);
	assert(*bufPtr == NULL);
	assert(bufSizePtr != NULL);

	buf = NULL;
	
	// Open the file.
	
	err = 0;
	fd = open(filePath, O_RDONLY);
	if (fd < 0) {
		err = errno;
	}
	
	// Get its length, checking that the cast from 
	// off_t to size_t doesn't drop information (that is, 
	// the file is too large for the address space).
	
	if (err == 0) {
		fileSize = lseek(fd, 0, SEEK_END);
		if (fileSize < 0) {
			err = errno;
		}
	}
	if (err == 0) {
		bufSize = (size_t) fileSize;
		
		if ( ((off_t) bufSize) != fileSize ) {
			err = EFBIG;
		}
	}

	// Allocate a buffer and mark it as executable.  We must allocate the memory 
    // using vm_allocate, for reasons that are explained below 
    // (in the comment next to our call to NSDestroyObjectFileImage).
	
	if (err == 0) {
        err = vm_allocate(mach_task_self(), (vm_address_t *) &buf, bufSize, true);
	}

	// Read into the buffer.

	if (err == 0) {
		bytesRead = pread(fd, buf, bufSize, 0);
		if (bytesRead < 0) {
			err = errno;
		} else if (bytesRead != bufSize) {
			err = EPIPE;
		}
	}
    
	// Clean up.
	
	if (fd != -1) {
		junk = close(fd);
		assert(junk == 0);
	}
	if (err != 0) {
		free(buf);
		buf = NULL;
		bufSize = 0;
	}
	*bufPtr = buf;
	*bufSizePtr = bufSize;
	
	assert( (err == 0) == (*bufPtr != NULL) );

	return err;
}

static Boolean GetBundleExecutable(const char *pathToBundle, char *buf, size_t bufLen)
	// Get the executable path for the specified bundle.  We do this using 
	// CFBundle APIs to avoid having to hard-code things like "Contents/MacOS".
{
	Boolean		ok;
	CFURLRef	u;
	CFBundleRef	b;
	CFURLRef	u2;
	
	u = NULL;
	b = NULL;
	u2 = NULL;

	// Create a bundle from the path.
	
	ok = true;
	u = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) pathToBundle, strlen(pathToBundle), true);
	if (u == NULL) {
		ok = false;
	}
	if (ok) {
		b = CFBundleCreate(NULL, u);
		if (b == NULL) {
			ok = false;
		}
	}
	
	// Ask the bundle for the path to the executable.
	
	if (ok) {
		u2 = CFBundleCopyExecutableURL(b);
		if (u2 == NULL) {
			ok = false;
		}
	}
	if (ok) {
		ok = CFURLGetFileSystemRepresentation(u2, true, (UInt8 *) buf, bufLen);
	}

	// Clean up.
	
	if (u != NULL) {
		CFRelease(u);
	}
	if (b != NULL) {
		CFRelease(b);
	}
	if (u2 != NULL) {
		CFRelease(u2);
	}
	return ok;
}

static NSObjectFileImageReturnCode WorkAroundRadarID4189935(
    void **             codeAddrPtr, 
    size_t *            codeSizePtr, 
    NSObjectFileImage * ofiPtr
)
    // Mac OS X 10.4 introduced a bug <rdar://problem/4189935> that causes  
    // NSCreateObjectFileImageFromMemory to fail if you point it at a fat 
    // Mach-O file.  This prevents our universal binary from working.
    // To work around this, we look in the fat Mach-O file, find the best 
    // architecture to use, extract the code for that architecture, and 
    // call NSCreateObjectFileImageFromMemory on that code.
    //
    // A couple of things to note:
    //
    // o This routine allocates a new buffer to hold the specific architecture's 
    //   code; on successful return, it will replace the values in *codeAddrPtr 
    //   and *codeSizePtr with values representing the new buffer.  The old 
    //   buffer is disposed.
    //
    // o The data in a fat Mach-O file header is always big endian; we must 
    //   convert the data to host byte ordering before we look at it.  We 
    //   do this in place, thus modifying the buffer described by *codeAddrPtr 
    //   and *codeSizePtr.  Architecturally this is kinda bogus, but it's 
    //   a reasonable given that this code is intended to be a short-lived 
    //   workaround.
{
    int                         err;
    int                         junk;
    NSObjectFileImageReturnCode dyldErr;
    struct fat_header *         fatHeader;
    struct fat_arch *           fatArchArray;
    uint32_t                    archIndex;
    struct fat_arch *           bestFatArch;
    const NXArchInfo *          ourArch;
    void *                      newCodeAddr;
    uint32_t                    newCodeSize;
    
    assert( codeAddrPtr != NULL);
    assert(*codeAddrPtr != NULL);
    assert( codeSizePtr != NULL);
    assert(*codeSizePtr != 0);
    assert( ofiPtr != NULL);
    assert(*ofiPtr == NULL);
    
    fprintf(stderr, "Attempt to work around <rdar://problem/4189935>...\n");
    
    newCodeAddr = NULL;
    
    assert( *codeSizePtr >= sizeof(*fatHeader) );
    
    // Convert fat header to host byte order.
    
    fatHeader = *codeAddrPtr;
    fatHeader->magic     = OSSwapBigToHostInt32(fatHeader->magic);
    fatHeader->nfat_arch = OSSwapBigToHostInt32(fatHeader->nfat_arch);
    
    assert(fatHeader->magic == FAT_MAGIC);
    assert(fatHeader->nfat_arch > 0);
    assert( *codeSizePtr >= (sizeof(*fatHeader) + (sizeof(*fatArchArray) * fatHeader->nfat_arch)) );

    // Convert each element of the fat arch array to host byte order.

    fatArchArray = (struct fat_arch *) (fatHeader + 1);
    for (archIndex = 0; archIndex < fatHeader->nfat_arch; archIndex++) {
        fatArchArray[archIndex].cputype    = OSSwapBigToHostInt32(fatArchArray[archIndex].cputype);
        fatArchArray[archIndex].cpusubtype = OSSwapBigToHostInt32(fatArchArray[archIndex].cpusubtype);
        fatArchArray[archIndex].offset     = OSSwapBigToHostInt32(fatArchArray[archIndex].offset);
        fatArchArray[archIndex].size       = OSSwapBigToHostInt32(fatArchArray[archIndex].size);
        fatArchArray[archIndex].align      = OSSwapBigToHostInt32(fatArchArray[archIndex].align);
    }
    
    // Get the currently running architecture.
    
    ourArch = NXGetLocalArchInfo();
    assert(ourArch != NULL);

    // Find the best match within the bundle's list of architectures.
    
    bestFatArch = NXFindBestFatArch(
        ourArch->cputype,
        ourArch->cpusubtype,
        fatArchArray,
        fatHeader->nfat_arch
    );

    // Create a new buffer with a copy of the best match.
    
    if (bestFatArch == NULL) {
        fprintf(stderr, "There is no appropriate architecture within the fat file.\n");
        err = EINVAL;
    } else {
        // We don't handle special alignments.  If the code we're going to use needs 
        // to be more aligned that page aligned, we're in trouble.
        
        assert( (1 << bestFatArch->align) <= getpagesize() );
        
        // The code we're going to use must actually be within the code buffer, 
        // otherwise we're really in the weeds.
        
        assert(bestFatArch->size <= *codeSizePtr);
        assert(bestFatArch->offset <= *codeSizePtr);
        assert( (bestFatArch->size + bestFatArch->offset) <= *codeSizePtr );
        
        newCodeSize = bestFatArch->size;
        err = (int) vm_allocate(mach_task_self(), (vm_address_t *) &newCodeAddr, newCodeSize, true);
        if (err == 0) {
            memcpy(newCodeAddr, ((char *) (*codeAddrPtr)) + bestFatArch->offset , newCodeSize);
        }
    }
    
    // Clean up.

    if (err == 0) {
        // Success!  Dispose of the old code buffer and replace it with the new one.

        junk = (int) vm_deallocate(mach_task_self(), (vm_address_t) *codeAddrPtr, *codeSizePtr);
        assert(junk == 0);
        
        *codeAddrPtr = newCodeAddr;
        *codeSizePtr = newCodeSize;
        
        dyldErr = NSObjectFileImageSuccess;
    } else {
        // The last thing we do that can error is the vm_allocate, thus there's no need to 
        // clean up the memory allocation on failure because there can be no failures after 
        // the allocation.  However, just to be future proof, I check this assumption here.
        
        assert(newCodeAddr == NULL);
        
        dyldErr = NSObjectFileImageFailure;
    }

    // Finally, run the new code through dyld.
    
    if (dyldErr == NSObjectFileImageSuccess) {
        dyldErr = NSCreateObjectFileImageFromMemory(
            *codeAddrPtr,
            *codeSizePtr,
            ofiPtr
        );
    }

    if (dyldErr == NSObjectFileImageSuccess) {
        fprintf(stderr, "... succeeded.\n");
    } else {
        fprintf(stderr, "... failed.\n");
    }
    
    assert( (dyldErr == NSObjectFileImageSuccess) == (*ofiPtr != NULL) );

    return dyldErr;
}

static void DoNSTest(const char *pathToBundle, Boolean loadManually)
	// Load and call the bundle the hard way, using dyld routines. 
	// loadManually controls whether we load the code into our own 
	// buffer (true), or whether we tell dyld about the file and let it load 
	// the code itself (false).
{
    int                         junk;
	char						codePath[1024];
	void *						codeAddr;
	size_t						codeSize;
	const char *				moduleName;
	const char *				message;
	NSObjectFileImageReturnCode dyldErr;
	NSObjectFileImage			ofi;
	enum DYLD_BOOL				ok;
	NSModule					m;
	NSSymbol					s;
	EntryPoint					f;
	
	codeAddr = NULL;
	ofi = NULL;
	m = NULL;
	
	// Get the path to the code within the bundle.

	ok = GetBundleExecutable(pathToBundle, codePath, sizeof(codePath));
	if ( ! ok ) {
		fprintf(stderr, "Could not locate executable with '%s'.", pathToBundle);
	} else {
		if (loadManually) {
			int				err;

			// Manually read the code file into a memory buffer that 
			// we allocate ourself, and then work from that.
			
			// Set moduleName for the call to NSLinkModule.
			
			moduleName = "[Memory Based Bundle]";
			message = "... from NSCreateObjectFileImageFromMemory";
			
			// Read the code into a memory buffer.  dyld wants the code to be 
			// page aligned, so we guarantee that as well.
			
			err = ReadFileIntoPageAlignedBuffer(codePath, &codeAddr, &codeSize);
			if (err != 0) {
				fprintf(stderr, "Error %d accessing '%s'.", err, codePath);
				dyldErr = NSObjectFileImageFailure;
			} else {			
				// Create the object file image from the memory buffer.
				
				// fprintf(stderr, "codeAddr = %p\n", codeAddr);
				
				dyldErr = NSCreateObjectFileImageFromMemory(
					codeAddr,
					codeSize,
					&ofi
				);

                // Yowsers!  NSCreateObjectFileImageFromMemory doesn't handle fat 
                // Mach-O files on 10.4.x <rdar://problem/4189935>, which means that we 
                // can't load our universal binary bundle directly.  We work around this 
                // by manually extracting the correct code from the fat Mach-O file.

                if (    (dyldErr == NSObjectFileImageFailure) 
                     && ( OSSwapBigToHostInt32(((const struct fat_header *) codeAddr)->magic) == FAT_MAGIC )
                   ) {
                    dyldErr = WorkAroundRadarID4189935(&codeAddr, &codeSize, &ofi);
                }
			}
		} else {
			// Working directly from the code file.

			// Set moduleName for the call to NSLinkModule.

			moduleName = codePath;
			message = "... from NSCreateObjectFileImageFromFile";

			// Create the object file image directly from the file.
			
			dyldErr = NSCreateObjectFileImageFromFile(
				codePath,
				&ofi
			);
		}

		if (dyldErr != NSObjectFileImageSuccess) {
			fprintf(stderr, "Could not create object file image.\n");
		} else {
            unsigned long options;

			// Link the module.  During development you might specify 
            // NSLINKMODULE_OPTION_BINDNOW (which forces dyld to link 
            // the module now, rather than on demand) to shake out any 
            // dependency problems, but things load faster if you don't 
            // include this flag in production code.
            
            options = NSLINKMODULE_OPTION_PRIVATE				// don't publish the bundle's exports to the global namespace
				    | NSLINKMODULE_OPTION_RETURN_ON_ERROR;		// return, rather than abort(), or error
            #if !defined(NDEBUG)
                options |= NSLINKMODULE_OPTION_BINDNOW;         // link the module now, rather than on demand
            #endif
			m = NSLinkModule(ofi, moduleName, options);
			
			// Look up and call the symbol.  Note that we have to prefix the 
			// name with an "_" because all C routines are passed to the 
			// linker with an "_" prefix, and dyld exposes us to this low-level 
			// detail.
			
			if (m == NULL) {
				fprintf(stderr, "Could not link module.\n");
			} else {
				s = NSLookupSymbolInModule(m, "_" kBundleEntryPointName);
				if (s == NULL) {
					fprintf(stderr, "Could not lookup symbol.\n");
				} else {
					f = NSAddressOfSymbol(s);
					if (f == NULL) {
						fprintf(stderr, "Could not get address of symbol.\n");
					} else {
						f(message);
					}
				}
			}
		}
	}
	
	if (m != NULL) {
		// NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED sounds promising, but it's not 
		// needed.  It affects dyld's remapping of original data, not the original 
		// data itself (ie the buffer at codeAddr),
		//
		// NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES is irrelevant because 
		// module is linked private.

		ok = NSUnLinkModule(m, NSUNLINKMODULE_OPTION_NONE);
		assert(ok);
	}
	if (ofi != NULL) {
		ok = NSDestroyObjectFileImage(ofi);
		assert(ok);
        
        // The buffer that you pass to NSCreateObjectFileImageFromMemory must be 
        // allocated using vm_allocate because NSCreateObjectFileImageFromMemory will 
        // dispose of that buffer using vm_deallocate!  Also, to avoid a double dispose, 
        // we NULL out codeAddr here, so that the vm_deallocate at the end of this 
        // function doesn't run.
        
        codeAddr = NULL;
	}
    if (codeAddr != NULL) {
        junk = (int) vm_deallocate(mach_task_self(), (vm_address_t) codeAddr, codeSize);
        assert(junk == 0);
    }
}

static void PrintUsage(void)
{
	fprintf(stderr, "MemoryBasedBundle ( -cf | -ns | -nsmem ) PathToBundle\n");
}

int main (int argc, const char * argv[])
{
	if (argc != 3) {
		PrintUsage();
		exit(EXIT_FAILURE);
	}
	
	if ( strcmp(argv[1], "-cf") == 0 ) {
		DoCFBundleTest(argv[2]);
	} else if ( strcmp(argv[1], "-ns") == 0 ) {
		DoNSTest(argv[2], false);
	} else if ( strcmp(argv[1], "-nsmem") == 0 ) {
		DoNSTest(argv[2], true);
	} else {
		PrintUsage();
		exit(EXIT_FAILURE);
	}
	
    return EXIT_SUCCESS;
}
