/*
	File:		EI_GraphicsImport.c
	
	Description: QuickTime graphics importer component

	Author:		QuickTime Engineering
	
	Copyright: 	© Copyright 1999-2003 Apple Computer, Inc. All rights reserved.
	
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
	   <2>		12/09/00	dts			updated for X
	   <1>	 	11/28/99	QTE			first file
*/

#if TARGET_OS_WIN32
    #include "EIComponentWindowsPrefix.h"
#endif

#if __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#else
    #include <Endian.h>
    #include <ImageCompression.h>
    #include <Movies.h>
    #include <Script.h>
    #include <TextUtils.h>
#endif

#include "EI_Image.h"
#include "EI_MakeImageDescription.h"
#include "EI_GraphicsImportVersion.h"

// Component globals
typedef struct {
	ComponentInstance		self;
	ComponentInstance		parent;
	ComponentInstance		delegateComponent;
} EI_GraphicsImportGlobalsRecord, *EI_GraphicsImportGlobals;

// Setup required for ComponentDispatchHelper.c
#define GRAPHICSIMPORT_BASENAME()	EI_GraphicsImport
#define GRAPHICSIMPORT_GLOBALS() 	EI_GraphicsImportGlobals storage

#define CALLCOMPONENT_BASENAME()	GRAPHICSIMPORT_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		GRAPHICSIMPORT_GLOBALS()

#define COMPONENT_UPP_PREFIX()		uppGraphicsImport
#define COMPONENT_DISPATCH_FILE		"EI_GraphicsImportDispatch.h"
#define COMPONENT_SELECT_PREFIX()  	kGraphicsImport

#define	GET_DELEGATE_COMPONENT()	(storage->delegateComponent)

#if __MACH__
	#include <CoreServices/Components.k.h>
	#include <QuickTime/ImageCompression.k.h>
	#include <QuickTime/ComponentDispatchHelper.c>
#else
	#include <Components.k.h>
	#include <ImageCompression.k.h>
	#include <ComponentDispatchHelper.c>
#endif

