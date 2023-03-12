/*
	File:		AppDrawing.c
	
	Contains:	Drawing code for our sample document

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
#include "AppDrawing.h"
#include "UIHandling.h"

/**** Macros and Defines ****/
#define	kNumDocumentPages 	1
#define PI      		3.14159265358979323846 /* pi */

#define kOurPDF CFSTR("Kitty.pdf")

/*
    these macros convert from QuickDraw coordinates to CoreGraphics (Quartz coordinates)
    by relying on fact that the port height (portBounds.bottom - portBounds.top) 
    connects the two coordinate systems
*/
#define QDxToCGx(portBounds, qdX)	( (qdX) )
#define QDyToCGy(portBounds, qdY)	( ((portBounds).bottom - (portBounds).top ) - (qdY) )

/********** our data types	***********/
	
typedef struct DataToDraw{
    PMPageFormat pageFormat;		// the page format for this document
    PMPrintSettings printSettings;	// the print settings for a given print job
    CGPDFDocumentRef pdfDoc;		// our PDF document to draw
    CGRect	cgRect;			// the CGRect for page 1 of our PDF document
}DataToDraw, *DataToDrawPtr;

/**** our private prototypes ***/

static PageDrawProc MyPageDrawProc;

static OSStatus RenderCFString(CGContextRef ctx, CFStringRef theString, ATSUStyle theStyle, short h, short v);
static OSStatus MakeATSUIStyle(short fontFamily, Style fontStyle, short fontSize, ATSUStyle *theStyle);
static OSStatus drawCircularText(CGContextRef context, short fontNum);


UInt32 MyGetDocumentNumPagesInDoc(const void *ourDataP)
{
#pragma unused (ourDataP)
    // more realistic code would use the document's page format to determine the number of pages
    // in our document
    return kNumDocumentPages;		// we always have kNumDocumentPages page per document
}

PageDrawProc *GetMyDrawPageProc(const void *ourDataP)
{
#pragma unused (ourDataP)
    return MyPageDrawProc;
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
        // release our page format and print settings if they exist
        if(ourDataToDrawP->pageFormat)
            (void)PMRelease(ourDataToDrawP->pageFormat);
            
        if(ourDataToDrawP->printSettings)
            (void)PMRelease(ourDataToDrawP->printSettings);
        
        // release our PDF document ref
        if(ourDataToDrawP->pdfDoc)
            CGPDFDocumentRelease(ourDataToDrawP->pdfDoc);
        
        free(ourDataToDrawP);	// free the outer structure
    }
    return noErr;
}

OSStatus MakeNewDocument(void)
{
    OSStatus err = noErr;
    // calloc sets all our struct fields to 0
    DataToDraw *ourDataP = (DataToDraw *)calloc(sizeof(DataToDraw), 1);	
    
    if(ourDataP){
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        if(mainBundle){
            CFURLRef ourPDFurl = CFBundleCopyResourceURL(mainBundle, kOurPDF, NULL, NULL);
            if(ourPDFurl){
                ourDataP->pdfDoc = CGPDFDocumentCreateWithURL(ourPDFurl);
                if(ourDataP->pdfDoc != NULL){
		    int pages = CGPDFDocumentGetNumberOfPages(ourDataP->pdfDoc);
		    if(pages > 0){
			// the art box specifies the bounds of our artwork. We are going to use page 1 only.
			ourDataP->cgRect = CGPDFDocumentGetArtBox(ourDataP->pdfDoc , 1);
			// make our destination rect origin at the Quartz 2d origin. 
			ourDataP->cgRect.origin.x = ourDataP->cgRect.origin.y = 0.;	
		    }else{
			CGPDFDocumentRelease(ourDataP->pdfDoc);
			ourDataP->pdfDoc = NULL;
		    }
		}else
                    err = kCantCreatePDFDocument;
                CFRelease(ourPDFurl);
            }else{
                err = kCantGetPDFURL;
            }
        }else
            err = kCantGetMainBundle;
    }else
        err = memFullErr;
       
    if (err == noErr){
	WindowRef qdWindow = MakeWindow(ourDataP);	// this will be our Window 

	if(qdWindow){
            Rect bounds;
	    SetPortWindowPort(qdWindow);
	    GetWindowPortBounds(qdWindow, &bounds);
	    ClipRect(&bounds);
	    InvalWindowRect(qdWindow, &bounds); 
            ShowWindow(qdWindow);
	}else
	    err = kCantCreateWindow;
    }
    return err;	
 }


