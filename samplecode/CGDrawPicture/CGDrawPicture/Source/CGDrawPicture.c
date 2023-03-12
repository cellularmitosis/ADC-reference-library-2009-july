/*
	File:		CGDrawPicture.c
	
	Contains:	Demonstration of drawing QuickDraw Pictures into 
			    CoreGraphics (Quartz) context with Carbon API

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

	Copyright © 1999-2001 Apple Computer, Inc., All Rights Reserved
*/
#include "CGDrawPicture.h"
#include "UIHandling.h"

#define kExtraMBarSpace		25
#define MIN(x,y)  		( ( (x)  >  (y)  ) ?  (y)  :  (x) )
#define kPICTHeaderSize 	512

#define ScaleCGRect(cgRect, scaleFactor); {(cgRect).origin.x*=(scaleFactor); \
					(cgRect).origin.y*=(scaleFactor); \
					(cgRect).size.width*=(scaleFactor); \
					(cgRect).size.height*=(scaleFactor);}

#define ScaleQDRect(qdRect, scaleFactor); { (qdRect).top*=(scaleFactor); \
					    (qdRect).left*=(scaleFactor); \
					    (qdRect).bottom*=(scaleFactor); \
					    (qdRect).right*=(scaleFactor); }

/********** our data types	***********/
typedef struct PICTDocData{
    Rect	theBounds;		/* the Bounds of the PICT */	
    PicHandle 	theData;		/*the PICT Data Handle*/
}PICTDocData;

typedef struct CGDocData{
	QDPictRef 	pictDataRef;		/* the picture reference for QDPictDrawToCGContext */
	CGRect		theCGBounds;		/* the bounds for the QDPictRef */	
}CGDocData;

typedef enum dataTypeToDraw{
    kTypeIsQD = 0,
    kTypeIsCG = 1
}dataTypeToDraw;
	
typedef struct DataToDraw{
    dataTypeToDraw type;		// The type of data to draw.			
    PMPageFormat pageFormat;		// the page format for this document
    PMPrintSettings printSettings;	// the print settings for a given print job
    union{
        PICTDocData 	pictData;	// our PICT data for drawing with QD
        CGDocData	cgData;		// our PICT data for drawing with Quartz
    }u;
}DataToDraw, *DataToDrawPtr;


static OSStatus ReadPICTData(const FSSpec *theSpec, Handle *theData);
static OSStatus SetPICTBounds(PICTDocData *thePICTImage);
static OSStatus MakePictDocument(const FSSpec *theSpecP, CGDocData *theCGDataP);
static OSStatus MakeTheWindows(const StrFileName name, const DataToDraw *ourPictDataP, const DataToDraw *ourCGDataP);
static OSStatus DrawPICTData(const void *dataToDrawP, float scaleFactor);


// global data
static Rect b;

UInt32 MyGetDocumentNumPagesInDoc(const void *ourDataP)
{
#pragma unused (ourDataP)
    return 1;		// we always have 1 page per document
}

PageDrawProc *GetMyDrawPageProc(const void *ourDataP)
{
#pragma unused (ourDataP)
    return DrawPICTData;
}

void SetPageFormatOnPrivateData(void *ourDataP, PMPageFormat pageFormat)
{
    if(ourDataP)
        ((DataToDrawPtr)ourDataP)->pageFormat = pageFormat;
        
    return;
}

void SetPrintSettingsOnPrivateData(void *ourDataP, PMPrintSettings printSettings)
{
    if(ourDataP)
        ((DataToDrawPtr)ourDataP)->printSettings = printSettings;
        
    return;
}

PMPageFormat GetPageFormatFromPrivateData(const void *ourDataP)
{
    return ourDataP ? ((DataToDrawPtr)ourDataP)->pageFormat : NULL;
}


PMPrintSettings GetPrintSettingsFromPrivateData(const void *ourDataP)
{
    return ourDataP ? ((DataToDrawPtr)ourDataP)->printSettings : NULL;
}

