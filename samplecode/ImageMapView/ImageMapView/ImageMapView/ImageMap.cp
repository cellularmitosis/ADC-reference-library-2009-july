/*
	File:		ImageMap.cp
	
	Project:	ImageMapView
	
	Abstract:	Basic HTML ImageMap parsing (uses XML parser).

    Version:	1.0

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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/

#include "ImageMap.h"

// -----------------------------------------------------------------------------
//	ImageMap constructor
// -----------------------------------------------------------------------------
//
ImageMap::ImageMap(
	CFURLRef	inImageUrl,
	CFURLRef	inMapUrl )
{
	CGImageSourceRef		imageSource;
	
	fImage = NULL;
	fPathCount = 0;
	fNames = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	fPaths = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

	// Create the image
	imageSource = ::CGImageSourceCreateWithURL( inImageUrl, 0 );
	check( imageSource != NULL );
	fImage = ::CGImageSourceCreateImageAtIndex( imageSource, 0, 0 );
	check( fImage != NULL );
	::CFRelease( imageSource );

	Parse( inMapUrl );
}

// -----------------------------------------------------------------------------
//	ImageMap denstructor
// -----------------------------------------------------------------------------
//
ImageMap::~ImageMap()
{
	::CFRelease( fImage );
	::CFRelease( fPaths );
	::CFRelease( fNames );
	::CFRelease( fMapName );
}

// -----------------------------------------------------------------------------
//	GetImage
// -----------------------------------------------------------------------------
//
CGImageRef
ImageMap::GetImage() const
{
	return (CGImageRef) ::CFRetain( fImage );
}

// -----------------------------------------------------------------------------
//	GetName
// -----------------------------------------------------------------------------
//
CFStringRef
ImageMap::GetName(
	CFIndex			inIndex ) const
{
	check( inIndex < fPathCount );
	return (CFStringRef) ::CFArrayGetValueAtIndex( fNames, inIndex );
}

// -----------------------------------------------------------------------------
//	GetPath
// -----------------------------------------------------------------------------
//
CGPathRef
ImageMap::GetPath(
	CFIndex			inIndex ) const
{
	check( inIndex < fPathCount );
	return (CGPathRef) ::CFArrayGetValueAtIndex( fPaths, inIndex );
}

// -----------------------------------------------------------------------------
//	GetPathBounds
// -----------------------------------------------------------------------------
//
CGRect
ImageMap::GetPathBounds(
	CFIndex			inIndex ) const
{
	CGRect			bounds;
	check( inIndex < fPathCount );
	bounds = ::CGPathGetBoundingBox( GetPath( inIndex ) );
	return bounds;
}

// -----------------------------------------------------------------------------
//	Parse
// -----------------------------------------------------------------------------
//
OSStatus
ImageMap::Parse(
	CFURLRef				inMapUrl )
{
	OSStatus				err = noErr;

	CFXMLParserRef			parser;
	CFXMLParserCallBacks	callbacks;
	CFXMLParserContext		context;

	callbacks.version = 0;
	callbacks.createXMLStructure = CreateXMLStructure;
	callbacks.addChild = AddChild;
	callbacks.endXMLStructure = EndXMLStructure;
	callbacks.resolveExternalEntity = NULL;
	callbacks.handleError = HandleError;
	
	context.version = 0;
	context.info = this;
	context.retain = NULL;
	context.release = NULL;
	context.copyDescription = NULL;
	
	parser = ::CFXMLParserCreateWithDataFromURL( kCFAllocatorDefault, inMapUrl, kCFXMLParserSkipMetaData | kCFXMLParserSkipWhitespace, kCFXMLNodeCurrentVersion, &callbacks, &context );
	check( parser != NULL );
	::CFXMLParserParse( parser );
	::CFRelease( parser );
	
	fPathCount = ::CFArrayGetCount( fPaths );

	return err;
}

// =============================================================================
//	Parsing Utility methods
// =============================================================================
//

// -----------------------------------------------------------------------------
//	CreateArrayOfIntegers
// -----------------------------------------------------------------------------
//
CFArrayRef
ImageMap::CreateArrayOfIntegers(
	CFStringRef			inString )
{
	CFMutableArrayRef	ints = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, NULL );
	CFIndex				length = ::CFStringGetLength( inString );
	UniChar				uniChar;
	CFIndex				i, digits = 0;
	SInt32				value = 0;
	for ( i = 0; i < length; i++ )
	{
		uniChar = ::CFStringGetCharacterAtIndex( inString, i );
		if ( uniChar >= '0' && uniChar <= '9' )
		{
			value = value * 10 + ( uniChar - '0' );
			digits++;
		}
		else
		{
			if ( digits > 0 )
				::CFArrayAppendValue( ints, (void*) value );
			value = 0;
			digits = 0;
		}
	}
	if ( digits > 0 )
		::CFArrayAppendValue( ints, (void*) value );

	return ints;
}

// =============================================================================
//	Static XML parsing callback methods
// =============================================================================
//

// -----------------------------------------------------------------------------
//	CreateXMLStructure
// -----------------------------------------------------------------------------
//
void*
ImageMap::CreateXMLStructure(
	CFXMLParserRef			inParser,
	CFXMLNodeRef			inNodeDesc,
	void					*inInfo )
{
    void*					returnValue = NULL;
	ImageMap*				imageMap = (ImageMap*) inInfo;
	CFXMLElementInfo		*elementInfo;

	switch ( ::CFXMLNodeGetTypeCode( inNodeDesc ) )
	{
        case kCFXMLNodeTypeDocument:
			// Don't care about this node
			break;
			
        case kCFXMLNodeTypeElement:
			{
				CFStringRef	nodeName = ::CFXMLNodeGetString( inNodeDesc );
				if ( ::CFStringCompare( nodeName, CFSTR( "map" ), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
				{
					// Make sure this is the first map
					check( ::CFArrayGetCount( imageMap->fPaths ) == 0 );

					// Store the name of the map
					elementInfo = (CFXMLElementInfo*) ::CFXMLNodeGetInfoPtr( inNodeDesc );
					check( elementInfo && !elementInfo->isEmpty );
					imageMap->fMapName = (CFStringRef) ::CFRetain( (CFStringRef) ::CFDictionaryGetValue( elementInfo->attributes, CFSTR( "name" ) ) );

					returnValue = (void*) 1;
				}
				else if ( ::CFStringCompare( nodeName, CFSTR( "area" ), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
				{
					CFStringRef			shape;
					CFArrayRef			ints;
					CFIndex				i;
					CGMutablePathRef	path = NULL;

					// Get the shape of the area
					elementInfo = (CFXMLElementInfo*) ::CFXMLNodeGetInfoPtr( inNodeDesc );
					check( elementInfo && !elementInfo->isEmpty );
					shape = (CFStringRef) ::CFDictionaryGetValue( elementInfo->attributes, CFSTR( "shape" ) );
					check( shape != NULL );

					if ( ::CFStringCompare( shape, CFSTR( "poly" ), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
					{
						// Shape poly has coords of the form: "X1,Y1,...,XN,YN"
						ints = CreateArrayOfIntegers( (CFStringRef) ::CFDictionaryGetValue( elementInfo->attributes, CFSTR( "coords" ) ) );
						check( ints != NULL && ( ::CFArrayGetCount( ints ) % 2 ) == 0 );
						// Make a path with the int pairs
						path = ::CGPathCreateMutable();
						check( path != NULL );
						::CGPathMoveToPoint( path, NULL, (int) ::CFArrayGetValueAtIndex( ints, 0 ), (int) ::CFArrayGetValueAtIndex( ints, 1 ) );
						for ( i = 2; i < ::CFArrayGetCount( ints ); i += 2 )
						{
							::CGPathAddLineToPoint( path, NULL, (int) ::CFArrayGetValueAtIndex( ints, i ), (int) ::CFArrayGetValueAtIndex( ints, i + 1 ) );
						}
						::CFRelease( ints );
					}
					else if ( ::CFStringCompare( shape, CFSTR( "circle" ), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
					{
						// Shape circle has coords of the form: "X,Y,Radius"
						ints = CreateArrayOfIntegers( (CFStringRef) ::CFDictionaryGetValue( elementInfo->attributes, CFSTR( "coords" ) ) );
						check( ints != NULL && ::CFArrayGetCount( ints ) == 3 );
						path = ::CGPathCreateMutable();
						check( path != NULL );
						::CGPathAddArc( path, NULL, (int) ::CFArrayGetValueAtIndex( ints, 0 ), (int) ::CFArrayGetValueAtIndex( ints, 1 ), (int) ::CFArrayGetValueAtIndex( ints, 2 ), 0, 2 * M_PI, true );
						::CFRelease( ints );
					}
					else if ( ::CFStringCompare( shape, CFSTR( "rect" ), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
					{	// Shape rect has coords of the form: "Left,Top,Right,Bottom"
						CGRect		rect;
						ints = CreateArrayOfIntegers( (CFStringRef) ::CFDictionaryGetValue( elementInfo->attributes, CFSTR( "coords" ) ) );
						check( ints != NULL && ::CFArrayGetCount( ints ) == 4 );
						rect.origin.x = (int) ::CFArrayGetValueAtIndex( ints, 0 );
						rect.origin.y = (int) ::CFArrayGetValueAtIndex( ints, 1 );
						rect.size.width = (int) ::CFArrayGetValueAtIndex( ints, 2 ) - rect.origin.x;
						rect.size.height = (int) ::CFArrayGetValueAtIndex( ints, 3 ) - rect.origin.y;
						path = ::CGPathCreateMutable();
						check( path != NULL );
						::CGPathAddRect( path, NULL, rect );
						::CFRelease( ints );
					}
					/*
					else // if ( ::CFStringCompare( shape, CFSTR( "default" ), kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
					{
					}
					*/

					// Store the path if it was generated
					if ( path != NULL )
					{
						::CGPathCloseSubpath( path );
						::CFArrayAppendValue( imageMap->fPaths, path );
						::CFRelease( path );
					}

					// Store the name of the area
					::CFArrayAppendValue( imageMap->fNames, (CFStringRef) ::CFDictionaryGetValue( elementInfo->attributes, CFSTR( "alt" ) ) );

					returnValue = (void*) 2;
				}
				else
				{
					#if DEBUG
					fprintf( stderr, "Ignoring node:\n" );
					::CFShow( nodeName );
					#endif
					returnValue = (void*) 3;
				}
			}
			break;

        default:
			#if DEBUG
			fprintf( stderr, "Ignoring unexpected node type %d\n", (int) ::CFXMLNodeGetTypeCode( inNodeDesc ) );
			#endif
			break;
	}

	return returnValue;
}