static OSStatus MyPageDrawProc(const void *refCon, const Rect *drawingRectP, UInt32 pageNumber)
{
#pragma unused (drawingRectP, pageNumber)
    OSStatus err = noErr;
    CGContextRef context = NULL;
    CGrafPtr port;
    DataToDraw *ourDataP = (DataToDraw *)refCon;
    Rect portRect;
    SInt16 tempNum;
    char *regularBuffer = "\pThis text is supposed to draw.\r";
    char *hiddenBuffer = "\pThis text SHOULD NOT draw!\r";
    Point qdLinePoint1;
    Point qdLinePoint2;

    GetPort(&port);
    
    GetFNum("\pTimes",&tempNum);
    TextFont(tempNum);
    TextSize(20);
    GetPortBounds(port, &portRect); 

    qdLinePoint1.h = 20;
    qdLinePoint1.v = 20;
    qdLinePoint2.h = 40;
    qdLinePoint2.v = 40;

    // draw the lines with QuickDraw
    MoveTo(qdLinePoint1.h, qdLinePoint1.v);
    LineTo(qdLinePoint2.h, qdLinePoint2.v);
    
    MoveTo(qdLinePoint1.h, qdLinePoint1.v);
    DrawString(regularBuffer);
    
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
	
	Note that QDBeginCGContext cannot be called a second time without first
	calling QDEndCGContext.
    */
    err = QDBeginCGContext(port, &context);
    if(!err){
        // draw the lines using Quartz
	
	// translate over so we draw to the right of the QD line drawn earlier
        CGContextTranslateCTM(context, qdLinePoint1.h, 0);	
        
	// the macros QDxToCGx, QDyToCGy transform the QD coordinates to the Quartz 2d coord system
        CGContextMoveToPoint(context, QDxToCGx(portRect, qdLinePoint1.h), 
                                    QDyToCGy(portRect, qdLinePoint1.v));
                                    
        CGContextAddLineToPoint(context, QDxToCGx(portRect, qdLinePoint2.h), 
                                            QDyToCGy(portRect, qdLinePoint2.v));
        CGContextStrokePath(context);
    
        MoveTo(qdLinePoint1.h, qdLinePoint1.v);
        DrawString(hiddenBuffer);	// this will not image since QuickDraw drawing does not image
                                        // after QDBeginCGContext but before QDEndCGContext
        
        /* after QDEndCGContext, the context parameter is NULL and
            we can't do any further imaging with Quartz into the context
            corresponding to the port until we again call QDBeginCGContext.
        */
        err = QDEndCGContext(port, &context);	
        if(err || context != NULL){
            printf("got an error from QDEndCGContext\n");
        }
    }else
        printf("got an error from QDBeginCGContext\n");

    // draw more QD lines and text, translated down
    SetOrigin(-40, -40);
    MoveTo(qdLinePoint1.h, qdLinePoint1.v);
    LineTo(qdLinePoint2.h, qdLinePoint2.v);

    MoveTo(qdLinePoint1.h, qdLinePoint1.v);
    DrawString(regularBuffer);

    err = QDBeginCGContext(port, &context);	
    if(!err){
	/*
	    QDBeginCGContext returns a context which has the initial gstate set to
	    default values, NOT the values in place at the time the last QDEndCGContext
	    was called.
	    
	    Also note that the CGContext and port origins are automatically synchronized after
	    QDBeginCGContext, that is the Quartz 2d origin and QuickDraw port origin are such
	    that CGy = (portBounds.bottom - portBounds.top)  - QDy
        */
        CGContextTranslateCTM(context, qdLinePoint1.h, 0);
        CGContextMoveToPoint(context, QDxToCGx(portRect, qdLinePoint1.h), 
                                        QDyToCGy(portRect, qdLinePoint1.v));
        CGContextAddLineToPoint(context, QDxToCGx(portRect, qdLinePoint2.h), 
                                                QDyToCGy(portRect, qdLinePoint2.v));
        CGContextStrokePath(context);

        MoveTo(qdLinePoint1.h, qdLinePoint1.v);
        DrawString(hiddenBuffer);	// this will not image since QuickDraw drawing does not image
                                        // after QDBeginCGContext but before QDEndCGContext

        err = QDEndCGContext(port, &context);
        if(err || context != NULL){
            printf("got an error from QDEndCGContext\n");
        }
    }else
        printf("got an error from QDBeginCGContext\n");

    // more lines and text, translated further down and over
    SetOrigin(-80, -80);
    MoveTo(qdLinePoint1.h, qdLinePoint1.v);
    LineTo(qdLinePoint2.h, qdLinePoint2.v);

    MoveTo(qdLinePoint1.h, qdLinePoint1.v);
    DrawString(regularBuffer);

    SetOrigin(0, 0);		// reset our origin for illustrative purposes
    
    err = QDBeginCGContext(port, &context);
    if(!err){
        SetOrigin(-80, -80);
        // at this point the QD port origin is out of sync with our CGContextRef origin. We must
        // call SyncCGContextOriginWithPort in order to maintain the "normal" relationship
        // between the two, i.e. where are macros QDxToCGx, QDyToCGy transform between the QD
	// coordinate system and the Quartz 2d coordinate system.
	(void)SyncCGContextOriginWithPort(context, port);	// sync our CG context origin to the new port origin

	//CGContextSaveGState(context);	// no need to save our gstate here since QDEndCGContext resets it
            
            // for this next drawing we translate the origin to the QD origin
            CGContextTranslateCTM(context, QDxToCGx(portRect, 0), QDyToCGy(portRect, 0));
            
            // we 'flip the coordinate system' in Y so we have QD style coordinates
            CGContextScaleCTM(context, 1, -1);
            
            // translate over so our drawing is offset from the QD drawing
            CGContextTranslateCTM(context, qdLinePoint1.h, 0);
            
            // scale so that our drawing will be scaled by 1.5. The linewidth will also 
	    // be scaled
            CGContextScaleCTM(context, 1.5, 1.5);
            
            // this drawing will be scaled by 1.5. Note that our Quartz coordinate system is
            // the "QD coordinate system" so we don't need to transform our
            // coordinates
            CGContextMoveToPoint(context, qdLinePoint1.h, qdLinePoint1.v);
            CGContextAddLineToPoint(context, qdLinePoint2.h, qdLinePoint2.v);
            
            CGContextStrokePath(context);

	//CGContextRestoreGState(context);   	// no need to restore our gstate since QDEndCGContext resets it

        MoveTo(qdLinePoint1.h, qdLinePoint1.v);
        DrawString(hiddenBuffer);	// this will not image since QuickDraw drawing does not image
                                        // after QDBeginCGContext but before QDEndCGContext
                                                

        err = QDEndCGContext(port, &context);
        if(err || context != NULL){
            printf("got an error from QDEndCGContext\n");
        }
    }else
        printf("got an error from QDBeginCGContext\n");

    SetOrigin(-220, -280);
    err = QDBeginCGContext(port, &context);
    if(!err){
        OSStatus tempErr;
        // CGContextSaveGState(context);	// no need to save our gstate here since QDEndCGContext resets it
        CGContextTranslateCTM(context, 0, QDyToCGy(portRect, 0));	// translate to port origin
                    
        err = drawCircularText(context, tempNum);	// draw our pinwheel of text
        
	//CGContextRestoreGState(context);   	// no need to restore our gstate since QDEndCGContext resets it

        tempErr = QDEndCGContext(port, &context);
        if(tempErr || context != NULL)
            printf("got an error from QDEndCGContext\n");
    }else
        printf("got an error from QDBeginCGContext\n");


    SetOrigin(-420, -180);
    err = QDBeginCGContext(port, &context);
    if(!err){
        OSStatus tempErr;
	/*
	    This is static so that we only need to create it once since, for this
	    piece of text, the style isn't changed. For this simple drawing we can benefit by  
	    keeping it around rather than creating the style anew each time.
	*/
	static ATSUStyle theStyle = NULL;
		
        // CGContextSaveGState(context);	// no need to save our gstate here since QDEndCGContext resets it
        CGContextTranslateCTM(context, 0, QDyToCGy(portRect, 0));	// translate to port origin
 
        /*
            Clip to our character!
            
            Note that we can clip to one run of characters at a given time
            and trying to clip to more than that will result in a zero clip.
            ATSUI rendering may break a given sequence of characters into more
            than one run. Here we are OK since there is only one character and
            therefore only one run
        */
        CGContextSetTextDrawingMode(context, kCGTextClip);

        if(!theStyle)	// make the style if it doesn't already exist
	    err = MakeATSUIStyle(tempNum, normal, 200, &theStyle);
        if(!err){
            CFStringRef ourString = CFSTR("Q");	
            err = RenderCFString(context, ourString, theStyle, -70, -70);
            // we want to keep the style for the next time
	    // our drawProc is called so we don't call
	    // ATSUDisposeStyle(theStyle);
        }else
            printf("got an error from MakeATSUIStyle\n");
        
	// set the pen mode to the fill the character shapes only
        CGContextSetTextDrawingMode(context, kCGTextFill);
                                      
	// draw our pinwheel of text, now clipped by the 'Q' character
	err = drawCircularText(context, tempNum);
        
	//CGContextRestoreGState(context);   	// no need to restore our gstate since QDEndCGContext resets it

        tempErr = QDEndCGContext(port, &context);
        if(tempErr || context != NULL)
            printf("got an error from QDEndCGContext\n");
    }else
        printf("got an error from QDBeginCGContext\n");


        SetOrigin(-220, -580);
        if(ourDataP->pdfDoc){
            err = QDBeginCGContext(port, &context);
            if(!err){
		// note that our clip is reset to the default, even though we clipped in 
		// the last drawing between QDBeginCGContext/QDEndCGContext
                OSStatus tempErr;
		// again we make this static so we only need to create it once
                static ATSUStyle theStyle = NULL;

                // CGContextSaveGState(context);	// no need to save our gstate here since QDEndCGContext resets it
                CGContextTranslateCTM(context, 0, QDyToCGy(portRect, 0));	// translate to port origin

		// draw our document's page 1
                CGContextDrawPDFDocument(context, ourDataP->cgRect, ourDataP->pdfDoc, 1);

		// translate over before drawing again
                CGContextTranslateCTM(context, -ourDataP->cgRect.size.height, 0);

		// we're going to clip to a character
                CGContextSetTextDrawingMode(context, kCGTextClip);
                
		if(!theStyle)	// make the style if it doesn't already exist
		    err = MakeATSUIStyle(tempNum, normal, 150, &theStyle);
		    
                if(!err){
                    CFStringRef ourString = CFSTR("Q");
                    err = RenderCFString(context, ourString, theStyle, 0, 40); // 100, 100
		    // we want to keep the style for the next time
		    // our drawProc is called so we don't call
		    // ATSUDisposeStyle(theStyle);
                }else
                    printf("got an error from MakeATSUIStyle\n");

                CGContextSetTextDrawingMode(context, kCGTextFill);

                CGContextTranslateCTM(context, ourDataP->cgRect.size.height, 0);

		// rotate by 90 degrees. Note that this routine takes its angle
		// argument in radians, not degrees
                CGContextRotateCTM(context, PI/2);

		// go ahead and draw page 1 again
                CGContextDrawPDFDocument(context, ourDataP->cgRect, ourDataP->pdfDoc, 1);

                //CGContextRestoreGState(context);   	// no need to restore our gstate since QDEndCGContext resets it
                tempErr = QDEndCGContext(port, &context);
                if(tempErr || context != NULL)
                    printf("got an error from QDEndCGContext\n");
            }else
                printf("got an error from QDBeginCGContext\n");
        }

	// reset our origin in preparation for future drawing
	SetOrigin(0, 0);

	return err;	
} //	MyPageDrawProc