OSStatus DisposeWindowPrivateData(void *ourDataP)
{
    
    DataToDrawPtr ourDataToDrawP = (DataToDrawPtr)ourDataP;
    if(ourDataToDrawP != NULL){
        switch(ourDataToDrawP->type){
            case kTypeIsCG:
		if(ourDataToDrawP->u.cgData.pictDataRef != NULL){
		    QDPictRelease(ourDataToDrawP->u.cgData.pictDataRef);
		}
		break;
            
            case kTypeIsQD:
		if(ourDataToDrawP->u.pictData.theData != NULL){
		    DisposeHandle((Handle)ourDataToDrawP->u.pictData.theData);
		}
		break;
            
            default:
		// assert(...)	// shouldn't get here!
                break;	
        }
        
        // release our page format and print settings if they exist
        if(ourDataToDrawP->pageFormat)
            (void)PMRelease(ourDataToDrawP->pageFormat);
            
        if(ourDataToDrawP->printSettings)
            (void)PMRelease(ourDataToDrawP->printSettings);
            
        free(ourDataToDrawP);	// free the outer structure
    }
    return noErr;
}

OSStatus doTheFile(const FSSpec *theSpec)
{
	OSStatus err = noErr;
        DataToDraw *ourPictDataP = (DataToDraw *)calloc(sizeof(DataToDraw), 1);
       
        if(ourPictDataP){
            ourPictDataP->type = kTypeIsQD;
            err = ReadPICTData(theSpec, (Handle *)&(ourPictDataP->u.pictData.theData) );
            if(!err)err = SetPICTBounds(&ourPictDataP->u.pictData);
            
            if(!err){
                DataToDraw *ourCGDataP = (DataToDraw *)calloc(sizeof(DataToDraw), 1);
                if(ourCGDataP){
                    ourCGDataP->type = kTypeIsCG;
                    err = MakePictDocument(theSpec, &(ourCGDataP->u.cgData) );
                    if(!err)
                        err = MakeTheWindows(theSpec->name, ourPictDataP, ourCGDataP);
                        
                    if(err)
                        (void)DisposeWindowPrivateData(&ourCGDataP);
                }else
                    err = memFullErr;
            }

            if(err)
                (void)DisposeWindowPrivateData(&ourPictDataP);
        }else
            err = memFullErr;
	
	return err;
}


static  OSStatus SetPICTBounds(PICTDocData *thePICTImage)
{
	Rect *theBounds = &(*thePICTImage).theBounds;
	
	*theBounds =  (*(*thePICTImage).theData)->picFrame;
	OffsetRect(theBounds, -theBounds->left, -theBounds->left);

	return noErr;
}

static OSStatus MakeTheWindows(const StrFileName name, const DataToDraw *ourPictDataP, const DataToDraw *ourCGDataP)
{
    OSStatus err = noErr;
    SInt16 windowTop = GetMBarHeight() + kExtraMBarSpace;
    SInt16 right,bottom;
    Rect bounds;

    if (err == noErr) {
	right = MIN(ourPictDataP->u.pictData.theBounds.right , gMainScreenRect.right );
	bottom = MIN(ourPictDataP->u.pictData.theBounds.bottom, gMainScreenRect.bottom - windowTop );
	SetRect(&bounds, 0 , 0, right, bottom);
    }
    
    if (err == noErr){
	WindowRef qdWindow;
	WindowRef cgWindow;
	
	qdWindow = MakeWindow(ourPictDataP);	// this will be our QD Window 

	if(qdWindow){
	    SetWTitle(qdWindow, name);
	    SizeWindow (qdWindow, bounds.right - bounds.left, 
		    bounds.bottom-bounds.top, true);
	    SetPortWindowPort(qdWindow);
	    ClipRect(&bounds);
	    GetWindowPortBounds(qdWindow, &b);
	    InvalWindowRect(qdWindow, &b); 
	    ShowWindow(qdWindow);
	}else
	    err = kCantCreateWindow;

	if(!err){
	    cgWindow = MakeWindow(ourCGDataP);	// this will be our CG Window 

	    if(cgWindow){
		Rect cgBounds;
		CFStringRef	tempString;
		CFMutableStringRef tempString2;
		cgBounds.top = 0;
		cgBounds.left = 0;
		cgBounds.right = CGRectGetWidth(ourCGDataP->u.cgData.theCGBounds);
		cgBounds.bottom = CGRectGetHeight(ourCGDataP->u.cgData.theCGBounds);

		CopyWindowTitleAsCFString(qdWindow, &tempString);

		tempString2 = CFStringCreateMutableCopy(kCFAllocatorDefault,
								0, tempString);
		CFStringAppend(tempString2, CFSTR(" with Quartz"));
	
		SetWindowTitleWithCFString(cgWindow, tempString2);
		CFRelease(tempString);
		CFRelease(tempString2);
		
		SizeWindow (cgWindow, cgBounds.right - cgBounds.left, 
			    cgBounds.bottom-cgBounds.top, true);
		SetPortWindowPort(cgWindow);
		GetWindowPortBounds(cgWindow, &b);
		RectRgn(gScratchRgn, &b);
		SetPortClipRegion(GetWindowPort(cgWindow), gScratchRgn);
		InvalWindowRect(cgWindow, &b); 
		ShowWindow(cgWindow);
	    }else
		err = kCantCreateWindow;
	}
    }
    return err;	
 }


