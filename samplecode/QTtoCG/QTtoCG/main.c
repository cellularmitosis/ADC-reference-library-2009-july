#include "eutil.h"


#include <Carbon/Carbon.h>

#include <QuickTime/ImageCompression.h>
#include <ApplicationServices/ApplicationServices.h>
#include <QuickTime/ImageCompression.h> // for image loading and decompression
#include <QuickTime/QuickTimeComponents.h> // for file type support

typedef struct {
	size_t width;
	size_t height;
	size_t bitsPerComponent;
	size_t bitsPerPixel;
	size_t bytesPerRow;
	size_t size;
	CGImageAlphaInfo ai;
	CGColorSpaceRef cs;
	unsigned char *data;
	CMProfileRef prof;
} BitmapInfo;


#define MAX_IMAGE_WIDTH 500
#define MAX_IMAGE_HEIGHT 600
#define WINDOW_LEFT 64
#define WINDOW_TOP 64

static Boolean GetImageFile (FSSpec * pfsspecImage);


void readBitmapInfo(GraphicsImportComponent gi, BitmapInfo *bi)
{
	ComponentResult result;
	ImageDescriptionHandle imageDescH = NULL;
	ImageDescription *desc;
	Handle profile = NULL;
	
	result = GraphicsImportGetImageDescription(gi, &imageDescH);
	if( noErr != result || imageDescH == NULL ) {
		error("Error while retrieving image description");
		exit(1);
	}
	
	desc = *imageDescH;
	
	bi->width = desc->width;
	bi->height = desc->height;
	bi->bitsPerComponent = 8;
	bi->bitsPerPixel = 32;
	bi->bytesPerRow = (bi->bitsPerPixel * bi->width + 7)/8;
	bi->ai = (desc->depth == 32) ? kCGImageAlphaFirst : kCGImageAlphaNoneSkipFirst;
	bi->size = bi->bytesPerRow * bi->height;
	bi->data = malloc(bi->size);
	
	bi->cs = NULL;
	bi->prof = NULL;
	GraphicsImportGetColorSyncProfile(gi, &profile);
	if( NULL != profile ) {
		CMError err;
		CMProfileLocation profLoc;
		Boolean bValid, bPreferredCMMNotFound;

		profLoc.locType = cmHandleBasedProfile;
		profLoc.u.handleLoc.h = profile;
		
		err = CMOpenProfile(&bi->prof, &profLoc);
		if( err != noErr ) {
			error("Cannot open profile");
			exit(1);
		}
		
		/* Not necessary to validate profile, but good for debugging */
		err = CMValidateProfile(bi->prof, &bValid, &bPreferredCMMNotFound);
		if( err != noErr ) {
			error("Cannot validate profile : Valid: %d, Preferred CMM not found : %d", bValid, 
				  bPreferredCMMNotFound);
			exit(1);
		}
		
		bi->cs = CGColorSpaceCreateWithPlatformColorSpace( &bi->prof );

		if( bi->cs == NULL ) {
			error("Error creating cg colorspace from csync profile");
			exit(1);
		}
		printf("Embedded profile found in image\n");
		DisposeHandle(profile);
	}	
	
	if( imageDescH != NULL)
		DisposeHandle((Handle)imageDescH);
}

void getBitmapData(GraphicsImportComponent gi, BitmapInfo *bi)
{
	GWorldPtr gWorld;
	QDErr err = noErr;
	Rect boundsRect = { 0, 0, bi->height, bi->width };
	ComponentResult result;
	
	if( bi->data == NULL ) {
		error("no bitmap buffer available");
		exit(1);
	}
	
	err = NewGWorldFromPtr( &gWorld, k32ARGBPixelFormat, &boundsRect, NULL, NULL, 0, 
							bi->data, bi->bytesPerRow );
	if (noErr != err) {
		error("error creating new gworld - %d", err);
		exit(1);
	}
	
	if( (result = GraphicsImportSetGWorld(gi, gWorld, NULL)) != noErr ) {
		error("error while setting gworld");
		exit(1);
	}
	
	if( (result = GraphicsImportDraw(gi)) != noErr ) {
		error("error while drawing image through qt");
		exit(1);
	}
	
	DisposeGWorld(gWorld);	
}


