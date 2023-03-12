/*

File: main.c

Abstract: Demonstrates how to use a QuickTime Pixel Buffer Visual Context with NewMovieFromProperties
		  to produce CVPixelBuffers, draw on them and render them as CGImages into an HIImageView.

Version: 1.0.1

© Copyright 2005-2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Revision History:
    <2> 07/08/2006  dts make sure to call EnterMovies
	<1>	08/08/2005	dts	initial release

*/ 

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

// custom carbon event types
enum {
	kEventAppDoSomething = 0xbaddbeef,
	kEventParamCVTimeStamp = 'tmst',
	typeCVTimeStamp =	'tmst'
};

// data we pass around
typedef struct {
	WindowRef			window;
    HIViewRef           imageView;
	Movie				movie;
    QTVisualContextRef  visualContext;
    CVImageBufferRef    image;
} WindowDataRecord, *WindowDataRecordPtr;

// use a global cuz it's easy
static WindowDataRecord gWindowData = {0};

// image view ID
static HIViewID kImageID = {'IMAG', 0};

#pragma mark - CGImage
/* DrawOnImage
		Utility function to draw some text on a 32 ARGB pixel buffer, we first need to wrap it in a CGBitmapContext.
*/
static OSStatus DrawOnImage(void *inBaseAddress, size_t inRowBytes, size_t inWidth, size_t inHeight, CGColorSpaceRef inColorSpace)
{
    CGContextRef theBitmapContext = NULL;
    
    if (NULL == inBaseAddress || 0 == inRowBytes || 0 == inWidth || 0 == inHeight || NULL == inColorSpace) return paramErr;
  
	theBitmapContext = CGBitmapContextCreate(inBaseAddress, inWidth, inHeight, 8, inRowBytes, inColorSpace, kCGImageAlphaPremultipliedFirst);
    if (NULL == theBitmapContext) return kCGErrorFailure;
    
    // what we want to draw
    const char *string = {"Hello World"};
    
    // some set up
    CGContextSelectFont(theBitmapContext, "Arial-Black", 32, kCGEncodingMacRoman);
    CGContextSetLineWidth(theBitmapContext, 1.5);
    CGContextSetRGBStrokeColor(theBitmapContext, 0.0, 0.0, 0.0, 1.0); // black
    CGContextSetRGBFillColor(theBitmapContext, 0.0, 1.0, 0.0, 1.0);   // bright green
  	CGContextSetTextPosition(theBitmapContext, 10, 10);
  	CGContextSetTextDrawingMode(theBitmapContext, kCGTextFillStroke);
  	
    // draw it
    CGContextShowText(theBitmapContext, string, strlen( string ));

  if (NULL != theBitmapContext) CGContextRelease(theBitmapContext);
  
  return noErr;
}

/* CreateCGImageFromPixelBuffer
		This function creates a CGImage from a CVImageBuffer passed in. 
*/
static CGImageRef CreateCGImageFromPixelBuffer(CVImageBufferRef inImage, OSType inPixelFormat)
{
	CGDataProviderRef provider = NULL;
	CGColorSpaceRef colorSpace = NULL;
	CGImageRef image = NULL;
    void *baseAddress;
    size_t bytesPerRow, width, height;
	size_t bitsPerComponent, bitsPerPixel;
	CGImageAlphaInfo alphaInfo;
    
    if (NULL == inImage || 0 == inPixelFormat) return NULL;
    
    baseAddress = CVPixelBufferGetBaseAddress(inImage);
    bytesPerRow = CVPixelBufferGetBytesPerRow(inImage);
    width = CVPixelBufferGetWidth(inImage);
    height = CVPixelBufferGetHeight(inImage);
	
	switch(inPixelFormat) {
    // in this app we're only dealing with 32 ARGB pixel buffers
    // see the Quartz 2D Programming Guide for a list of supported formats
    case k32ARGBPixelFormat:
        bitsPerComponent = 8;
        bitsPerPixel = 32;
        alphaInfo = kCGImageAlphaFirst;
        
        break;
    default:
        printf("I don't know what to do with this format!\n");
        goto Bail;
        
        break;
	}
    
    // Colorspace can be device, calibrated, or ICC profile based.
	colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    require(NULL != colorSpace, Bail);
            
    // Draw some text on the image just for something to do
    DrawOnImage(baseAddress, bytesPerRow, width, height, colorSpace);
	
	// Create a data provider with a pointer to the memory bits
	provider = CGDataProviderCreateWithData(NULL, baseAddress, bytesPerRow * height, NULL);
	require(NULL != provider, Bail);
    	
	// Create the image
	image = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpace, alphaInfo, provider, NULL, false, kCGRenderingIntentDefault);