/*
    The approach used for ATSUI text is described in
    Technical Q&A QA1027 and the code here is based on the sample
    code shown there.
    
    http://developer.apple.com/qa/qa2001/qa1027.html
*/

/* MakeATSUIStyle creates a simple ATSUI style record
    that based on the fontFamily ID, QD style and fontsize. That
    style record can be used in calls to the ATSUI text drawing routines.
*/
static OSStatus MakeATSUIStyle(short fontFamily, Style fontStyle, short fontSize, ATSUStyle *theStyle) 
{
    OSStatus err;
    ATSUStyle localStyle;
    ATSUFontID atsuFont;
    Fixed atsuSize;
    short atsuOrientation;
    Boolean trueVar = true, falseVar = false;

        /* Three parrallel arrays for setting up attributes. */
    ATSUAttributeTag theTags[] = {
            kATSUFontTag, kATSUSizeTag, kATSUVerticalCharacterTag,
            kATSUQDBoldfaceTag, kATSUQDItalicTag, kATSUQDUnderlineTag,
            kATSUQDCondensedTag, kATSUQDExtendedTag
        };
    ByteCount theSizes[] = {
            sizeof(ATSUFontID), sizeof(Fixed), sizeof(UInt16),
            sizeof(Boolean), sizeof(Boolean), sizeof(Boolean),
            sizeof(Boolean), sizeof(Boolean)
        };
    ATSUAttributeValuePtr theValues[] = {
            NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL
        };

        /* set up locals */
    localStyle = NULL;
    atsuFont = 0;
    atsuSize = FixRatio(fontSize, 1);
    atsuOrientation = kATSUStronglyHorizontal;
    /* or atsuOrientation = kATSUStronglyVertical */

        /* set the values array to point to our locals */
    theValues[0] = &atsuFont;
    theValues[1] = &atsuSize;
    theValues[2] = &atsuOrientation;
    theValues[3] = ((fontStyle & bold) != 0 ? &trueVar : &falseVar);
    theValues[4] = ((fontStyle & italic) != 0 ? &trueVar : &falseVar);
    theValues[5] = ((fontStyle & underline) != 0 ? &trueVar : &falseVar);
    theValues[6] = ((fontStyle & condense) != 0 ? &trueVar : &falseVar);
    theValues[7] = ((fontStyle & extend) != 0 ? &trueVar : &falseVar);

    err = ATSUFONDtoFontID( fontFamily, fontStyle, &atsuFont);
    if (err != noErr) goto bail;

        /* create a style */
    err = ATSUCreateStyle(&localStyle);
    if (err != noErr) goto bail;

        /* set the style attributes */
    err = ATSUSetAttributes( localStyle,
        sizeof(theTags)/sizeof(theTags[0]),
        theTags, theSizes, theValues );
    if (err != noErr) goto bail;

        /* store the new style for the caller */
    *theStyle = localStyle;
    return noErr;
bail:
    if (localStyle != NULL) ATSUDisposeStyle(localStyle);
    return err;
}