void rescaleImage ( size_t* imageWidthPtr, size_t* imageHeightPtr, size_t maxWidth, size_t maxHeight )
{
	size_t width = *imageWidthPtr;
	size_t height = *imageHeightPtr;
	double widthFactor = (double)width / (double)maxWidth;
	double heightFactor = (double)height / (double)maxHeight;
	
	if ( widthFactor > heightFactor )
	{
		//	we're further off in the x axis than we are in the y
		*imageWidthPtr = width / widthFactor;
		*imageHeightPtr = height / widthFactor;
	}
	else
	{
		//	we're further off in the y axis than we are in the x
		*imageWidthPtr = width / heightFactor;
		*imageHeightPtr = height / heightFactor;
	}
}



void putContentInWindow( WindowRef window )
{
	GraphicsImportComponent gi;
	FSSpec fss;
	BitmapInfo bi;
    CGContextRef context;
	Rect bounds;
	CFStringRef titleString;
	size_t imageWidth, imageHeight;
	
	
	//	Here we call the nav services routines to get the FSSpec for the image
	GetImageFile( &fss );

	GetGraphicsImporterForFile(&fss, &gi);
	
	readBitmapInfo(gi, &bi);
	getBitmapData(gi, &bi);

	CloseComponent(gi);
	
	titleString = CFStringCreateWithFormat( NULL, NULL, CFSTR("Without profile    |    With profile   ") );
	SetWindowTitleWithCFString( window, titleString );
	

	printf("Image size = %ld x %ld\n", bi.width, bi.height);
	
	if( bi.width <= 0 || bi.width > 32767 || bi.height <= 0 || bi.height > 32767) {
		error("Invalid image size");
		exit(1);
	}
	
	//	We should check and make sure that the image isn't too large to fit comfortably onscreen
	imageWidth = bi.width;
	imageHeight = bi.height;
	if ( imageWidth > MAX_IMAGE_WIDTH || imageHeight > MAX_IMAGE_HEIGHT )
	{
		rescaleImage( &imageWidth, &imageHeight, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT );
	}
	
	//	get the current bounds of the window, move the content area to (WINDOW_LEFT, WINDOW_TOP)
	//	and resize to hold two images side by side
	GetWindowBounds( window, kWindowContentRgn, &bounds );
	OffsetRect( &bounds, WINDOW_LEFT - bounds.left, WINDOW_TOP - bounds.top );
	SetRect( &bounds, bounds.left, bounds.top, bounds.left + imageWidth * 2, bounds.top + imageHeight );
	SetWindowBounds( window, kWindowContentRgn, &bounds );
	
	//	set up the port and prepare to draw
	SetPortWindowPort( window );
	QDBeginCGContext( GetWindowPort( window ), &context );

	{
        /* Draw a grid of alternating rectangles for the background */
		int i, j;
		CGRect rectangle;
        i = j = 0;
        while( i < imageHeight ) {
            rectangle = CGRectMake(j, i, 10, 10);
            if ((i + j) % 20)
                CGContextSetRGBFillColor(context, 0.5, 0.5, 0.5, 1);
            else
                CGContextSetRGBFillColor(context, 0.8, 0.8, 0.8, 1);
            CGContextFillRect(context, rectangle);

            j+=10;
            if (j > 2 * imageWidth) {
                j = 0; i+=10;
            }
        }
   	    CGContextFlush(context);
	}

	{
		//	Create a CGImage from the bitmap data using the DeviceRGB color space
		CGDataProviderRef provider;
		CGImageRef image;
	    CGRect rect;
	    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	    	
	    provider = CGDataProviderCreateWithData(NULL, bi.data, bi.size, NULL);
		image = CGImageCreate(bi.width, bi.height, bi.bitsPerComponent, bi.bitsPerPixel,
							  bi.bytesPerRow, cs, bi.ai, provider, NULL, 0, kCGRenderingIntentDefault);
		CGColorSpaceRelease(cs);
		
	    //	Draw the image without any embedded profile on the left half of the window
		rect = CGRectMake(0, 0, imageWidth, imageHeight);
		CGContextDrawImage(context, rect, image);

	    CGImageRelease(image);
	    CGContextFlush(context);

		//	If there was any embedded profile, draw the color managed image in the right half of the window
		if( bi.cs != NULL ) {
			image = CGImageCreate(bi.width, bi.height, bi.bitsPerComponent, bi.bitsPerPixel,
								  bi.bytesPerRow, bi.cs, bi.ai, provider, NULL, 0, kCGRenderingIntentDefault);
			CGColorSpaceRelease(bi.cs);
			
			rect = CGRectMake( imageWidth, 0, imageWidth, imageHeight );

			CGContextDrawImage(context, rect, image);
		    CGImageRelease(image);
		    CGContextFlush(context);
		}
	    CGDataProviderRelease(provider);
	}

	//	We're done drawing
	QDEndCGContext( GetWindowPort( window ), &context );

	//	free the bitmap data
	if( bi.data )
		free( bi.data );	
}