static OSStatus MakePictDocument(const FSSpec *theSpecP, CGDocData *theCGDataP)
{
    OSStatus err = noErr;
    FSRef fsRef;
    CFURLRef url = NULL;
    
    err = FSpMakeFSRef(theSpecP, &fsRef);
    if(!err){
	url = CFURLCreateFromFSRef(NULL, &fsRef);
        if(url){
            theCGDataP->pictDataRef = QDPictCreateWithURL(url);
		
            if(theCGDataP->pictDataRef){
                theCGDataP->theCGBounds = QDPictGetBounds(theCGDataP->pictDataRef);
		// we'll anchor our bounding rect to a 0,0 origin for our drawing
		// purposes
		theCGDataP->theCGBounds.origin.x = theCGDataP->theCGBounds.origin.y = 0;
            }else
		err = kNoQDPictErr;
		
            CFRelease(url);
        }else
            err = memFullErr;
    }
    return err;
}

// Read the PICT data from the file into a Handle
static OSStatus ReadPICTData(const FSSpec *theSpec, Handle *theData)
{
	short refNum;
	OSStatus err;
	long size;
	Handle ourDataH = NULL;
	
	err = FSpOpenDF(theSpec, fsRdPerm, &refNum);
	if (err == noErr) {
	    // position the file at the start
	    err = SetFPos(refNum, fsFromStart, 0);
	    // determine the total size of the file
	    if(err == noErr)
		err = GetEOF(refNum, &size);
		
	    if (err == noErr) {
		    // skip PICT header
		    size -= kPICTHeaderSize;		
		    err = SetFPos(refNum, fsFromStart, kPICTHeaderSize);
	    }
	    if (err == noErr) {
		ourDataH = NewHandle(size);
		if(ourDataH){
		    HLock(ourDataH);
		    err = FSRead(refNum, &size, *ourDataH);
		    if(err){
			DisposeHandle(ourDataH);
			ourDataH = NULL;
		    }
		    HUnlock(ourDataH);
		} else 
		    err = MemError();
	    }
	    FSClose(refNum);
	}
	*theData = ourDataH;
	return err;
}

Boolean CanSaveAsPDF(const void *ourDataP)
{
    Boolean canSaveAsPDF = false;
    DataToDrawPtr ourDataToDrawP = (DataToDrawPtr)ourDataP;
    if(ourDataToDrawP && ourDataToDrawP->type == kTypeIsCG){
        canSaveAsPDF = true;
    }
    return canSaveAsPDF;
}