/* RenderCFString renders a CFString at (h, v) in the
    supplied CGContextRef using ATSUI using the style specified in
    the atsui style record. */
static OSStatus RenderCFString(CGContextRef ctx, CFStringRef theString,  
			    ATSUStyle theStyle, short h, short v) 
{
    ATSUTextLayout theLayout = NULL;
    CFIndex textLength;
    OSStatus err = noErr;
    UniChar *uniBuffer;
    CFRange uniRange;
    
    /* set up our locals, verify parameters... */
    if (theString == NULL || theStyle == NULL) return paramErr;
    uniBuffer = NULL;
    textLength = CFStringGetLength(theString);
    if (textLength == 0) return noErr;
    
    /* get our data */
    uniRange = CFRangeMake(0, textLength);
    uniBuffer = (UniChar *) NewPtr( textLength * sizeof(UniChar) );
    if (uniBuffer == NULL) { err = memFullErr; goto bail; }
    CFStringGetCharacters( theString,  uniRange, uniBuffer);

    /* create the ATSUI layout */
    err = ATSUCreateTextLayoutWithTextPtr( uniBuffer, 0,
        textLength, textLength, 1,
        (unsigned long *) &textLength, &theStyle,
        &theLayout);
    if (err != noErr) goto bail;

    if(!err)
    {
        // Assign the CGContext to the layout
        // so the text is imaged into the CG Context supplied
        ByteCount iSize = sizeof(CGContextRef);
        ATSUAttributeTag iTag = kATSUCGContextTag;
        ATSUAttributeValuePtr iValuePtr=&ctx; 
        ATSUSetLayoutControls( theLayout, 1, &iTag, &iSize, &iValuePtr );
    }

    /* draw the text */
    err = ATSUDrawText(theLayout, 0, textLength,
            FixRatio(h, 1), FixRatio(v, 1));
    if (err != noErr) goto bail;

    /* clean up */
bail:
    if (uniBuffer != NULL) DisposePtr((Ptr) uniBuffer);
    if (theLayout != NULL) ATSUDisposeTextLayout(theLayout);
    return err;
}

static OSStatus drawCircularText(CGContextRef context, short fontNum)
{
    OSStatus err = noErr;
    // we make this style static so it only needs to be created once
    static ATSUStyle theStyle = NULL;
    
    if(!theStyle)
	err = MakeATSUIStyle(fontNum, normal, 20, &theStyle);
	
    if(!err){
        CFStringRef ourString = CFSTR("Rotated Text With Quartz");
        int i, numTextGuys;
        float angleIncrement;
        numTextGuys = 6;
        angleIncrement = (float)2*PI/numTextGuys;
        for(i = 0; i < numTextGuys ; i++){
            err = RenderCFString(context, ourString, theStyle, 0, 0);
            if(err)
                printf("got an error from RenderCFString\n");
            CGContextRotateCTM(context, angleIncrement);
        }
        CGContextRotateCTM(context, angleIncrement); // one last for fun!

	// we want to keep the style for the next time
	// our drawProc is called so we don't call
	// ATSUDisposeStyle(theStyle);
    }else
        printf("got an error from MakeATSUIStyle\n");

    return err;
}
