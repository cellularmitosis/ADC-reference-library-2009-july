/*
 
 File: AEFxHelpers.c
 
 Abstract: Contains helper functions that allocate and free data and 
            create and get the values of parameters in an After Effects
            plug-in. The interface to these functions is identical to the
            interface for doing the same things in an FxPlug, so you can
            write your parameter handling code once and have it work in
            plugins for both applications.

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

#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "Param_Utils.h"
#include "AE_Macros.h"

#include "FxHelpers.h"



#pragma mark = MEMORY HANDLING =

/*
    Function:   	FXHAllocateHandle (appSpecific, size);
 
    Parameters:     appSpecific ->  PF_InData pointer for After Effects
                    size        ->  The number of bytes to allocate
    
    Description:    Allocates a moveable block of memory.
 
 */

Handle FXHAllocateHandle (void* appSpecific, size_t size)
{
    PF_InData*  in_data = (PF_InData*)appSpecific;
    
    return (Handle)PF_NEW_HANDLE (size);
}



/*
    Function:       FXHFreeHandle (appSpecific, memToFree);
    
    Parameters:     appSpecific ->  PF_InData pointer for After Effects
                    memToFree   ->  Handle to the memory you want to free
    
    Description:    Frees the memory pointed to by the passed in handle.
 
 */

void FXHFreeHandle (void* appSpecific, Handle memToFree)
{
    PF_InData*  in_data = (PF_InData*)appSpecific;
    
    PF_DISPOSE_HANDLE (memToFree);
}



#pragma mark = PARAMETER CREATION = 

/*
    Function:   FXHCreateFloatSlider (appSpecific, paramName, paramID,
                                       defaultValue, paramMin, paramMax,
                                       sliderMin, sliderMax, precision,
                                       flags);

    Parameters: appSpecific ->  PF_InData pointer for After Effects
                paramName   ->  A C string containing the name of the parameter
                paramID     ->  The ID of the parameter
                defaultValue    ->  The default value of the parameter
                paramMin    ->  The minimum value of the parameter (may
                                be different than slider min)
                paramMax    ->  The maximum value of the parameter (may
                                be different than slider max)
                sliderMin   ->  The minimum value the slider allows for
                                this parameter (user can scrub down to
                                paramMin)
                sliderMax   ->  The maximum value the slider allows for
                                this parameter (user can scrub up to
                                paramMax)
                precision   ->  Pass in an integer value defining the number
                                of decimal places to display
                flags       ->  parameter flags

    Description: Creates a new floating point slider.

*/

long FXHCreateFloatSlider (void* appSpecific, void* paramName, long paramID,
                           float defaultValue, float paramMin, float paramMax,
                           float sliderMin, float sliderMax, float precision,
                           long flags)
{
    PF_InData*  in_data = (PF_InData*)appSpecific;
    PF_ParamDef def;
    AEFX_CLR_STRUCT (def);
    
    PF_ADD_FLOAT_SLIDER ((char*)paramName, paramMin, paramMax, sliderMin,
                         sliderMax, 0.0, defaultValue, (A_short)precision, 0,
                         flags, paramID);
    
    return PF_Err_NONE;
}



/*
    Function:       FXHCreateFloatSlider (appSpecific, paramName, paramID,
                                   defaultValue, paramMin, paramMax,
                                   sliderMin, sliderMax, precision,
                                   flags);

    Parameters:     appSpecific ->  PF_InData for After Effects
                    paramName   ->  A C string with the localized name of the parameter
                    paramID     ->  The ID of the parameter
                    defaultValue    ->  The default value of the parameter
                    paramMin    ->  The minimum value of the parameter (may
                                    be different than slider min)
                    paramMax    ->  The maximum value of the parameter (may
                                    be different than slider max)
                    sliderMin   ->  The minimum value the slider allows for
                                    this parameter (user can scrub down to
                                    paramMin)
                    sliderMax   ->  The maximum value the slider allows for
                                    this parameter (user can scrub up to
                                    paramMax)
                    precision   ->  Pass in an integer value defining the number
                                    of decimal places to display
                    flags       ->  parameter flags

    Description: Creates a new fixed point slider.

*/

long FXHCreateFixedSlider (void* appSpecific, void* paramName, long paramID,
                           float defaultValue, float paramMin, float paramMax,
                           float sliderMin, float sliderMax, float precision,
                           long flags)
{
    PF_InData*  in_data = (PF_InData*)appSpecific;
    PF_ParamDef def;
    AEFX_CLR_STRUCT (def);
    
    PF_ADD_FIXED ((char*)paramName, paramMin, paramMax, sliderMin, sliderMax,
                  defaultValue, (A_short)precision, 0, flags, paramID);
    
    return PF_Err_NONE;
}



#pragma mark = PARAMATER GETTERS =

/*
    Function:       GetFloatParam (appSpecific, paramID, time);
 
    Parameters:     appSpecific ->  The params pointer (PF_ParamDef**)
                    paramID     ->  The ID of the parameter
                    time        ->  Unused in AE (could modify this function
                                    to get PF_InData passed in, check against
                                    current time and do a check out if
                                    different).
 
    Description:    Returns the floating point value of the passed in parameter.
 
 */

float GetFloatParam (void* appSpecific, long paramID, void* time)
{
#pragma unused (time)
    PF_ParamDef**    params   = (PF_ParamDef**)(appSpecific);
    
    return params [ paramID ]->u.fs_d.value;
}



/*
     Function:       GetFixedParam (appSpecific, paramID, time);
     
     Parameters:     appSpecific ->  The params pointer (PF_ParamDef**)
                     paramID     ->  The ID of the parameter
                     time        ->  Unused in AE (could modify this function
                                       to get PF_InData passed in, check against
                                       current time and do a check out if
                                       different).
     
     Description:    Returns the fixed point value of the passed in parameter.

 */

PF_Fixed GetFixedParam (void* appSpecific, long paramID, void* time)
{
#pragma unused (time)
    PF_ParamDef**    params   = (PF_ParamDef**)(appSpecific);
    
    return params [ paramID ]->u.fd.value;
}