Bail:
	// Once the image is created we can release our reference to the provider and the colorspace, they are retained by the image
	if (NULL != provider) CGDataProviderRelease(provider);
	if (NULL != colorSpace) CGColorSpaceRelease(colorSpace);

	return image;
}

/* MyApplicationEventHandler
    Replace the image in an HIImageView and force it to be displayed immediately.

*/
static OSStatus PutCGImageInHIImageView(CGImageRef inImage, HIViewRef inImageView)
{
    OSStatus status;
    
    if (NULL == inImage || NULL == inImageView) return paramErr;
  
    // Pass our view the new image
    status = HIImageViewSetImage(inImageView, inImage);
    require_noerr(status, Bail);
    
    status = HIViewSetNeedsDisplay(inImageView, true);
    require_noerr(status, Bail);
    
    status = HIViewRender(inImageView);
    require_noerr(status, Bail);
    
    status = HIWindowFlush(HIViewGetWindow(inImageView));
    
Bail:
    return status;
}

#pragma mark - EventHandler

/* MyApplicationEventHandler
		Main application carbon event handler takes care of the HI commands, menu commands and our custom event.
*/
static pascal OSStatus MyApplicationEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
#pragma unused(inHandlerCallRef)

	UInt32 theEventClass = GetEventClass(inEvent);
	UInt32 theEventKind = GetEventKind(inEvent);
	
	OSStatus status	= eventNotHandledErr;
	
	WindowDataRecordPtr pData = (WindowDataRecordPtr)inUserData;
	if (pData == NULL ) return status;
	
	switch (theEventClass) {
	case kEventClassCommand:
    case kEventClassWindow:	
		if (kEventCommandProcess == theEventKind) {
			HICommand theCommand;
			
            GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &theCommand);
			
			switch (theCommand.commandID) {
			case kHICommandOK:
            {
                TimeRecord currentTime;
                
                MoviesTask(pData->movie, 0);
                
                // Move the movie time forward
                GetMovieTime(pData->movie, &currentTime);
                SInt64 timeValue = WideToSInt64(currentTime.value);
                timeValue += GetMovieTimeScale(pData->movie) / 10;
                currentTime.value = SInt64ToWide(timeValue);

                SetMovieTime(pData->movie, &currentTime);
                
                if (GetMovieDuration(pData->movie) < currentTime.value.lo) {
                    timeValue = 0;
                    currentTime.value = SInt64ToWide(timeValue);
                    SetMovieTime(pData->movie, &currentTime);
                }

                break;
            }
			default:
				break;
			}
		}
        
        if (kEventWindowClosed == theEventKind) {
        	WindowRef theWindow = NULL;
            
            GetEventParameter(inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &theWindow);

			if (theWindow == pData->window) {
            	QuitApplicationEventLoop();
            }
            
            break;
        }
        
		break;
    case kEventClassApplication:
    {
        // Handle our custom carbon event
        if (kEventAppDoSomething == theEventKind) {
        
            CVImageBufferRef newImage = NULL;
            CVTimeStamp aTimeStamp;
            
            // Get the CVTimeStamp
            GetEventParameter(inEvent, kEventParamCVTimeStamp, typeCVTimeStamp, NULL, sizeof(aTimeStamp), NULL, &aTimeStamp);
            
            // If we have an old image hanging around release it now
            if (NULL != pData->image) {
                CVPixelBufferUnlockBaseAddress((CVPixelBufferRef)pData->image, 0);
                CVPixelBufferRelease(pData->image);
                pData->image = NULL;
            }
            
            // Check to make sure we do have an image for this time and if so then grab it
            if (QTVisualContextIsNewImageAvailable(pData->visualContext, &aTimeStamp)) {
    
            	status = QTVisualContextCopyImageForTime(pData->visualContext, kCFAllocatorDefault, &aTimeStamp, &newImage);
                require_noerr(status, CopyImageForTimeFailed);
                
                if (NULL != newImage) {
                    printf("CoreVideo image is available.\n");
                    
                    OSType formatType = CVPixelBufferGetPixelFormatType((CVPixelBufferRef)newImage);
                    printf("PixelBuffer FormatType: %lx\n\n", formatType);
                    
                    pData->image = newImage;
                    
                    // Lock the base address of the pixel buffer
                    if (noErr == (CVPixelBufferLockBaseAddress((CVPixelBufferRef)newImage, 0))) {
                        
                        // Get a CGImage from the returned CV pixel buffer
                        CGImageRef theCGImage = CreateCGImageFromPixelBuffer(pData->image, formatType);
                        if (NULL != theCGImage) {
                    	
                            // Draw the Image
                            PutCGImageInHIImageView(theCGImage, pData->imageView);
                            
                            CFRelease(theCGImage);
                            
                            status = noErr;
                        }
                    }
                }
            }
            
            QTVisualContextTask(pData->visualContext);
        }
        
        break;
    }
    default:
        break;
    }
    
