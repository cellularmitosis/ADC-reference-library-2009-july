/*
 * File: WDTableElement.java
 *
 * Description: Java file associated with WDTableElement.wo
 *
 * Author: Apple Computer
 *
 * Copyright: © Copyright 2001 Apple Computer, Inc. All rights reserved.
 *
 * Disclaimer:
 *      IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 *      ("Apple") in consideration of your agreement to the following terms, and your
 *      use, installation, modification or redistribution of this Apple software
 *      constitutes acceptance of these terms.  If you do not agree with these terms,
 *      please do not use, install, modify or redistribute this Apple software.
 *
 *      In consideration of your agreement to abide by the following terms, and subject
 *      to these terms, Apple grants you a personal, non-exclusive license, under AppleUs
 *      copyrights in this original Apple software (the "Apple Software"), to use,
 *      reproduce, modify and redistribute the Apple Software, with or without
 *      modifications, in source and/or binary forms; provided that if you redistribute
 *      the Apple Software in its entirety and without modifications, you must retain
 *      this notice and the following text and disclaimers in all such redistributions of
 *      the Apple Software.  Neither the name, trademarks, service marks or logos of
 *      Apple Computer, Inc. may be used to endorse or promote products derived from the
 *      Apple Software without specific prior written permission from Apple.  Except as
 *      expressly stated in this notice, no other rights or licenses, express or implied,
 *      are granted by Apple herein, including but not limited to any patent rights that
 *      may be infringed by your derivative works or by other works in which the Apple
 *      Software may be incorporated.
 *
 *      The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 *      WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 *      WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *      PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 *      COMBINATION WITH YOUR PRODUCTS.
 *
 *      IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 *      CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *      GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *      ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 *      OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 *      (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 *      ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *                              
 * Change History (most recent first):
 *
 */

import com.webobjects.foundation.*;
import com.webobjects.appserver.*;


public class WDTableElement extends WOComponent {


	/**
	 * Constructor 
	 */
	 
    public WDTableElement(WOContext context) {
        super(context);
    }


	/**
	 * Override of method to set the synchronization policy for the component.
	 * Here we set it to false, to not synchronize the variables on this component
	 * with its bindings (from the parent component)
	 */
	 
	public boolean synchronizesVariablesWithBindings() {
		return false;
	}
	
	
	/**
	 * Method to return the background image URL, if the binding for
	 * the filename is present
	 */
	 
	public String backgroundImage() {
	
		// Check for a binding ...
		if ( hasBinding( "filename" ) ) {
		
			String filename = (String)valueForBinding( "filename" );
			String framework = (String)valueForBinding( "framework" );
			WOResourceManager rm = application().resourceManager();
			
			return rm.urlForResourceNamed( filename, framework, null, context().request() );
		}
		
		return null;
	}
	
}