// -----------------------------------------------------------------------------
//	AddChild
// -----------------------------------------------------------------------------
//
void
ImageMap::AddChild(
	CFXMLParserRef			inParser,
	void					*inParent,
	void					*inChild,
	void					*inInfo )
{
	// Don't do anything, it's all taken care of in CreateXMLStructure
}

// -----------------------------------------------------------------------------
//	EndXMLStructure
// -----------------------------------------------------------------------------
//
void
ImageMap::EndXMLStructure(
	CFXMLParserRef			inParser,
	void					*inXmlType,
	void					*inInfo )
{
	// Don't do anything, it's all taken care of in CreateXMLStructure
}

// -----------------------------------------------------------------------------
//	HandleError
// -----------------------------------------------------------------------------
//
Boolean
ImageMap::HandleError(
	CFXMLParserRef			inParser,
	CFXMLParserStatusCode	inError,
	void					*inInfo )
{
	if ( inError == kCFXMLErrorMalformedCloseTag )
	{
		// Ignore kCFXMLErrorMalformedCloseTag, as the <area> tags are not closed
		// in the HTML. There's probably some way to specify allowing optional HTML
		// close tags. An exercise for you!
		return true;
	}
	else
	{
		#if DEBUG
		fprintf( stderr, "HandleError didn't much like error %d\n", (int) inError );
		#endif
		return false;
	}
}
