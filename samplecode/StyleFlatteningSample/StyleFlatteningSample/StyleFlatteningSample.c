/*
	File:		StyleFlatteningSample.c
	
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

                Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/
// This code will run on Mac OS X 10.2 (or later) ONLY!!!

#include <Carbon/Carbon.h>

// ------------------------------------------------------------------------------------
// Constants
// ------------------------------------------------------------------------------------

enum {
	
	kInfoStringBufferSize = 255,
	kInfoStringUnicodeBufferSize = 2 * kInfoStringBufferSize,
	
	kGlobalStyleArraySize = 5,
	kGlobalStyleRunInfoArraySize = 6,
	
	kGlobalXOffsetValue = 5,
	kGlobalYSpaceBetweenTests = 30
	
};

static UniChar 		gTestString[] = 
	{ 	0x0054, 0x0068, 0x0069, 0x0073, 0x0020,							// This<sp>
		0x0069, 0x0073,	0x0020,											// is<sp>
		0x0061, 0x0020, 0x0073, 0x006D, 0x0061, 0x006C, 0x006C, 0x0020,	// a<sp>small<sp>
		0x004C, 0x0069, 0x0074, 0x0074, 0x006C, 0x0065, 0x0020,			// Little<sp>
		0x0054, 0x0045, 0x0053, 0x0054, 0x002E,	0x0020,					// Test.
		0x0046, 0x006C, 0x0075, 0x0066, 0x0066, 0x0079, 0x0020,			// Fluffy<sp>
		0x0050, 0x0069, 0x006C, 0x006C, 0x006F, 0x0077, 0x0073 };		// Pillows
		
static UniCharCount	gTestStringLen = 43;


static ATSUStyle		gInfoStyle;
static ATSUStyle		gGlobalStyleArray[kGlobalStyleArraySize];
static ATSUStyleRunInfo	gGlobalStyleRunInfoArray[kGlobalStyleRunInfoArraySize];

static char				gStyle0Font[] = "American Typewriter Bold\0";
static Fixed			gStyle0FontSize = (Fixed) (24 << 16);
static RGBColor			gStyle0FontColor = {0,0,0};

static char				gStyle1Font[] = "Geneva\0";
static Fixed			gStyle1FontSize = (Fixed) (24 << 16);
static RGBColor			gStyle1FontColor = {0xFFFF,0,0};

static char				gStyle2Font[] = "Copperplate Bold\0";
static Fixed			gStyle2FontSize = (Fixed) (10 << 16);
static RGBColor			gStyle2FontColor = {0,0xFFFF,0};

static char				gStyle3Font[] = "Zapfino\0";
static Fixed			gStyle3FontSize = (Fixed) (10 << 16);
static RGBColor			gStyle3FontColor = {0,0,0xFFFF};

static char				gStyle4Font[] = "Helvetica Bold\0";
static Fixed			gStyle4FontSize = (Fixed) (18 << 16);
static RGBColor			gStyle4FontColor = {0,0,0};

static char				gInfoStyleFont[] = "Monaco\0";
static Fixed			gInfoStyleFontSize = (Fixed) (10 << 16);
static RGBColor			gInfoStyleFontColor = {0xFFFF,0,0};

// ------------------------------------------------------------------------------------
// Function Prototypes
// ------------------------------------------------------------------------------------

static
OSStatus RunRoundTripFlatteningSample( WindowRef windowRef );

static
OSStatus RoundTripFlatten( CGContextRef cgContext, Fixed currentXPos,
	Fixed *currentYPos, ATSUStyle styleArray[], ItemCount styleArraySize,
	ATSUStyleRunInfo styleRunArray[], ItemCount styleRunArraySize,
	UniChar *stringPtr, UniCharCount stringLen );
	
static
OSStatus DrawLayoutForStyleAndRunInfo(  ATSUStyle styleArray[],
	ItemCount styleArraySize, ATSUStyleRunInfo styleRunArray[],
	ItemCount styleRunArraySize, UniChar *stringPtr, UniCharCount stringLen,
	CGContextRef cgContext,	Fixed currentXPos, Fixed *currentYPos  );
	
static
OSStatus SetYPositionForLineHeight( ATSUTextLayout currentLayout,
	Fixed *currentYPos );
	
static
OSStatus InitializeGlobalStyles( void );

static
OSStatus CreateStyleObjectWithFontName( char *fontName, Fixed *fontSize,
	RGBColor *fontColor, ATSUStyle *newStyle );
	
static
OSStatus AddFunkyVariationsAndFeatures( ATSUStyle styleToMangle );

static
OSStatus AddTextLabel( const char *textString,
	CGContextRef cgContext,	Fixed currentXPos, Fixed *currentYPos );


// ------------------------------------------------------------------------------------
// main
// ------------------------------------------------------------------------------------


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
	
	// Run the sample code
	err = RunRoundTripFlatteningSample( window );
	check_noerr( err );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

// ------------------------------------------------------------------------------------
// RunRoundTripFlatteningSample												[EXPORT]
// ------------------------------------------------------------------------------------

static
OSStatus RunRoundTripFlatteningSample( 
	WindowRef	windowRef )
{
	OSStatus				err;
	CGrafPtr				windowPort;
	CGContextRef			cgContext;
	Rect					windowRect;
	PixMapHandle			pixMap;
	ATSUTextMeasurement		xPosition;
	ATSUTextMeasurement		yPosition;

	// initialize the global styles 
	err = InitializeGlobalStyles();
	require_noerr( err, RunATSUIFlatteningTest_err );
	
	// get the port for the window
	windowPort = GetWindowPort( windowRef );
	
	// get the port's pixMap. This will be used to find the viewable window area
	pixMap = GetPortPixMap( windowPort );
	
	// get the bounds for the port
	GetPortBounds( windowPort, &windowRect );
	
	// Offset the rect to obtain the window's viewable area
	OffsetRect( &windowRect, -(**pixMap).bounds.left, -(**pixMap).bounds.top );
	
	// calculate the x and y positions
	xPosition = Long2Fix( windowRect.left + kGlobalXOffsetValue );
	yPosition = Long2Fix( windowRect.bottom - kGlobalYSpaceBetweenTests );
	
	// start the cgContext for the port
	err = QDBeginCGContext( windowPort, &cgContext );
	require_noerr( err, RunATSUIFlatteningTest_err );
	
	// add a label to the test
	err = AddTextLabel( "Reference Style\0", cgContext, xPosition, 
		&yPosition );
	require_noerr( err, RunATSUIFlatteningTest_err );
	
	// create the layout for the default run information
	err = DrawLayoutForStyleAndRunInfo( gGlobalStyleArray, kGlobalStyleArraySize,
		gGlobalStyleRunInfoArray, kGlobalStyleRunInfoArraySize, gTestString,
		gTestStringLen, cgContext, xPosition, &yPosition );
	require_noerr( err, RunATSUIFlatteningTest_err );
	
	// add some space to separate the tests
	yPosition -= Long2Fix( kGlobalYSpaceBetweenTests );
	
	// add a label to the test
	err = AddTextLabel( "Flattened And Unflattened Sample\0", cgContext,
		xPosition, &yPosition );
	require_noerr( err, RunATSUIFlatteningTest_err );
	
	// run the default options for this test
	err = RoundTripFlatten( cgContext, xPosition, &yPosition, gGlobalStyleArray,
		kGlobalStyleArraySize, gGlobalStyleRunInfoArray, kGlobalStyleRunInfoArraySize,
		gTestString, gTestStringLen );
	require_noerr( err, RunATSUIFlatteningTest_err );
	
	// kill the cgContext for the port
	err = QDEndCGContext( windowPort, &cgContext );
	require_noerr( err, RunATSUIFlatteningTest_err );
	

RunATSUIFlatteningTest_err:

	return err;

}

// ------------------------------------------------------------------------------------
// RoundTripFlatten															[INTERNAL]
// ------------------------------------------------------------------------------------

// All this function illustrates is how to flatten style information and then re-
// construct it from the flattened stream.

static
OSStatus RoundTripFlatten(
	CGContextRef					cgContext,
	Fixed							currentXPos,
	Fixed							*currentYPos,
	ATSUStyle						styleArray[], 
	ItemCount						styleArraySize,
	ATSUStyleRunInfo				styleRunArray[],
	ItemCount						styleRunArraySize,
	UniChar							*stringPtr,
	UniCharCount					stringLen )
{
	OSStatus			err;
	ItemCount			i;
	ByteCount			suggestedStreamBufferSize;
	ItemCount			suggestedNumberOfRunInfo;
	ItemCount			suggestedNumberOfStyleObjects;
	ByteCount			actualStreamBufferSize;
	ItemCount			actualNumberOfRunInfo;
	ItemCount			actualNumberOfStyleObjects;
	void				*streamBuffer = NULL;
	ATSUStyle			*unflattenedStyleArray = NULL;
	ATSUStyleRunInfo	*unflattenedStyleRunArray = NULL;
	
	
	// get the size of the outgoing stream
	err = ATSUFlattenStyleRunsToStream( kATSUDataStreamUnicodeStyledText,
		kATSUFlattenOptionNoOptionsMask, styleRunArraySize, styleRunArray,
		styleArraySize, styleArray, 0, NULL, &suggestedStreamBufferSize );
	require_noerr( err, RoundTripFlatteningSample_err );
	
	// allocate a buffer big enough to hold the stream
	streamBuffer = (void *) malloc( suggestedStreamBufferSize );
	require_action( streamBuffer != NULL, RoundTripFlatteningSample_err,
		err = memFullErr );
		
		
	// now, flatten the stream
	err = ATSUFlattenStyleRunsToStream( kATSUDataStreamUnicodeStyledText,
		kATSUFlattenOptionNoOptionsMask, styleRunArraySize, styleRunArray,
		styleArraySize, styleArray, suggestedStreamBufferSize,
		streamBuffer, &actualStreamBufferSize );
	require_noerr( err, RoundTripFlatteningSample_err );
	
	
	
	
	// next, try to unflatten the stream that was just flattened. First, try
	// to get counts of the stuff in the stream, so that we can allocate
	// accordingly.
	err = ATSUUnflattenStyleRunsFromStream( kATSUDataStreamUnicodeStyledText,
		kATSUUnFlattenOptionNoOptionsMask, actualStreamBufferSize, streamBuffer,
		0, 0, NULL, NULL,  &suggestedNumberOfRunInfo,
		&suggestedNumberOfStyleObjects );
	require_noerr( err, RoundTripFlatteningSample_err );
			
	// allocate some buffer space for the style
	unflattenedStyleArray = (ATSUStyle *) malloc( sizeof( ATSUStyle ) *
		suggestedNumberOfStyleObjects );
	require_action( unflattenedStyleArray != NULL, RoundTripFlatteningSample_err,
		err = memFullErr );
		
	// allocate some buffer space for the style runs
	unflattenedStyleRunArray = (ATSUStyleRunInfo *) malloc( 
		sizeof( ATSUStyleRunInfo ) * suggestedNumberOfRunInfo );
	require_action( unflattenedStyleRunArray != NULL, RoundTripFlatteningSample_err,
		err = memFullErr );
		
		
	// try to unflatten the stream for real now.
	err = ATSUUnflattenStyleRunsFromStream( kATSUDataStreamUnicodeStyledText,
		kATSUUnFlattenOptionNoOptionsMask, actualStreamBufferSize, streamBuffer,
		suggestedNumberOfRunInfo, suggestedNumberOfStyleObjects,
		unflattenedStyleRunArray, unflattenedStyleArray, &actualNumberOfRunInfo,
		&actualNumberOfStyleObjects );
	require_noerr( err, RoundTripFlatteningSample_err );
	
	// try to draw the text with the unflattened stream
	err = DrawLayoutForStyleAndRunInfo( unflattenedStyleArray,
		actualNumberOfStyleObjects, unflattenedStyleRunArray, actualNumberOfRunInfo,
		stringPtr, stringLen, cgContext, currentXPos, currentYPos );
	require_noerr( err, RoundTripFlatteningSample_err );
	
	// now, we need to get rid of the text layouts that were produced by the 
	// ATSUUnflattenStyleRunsFromStream
	for ( i = 0; i < actualNumberOfStyleObjects; i++ )
	{
		ATSUDisposeStyle( unflattenedStyleArray[i] );
	}
	
	// finally, we can dispose of the buffers that we had allocated
	free( streamBuffer );
	free( unflattenedStyleArray );
	free( unflattenedStyleRunArray );
	
		
				
RoundTripFlatteningSample_err:
	
	// flush the context
	CGContextFlush( cgContext );

	// return noErr so that our error and debugging messages printout
	return noErr;	

}

// ------------------------------------------------------------------------------------
// DrawLayoutForStyleAndRunInfo												[INTERNAL]
// ------------------------------------------------------------------------------------

// Normally, this is not the most optimal way of using ATSUI. The reason being is that
// a lot is wasted in the continual creation and destruction of the ATSUTextLayout
// object. However, this is just a sample and it's execution speed is not that
// critical - in fact, it's not all that critical that anything actually gets drawn
// on the screen.

static
OSStatus DrawLayoutForStyleAndRunInfo( 
	ATSUStyle			styleArray[], 
	ItemCount			styleArraySize,
	ATSUStyleRunInfo	styleRunArray[],
	ItemCount			styleRunArraySize,
	UniChar				*stringPtr,
	UniCharCount		stringLen,
	CGContextRef		cgContext,
	Fixed				currentXPos,
	Fixed				*currentYPos  )
{
	OSStatus				err;
	ATSUStyle				*seperateStyles = NULL;
	UniCharCount			*seperateRunLengths = NULL;
	ItemCount				numberOfRuns = 0;
	ATSUAttributeTag		attrTag;
	ByteCount				attrSize;
	ATSUAttributeValuePtr	attrPtr;
	ATSUTextLayout			newLayout;
	
	// check to see if we have a style array
	if ( styleArray != NULL )
	{
		// if the styleRunArray is NULL, then there is only one run
		if ( styleRunArray == NULL )
		{
			numberOfRuns = 1;
			seperateStyles = styleArray;
			seperateRunLengths = &stringLen;
		}
		else
		{
			ItemCount	i;
		
			// the number of runs is equal to the number of runs passed in
			numberOfRuns = styleRunArraySize;
		
			// allocate a seperateStyles and seperateRuns array
			seperateStyles = (ATSUStyle *) malloc( sizeof( ATSUStyle ) * 
				numberOfRuns );
			require_action( seperateStyles != NULL, 
				CreateLayoutForStyleAndRunInfo_err, err = memFullErr );
				
			seperateRunLengths = (UniCharCount *) malloc( sizeof( UniCharCount ) *
				numberOfRuns );
			require_action( seperateRunLengths != NULL, 
				CreateLayoutForStyleAndRunInfo_err, err = memFullErr );
				
			// loop through and assign the runs to the seperate arrays. I'm not
			// sure that this is the best way to do this. Perhaps it's best to
			// simply create the layout and assign the style runs to it individually.
			for ( i = 0; i < numberOfRuns; i++ )
			{
				seperateStyles[i] = styleArray[styleRunArray[i].styleObjectIndex];
				seperateRunLengths[i] = styleRunArray[i].runLength;
			}
			
		}
		
	}

	// first of all, create the layout with the text information passed in
	err = ATSUCreateTextLayoutWithTextPtr( stringPtr, kATSUFromTextBeginning,
		kATSUToTextEnd, stringLen, numberOfRuns, seperateRunLengths, seperateStyles,
		&newLayout );
	require_noerr( err, CreateLayoutForStyleAndRunInfo_err );
	
	// if we've got a layout, then assign the CGContext to it
	attrTag = kATSUCGContextTag;
	attrSize = sizeof( CGContextRef );
	attrPtr = &cgContext;
	
	err = ATSUSetLayoutControls( newLayout, 1, &attrTag, &attrSize, &attrPtr );
	require_noerr( err, CreateLayoutForStyleAndRunInfoLayout_err );
	
	// set the Y position before we draw
	err = SetYPositionForLineHeight( newLayout, currentYPos );
	require_noerr( err, CreateLayoutForStyleAndRunInfoLayout_err );
	
	// do some drawin'
	err = ATSUDrawText( newLayout, kATSUFromTextBeginning, kATSUToTextEnd,
		currentXPos, *currentYPos );
	require_noerr( err, CreateLayoutForStyleAndRunInfoLayout_err );
	
CreateLayoutForStyleAndRunInfoLayout_err:

	ATSUDisposeTextLayout( newLayout );
	
CreateLayoutForStyleAndRunInfo_err:

	if ( ( styleRunArray != NULL ) && ( styleArray != NULL ) ) 
	{
		if ( seperateStyles != NULL )
		{
			free( seperateStyles );
		}
		
		if ( seperateRunLengths != NULL );
		{
			free( seperateRunLengths );
		}
		
	}
	
	return err;

}

// ------------------------------------------------------------------------------------
// SetYPositionForLineHeight												[INTERNAL]
// ------------------------------------------------------------------------------------

static
OSStatus SetYPositionForLineHeight(
	ATSUTextLayout		currentLayout,
	Fixed				*currentYPos )
{
	OSStatus				err;
	ATSTrapezoid			glyphBounds;
	ItemCount				numGlyphBounds;
	Fixed					lineHeight;
	
	// we need to calculate where the line should start, so get the height of the
	// line.
	err = ATSUGetGlyphBounds( currentLayout, 0, 0, kATSUFromTextBeginning,
		kATSUToTextEnd, kATSUseCaretOrigins, 1, &glyphBounds, &numGlyphBounds );
	require_noerr( err, SetYPositionForLineHeight_err );
	
	lineHeight = glyphBounds.lowerRight.y - glyphBounds.upperRight.y;
	
	// reset the line position based on the line height
	*currentYPos -= lineHeight;
	
SetYPositionForLineHeight_err: 

	return err;
}


// ------------------------------------------------------------------------------------
// InitializeGlobalStyles													[INTERNAL]
// ------------------------------------------------------------------------------------

static
OSStatus InitializeGlobalStyles( void )
{
	OSStatus err;

	// initialize the runs one-by-one, based on our test string
	gGlobalStyleRunInfoArray[0].runLength = 5;
	gGlobalStyleRunInfoArray[1].runLength = 3;
	gGlobalStyleRunInfoArray[2].runLength = 8;
	gGlobalStyleRunInfoArray[3].runLength = 7;
	gGlobalStyleRunInfoArray[4].runLength = 6;
	gGlobalStyleRunInfoArray[5].runLength = 14;
	
	gGlobalStyleRunInfoArray[0].styleObjectIndex = 0;
	gGlobalStyleRunInfoArray[1].styleObjectIndex = 1;
	gGlobalStyleRunInfoArray[2].styleObjectIndex = 2;
	gGlobalStyleRunInfoArray[3].styleObjectIndex = 0;
	gGlobalStyleRunInfoArray[4].styleObjectIndex = 3;
	gGlobalStyleRunInfoArray[5].styleObjectIndex = 4;
	
	// setup the first style object
	err = CreateStyleObjectWithFontName( gStyle0Font, &gStyle0FontSize,
		&gStyle0FontColor, &gGlobalStyleArray[0] );
	require_noerr( err, InitializeGlobalStyles_err );
	
	// setup the second style object
	err = CreateStyleObjectWithFontName( gStyle1Font, &gStyle1FontSize,
		&gStyle1FontColor, &gGlobalStyleArray[1] );
	require_noerr( err, InitializeGlobalStyles_err );
	
	// setup the third style object
	err = CreateStyleObjectWithFontName( gStyle2Font, &gStyle2FontSize,
		&gStyle2FontColor, &gGlobalStyleArray[2] );
	require_noerr( err, InitializeGlobalStyles_err );
	
	// setup the fourth style object
	err = CreateStyleObjectWithFontName( gStyle3Font, &gStyle3FontSize,
		&gStyle3FontColor, &gGlobalStyleArray[3] );
	require_noerr( err, InitializeGlobalStyles_err );

	// add some variations to the third style object
	err = AddFunkyVariationsAndFeatures( gGlobalStyleArray[3] );
	require_noerr( err, InitializeGlobalStyles_err );
	
	// setup the fifth style object
	err = CreateStyleObjectWithFontName( gStyle4Font, &gStyle4FontSize,
		&gStyle4FontColor, &gGlobalStyleArray[4] );
	require_noerr( err, InitializeGlobalStyles_err );	
	
	// setup the info string style object
	err = CreateStyleObjectWithFontName( gInfoStyleFont, &gInfoStyleFontSize,
		&gInfoStyleFontColor, &gInfoStyle );
	require_noerr( err, InitializeGlobalStyles_err );
	
	
InitializeGlobalStyles_err:

	return err;
	
}

// ------------------------------------------------------------------------------------
// CreateStyleObjectWithFontName											[INTERNAL]
// ------------------------------------------------------------------------------------

static
OSStatus CreateStyleObjectWithFontName(
	char		*fontName,
	Fixed		*fontSize,
	RGBColor	*fontColor,
	ATSUStyle	*newStyle )
{
	OSStatus				err;
	ATSUFontID				fontID;
	ATSUAttributeTag		tags[3];
	ByteCount				tagValueSizes[3];
	ATSUAttributeValuePtr	tagValuePtrs[3];
	
	
	// try to find the font based on the font name
	err = ATSUFindFontFromName( fontName, strlen( fontName ), kFontFullName,
		kFontNoPlatform, kFontNoScript, kFontNoLanguage, &fontID );
	require_noerr( err, CreateStyleObjectWithFontName_err );

	// first, create the new style object
	err = ATSUCreateStyle( newStyle );
	
	// set up the three tags that we are setting in the style
	tags[0] = kATSUFontTag;
	tagValueSizes[0] = sizeof( ATSUFontID );
	tagValuePtrs[0] = &fontID;
	
	tags[1] = kATSUSizeTag;
	tagValueSizes[1] = sizeof( Fixed );
	tagValuePtrs[1] = fontSize;
	
	tags[2] = kATSUColorTag;
	tagValueSizes[2] = sizeof( RGBColor );
	tagValuePtrs[2] = fontColor;
	
	// set the attributes in the style object
	err = ATSUSetAttributes( *newStyle, 3, tags, tagValueSizes, tagValuePtrs );
	check_noerr( err );
	
	// if there was an error, then dispose of the style
	if ( err != noErr ) {
		ATSUDisposeStyle( *newStyle );
	}
	
CreateStyleObjectWithFontName_err: 

	return err;
	
}

// ------------------------------------------------------------------------------------
// AddFunkyVariationsAndFeatures											[INTERNAL]
// ------------------------------------------------------------------------------------

static
OSStatus AddFunkyVariationsAndFeatures(
	ATSUStyle	styleToMangle )
{
	OSStatus				err;
	ItemCount				i;
	ATSUFontID				fontID;
	ItemCount				count;
	ItemCount				selectorCount;
	ATSUFontVariationAxis	variationAxis;
	ATSUFontVariationValue	variationMinimum;
	ATSUFontVariationValue	variationMaximum;
	ATSUFontVariationValue	variationDefault;
	Boolean					selectorMutex;
	ATSUFontFeatureType		*typeBuffer = NULL;
	ATSUFontFeatureSelector	*selectorBuffer = NULL;
	Boolean					*defaultBuffer = NULL;
	ItemCount				selectorBufferSize = 0;
	
	
	// get the fontID from the style
	err = ATSUGetAttribute( styleToMangle, kATSUFontTag, sizeof( ATSUStyle ),
		&fontID, NULL );
	require_noerr( err, AddFunkyVariationsAndFeatures_err );

	// get a count of all of the variations supported by the current font
	err = ATSUCountFontVariations( fontID, &count );
	require_noerr( err, AddFunkyVariationsAndFeatures_err );
	
	// loop through, setting all of the variations to the max!
	for ( i = 0; i < count; i++ )
	{
		// get the settings for the variations
		err = ATSUGetIndFontVariation( fontID, i, &variationAxis,
			&variationMinimum, &variationMaximum, &variationDefault );
		require_noerr( err, AddFunkyVariationsAndFeatures_err );
		
		// set the variation for this font to the max!
		err = ATSUSetVariations( styleToMangle, 1, &variationAxis,
			&variationMaximum );
		require_noerr( err, AddFunkyVariationsAndFeatures_err );
		
	}
	
	
	// get a count of all of the features
	err = ATSUCountFontFeatureTypes( fontID, &count );
	require_noerr( err, AddFunkyVariationsAndFeatures_err );
	
	// allocate a buffer for the feature types
	typeBuffer = (ATSUFontFeatureType *) malloc( sizeof( ATSUFontFeatureType )
		* count );
	require_action( typeBuffer != NULL, AddFunkyVariationsAndFeatures_err,
		err = paramErr );
		
	// get all of the font features
	err = ATSUGetFontFeatureTypes( fontID, count, typeBuffer, &selectorCount );
	require_noerr( err, AddFunkyVariationsAndFeatures_err );
	
	// loop through, setting all of the features
	for ( i = 0; i < count; i++ )
	{
		// get the selector count
		err = ATSUCountFontFeatureSelectors( fontID, typeBuffer[i],
			&selectorCount );
		require_noerr( err, AddFunkyVariationsAndFeatures_err );
		
		// if the selector buffer size is greater than what we had before,
		// then allocate some new buffers. Just use realloc, as when
		// the buffers are pointing to NULL, it will simply act as malloc.
		if ( selectorBufferSize < count ) 
		{
			ItemCount		newCount;
			
			// set the new size to be twice the old size
			newCount = 2 * count;
			
			// allocate  the selectorBuffer
			selectorBuffer = (ATSUFontFeatureSelector *) realloc( 
				selectorBuffer, newCount * sizeof( ATSUFontFeatureSelector ) );
			require_action( selectorBuffer != NULL,
				AddFunkyVariationsAndFeatures_err, err = memFullErr );
				
			// allocate the defaultBuffer
			defaultBuffer = (Boolean  *) realloc( defaultBuffer,
				newCount * sizeof( Boolean ) );
			require_action( defaultBuffer != NULL,
				AddFunkyVariationsAndFeatures_err, err = memFullErr );
				
			selectorBufferSize = newCount;
		}
		
		// get the font feature selectors
		err = ATSUGetFontFeatureSelectors( fontID, typeBuffer[i],
			selectorBufferSize, selectorBuffer, defaultBuffer, &selectorCount,
			&selectorMutex );
		require_noerr( err, AddFunkyVariationsAndFeatures_err );
		
		// if the features are not mutually exclusive, then go ahead and
		// set them all.
		if ( selectorMutex == false )
		{
			ItemCount	j;
			
			// loop through and set all of the features
			for ( j = 0; j < selectorCount; j++ )
			{
				if ( defaultBuffer[j] == false )
				{
					err = ATSUSetFontFeatures( styleToMangle, 1, &typeBuffer[i],
						&selectorBuffer[j] );
					require_noerr( err, AddFunkyVariationsAndFeatures_err );
				}
			}
			
		}
		else
		{
			// just set the last option, then since we can't set them all
			err = ATSUSetFontFeatures( styleToMangle, 1, &typeBuffer[i],
				&selectorBuffer[selectorCount - 1] );
			require_noerr( err, AddFunkyVariationsAndFeatures_err );
		}
		
	}
		
	// that should be enough style perversion for now!
	
	
AddFunkyVariationsAndFeatures_err:

	if ( typeBuffer != NULL )
	{
		free( typeBuffer );
	}
	
	if ( selectorBuffer != NULL )
	{
		free( selectorBuffer );
	}
	
	if ( defaultBuffer != NULL )
	{
		free( defaultBuffer );
	}

	return err;
	
}

// ------------------------------------------------------------------------------------
// AddTextLabel																[INTERNAL]
// ------------------------------------------------------------------------------------

static
OSStatus AddTextLabel(
	const char		*textString,
	CGContextRef	cgContext,
	Fixed			currentXPos,
	Fixed			*currentYPos )
{
	OSStatus			err;
	UniChar				*textBuffer = NULL;
	ByteCount			textBufferLen;
	ByteCount			sourceRead;
	ByteCount			convertedTextLen;
	TextToUnicodeInfo	textToUnicodeInfo;
	TextEncoding		macEncoding;
	
	// create an encoding for mac roman
	macEncoding = CreateTextEncoding( kTextEncodingMacRoman,
		kTextEncodingDefaultVariant, kTextEncodingDefaultFormat );
		
	// create a text to unicode converter
	err = CreateTextToUnicodeInfoByEncoding( macEncoding,
		&textToUnicodeInfo );
	require_noerr( err, AddTextLabel_err );
	
	textBufferLen = strlen( textString );
	
	// loop until we have a buffer big enough to hold the whole conversion
	do {

		// allocate a textBuffer. Twice the size of the text should be about right.
		textBufferLen = textBufferLen * 2;
		textBuffer = (UniChar *) realloc( textBuffer, 
			sizeof( char ) * textBufferLen );
		require_action( textBuffer != NULL, AddTextLabel_err, 
			err = memFullErr );
	
		// run the conversion. If we get a full error, then reallocate otherwise,
		// bail
		err = ConvertFromTextToUnicode( textToUnicodeInfo, strlen( textString ),
			textString, kUnicodeUseFallbacksMask | kUnicodeLooseMappingsMask, 0,
			NULL, NULL, NULL, textBufferLen, &sourceRead, &convertedTextLen,
			textBuffer );
			
	} while ( err == kTECOutputBufferFullStatus );
	
	// dispose of the converter
	DisposeTextToUnicodeInfo( &textToUnicodeInfo );
	
	// fallback error is okay. Everything else is bad.
	if ( err != kTECUsedFallbacksStatus )
	{
		require_noerr( err, AddTextLabel_err );
	}
	
	// finally, we can get to actual ATSUI stuff. Create the layout with
	// all of the default stuff for the test labels.
	err = DrawLayoutForStyleAndRunInfo( &gInfoStyle, 1, NULL, 0,
		textBuffer, (UniCharCount) (convertedTextLen / 2), cgContext,
		currentXPos, currentYPos );
	require_noerr( err, AddTextLabel_err );
	
AddTextLabel_err:

	if ( textBuffer != NULL )
	{
		free( textBuffer );
	}
	
	return err;

}

