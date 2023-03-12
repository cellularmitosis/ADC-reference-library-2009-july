/*
 * File: WDFooter.java
 *
 * Description: Java file associated with WDFooter.wo
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
import java.io.*;


public class WDFooter extends WOComponent {

	// Instance variable for the project source path (if found)
	protected String projectPath;
		
	// Instance variable for the "not found" value for the path
	public static final String NOT_FOUND_PATH = new String( "NOT FOUND" );
	
	
	/**
	 * Constructor - used to get the initial value of the project path from
	 * the session.  If the value does not already exist in the session, the
	 * path is searched and set accordingly.  If the path has been searched for
	 * and nothing was found, the value of "not found" will be returned from 
	 * the session.
	 */
	 
    public WDFooter( WOContext context ) {

		// Init
        super( context );

		// Get the value from the session (if any)
		projectPath = (String)session().objectForKey( "projectPath" );
		
		// If null, get the value
		if ( projectPath == null ) {
			projectPath = searchForProject();
		}
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
	 * Method to return a boolean value to show the link for the project. This is
	 * the boolean to select if the link CAN be shown;  the "if" it is shown is
	 * from the next method.
	 */
	 
	public boolean shouldShowLink() {
		if  ( hasBinding( "showLink" ) ) 
			return new Boolean( (String)valueForBinding( "showLink" ) ).booleanValue();
		else
			return true;
	}
	
	
	/**
	 * Method to return a boolean value to conditionally show (or not) the link
	 * to open the project.  The link is shown if the projectPath exists AND is
	 * not the "not found" value ...
	 */
	 
	public boolean showProjectLink() {
		return ( projectPath != null && !projectPath.equals( NOT_FOUND_PATH ) );
	}


	/**
	 * Method to open the project source file (Project Builder) file.  This method
	 * is only performed if the project search path is found;  if not, there is a 
	 * conditional that will not show the hyperlink to perform this method.  To 
	 * open the project we use the 'open' command with the JavaCommandShell.
	 */

	public WOComponent openProject() {
		NSDictionary output = WDCommandShell.invokeCommand( new String( "open " + projectPath ) );
		return null;
	}
	
	
	/**
	 * Method to find the project file (Project Builder file) for the running
	 * appplication.  If it is found, it is set as the project path and stored
	 * in the session;  otherwise, a "not found" value is added.
     */

	public String searchForProject() {
	
		// Set the initial value to "not found"
		String pathValue = NOT_FOUND_PATH;
		
		// Get the current path, and look for the project folder above
		String currentPath = (String)System.getProperty( "user.dir" );
		String projPath = currentPath + "/../../" + application().name() + ".pbproj";

		// If the project source file exists, set it as the string
		File projectFile = new File( projPath );
		if ( projectFile.exists() ) {
			pathValue = projPath;
		} 
		
		// Set the value into the session
		session().setObjectForKey( pathValue, "projectPath" );
		
		// Return the value to the local ivar
		return pathValue;
	}
}
