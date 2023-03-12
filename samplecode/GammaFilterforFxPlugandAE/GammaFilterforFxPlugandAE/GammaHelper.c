/*
 File: GammaHelper.c
 
 Abstract: This file contains the source that is used by
            both the After Effects plug-in and the
            FxPlug version of the Gamma filter. It is
            included in both projects as-is.
 
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

#include "GammaHelper.h"
#include "FxHelpers.h"


/*
    Function:       SetupGammaParameters (appSpecific);
 
    Parameters:     appSpecific ->  Application specific data to pass to FxHelper
                                    functions. In AE this is PF_InData. In Motion
                                    it should be a pointer to an object which
                                    implements the FxParameterCreationAPI.
    
    Description:    Sets up the parameters for the gamma filter.
 
*/

long SetupGammaParameters (void* appSpecific)
{
    // Call the app-specific function to create a fixed point slider
    // for the gamma parameter
    long    err = FXHCreateFixedSlider (appSpecific, "Gamma", kGammaParamID, 
                                        kGammaDefault, kGammaMin, kGammaMax,
                                        kGammaMin, kGammaMax, 1, 0);
    
    return err;
}


/*
    Function:       AllocateAndInitGammaTable (appSpecific);
 
    Parameters:     appSpecific ->  Application specific data to pass to 
                                    FxHelper functions. In AE, this is
                                    PF_InData. In Motion it can be NULL.

    Description:    Allocates memory for a gamma lookup table. If it fails to
                    allocate the memory, it returns NULL.
 
 */

PF_Handle AllocateAndInitGammaTable (void* appSpecific)
{
    // Allocate the table
    PF_Handle   table   = FXHAllocateHandle (appSpecific, sizeof (Gamma_Table));
    
    if (table != NULL) 
    {
        Gamma_Table		*g_tableP;
        A_long			iL = 0;
        
        // Get a pointer to the table
        g_tableP = *(Gamma_Table**)table;
        
        // Save the gamma value so we only regenerate when we need to
        // The value is saved as fixed point because that's how the original
        // AE SDK code was written
        g_tableP->gamma_val = (1L << 16);
        
        // Generate the initial table with a gamma of 1.0
        for (iL = 0; iL <= PF_MAX_CHAN8; iL++)
        {
            g_tableP->lut [ iL ] = (A_u_char)iL;
        }
    }
    
    return table;
}



/*
    Function:       RegenerateTableIfNecessary (g_tableP, desiredGamma);
 
    Parameters:     g_tableP    <-> A pointer to the gamma table data. This
                                    function may modify the table.
                    desiredGamma    ->  The gamma value that the table
                                        should contain data for.
    
    Description:    If the stored gamma is different from the desired gamma,
                    the table is regenerated with the new value.
                    
 */

void RegenerateTableIfNecessary (Gamma_Table* g_tableP, PF_Fixed desiredGamma)
{
    // If the gamma value we saved last time we generated the table is different
    // than the desired value, then we need to generate the table again
    if (g_tableP->gamma_val != desiredGamma)
    {
        double	temp;
        double  gamma;
        A_long	x = 0;

        // We're generating a new table, so save the desired gamma value for
        // the next time around.
        g_tableP->gamma_val	= desiredGamma;
        gamma = (PF_FpLong)desiredGamma / (double)(1L << 16);
        gamma = 1.0 / gamma;
        
        for (x = 0; x <= PF_MAX_CHAN8; ++x)
        {
            temp = pow ((PF_FpLong)x / 255.0, gamma);
            g_tableP->lut [ x ] = (A_u_char)(temp * 255.0);
        }
    }
}



/*
    Function:       GammaFunc (refcon, x, y, inP, outP)
 
    Parameters:     refcon      ->  Pointer to the lookup table
                    x           ->  The x position of the pixel being worked on (unused)
                    y           ->  The y position of the pixel being worked on (unused)
                    inP         ->  Pointer to the input pixel
                    outP        ->  Pointer to the output pixel
 
    Description:    Computes the gamma-corrected pixel given the lookup table.
 
 */

PF_Err GammaFunc (A_long refcon, A_long x, A_long y, PF_Pixel *inP, PF_Pixel *outP)
{
	PF_Err		err = PF_Err_NONE;
    
    // Cast our refcon to be a pointer to our gamma table data
	GammaInfo	*giP = (GammaInfo*)refcon;	
	
	if (giP != NULL)
    {
        // Use the lookup table to apply the gamma correction
		outP->alpha	= inP->alpha;
		outP->red	= giP->lut[ inP->red ];
		outP->green	= giP->lut[ inP->green ];
		outP->blue	= giP->lut[ inP->blue ];
	}
    else
    {
		err = PF_Err_BAD_CALLBACK_PARAM;
	}
    
	return err;
}