OSStatus DrawPICTData(const void *dataP, float scaleFactor)
{
    OSStatus err = noErr;
    DataToDrawPtr ourDataToDrawP = (DataToDrawPtr)dataP;
    Boolean rotate = false;
    Boolean framePict = false;
    switch(ourDataToDrawP->type){
        case kTypeIsCG:
            if(!err){
                CGrafPtr ourPort;
                CGContextRef ctx;
                GetPort(&ourPort);
		/*
		    Use of QDBeginCGContext/QDEndCGContext is preferred to the use of
		    CreateCGContextForPort since the use of QDBeginCGContext/QDEndCGContext
		    is supported at during printing whereas the use of CreateCGContextForPort is
		    only supported for drawing during non-printing situations.
		    
		    QDBeginCGContext/QDEndCGContext are only available on MacOS X v10.1 and later. QDBeginCGContext 
		    returns a CGContextRef corresponding to the port passed in. The graphics state is initialized 
		    to default values. The initial coordinate system of the context is at the lower left
		    corner of the port passed in.
		    
		    Calling QDBeginCGContext on a port has the side effect of replacing the port
		    bottlenecks with bottlenecks that do no imaging. Use of QuickDraw in the port 
		    when you can also draw to this context would fail during printing and so the 
		    behavior is implemented as described here. In a future version of MacOS X, 
		    QuickDraw drawing to a grafPort in this state will generate messages
		    to the console. 
		    
		    Calling QDEndCGContext gives up the CGContextRef and restores the QuickDraw 
		    bottlenecks on the port to their previous state. 
		    
		    CGContextRelease should *NEVER* be called on a context obtained with
		    QDBeginCGContext. Once an application calls QDEndCGContext on
		    a context obtained with QDBeginCGContext, the application no longer
		    has a valid reference to that context and should not draw to it. In order
		    to again draw to a CGContextRef corresponding to a given port, again call
		    QDBeginCGContext.
		*/
                err = QDBeginCGContext(ourPort, &ctx);
		// default coordinate system, QD bottlenecks disabled. No QD drawing allowed
                if(!err){
		    Rect portRect;
                    OSStatus tempErr;
                    CGRect drawRect = ourDataToDrawP->u.cgData.theCGBounds;
		    GetPortBounds(ourPort, &portRect); 

		    /* since we're doing app scaling, we need to scale up our drawing rect
                        to an appropriate size to account for the scaling. This would normally
                        apply to all our data we draw with app scaling.
                        
                        See the CGDrawPicture ReadME file for more information on pattern scaling and the
                        issue we are addressing here.
                    */
		    if(scaleFactor != 1.)
			ScaleCGRect(drawRect, scaleFactor);

		    // draw our PICT so that top of PICT is at the port origin. To do this
		    // we offset our rect so that the Y value is at (portHeight - drawRectHeight)
		    drawRect = CGRectOffset(drawRect, 
				0, 
				(portRect.bottom - portRect.top) - drawRect.size.height);

		    // in case we want to rotate about the center of the PICT
                    if(rotate){
                        CGPoint p;
                        p.x = drawRect.origin.x + drawRect.size.width/2;
                        p.y = drawRect.origin.y + drawRect.size.height/2;
                        CGContextTranslateCTM(ctx, p.x, p.y);
                        CGContextRotateCTM(ctx, -45);
                        CGContextTranslateCTM(ctx, -p.x, -p.y);
                    }
		    // in case we want a frame around the PICT
                    if(framePict)
                        CGContextStrokeRectWithWidth(ctx, drawRect, 1.);

                    CGContextSaveGState(ctx);
		    
			/* in order for our app scaling to not scale QD patterns in our
			    picture at print time we need to factor out app scaling from CTM
			    prior to drawing our picture. Pattern scaling is broken
			    in MacOS 10.1.0 but will be fixed soon.
			*/
			
                        CGContextScaleCTM(ctx, scaleFactor, scaleFactor);

			/* Now we need to transform the rect we would otherwise draw
			    with so that we account for the fact that we've factored
			    out the application scaling from the CTM
			*/
			if(scaleFactor != 1.)
			    ScaleCGRect(drawRect, 1/scaleFactor);
			    
			/* now when we draw our picture, our patterns are scaled to the
			    CTM at the time we draw the picture. The scaling of the PICT
			    needed to fit the PICT in the supplied drawRect is not applied
			    to the patterns. If we DO want the patterns to scale, we should
			    apply that scale to the CTM prior to drawing our PICT
			*/
			err = QDPictDrawToCGContext(ctx, drawRect, ourDataToDrawP->u.cgData.pictDataRef);
			
		    // restore our gstate to what it was prior to our scaling
                    CGContextRestoreGState(ctx);
		    
		    /* if we called QDBeginCGContext and it returned noErr then we MUST call
			QDEndCGContext or else we'll never get any QuickDraw imaging again to our
			port. Note that future calls to QDBeginCGContext do not recall any
			changes we've made to our CGContextRef and instead we get default coordinates
			and gstate values.
		    */
                    tempErr = QDEndCGContext(ourPort, &ctx);
                    if(!err)
                        err = tempErr;
                }else
                    err = kCantMakeContextErr;
            }
            break;
        
        case kTypeIsQD:
            if(!err){
                Rect pictureRect;
                GetClip(gScratchRgn);
                pictureRect = ourDataToDrawP->u.pictData.theBounds;
                /* since we're doing app scaling, we need to scale up our drawing rect
                    to an appropriate size to account for the scaling. This would normally
                    apply to all our data we draw with app scaling.
                    
                    See the CGDrawPicture ReadME file for more information on pattern scaling and the
                    issue we are addressing here.
                */
		if(scaleFactor != 1.)
		    ScaleQDRect(pictureRect, scaleFactor);

                ClipRect(&pictureRect);
                DrawPicture(ourDataToDrawP->u.pictData.theData, &pictureRect);
                SetClip(gScratchRgn);
            }
            break;
        
        default:
	    // assert(...)	// shouldn't get here  
            break;	
    }
    return err;
}

