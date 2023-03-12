/*
	File:		DirectAccessCallbacks.c
	
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

#include "HIElements.h"
#include "MenuHandler.h"
#include "DirectAccessCallbacks.h"
extern float  sinf( float x );  // definition for libstdc sinf
// ------------------------------------------------------------------------------
// Data Structures
// ------------------------------------------------------------------------------

typedef struct ReplacementGlyphInfoStruct {

	ATSGlyphRef			glyphID;
	Fixed				glyphHalfAdvance;
	ATSUStyleSettingRef	styleRef;
	ATSUStyleSettingRef	*styleSettingArray;
		
	ATSUTextLayout		replacementLayout;
	ATSUStyle			replacementStyle;

} ReplacementGlyphInfoStruct;

// ------------------------------------------------------------------------------
// Global Variables
// ------------------------------------------------------------------------------

static UniChar						gReplacementChar = 0x2022;
static RGBColor						gReplacementCharRGBColor = {0,0,0xFFFF};
static UniCharCount					gReplacementCharLength = 1;
static ReplacementGlyphInfoStruct	gReplacementGlyph;
static Boolean						gReplacementGlyphInitialized = false;

// ------------------------------------------------------------------------------
// Function Prototypes
// ------------------------------------------------------------------------------

#define FixedToFloat(a) ((float)(a) / fixed1)
#define FloatToFixed(a) ((Fixed)((float)(a) * fixed1))

static OSStatus GlyphReplacementCallback(
	ATSULayoutOperationSelector iCurrentOperation, ATSULineRef iLineRef,
	UInt32 iRefCon, void *iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus *oCallbackStatus );
	
static OSStatus StretchyGlyphCallback(
	ATSULayoutOperationSelector	iCurrentOperation, ATSULineRef iLineRef,
	UInt32 iRefCon, void *iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus *oCallbackStatus );
	
static OSStatus ShrinkyGlyphCallback(
	ATSULayoutOperationSelector iCurrentOperation, ATSULineRef iLineRef,
	UInt32 iRefCon, void *iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus *oCallbackStatus );
	
static OSStatus GlyphWaveCallback(
	ATSULayoutOperationSelector iCurrentOperation, ATSULineRef	iLineRef,
	UInt32 iRefCon, void *iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus *oCallbackStatus );
	
static OSStatus GetReplacementGlyphStruct(
	ReplacementGlyphInfoStruct **replacementStruct );
	
// ------------------------------------------------------------------------------
// InstallGlyphReplacementCallback										[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus InstallGlyphReplacementCallback(
	ATSUTextLayout textLayout )
{
	OSStatus 								err;
	ATSULayoutOperationOverrideSpecifier 	overrideSpec;
	ATSUAttributeTag						attrTag;
	ByteCount								attrSize;
	ATSUAttributeValuePtr					attrPtr;

	// setup the post layout adjustment
	overrideSpec.operationSelector = kATSULayoutOperationPostLayoutAdjustment;
	overrideSpec.overrideUPP = GlyphReplacementCallback;
	
	// set the override spec in the layout object
	attrTag = kATSULayoutOperationOverrideTag;
	attrSize = sizeof( ATSULayoutOperationOverrideSpecifier );
	attrPtr = &overrideSpec;
	
	// set the override spec in the layout
	err = ATSUSetLayoutControls( textLayout, 1, &attrTag, &attrSize, &attrPtr );
	require_noerr( err, InstallGlyphReplacementCallback_err );
	
InstallGlyphReplacementCallback_err:

	return err;

}

// ------------------------------------------------------------------------------
// InstallStrechyGlyphCallback											[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus InstallStrechyGlyphCallback(
	ATSUTextLayout textLayout )
{
	OSStatus 								err;
	ATSULayoutOperationOverrideSpecifier 	overrideSpec;
	ATSUAttributeTag						attrTag;
	ByteCount								attrSize;
	ATSUAttributeValuePtr					attrPtr;

	// setup the post layout adjustment
	overrideSpec.operationSelector = kATSULayoutOperationJustification;
	overrideSpec.overrideUPP = StretchyGlyphCallback;
	
	// set the override spec in the layout object
	attrTag = kATSULayoutOperationOverrideTag;
	attrSize = sizeof( ATSULayoutOperationOverrideSpecifier );
	attrPtr = &overrideSpec;
	
	// set the override spec in the layout
	err = ATSUSetLayoutControls( textLayout, 1, &attrTag, &attrSize, &attrPtr );
	require_noerr( err, InstallStrechyGlyphCallback_err );
	
InstallStrechyGlyphCallback_err:

	return err;
}

// ------------------------------------------------------------------------------
// InstallShrinkyGlyphCallback											[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus InstallShrinkyGlyphCallback(
	ATSUTextLayout textLayout )
{
	OSStatus 								err;
	ATSULayoutOperationOverrideSpecifier 	overrideSpec;
	ATSUAttributeTag						attrTag;
	ByteCount								attrSize;
	ATSUAttributeValuePtr					attrPtr;

	// setup the post layout adjustment
	overrideSpec.operationSelector = kATSULayoutOperationJustification;
	overrideSpec.overrideUPP = ShrinkyGlyphCallback;
	
	// set the override spec in the layout object
	attrTag = kATSULayoutOperationOverrideTag;
	attrSize = sizeof( ATSULayoutOperationOverrideSpecifier );
	attrPtr = &overrideSpec;
	
	// set the override spec in the layout
	err = ATSUSetLayoutControls( textLayout, 1, &attrTag, &attrSize, &attrPtr );
	require_noerr( err, InstallShrinkyGlyphCallback_err );
	
InstallShrinkyGlyphCallback_err:

	return err;
}

// ------------------------------------------------------------------------------
// UpdateCallbackContexts												[EXPORT]
// ------------------------------------------------------------------------------

extern
OSStatus UpdateCallbackContexts( void )
{

	OSStatus err = noErr;

	// if the override struct is invalid, make sure we invalidate it, this
	// installation could have come as the result of a font or size change.
	if ( gReplacementGlyphInitialized == true )
	{
		// dispose of the style object in it
		err = ATSUDisposeStyle( gReplacementGlyph.replacementStyle );
		require_noerr( err, UpdateCallbackContexts_err );
		
		// dispose of the layout object in it
		err = ATSUDisposeTextLayout( gReplacementGlyph.replacementLayout );
		require_noerr( err, UpdateCallbackContexts_err );
		
		// dipose of the style ref array inside of it
		err = ATSUDirectReleaseLayoutDataArrayPtr( NULL,
			kATSUDirectDataStyleSettingATSUStyleSettingRefArray,
			(void **) &gReplacementGlyph.styleSettingArray );	
		require_noerr( err, UpdateCallbackContexts_err );
		
		// the glyph is no longer initialize, so flag it as such
		gReplacementGlyphInitialized = false;
	}
	
	
UpdateCallbackContexts_err:
	
	return err;
	
}

// ------------------------------------------------------------------------------
// InstallGlyphWaveCallback											[INTERNAL]
// ------------------------------------------------------------------------------

extern
OSStatus InstallGlyphWaveCallback(
	ATSUTextLayout textLayout )
{
	OSStatus 								err;
	ATSULayoutOperationOverrideSpecifier 	overrideSpec;
	ATSUAttributeTag						attrTag;
	ByteCount								attrSize;
	ATSUAttributeValuePtr					attrPtr;

	// setup the post layout adjustment
	overrideSpec.operationSelector = kATSULayoutOperationPostLayoutAdjustment;
	overrideSpec.overrideUPP = GlyphWaveCallback;
	
	// set the override spec in the layout object
	attrTag = kATSULayoutOperationOverrideTag;
	attrSize = sizeof( ATSULayoutOperationOverrideSpecifier );
	attrPtr = &overrideSpec;
	
	// set the override spec in the layout
	err = ATSUSetLayoutControls( textLayout, 1, &attrTag, &attrSize, &attrPtr );
	require_noerr( err, InstallGlyphWaveCallback_err );
	
InstallGlyphWaveCallback_err:

	return err;
}

// ------------------------------------------------------------------------------
// GlyphWaveCallback												[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus GlyphWaveCallback(
	ATSULayoutOperationSelector			iCurrentOperation,
	ATSULineRef							iLineRef,
	UInt32								iRefCon,
	void								*iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus	*oCallbackStatus )
{
	OSStatus 		err;
	Fixed			*deltaYArray;
	Fixed			*deltaXArray;
	Fixed			slack;
	ATSLayoutRecord	*layoutRecordArray;
	ItemCount		recordArrayCount;
	ItemCount		i;
	Fixed			scaleFactor = 0;
	float			amplitude;

	// grab the glyph array for the line. We're going to calculate the sine
	// based on the real positions of the glyphs for a much smoother layout.
	err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
		kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, false,
		(void **) &layoutRecordArray, &recordArrayCount );
	require_noerr( err, GlyphWaveCallback_err );
	
	// grab the advance deltas for the line
	err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
		kATSUDirectDataAdvanceDeltaFixedArray, true,
		(void **) &deltaXArray, &recordArrayCount );
	require_noerr( err, GlyphWaveCallbackDeltaXArray_err );
	
	// grab the baseline deltas for the line
	err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
		kATSUDirectDataBaselineDeltaFixedArray, true,
		(void **) &deltaYArray, &recordArrayCount );
	require_noerr( err, GlyphWaveCallbackDeltaYArray_err );	
	
	// run through and find the largest positional difference in the entire
	// line. This will help us set the scale factor.
	for ( i = 1; i < recordArrayCount; i++ )
	{
		// get the slack (really the difference between the two realpositions,
		// I'm just too lazy to add another variable to the stack with a
		// different name.
		slack = (layoutRecordArray[i].realPos - layoutRecordArray[i-1].realPos);
		
		// check to see if this is the greatest difference. If it is, then
		// set our scale factor
		if ( slack > scaleFactor )
		{
			scaleFactor = slack;
		}
		
	}
	
	// workaround for bug #2913628. If the scale factor is still zero after
	// that, it means that proper realPos aren't being returned, so just fix
	// it at the point size.
	if ( scaleFactor == 0 )
	{
		scaleFactor = gFontSize << 16;
	}
		
	
	// set the amplitude to be the scale factor
	amplitude = FixedToFloat( scaleFactor );
	
	// we're going to anchor the first glyph, so start at the second glyph and
	// start streatching. We don't care too much about the terminator glyph, so
	// don't include that in the wave.
	for ( i = 1; i < recordArrayCount - 1; i++ )
	{
		// calculate the glyph slack. We're trying to impose a fixed width on
		// all of the glyphs of at least their point size
		slack =  scaleFactor - (layoutRecordArray[i].realPos - 
			layoutRecordArray[i-1].realPos);
			
		// add the slack value into the previous glyph's deltaX and this
		// glyph's realPosition. This is done to impose a fixed width on
		// all of the glyphs.
		layoutRecordArray[i].realPos += slack;
		deltaXArray[i-1] += slack;
	
		// calculate the sin deltaY for the current glyph
		deltaYArray[i] = FloatToFixed( sinf( i ) * amplitude );
	};
	
	// dispose of the base line delta array
	ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
		kATSUDirectDataBaselineDeltaFixedArray, (void **) &deltaYArray );
		
		
GlyphWaveCallbackDeltaYArray_err:

	// dispose of the delta x array
	ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
		kATSUDirectDataAdvanceDeltaFixedArray, (void **) &deltaXArray );

GlyphWaveCallbackDeltaXArray_err:

	// dispose of the layout record array
	ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
		kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, 
		(void **) &layoutRecordArray );
	
GlyphWaveCallback_err:

	// return as handled so ATSUI won't do anything to the line
	*oCallbackStatus = kATSULayoutOperationCallbackStatusHandled;


	return err;

}

// ------------------------------------------------------------------------------
// StretchyGlyphCallback											[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus StretchyGlyphCallback(
	ATSULayoutOperationSelector			iCurrentOperation,
	ATSULineRef							iLineRef,
	UInt32								iRefCon,
	void								*iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus	*oCallbackStatus )
{
	OSStatus 		err;
	Fixed			*deltaXArray;
	ItemCount		recordArrayCount;
	ItemCount		i;

	// grab the advance deltas for the line
	err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
		kATSUDirectDataAdvanceDeltaFixedArray, true,
		(void **) &deltaXArray, &recordArrayCount );
	require_noerr( err, StretchyGlyphCallback_err );
	
	// we're going to anchor the first glyph, so start at the second glyph and
	// start streatching!
	for ( i = 1; i < recordArrayCount; i++ ) {
		deltaXArray[i] += (Fixed) ((gFontSize * i) << 16);
	};
	
	// dispose of the array
	err = ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
		kATSUDirectDataAdvanceDeltaFixedArray, (void **) &deltaXArray );

StretchyGlyphCallback_err:

	// return as handled so ATSUI won't do anything to the line
	*oCallbackStatus = kATSULayoutOperationCallbackStatusHandled;

	return err;

}

// ------------------------------------------------------------------------------
// ShrinkyGlyphCallback												[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus ShrinkyGlyphCallback(
	ATSULayoutOperationSelector			iCurrentOperation,
	ATSULineRef							iLineRef,
	UInt32								iRefCon,
	void								*iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus	*oCallbackStatus )
{
	OSStatus 		err;
	Fixed			*deltaXArray;
	ItemCount		recordArrayCount;
	ItemCount		i;
	Fixed			shrinkFactor;
	
	// set the shrink factor to a factor of the current font size
	shrinkFactor = ( gFontSize / kFontSizeMinimum ) << 16;

	// grab the advance deltas for the line
	err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
		kATSUDirectDataAdvanceDeltaFixedArray, true,
		(void **) &deltaXArray, &recordArrayCount );
	require_noerr( err, ShrinkyGlyphCallback_err );
	
	// we're going to anchor the first glyph, so start at the second glyph and
	// start streatching!
	for ( i = 1; i < recordArrayCount; i++ ) {
		deltaXArray[i] -= shrinkFactor;
	};
	
	// dispose of the array
	err = ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
		kATSUDirectDataAdvanceDeltaFixedArray, (void **) &deltaXArray );

ShrinkyGlyphCallback_err:

	// return as handled so ATSUI won't do anything to the line
	*oCallbackStatus = kATSULayoutOperationCallbackStatusHandled;

	return err;

}

// ------------------------------------------------------------------------------
// GetReplacementGlyphStruct										[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus GetReplacementGlyphStruct(
	ReplacementGlyphInfoStruct **replacementStruct )
{
	OSStatus	err = noErr;
	
	// assume that there ain't no replacement glyph
	*replacementStruct = NULL;

	// check to see if the replacement glyph has been initialized. If it hadn't,
	// then create the text layout and copy the style in
	if ( gReplacementGlyphInitialized == false )
	{
		UInt16					*styleIndexArray;
		ATSLayoutRecord			*layoutRecordArray;
		UInt16					styleIndex;
		ItemCount				recordCount;
		ATSUAttributeTag		tag = kATSUColorTag;
		ByteCount				tagValueSize = sizeof(RGBColor);
		ATSUAttributeValuePtr	tagValuePtr = &gReplacementCharRGBColor;
	
		// copy the style into the replacement struct. Once I fix some bugs in
		// the DirectAccess code, we won't need to keep the style around anymore.
		err = ATSUCreateAndCopyStyle( gGlobalStyle,
			&gReplacementGlyph.replacementStyle );
		require_noerr( err, GetReplacementGlyphStruct_err );
		
		// set the attributes in the style object. This should change the glyph
		// color to a pretty blue. I like blue glyphs.
		err = ATSUSetAttributes( gReplacementGlyph.replacementStyle, 1, &tag, 
			&tagValueSize, &tagValuePtr );
		check_noerr( err );
		
		// create a new layout
		err = ATSUCreateTextLayoutWithTextPtr( &gReplacementChar,
			kATSUFromTextBeginning, kATSUToTextEnd, 1, 1,
			&gReplacementCharLength, &gReplacementGlyph.replacementStyle,
			&gReplacementGlyph.replacementLayout );
		require_noerr( err, GetReplacementGlyphStruct_err );
			
		// make sure that we turn on full font substitution in case the font
		// doesn't have a glyph for the codepoint
		err = ATSUSetTransientFontMatching( gReplacementGlyph.replacementLayout,
			true );
		require_noerr( err, GetReplacementGlyphStruct_err );

		// use the direct access call to get a pointer to the glyph array from
		// the replacement text layout.
		err = ATSUDirectGetLayoutDataArrayPtrFromTextLayout(
			gReplacementGlyph.replacementLayout, 0,
			kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, 
			(void **) &layoutRecordArray, &recordCount );
		require_noerr( err, GetReplacementGlyphStruct_err );
		
		// make sure that we got an array back
		require_action( layoutRecordArray != NULL,
			GetReplacementGlyphStruct_err, err = paramErr );
			
		// since we only have one character in the replacement buffer, we can
		// assume that the first glyph in the glyph array is the replacement
		// character we are looking for
		gReplacementGlyph.glyphID = layoutRecordArray[0].glyphID;
		
		// set the glyph half advance to be the difference between the real positions
		// divided in half. This isn't the real advance. To calculate that, you would 
		// need to take into account the advance deltas.
		gReplacementGlyph.glyphHalfAdvance = ( layoutRecordArray[1].realPos - 
			layoutRecordArray[0].realPos ) / 2;
		
		// now, dispose of the layout record array
		err = ATSUDirectReleaseLayoutDataArrayPtr( NULL,
			kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
			(void **) &layoutRecordArray );
		require_noerr( err, GetReplacementGlyphStruct_err );
		
		// now, try to get a style index array. This will probably be NULL,
		// but you never know. We just need the style index for the glyph at
		// index 0
		err = ATSUDirectGetLayoutDataArrayPtrFromTextLayout(
			gReplacementGlyph.replacementLayout, 0,
			kATSUDirectDataStyleIndexUInt16Array,
			(void **) &styleIndexArray, &recordCount );
		require_noerr( err, GetReplacementGlyphStruct_err );
		
		// check to see if we got an array back
		if ( styleIndexArray != NULL )
		{
			// we got an array back, so make sure that we set the the style
			// index as the style index for glyph zero
			styleIndex = styleIndexArray[0];
			
			// dispose of the style index array
			err = ATSUDirectReleaseLayoutDataArrayPtr( NULL,
				kATSUDirectDataStyleIndexUInt16Array, (void **) &styleIndexArray );
			require_noerr( err, GetReplacementGlyphStruct_err );
		} 
		else
		{
			// otherwise, if there is no style index array, the style index is
			// assumed to be zero, as there aren't 
			styleIndex = 0;
		}
		
		// finally, we to use the direct access call to get a pointer to the
		// style settings ref array. We need to hang onto this array in the
		// structure because if it's disposed of, then our retained style
		// ref will be disposed of as well. It doesn't matter that much since
		// this is a copy anyway.
		err = ATSUDirectGetLayoutDataArrayPtrFromTextLayout(
			gReplacementGlyph.replacementLayout, 0,
			kATSUDirectDataStyleSettingATSUStyleSettingRefArray,
			(void **) &gReplacementGlyph.styleSettingArray, &recordCount );
		require_noerr( err, GetReplacementGlyphStruct_err );
		
		// make sure that we got an array back, too!
		require_action( gReplacementGlyph.styleSettingArray != NULL,
			GetReplacementGlyphStruct_err, err = paramErr );

		// now, the last piece of data can be filled in - the style setting
		// for the replacement glyph struct
		gReplacementGlyph.styleRef = 
			gReplacementGlyph.styleSettingArray[styleIndex];
			
		// everything should be initialized, so set the flag saying so
		gReplacementGlyphInitialized = true;
		
	}
			
	// all done, so return the global struct
	*replacementStruct = &gReplacementGlyph;

GetReplacementGlyphStruct_err:

	return err;

}

// ------------------------------------------------------------------------------
// GlyphReplacementCallback											[INTERNAL]
// ------------------------------------------------------------------------------

static
OSStatus GlyphReplacementCallback(
	ATSULayoutOperationSelector			iCurrentOperation,
	ATSULineRef							iLineRef,
	UInt32								iRefCon,
	void								*iOperationExtraParameter,
	ATSULayoutOperationCallbackStatus	*oCallbackStatus )
{
	OSStatus 					err;
	ReplacementGlyphInfoStruct	*replacementGlyph;
	
	// grab the replacment glyph
	err = GetReplacementGlyphStruct( &replacementGlyph );
	require_noerr( err, GlyphReplacementCallback_err );
	
	// make sure that we're only in here for the post-layout callback.
	if ( iCurrentOperation == kATSULayoutOperationPostLayoutAdjustment )
	{	
		OSStatus		disposalErr;
		ATSLayoutRecord	*recordArray;
		Fixed			*advanceDeltaArray = NULL;
		ItemCount 		recordArrayCount;
		Boolean			styleAdded = false;
		UInt16			styleIndex;
		UInt16			*styleIndexArray = NULL;
		ItemCount 		i;
		
		// Get a direct pointer to the glyph array. This should be sweet!
		err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
			kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, false,
			(void **) &recordArray, &recordArrayCount );
		require_noerr( err, GlyphReplacementCallback_err );
		
		// make sure that we got some glyphs
		require_action( recordArray != NULL, GlyphReplacementCallback_err,
			err = paramErr );
			
		// loop through all of the glyphs, looking for whitespace. We don't
		// really care about the terminator glyph here 
		for ( i = 0; i < recordArrayCount - 1; i++ )
		{
			// make sure that this isn't a deleted glyph. We don't want to be
			// replacing these with anything, no matter what their flags are
			if ( recordArray[i].glyphID != kATSDeletedGlyphcode )
			{
				// check to see if this glyph is a whitespace character.
				if ( recordArray[i].flags & kATSGlyphInfoIsWhiteSpace )
				{
					Fixed		minDistance;
					Fixed		adjustAmount;
					ItemCount	j;
				
					// this is a whitespace character. It's a candidate for
					// replacement, so replace it. First, check to see if
					// the style has been added yet.
					if ( styleAdded == false )
					{
						ItemCount 	styleIndexCount;
					
						// get the style index array. Create it if it's not there!
						err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
							kATSUDirectDataStyleIndexUInt16Array, true,
							(void **) &styleIndexArray, &styleIndexCount );
						require_noerr( err, 
							GlyphReplacementCallbackStyleSetting_err );
						
						// make sure that we got an array
						require_action( styleIndexArray != NULL,
							GlyphReplacementCallbackStyleSetting_err, 
							err = paramErr );
							
						// make sure that the array is parallel to the record array
						require_action( styleIndexCount == recordArrayCount,
							GlyphReplacementCallbackStyleSetting_err,
							err = paramErr );
						
						// add the style to the style settings array for this line
						err = ATSUDirectAddStyleSettingRef( iLineRef,
							replacementGlyph->styleRef, &styleIndex );
						require_noerr( err,
							GlyphReplacementCallbackStyleSetting_err );
						
						// the style's been added. No need to do it again.
						styleAdded = true;
						
					}
					
					// change the glyph's ID to the replacement glyph's ID
					recordArray[i].glyphID = replacementGlyph->glyphID;
					
					// change the style index to index the style setting that
					// we added for the replacement glyph's style setting
					styleIndexArray[i] = styleIndex;
					
					// now, we need to position the glyph by messing with the
					// real position. The real position is currently set to the
					// real position of the whitespace character. This won't
					// look very good for non-monospaced fonts.	
					
					// if we don't already have a pointer to the advance delta
					// array, go ahead and get one now. Make sure that it's
					// created.
					err = ATSUDirectGetLayoutDataArrayPtrFromLineRef( iLineRef,
						kATSUDirectDataAdvanceDeltaFixedArray, true,
						(void **) &advanceDeltaArray, &recordArrayCount );
					require_noerr( err, GlyphReplacementCallbackStyleSetting_err );		
					
					// find the next non-deleted glyph. It may turn out that
					// it's the final, terminator glyph
					for ( j = i + 1; j < recordArrayCount - 1; j++ )
					{
						if ( recordArray[j].glyphID != kATSDeletedGlyphcode )
						{
							break;
						}
					}
					
					// To avoid a collision with the next glyph, calculate the distance
					// between the current real position and the next glyph.
					minDistance = recordArray[j].realPos - recordArray[i].realPos;
					
					if ( ( recordArray[i].realPos + replacementGlyph->glyphHalfAdvance ) >
						( recordArray[i].realPos + minDistance ) )
					{
						
						// we need to back it up a bit so that the replacement glyph
						// doesn't collide with the next glyph
						adjustAmount = ( ( recordArray[i].realPos + replacementGlyph->glyphHalfAdvance ) 
							- ( recordArray[i].realPos + minDistance ) );
					}
					else
					{
						adjustAmount = 0;
					}
					
					// back the new real position up a bit to properly center the
					// replacement glyph over the new position.
					adjustAmount -= replacementGlyph->glyphHalfAdvance;
					
					// now, we're going to try to avoid a really bad looking collision with the 
					// previous glyph, if there is one. A slight collision is okay.
					if ( i > 0 )
					{
						Fixed	prevAdvancePos;
						Fixed 	overHang;
						
						// calculate the previous glyph's advance
						prevAdvancePos = recordArray[i].realPos - advanceDeltaArray[i-1];
						
						// calculate the overhang
						overHang = prevAdvancePos - ( recordArray[i].realPos + adjustAmount );
						
						// check to see if we have an overhang. We're not going to do anything
						// if we don't
						if ( overHang > 0 )
						{						
							// allow no more than a quarter of the replacement glyph to collide
							if ( overHang > ( replacementGlyph->glyphHalfAdvance / 4 ) )
							{
								adjustAmount += ( replacementGlyph->glyphHalfAdvance / 4 );
							}
							else
							{
								adjustAmount += overHang;
							}
							
						}						
						
					}
					
					// if the adjust amount is greater than zero, then we need to make sure that
					// the real position and the delta X for the glyphs are adjust 
					
					// adjust the real position in the glyph
					recordArray[i].realPos += adjustAmount;
					
					// adjust the advance delta to take up any of the slack that was caused by
					// our adjustment amount.
					advanceDeltaArray[i] -= adjustAmount;
					
				}
				
			}
			
		}			 

GlyphReplacementCallbackStyleSetting_err:

		// if we got the style index array, make sure that we dispose of it
		if ( styleIndexArray != NULL )
		{
			disposalErr = ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
				kATSUDirectDataStyleIndexUInt16Array, (void **) &styleIndexArray );
			check_noerr( disposalErr );
		}
		
		// if we've got an advance delta array, make sure that we dispose of it
		if ( advanceDeltaArray != NULL )
		{
			disposalErr = ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
				kATSUDirectDataAdvanceDeltaFixedArray, (void **) &advanceDeltaArray );
			check_noerr( disposalErr );
		}
		
		// dispose of the layout record array
		disposalErr = ATSUDirectReleaseLayoutDataArrayPtr( iLineRef,
			kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
			(void **) &recordArray );
		check_noerr( disposalErr );
			
	} else {
		
		// oops. We've been called for something we weren't setup to handle.
		// Make sure that we tell ATSUI that's been bad.
		err = paramErr;
	
	}


GlyphReplacementCallback_err:

	// if we didn't get an error, make sure that we return the status as 
	// handled since we did all of the replacement here. It actually doesn't
	// matter for the post-layout callback.
	if ( err == noErr ) {
		*oCallbackStatus = kATSULayoutOperationCallbackStatusHandled;
	} else {
		*oCallbackStatus = kATSULayoutOperationCallbackStatusContinue;
	}
	

	return err;
}