int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it.
    ShowWindow( window );
	
	//	Call the routine that uses QT and CG to load an image and draw it into the window
	putContentInWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}



//	function borrowed from OpenGL Image by ggs
static Boolean GetImageFile (FSSpec * pfsspecImage)
{
	enum { kNumImageTypes = 17 }; // number of formats to support for Nav
	NavReplyRecord replyNav; // Navigation reply used to load image
	//  list of file type for Nav (one less since the sturcture accounts for one already)
	NavTypeListHandle hTypeList = (NavTypeListHandle) NewHandleClear (sizeof (NavTypeList) + sizeof (OSType) * (kNumImageTypes - 1)); 
	AEKeyword theKeyword; // keyword used to "decrypt" the nav reply
	DescType actualType; // another nav "decrption" variable
	Size actualSize; // yet again another nav thingy
	OSStatus err = noErr; // err return value

	HLock ((Handle) hTypeList);
	// QT file list (should be able to just use 'qtif' but does not work on Mac OS X as of 10.0.4 so must include all)
	(**hTypeList).osTypeCount = kNumImageTypes;
	(**hTypeList).osType[0] = kQTFileTypeQuickTimeImage;
	(**hTypeList).osType[1] = 'SGI ';
	(**hTypeList).osType[2] = kQTFileTypePhotoShop;
	(**hTypeList).osType[3] = 'GIF ';
	(**hTypeList).osType[4] = kQTFileTypeGIF;
	(**hTypeList).osType[5] = 'JPEG';
	(**hTypeList).osType[6] = 'JPG ';
	(**hTypeList).osType[7] = kQTFileTypePicture;
	(**hTypeList).osType[8] = kQTFileTypeMacPaint;
	(**hTypeList).osType[9] = 'grip';
	(**hTypeList).osType[10] = 'BMPp';
	(**hTypeList).osType[11] = kQTFileTypeTIFF;
	(**hTypeList).osType[12] = kQTFileTypeText;
	(**hTypeList).osType[13] = kQTFileTypeTargaImage;
	(**hTypeList).osType[14] = kQTFileTypeSGIImage;
	(**hTypeList).osType[15] = kQTFileTypeBMP;
	(**hTypeList).osType[16] = '????';

	// use nav to have the user choose a file to open
	err = NavChooseFile (NULL, &replyNav, NULL, NULL, NULL, NULL, hTypeList, NULL); 
	if ((err == noErr) && (replyNav.validRecord)) // if we have a valid reply (i.e. user did not cancel
		err = AEGetNthPtr (&(replyNav.selection), 1, typeFSS, &theKeyword, &actualType, // extract fsspec of reply
						   pfsspecImage, sizeof (FSSpec), &actualSize);
	if ((err != noErr) || (!replyNav.validRecord)) // if we had an error or did not get a valid reply
		return false; // go away
		
	NavDisposeReply (&replyNav); // be good citizens and dispose our reply
	HUnlock ((Handle) hTypeList);
	DisposeHandle ((Handle) hTypeList);
	
	return true;
}