CopyImageForTimeFailed:
    return status;
}

#pragma mark - Visual Context

/* Utility to set a SInt32 value in a CFDictionary
*/
static OSStatus SetNumberValue(CFMutableDictionaryRef inDict, CFStringRef inKey, SInt32 inValue)
{
    CFNumberRef number;

    number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inValue);
    if (NULL == number) return coreFoundationUnknownErr;

    CFDictionarySetValue(inDict, inKey, number);

    CFRelease(number);

    return noErr;
}

/* Create a QuickTime Pixel Buffer Context
	This function creates a QuickTime Visual Context which will produce CVPixelBuffers
    
*/
static OSStatus CreatePixelBufferContext(SInt32 inPixelFormat, CGRect *inBounds, QTVisualContextRef *outVisualContext)
{
    QTVisualContextRef      theContext = NULL;
    CFMutableDictionaryRef  pixelBufferOptions = NULL;
    CFMutableDictionaryRef  visualContextOptions = NULL;
    OSStatus                status = noErr;
    
    *outVisualContext = NULL;
    
    if (0 == inPixelFormat || CGRectIsNull(*inBounds)) {status = paramErr; goto Bail;}

	// Pixel Buffer attributes
    pixelBufferOptions = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                   &kCFTypeDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);
    if (NULL == pixelBufferOptions) {status = coreFoundationUnknownErr; goto Bail;}

	// the pixel format we want
	SetNumberValue(pixelBufferOptions, kCVPixelBufferPixelFormatTypeKey, inPixelFormat);
	
	// size
    SetNumberValue(pixelBufferOptions, kCVPixelBufferWidthKey, inBounds->size.width);
    SetNumberValue(pixelBufferOptions, kCVPixelBufferHeightKey, inBounds->size.height);
    
    // alignment
    SetNumberValue(pixelBufferOptions, kCVPixelBufferBytesPerRowAlignmentKey, 16);

	// QT Visual Context attributes
    visualContextOptions = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                     &kCFTypeDictionaryKeyCallBacks,
                                                     &kCFTypeDictionaryValueCallBacks);
    if (NULL == visualContextOptions) {status = coreFoundationUnknownErr; goto Bail; }

	// set the pixel buffer attributes for the visual context
    CFDictionarySetValue(visualContextOptions,
    					 kQTVisualContextPixelBufferAttributesKey,
    					 pixelBufferOptions);

	// create a Pixel Buffer visual context
    status = QTPixelBufferContextCreate(kCFAllocatorDefault,
    								 visualContextOptions,
    								 &theContext);
	require_noerr(status, Bail);

    *outVisualContext = theContext;
    theContext = NULL;

Bail:
    if (NULL != visualContextOptions) CFRelease(visualContextOptions);
    if (NULL != pixelBufferOptions) CFRelease(pixelBufferOptions);
    if (NULL != theContext) QTVisualContextRelease(theContext);
    
    return status;
}

#pragma mark - VisualContext Callback

/* A Callback to receive notifications when a new image becomes available.
	This callback is called from random threads and is best used as a notification that something has
    changed during playback to the visual context.
    
    Applications using the CoreVideo display link to drive rendering do not need to install this callback,
    since they will already be checking for new images at a sufficient rate.
*/
static void MyQTVisualContextImageAvailableCallback(QTVisualContextRef visualContext, const CVTimeStamp *timeStamp, void *refCon)
{
    // Print out some information about the timeStamp
    printf("CVTimeStamp ----------\nFlags: %llu\n", timeStamp->flags);
    if (timeStamp->flags & kCVTimeStampVideoTimeValid) {
        printf("VideoTime: %lld\n", timeStamp->videoTime);
    }
    if (timeStamp->flags & kCVTimeStampHostTimeValid) {
        printf("HostTime: %llu\n", timeStamp->hostTime);
    }
    if (timeStamp->flags & kCVTimeStampRateScalarValid) {
        printf("Rate: %1.0g\n", timeStamp->rateScalar);
    }
    
    EventRef theEventRef = NULL;
    
    // Because we're not performing realtime playback this is fine, not something you want to do
    // for playback because by the time your event handler will recieve these events the images
    // for these times may not be avaiable...
	CreateEvent(NULL, kEventClassApplication, kEventAppDoSomething, 0, kEventAttributeNone, &theEventRef);
	SetEventParameter(theEventRef, kEventParamCVTimeStamp, typeCVTimeStamp, sizeof(CVTimeStamp), timeStamp);

    if (theEventRef) {
        PostEventToQueue(GetMainEventQueue(), theEventRef, kEventPriorityStandard);
        ReleaseEvent(theEventRef);
    }
}

