/*
  File: Gamma_Table_FxPlug.m
 
 Abstract: This is the header for the Gamma Table example FxPlug. This
            class calls helper functions which can be called from
            both an FxPlug and an After Effects plug-in.

            NOTE: To use this code you need to have both the FxPlug
            SDK and the After Effects SDK installed. The FxPlug SDK
            can be downloaded from:

            <http://connect.apple.com>

            and choosing "Downloads" then choosing "Applications."

            The After Effects SDK can be downloaded from Adobe's
            web site at:

            <http://www.adobe.com/devnet/aftereffects/>
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#import "Gamma_Table_FxPlug.h"
#include "GammaHelper.h"
#include "FxHelpers.h"

#import <FxPlug/FxPlugSDK.h>

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

// Set DO_HARDWARE to 1 to try out hardware acceleration
// Set DO_HARDWARE to 0 to use software-only rendering
#define DO_HARDWARE     0

#if DO_HARDWARE
//---------------------------------------------------------
// gammaARBFragmentProgram
//
// ARB fragment program, stored as a C string.
//---------------------------------------------------------

static const char *gammaARBFragmentProgram =
    "!!ARBfp1.0\n"
    "TEMP inSample;\n"
    "TEX inSample, fragment.texcoord[0], texture[0], RECT;\n"
    "POW result.color.r, inSample.r, program.local [ 0 ].r;\n"
    "POW result.color.g, inSample.g, program.local [ 0 ].g;\n"
    "POW result.color.b, inSample.b, program.local [ 0 ].b;\n"
    "MOV result.color.a, inSample.a;\nEND";
#endif // DO_HARDWARE

@implementation Gamma_Table_FxPlug

//---------------------------------------------------------
// initWithAPIManager:
//
// This method is called when a plug-in is first loaded, and
// is a good point to conduct any checks for anti-piracy or
// system compatibility. Returning NULL means that a plug-in
// chooses not to be accessible for some reason.
//---------------------------------------------------------

- (id)initWithAPIManager:(id)apiManager;
{
	const GLubyte *extensions = glGetString( GL_EXTENSIONS );

	BOOL cardSupportsARB = strstr( (const char *)extensions,
                                   "GL_ARB_fragment_program" ) != NULL;

	_apiManager		= apiManager;
	_cachedBright 	= -1;
	_cachedLUT		= NULL;
	_lutDepth		= 0;

#if DO_HARDWARE
	if ( cardSupportsARB )
	{
		GLint isUnderNativeLimits;

		glGenProgramsARB( 1, &_programID );
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, _programID );
		glProgramStringARB( GL_FRAGMENT_PROGRAM_ARB,
							GL_PROGRAM_FORMAT_ASCII_ARB,
							strlen( gammaARBFragmentProgram ),
							gammaARBFragmentProgram );

		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, _programID );

		glGetProgramivARB( GL_FRAGMENT_PROGRAM_ARB,
						   GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB,
						   &isUnderNativeLimits );

		_canDoHardware = ( isUnderNativeLimits == 1 );
	}
#else // if !DO_HARDWARE
    _canDoHardware = NO;
#endif // DO_HARDWARE
    
    _gammaTable = AllocateAndInitGammaTable (NULL);

	return self;
}

//---------------------------------------------------------
// dealloc
//
// Override of standard NSObject dealloc. Called when plug-in
// instance is deallocated.
//---------------------------------------------------------

- (void)dealloc
{
    if (_programID != 0) 
    {
        glDeleteProgramsARB( 1, &_programID );
    }

    if (_gammaTable != NULL)
    {
        FXHFreeHandle (NULL, _gammaTable);
    }

	[super dealloc];
}

//---------------------------------------------------------
// variesOverTime
//
// This method should return YES if the plug-in's output can
// vary over time even when all of its parameter values remain
// constant.
//---------------------------------------------------------

- (BOOL)variesOverTime
{
	return NO;
}

//---------------------------------------------------------
// addParameters
//
// This method is where a plug-in defines its list of parameters.
//---------------------------------------------------------

- (BOOL)addParameters
{
	id parmsApi;

	parmsApi = [_apiManager apiForProtocol:@protocol(FxParameterCreationAPI)];

	if ( parmsApi != NULL )
	{
       SetupGammaParameters (parmsApi);
        
		return YES;
	}
	else
		return NO;
}

//---------------------------------------------------------
// parameterChanged:
//
// This method will be called whenever a parameter value has changed.
// This provides a plug-in an opportunity to respond by changing the
// value or state of some other parameter.
//---------------------------------------------------------

- (BOOL)parameterChanged:(UInt32)parmId
{
	return YES;
}

- (NSDictionary*)properties
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithBool:NO], kFxPropertyKey_SupportsRowBytes,
        [NSNumber numberWithBool:NO], kFxPropertyKey_SupportsR408,
        [NSNumber numberWithBool:NO], kFxPropertyKey_SupportsR4fl,
        [NSNumber numberWithBool:YES], kFxPropertyKey_PixelIndependent,
        [NSNumber numberWithBool:NO], kFxPropertyKey_MayRemapTime,
        [NSNumber numberWithBool:YES], kFxPropertyKey_PreservesAlpha,
        nil];
}



//---------------------------------------------------------
// getOutputWidth:height:withInput:withInfo:
//
// This is where a filter defines the width and height of
// its output, given a description of its input.
//---------------------------------------------------------

- (BOOL)getOutputWidth:(UInt32 *)width
				height:(UInt32 *)height
			 withInput:(FxImageInfo)inputInfo
			  withInfo:(FxRenderInfo)renderInfo
{
	if ( width != NULL && height != NULL )
	{
		*width	= inputInfo.width;
		*height = inputInfo.height;
		return YES;
	}
	else
		return NO;
}

//---------------------------------------------------------
// renderOutput:withInput:withInfo:
//
// This method renders the plug-in's output into the given
// destination, using the given FxImage object as its image
// input, with the given render options. The plug-in may
// retrieve parameters as needed here, using the appropriate
// host APIs. The output image will either be an FxBitmap
// or an FxTexture, depending on the plug-in's capabilities,
// as declared in the frameSetup:hardware:software: method.
//---------------------------------------------------------

- (BOOL)renderOutput:(FxImage *)outputImage
		   withInput:(FxImage *)inputImage
			withInfo:(FxRenderInfo)renderInfo
{
	BOOL retval = YES;

	id parmsApi;

	parmsApi = [_apiManager apiForProtocol:@protocol(FxParameterRetrievalAPI)];

	if ( parmsApi != NULL )
	{
        // Call our helper method to get a floating point parameter. Note
        // that this same method can also be called from an After Effects 
        // plug-in
        double gamma = GetFloatParam (parmsApi, kGammaParamID,
                                &renderInfo.frame);

		if ( [inputImage imageType] == kFxImageType_TEXTURE )
		{
            // If hardware rendering is turned on, render in hardware
			double left, right, top, bottom;
			double tLeft, tRight, tTop, tBottom;
			FxTexture *inTex = (FxTexture *)inputImage;
			FxTexture *outTex = (FxTexture *)outputImage;

			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			[inTex getTextureCoords:&tLeft
							  right:&tRight
							 bottom:&tBottom
								top:&tTop];

			[outTex getTextureCoords:&left
							   right:&right
							  bottom:&bottom
								 top:&top];

			glEnable( GL_FRAGMENT_PROGRAM_ARB );
			glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, _programID );

			// Don't affect alpha!
			glProgramLocalParameter4fARB( GL_FRAGMENT_PROGRAM_ARB, 0,
                                          1.0 / gamma, 1.0 / gamma,
                                          1.0 / gamma, 1.0 );

			[inTex bind];
			[inTex enable];

			glBegin(GL_QUADS);
			{
				glTexCoord2f( tLeft, tBottom );
				glVertex2f( left, bottom );
				glTexCoord2f( tRight, tBottom );
				glVertex2f( right, bottom );
				glTexCoord2f( tRight, tTop );
				glVertex2f( right, top );
				glTexCoord2f( tLeft, tTop );
				glVertex2f( left, top );
			}
			glEnd();

			glDisable( GL_FRAGMENT_PROGRAM_ARB );

			[inTex disable];
            glBindTexture (GL_TEXTURE_RECTANGLE_EXT, 0);
		}
		else if ( [inputImage imageType] == kFxImageType_BITMAP )
		{
            // Use software rendering
			FxBitmap	*inMap		= (FxBitmap *)inputImage;
			FxBitmap	*outMap 	= (FxBitmap *)outputImage;

			switch( [outputImage depth] )
			{
				case 8:
				{
                    // Call the generic function which regenerates the gamma lookup
                    // table. This same method can be called from an After Effects
                    // plug-in
                    PF_Fixed    desiredGamma    = (PF_Fixed)(gamma * 65536.0);
                    RegenerateTableIfNecessary (*(Gamma_Table**)_gammaTable,
                                                desiredGamma);
                    
                    // Get the pointers to the input and output images pixels
					UInt8 *inData	= (UInt8 *)[inMap dataPtr];
					UInt8 *outData	= (UInt8 *)[outMap dataPtr];
					
					UInt32      y;
                    UInt32      outHeight   = [outMap height];
                    UInt32      outWidth    = [outMap width];
                    PF_Err      err         = PF_Err_NONE;
                    GammaInfo   gammaInfo   = {
                        (*(Gamma_Table**)_gammaTable)->lut
                    };
                    
                    // Now we iterate over the pixels and call our generic function
                    // for applying the gamma to each pixel. Note that the GammaFunc
                    // function is called by both the FxPlug and the After Effects
                    // plug-in, eliminating the need to write it twice!
					for ( y = 0; (y < outHeight) && (err == PF_Err_NONE); ++y )
					{
						UInt32 x;
						for ( x = 0; (x < outWidth) && (err == PF_Err_NONE); ++x )
						{
                            err = GammaFunc ((A_long)(&gammaInfo),
                                             (A_long)x,
                                             (A_long)y,
                                             (PF_Pixel*)inData,
                                             (PF_Pixel*)outData);
                            inData += 4;
                            outData += 4;
						}
					}
                    
                    if (err != PF_Err_NONE)
                    {
                        retval = NO;
                    }
					break;
				}
				case 16:
				{
                    // NOTE: You would normally have to write this, too!
                    retval = NO;
					break;
				}
				case 32:
				{
                    // NOTE: You would normally have to write this, too!
                    retval = NO;
					break;
				}
			}
		}
		else
			retval = NO;
	}
	else
		retval = NO;

	return retval;
}

//---------------------------------------------------------
// frameSetup:inputInfo:hardware:software:
//
// This method will be called before the host app sets up a
// render. A plug-in can indicate here whether it supports
// CPU (software) rendering, GPU (hardware) rendering, or
// both.
//---------------------------------------------------------

- (BOOL)frameSetup:(FxRenderInfo)renderInfo
		 inputInfo:(FxImageInfo)inputInfo
		  hardware:(BOOL *)canRenderHardware
		  software:(BOOL *)canRenderSoftware
{
	*canRenderSoftware = YES;
	*canRenderHardware = _canDoHardware;

	return YES;
}

//---------------------------------------------------------
// frameCleanup
//
// This method is called when the host app is done with a frame.
// A plug-in may release any per-frame retained objects
// at this point.
//---------------------------------------------------------

- (BOOL)frameCleanup
{
	return YES;
}

@end