// Component Open Request - Required
pascal ComponentResult EI_GraphicsImportOpen(EI_GraphicsImportGlobals store, ComponentInstance self)
{
	ComponentResult err = noErr;

	// Allocate memory for our globals, set them up and inform the component manager that we've done so
	store = (EI_GraphicsImportGlobals)NewPtrClear(sizeof(EI_GraphicsImportGlobalsRecord));
	if (err = MemError()) goto bail;
	
	store->self = self;
	store->parent = self;

	SetComponentInstanceStorage(self, (Handle)store);

	// Open and target an instance of the base graphic importer as we delegate
	// most of our calls to the base importer instance
	err = OpenADefaultComponent(GraphicsImporterComponentType, GraphicsImporterComponentType, &store->delegateComponent);
	if (err) goto bail;
	
	err = ComponentSetTarget(store->delegateComponent, self);
	if (err) goto bail;

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult EI_GraphicsImportClose(EI_GraphicsImportGlobals store, ComponentInstance self)
{
#pragma unused(self)
	
	// Make sure to close the base component and dealocate our storage
	if (store) {
		if (store->delegateComponent)
			CloseComponent(store->delegateComponent);

		DisposePtr((Ptr)store);
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult EI_GraphicsImportVersion(EI_GraphicsImportGlobals store)
{
#pragma unused(store)

	return kEI_GraphicsImportVersion;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component)
pascal ComponentResult EI_GraphicsImportTarget(EI_GraphicsImportGlobals store, ComponentInstance target)
{
	store->parent = target;
	
	return noErr;
}

#pragma mark-

// GraphicsImportGetImageDescription
//		Examine the image file and construct an image description for it.
// The base importer uses this image description to respond to other calls such as GraphicsImportGetNaturalBounds and
// GraphicsImportDraw. In the case of GraphicsImportDraw, the image description is passed to the Image Compression
// Manager, so the cType field must identify a codec that will be able to draw the image. If a graphics importer needs to pass
// extra information that the codec will need at PreDecompress time, it can pass it in an image description extension.
pascal ComponentResult EI_GraphicsImportGetImageDescription(EI_GraphicsImportGlobals store, ImageDescriptionHandle *descOut)
{
	ComponentResult err = noErr;
	ImageFrame frame;
	unsigned long offset, size;
	short colorCount;
	Ptr colors = NULL;

	if (!descOut)
		return paramErr;

	// Get the offset and size of the image data
	err = GraphicsImportGetDataOffsetAndSize(store->parent, &offset, &size);
	if (err) goto bail;

	// Read some data, in this case an ImageFrame (this is specific to the Electric Image Format)
	err = GraphicsImportReadData(store->parent, &frame, offset, sizeof(frame));
	if (err) goto bail;

	// framePalettes contains the number of entries in the color table
	colorCount = EndianU16_BtoN(frame.framePalettes);

	// Alocate some storage for the color table and get it
	colors = NewPtr(3 * colorCount);
	if (err = MemError()) goto bail;

	err = GraphicsImportReadData(store->parent, colors, offset + sizeof(frame), 3 * colorCount);
	if (err) goto bail;

	// Create the Image Description
	err = EI_MakeImageDescription(&frame, colorCount, (UInt8 *)colors, descOut);
	if (err) goto bail;

bail:
	if (colors)
		DisposePtr(colors);

	return err;	
}

// GraphicsImportGetDataOffsetAndSize
// 		Returns the offset and size of the compressed image data within a file.
// Sometimes, the data to be passed to the image decompressor is only a portion of the file. In these cases, implement
// the GraphicsImportGetDataOffsetAndSize function to indicate the byte range to send to the image decompressor.
// It is often useful for this call to first call the base importer's implementation to find out the size of the input data stream.
pascal ComponentResult EI_GraphicsImportGetDataOffsetAndSize(EI_GraphicsImportGlobals store, unsigned long *offsetOut, unsigned long *sizeOut)
{
	ComponentResult err = noErr;
	ImageHeader header;
	ImageFrame frame;
	unsigned long offset, size;

	// Call the base importer first
	err = GraphicsImportGetDataOffsetAndSize(store->delegateComponent, &offset, &size);
	if (err) goto bail;

	// Read some data
	err = GraphicsImportReadData(store->parent, &header, offset, sizeof(header));
	if (err) goto bail;

	// If we don't like what we see bail!
	if ((EndianU16_BtoN(header.imageVersion) != 5) || (EndianU32_BtoN(header.imageFrames) < 1)) {
		err = codecBadDataErr;
		goto bail;
	}

	// Read some more data
	err = GraphicsImportReadData(store->parent, &frame, offset + sizeof(header), sizeof(frame));
	if (err) goto bail;

	// Set the correct offset into the image and adjust the size appropriately
	if (offsetOut) *offsetOut = offset + sizeof(ImageHeader);
	if (sizeOut) *sizeOut = sizeof(ImageFrame) + (EndianU16_BtoN(frame.framePalettes) * 3) + EndianU32_BtoN(frame.frameSize);

bail:
	return err;
}

// GraphicsImportValidate
// 		Attempt to ascertain quickly whether a file matches the importer's format.
// This is especially useful for formats which start with identifying codes or "magic numbers" (such as PNG and TIFF)
// in situations where image files do not have correct file types or suffixes. In situations like this, the Image Compression
// Manager may ask many graphics importers in turn to validate until it finds one that accepts the file, so it is important that
// GraphicsImportValidate calls not be too slow. Importers that implement the GraphicsImportValidate call
// should have the canMovieImportValidateFile bit set in their component flags.
pascal ComponentResult EI_GraphicsImportValidate(EI_GraphicsImportGlobals store, Boolean *valid)
{
	ComponentResult err = noErr;
	unsigned char header[2];

	*valid = false;

	err = GraphicsImportReadData(store->parent, &header[0], 0, 2);
	if (err) goto bail;
	
	// If your image format has a "magic number" at the start, this is where you should look for it.
	
	// Electric Image files don't have a magic number, but they do have a version code, which should
	// be a big-endian two-byte integer "5".  So we'll check that. 	
	if( ( 0 == header[0] )
	 && ( 5 == header[1] ) )
		*valid = true;
		
bail:
	return err;
}

// GraphicsImportGetMetaData
// 		Extract metadata from an image file and add it to a user data structure.
// Note that userData must already be allocated. If the user data passed to GraphicsImportGetMetaData
// belongs to a movie, track or media, then whatever metadata is extracted will be added to that movie, track or media.
pascal ComponentResult EI_GraphicsImportGetMetaData(EI_GraphicsImportGlobals store, void *userData)
{
#pragma unused(store)

	ComponentResult err = noErr;
	Handle	dataHandle = NULL;
	UserData userDataOut = (UserData)userData;

	// For the sake of example we'll return a fixed string as metadata.
	char *theCString = { "File created with Electric Image" };
	
	#if !TARGET_API_MAC_CARBON
		StringPtr thePString = c2pstr(theCString);
	#else
		Str255 thePString;
		CopyCStringToPascal(theCString, thePString);
	#endif // TARGET_API_MAC_CARBON

	if (NULL == userData) {
		err = paramErr;
		goto bail;
	}

	dataHandle = NewHandle(0);
	if (NULL == dataHandle) {
		err = memFullErr;
		goto bail;
	}
	
	err = PtrAndHand(&thePString[1], dataHandle, thePString[0]);
	if (err) goto bail;
	
	err = AddUserDataText(userDataOut, dataHandle, kUserDataTextInformation, 1, langEnglish);

bail:
	if (dataHandle)
		DisposeHandle(dataHandle);

	return err;
}

// GraphicsImportGetMIMETypeList
// 		Returns a list of MIME types supported by the graphics import component.
// To indicate that your graphics import component supports this function, set the hasMovieImportMIMEList flag in the
// componentFlags field of the component description record.
pascal ComponentResult EI_GraphicsImportGetMIMETypeList(EI_GraphicsImportGlobals store, void *qtAtomContainerPtr)
{
	// Note that GetComponentResource is only available in QuickTime 3.0 or later.
	// However, it's safe to call it here because GetMIMETypeList is only defined in QuickTime 3.0 or later.
	return GetComponentResource((Component)store->self, FOUR_CHAR_CODE('mime'), 128, (Handle *)qtAtomContainerPtr);
}

#pragma mark-

// When building the *Application Version Only* make our component available for use by applications (or other clients).
// Once the Component Manager has registered a component, applications can find and open the component using standard
// Component Manager routines.
#if !STAND_ALONE && !TARGET_OS_WIN32
void EI_GraphicsImporterRegister(void);
void EI_GraphicsImporterRegister(void)
{
	ComponentDescription foo;	
	#if !TARGET_API_MAC_CARBON
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineProc(EI_GraphicsImportComponentDispatch);
	#else
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineUPP((ComponentRoutineProcPtr)EI_GraphicsImportComponentDispatch);
	#endif

  	foo.componentType = GraphicsImporterComponentType;
  	foo.componentSubType = FOUR_CHAR_CODE('EIDI');
  	foo.componentManufacturer = kAppleManufacturer;
  	foo.componentFlags = 0;
  	foo.componentFlagsMask = 0;

	RegisterComponent(&foo, componentEntryPoint, registerComponentGlobal, NULL, NULL, NULL);
}
#endif // !STAND_ALONE && !TARGET_OS_WIN32