#pragma mark -

int main(int argc, char* argv[])
{
    IBNibRef	nibRef;
    CFBundleRef bundleRef;
    CFURLRef movieLocation;
    WindowRef	window;
    OSStatus	status;
    
    QTVisualContextRef theVisualContext = NULL;
    Movie theMovie = NULL;
	
    HIViewRef contentView = NULL;
	HIRect theBounds;
    
    EventHandlerRef theEventRef = NULL;    
	EventTypeSpec theEventTypes[] = {{kEventClassCommand, kEventCommandProcess},
                                     {kEventClassWindow, kEventWindowClosed},
    								 {kEventClassApplication, kEventAppDoSomething}};
    Boolean trueValue = true;
    QTNewMoviePropertyElement newMovieProperties[3] = {0};
    
    // Initialize QuickTime
    EnterMovies();
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    status = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr(status, CantGetNibRef);
    
    // Get the bundle reference
    bundleRef = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.dts.QTPixelBufferVCToCGImage"));
    require(NULL != bundleRef, CantGetBundleRef);

	// Get the Movie path
    movieLocation = CFBundleCopyResourceURL(bundleRef, CFSTR("SampleMovie"), CFSTR("mov"), NULL);
    require(NULL != movieLocation, CantGetMovieLocation);
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created
    status = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr(status, CantSetMenuBar);
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created
    status = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr(status, CantCreateWindow);

    // We don't need the nib reference anymore
    DisposeNibReference(nibRef);
    
    // The window was created hidden so show it
    ShowWindow(window);
    
    // Get the root control
    status == GetRootControl(window, &contentView);
    require_noerr(status, CantGetRootControl);
    
    // Find the Image View
    status = HIViewFindByID(contentView, kImageID, &gWindowData.imageView);
    require_noerr(status, CantFindImageView);
    
    // Find its size
    HIViewGetBounds(gWindowData.imageView, &theBounds);
    
    // Create the QT Pixel Buffer Visual Context
    status = CreatePixelBufferContext(k32ARGBPixelFormat, &theBounds, &theVisualContext);
    require_noerr(status, CantCreateVisualContext);
    
    // The Movie location 
    newMovieProperties[0].propClass = kQTPropertyClass_DataLocation;
    newMovieProperties[0].propID = kQTDataLocationPropertyID_CFURL;
    newMovieProperties[0].propValueSize = sizeof(CFURLRef);
    newMovieProperties[0].propValueAddress = &movieLocation;

	// The Movie visual context
    newMovieProperties[1].propClass = kQTPropertyClass_Context;
    newMovieProperties[1].propID = kQTContextPropertyID_VisualContext;
    newMovieProperties[1].propValueSize = sizeof(theVisualContext);
    newMovieProperties[1].propValueAddress = &theVisualContext;

	// The Movie active
    newMovieProperties[2].propClass = kQTPropertyClass_NewMovieProperty;
    newMovieProperties[2].propID = kQTNewMoviePropertyID_Active;
    newMovieProperties[2].propValueSize = sizeof(trueValue);
    newMovieProperties[2].propValueAddress = &trueValue;
    
    // Instantiate the Movie
    status = NewMovieFromProperties(3, newMovieProperties, 0, NULL, &theMovie);
 	require_noerr(status, NewMovieFailed);
    
    // Save some stuff
    gWindowData.window = window;
    gWindowData.movie = theMovie;
    gWindowData.visualContext = theVisualContext;
    
    // Install our visual context callback we'll use as a notification mechanism
    status = QTVisualContextSetImageAvailableCallback(theVisualContext, MyQTVisualContextImageAvailableCallback, &gWindowData);
 	require_noerr(status, CantInstallVCCallback);
    
    // Install our event handler
    status = InstallApplicationEventHandler(MyApplicationEventHandler, GetEventTypeCount(theEventTypes), theEventTypes, &gWindowData, &theEventRef);
 	require_noerr(status, CantInstallEventHandler);
    
    // Call the event loop
    RunApplicationEventLoop();

CantInstallEventHandler:
CantInstallVCCallback:
NewMovieFailed:
	CFRelease(theVisualContext);
    
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
CantGetBundleRef:
CantGetMovieLocation:
CantFindImageView:
CantGetRootControl:
CantCreateVisualContext:
	return status;
}
