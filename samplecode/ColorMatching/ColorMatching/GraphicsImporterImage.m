/*
	File:		GraphicsImporterImage.m
	
	Description: Contains QuickTime Graphics Importer routines which operate on a given
				 image file.

	Author:		Apple Developer Technical Support
	
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
	   <1>	 	11/2/03 	SRK		first file
*/

#import "GraphicsImporterImage.h"
#import "ColorSyncUtils.h"


@implementation GraphicsImporterImage

//////////
//
// initGraphicsImporterImage:
// find a QuickTime Graphics Importer for the image file.
//
//////////

-(id)initGraphicsImporterImage:(NSString *)imageFilePath
{
    OSErr err = noErr;
    
    mGraphicsImporterComponent = NULL;
	
	if (imageFilePath == NULL)
		return nil;
    mImageFilePath = [imageFilePath copy];

    err = [ColorSyncUtils graphicsImporterForImageFileFromPath:imageFilePath graphicsImporter:&mGraphicsImporterComponent];
    if (err == noErr)
    {
        return self;
    }
	
    // return nil if we failed to get an importer component
    return nil;
}

//////////
//
// imageEmbeddedProfile:
// return any embedded profile for the image.
//
//////////

-(CMError)imageEmbeddedProfile:(CMProfileRef *)aProfileRef
{
    return( [ColorSyncUtils embeddedProfileForImageFileFromPath:mImageFilePath profileReference:aProfileRef]);
}

//////////
//
// imageEmbedDefaultRGBProfile:
// embed the default ColorSync rgb profile into the image.
//
//////////

-(CMError)imageEmbedDefaultRGBProfile
{
    return ([ColorSyncUtils embedDefaultRGBProfileIntoImageFile:mImageFilePath]);
}

//////////
//
// setImageProfileFromPath:
// embed the profile specified by path into the image
//
//////////

-(OSErr)setImageProfileFromPath:(NSString *)profilePathName
{
	OSErr err = -1;

    if (profilePathName)
    {
        FSSpec profileFSSpec, imageFSSpec;
        
        err = [Utils pathnameToFSSpec:profilePathName fsSpec:&profileFSSpec]; 
        if (err == noErr)
        {
			err = [Utils pathnameToFSSpec:mImageFilePath fsSpec:&imageFSSpec];
			if (err == noErr)
			{
				CMError cmErr = [ColorSyncUtils embedProfileInImageFile:&imageFSSpec profileFile:&profileFSSpec];
                
                err = cmErr;
			}
        }
    }
    
    return err;
}


//////////
//
// drawImageToGWorld:
// use the QuickTime Graphics Importer to draw the image to the specified port.
//
//////////

-(OSErr)drawImageToGWorld:(CGrafPtr)destPort
{
    OSErr 		err = -1;
    CGrafPtr	savePort;
    GDHandle	saveGDev;

    GetGWorld(&savePort, &saveGDev);

	if (destPort == NULL)
		return err;
    err = GraphicsImportSetGWorld(mGraphicsImporterComponent, destPort, nil);
	if (err == noErr)
	{
		err = GraphicsImportDraw(mGraphicsImporterComponent);
	}
    
    SetGWorld(savePort, saveGDev);
    
    return (err);
}


//////////
//
// setPantherColorSyncDestProfileFromPath:
// specify a new QuickTime Graphics Importer ColorSync destination profile for color matching.
//
//////////

-(ComponentResult)setPantherColorSyncDestProfileFromPath:(NSString *)aProfilePath
{
	CMProfileRef profileRef = nil;
	CMError cmErr = -1;
	
	if (aProfilePath == NULL)
		return cmErr;
	cmErr = [ColorSyncUtils openProfileFromNSStringPath:aProfilePath theProfile:&profileRef];
	if (cmErr == noErr)
	{
		ComponentResult result = GraphicsImportSetDestinationColorSyncProfileRef(mGraphicsImporterComponent,
profileRef);

		[ColorSyncUtils closeProfileRef:profileRef];

		return result;
	}
	
	return cmErr;
}

//////////
//
// pantherColorSyncDestProfileRef:
// retrieve the current QuickTime Graphics Importer ColorSync destination profile.
//
//////////

-(ComponentResult)pantherColorSyncDestProfileRef:(CMProfileRef *)aProfileRef
{
	return (GraphicsImportGetDestinationColorSyncProfileRef (mGraphicsImporterComponent,aProfileRef));
}

//////////
//
// setPantherColorSyncOverrideSourceProfileFromPath:
// set the QuickTime Graphics Importer ColorSync override source profile.
//
//////////