/*
    A routine to make a PDF document correponding to arbitrary drawing
    Quartz drawing.
*/
OSStatus MakePDFDocument(const FSSpec *fileSpecP, const void *ourDataRef)	
{
    OSStatus err = noErr;
    DataToDrawPtr ourDataToDrawP = (DataToDrawPtr)ourDataRef;
    FSRef fsRef;
    CFURLRef url = NULL;
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
						&kCFTypeDictionaryKeyCallBacks, 
						&kCFTypeDictionaryValueCallBacks); 
    /*
	Add some producer information to our PDF file.
    */
    if(dict){
	CFStringRef stringRef = CFStringCreateWithPascalString(kCFAllocatorDefault, 
					fileSpecP->name,
					kCFStringEncodingUTF8);
	CFDictionaryAddValue(dict, CFSTR("Title"), stringRef);
	CFRelease(stringRef);
	CFDictionaryAddValue(dict, CFSTR("Producer"), CFSTR("Sample CGDrawPicture Application"));
    }else
	err = memFullErr;
    
    if(!err){
	err = FSpMakeFSRef(fileSpecP, &fsRef);	// make an FSRef
    }
    if(!err){
	url = CFURLCreateFromFSRef(NULL, &fsRef);
	err = FSpDelete(fileSpecP);	// delete the file we just made for making the FSRef
	if(!err && url){
	    CGContextRef ctx = CGPDFContextCreateWithURL(url, &ourDataToDrawP->u.cgData.theCGBounds, dict);
	    if(ctx){
		CGContextBeginPage(ctx, &ourDataToDrawP->u.cgData.theCGBounds);
		err = QDPictDrawToCGContext(ctx, ourDataToDrawP->u.cgData.theCGBounds, 
                                                ourDataToDrawP->u.cgData.pictDataRef);
		CGContextEndPage(ctx);
		CGContextRelease(ctx);
	    }else
		err = kCantMakeContextErr;
	    CFRelease(url);
	}else{
            if(!err)
                err = kCantMakeURLErr;
        }
    }
    if(dict)CFRelease(dict);
    return err;
}