-(ComponentResult)setPantherColorSyncOverrideSourceProfileFromPath:(NSString *)aProfilePath
{
	CMProfileRef profileRef = nil;
	CMError cmErr = -1;

	if (aProfilePath == NULL)
		return cmErr;
	cmErr = [ColorSyncUtils openProfileFromNSStringPath:aProfilePath theProfile:&profileRef];
	if (cmErr == noErr)
	{
		ComponentResult result = GraphicsImportSetOverrideSourceColorSyncProfileRef(mGraphicsImporterComponent,
profileRef);

		[ColorSyncUtils closeProfileRef:profileRef];

		return result;
	}
	
	return cmErr;
}

//////////
//
// pantherColorSyncOverrideSourceProfileRef:
// get the QuickTime Graphics Importer ColorSync override source profile.
//
//////////

-(ComponentResult)pantherColorSyncOverrideSourceProfileRef:(CMProfileRef *)aProfileRef
{
	return (GraphicsImportGetOverrideSourceColorSyncProfileRef (mGraphicsImporterComponent,aProfileRef));
}

//////////
//
// setBoundsRect:
// set the image bounds (display) rectangle.
//
//////////

-(OSErr)setBoundsRect:(Rect *)boundsRect
{
	return (GraphicsImportSetBoundsRect(mGraphicsImporterComponent, boundsRect));
}

//////////
//
// boundsRect:
// get the image bounds (display) rectangle.
//
//////////

-(OSErr)boundsRect:(Rect *)boundsRect
{
	return (GraphicsImportGetBoundsRect(mGraphicsImporterComponent, boundsRect));
}

//////////
//
// imageNSRect:
// get the image natural bounds rectangle as an NSRect.
//
//////////

-(OSErr)imageNSRect:(NSRect *)aRect
{
    OSErr 	err;
    Rect 	naturalBoundsRect;

    err = GraphicsImportGetNaturalBounds(mGraphicsImporterComponent, &naturalBoundsRect);
    if (err == noErr)
    {
		*aRect = [Utils MacRectToNSRect:naturalBoundsRect];		
    }
	
	return err;
}

//////////
//
// imageMacRect:
// get the image natural bounds rectangle as a Macintosh Rect.
//
//////////

-(OSErr)imageMacRect:(Rect *)sourceRect
{
    return(GraphicsImportGetNaturalBounds(mGraphicsImporterComponent, sourceRect));
}

//////////
//
// imageFilePath:
// get the image file path.
//
//////////

-(NSString *)imageFilePath
{
    return [[mImageFilePath retain] autorelease];
}

//////////
//
// graphicsImportComponent:
// get the QuickTime Graphics Importer component for the image.
//
//////////

-(GraphicsImportComponent)graphicsImportComponent
{
	return mGraphicsImporterComponent;
}

//////////
//
// QTGetDestColorSyncProfile:
// get the QuickTime Graphics Importer destination ColorSync profile (Panther only).
//
//////////

-(ComponentResult)QTGetDestColorSyncProfile:(CMProfileRef *)aProfileRef
{
	return (GraphicsImportGetDestinationColorSyncProfileRef (mGraphicsImporterComponent,aProfileRef));
}

//////////
//
// QTSetDestColorSyncProfile:
// set the QuickTime Graphics Importer destination ColorSync profile (Panther only).
//
//////////

-(ComponentResult)QTSetDestColorSyncProfile:(CMProfileRef)aProfileRef
{
	return (GraphicsImportSetDestinationColorSyncProfileRef (mGraphicsImporterComponent,aProfileRef));
}

//////////
//
// QTGetSourceOverrideColorSyncProfile:
// get the QuickTime Graphics Importer source override ColorSync profile (Panther only).
//
//////////

-(ComponentResult)QTGetSourceOverrideColorSyncProfile:(CMProfileRef *)aProfileRef
{
	return(GraphicsImportGetOverrideSourceColorSyncProfileRef (mGraphicsImporterComponent,aProfileRef));
}

//////////
//
// QTSetSourceOverrideColorSyncProfile:
// set the QuickTime Graphics Importer source override ColorSync profile (Panther only).
//
//////////

-(ComponentResult)QTSetSourceOverrideColorSyncProfile:(CMProfileRef)aProfileRef
{
	return(GraphicsImportSetOverrideSourceColorSyncProfileRef (mGraphicsImporterComponent,aProfileRef));
}

//////////
//
// dealloc:
// close our QuickTime Graphics Importer component and dispose of other data structures.
//
//////////

-(void)dealloc
{
    if (mGraphicsImporterComponent)
    {
        CloseComponent(mGraphicsImporterComponent);
    }

    if (mImageFilePath)
    {
        [mImageFilePath release];
    }

}

@end
