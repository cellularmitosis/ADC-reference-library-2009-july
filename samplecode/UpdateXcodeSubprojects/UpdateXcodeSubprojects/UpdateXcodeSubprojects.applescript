(*
    UpdateXcodeSubprojects.scpt
    Copyright (c) 2005, Apple Computer, Inc., all rights reserved.
        
Upgrading a project to Xcode 2.1 format doesn't automatically update its subprojects; and when you upgrade subprojects manually, you need to fix up the subproject references and names in the parent project.  This script automatically upgrades and fixes references to all subprojects in the frontmost project.  It's also a useful example of how scriptability works in Xcode.


 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in consideration of your agreement to the following terms, and your use, installation,  modification or redistribution of this Apple software constitutes acceptance of these  terms.  If you do not agree with these terms, please do not use, install, modify or  redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these  terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in  this original Apple software (the "Apple Software"), to use, reproduce, modify and  redistribute the Apple Software, with or without modifications, in source and/or binary  forms; provided that if you redistribute the Apple Software in its entirety and without  modifications, you must retain this notice and the following text and disclaimers in all  such redistributions of the Apple Software.  Neither the name, trademarks, service marks  or logos of Apple Computer, Inc. may be used to endorse or promote products derived from  the Apple Software without specific prior written permission from Apple. Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your  derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,  EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS  USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,  REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND  WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR  OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*)

(*
This script will automatically update any referenced projects in the currently active Xcode project to version 2.1, recursively updating paths and sub-referenced projects.
*)


on run
	-- This updates the subprojects in the frontmost project in Xcode.
	updateCurrentProjectRefs()
end run

on updateCurrentProjectRefs()
	-- Set up a queue of subprojects and their subprojects.
	set projRefList to {}
	tell application "Xcode"
		-- Add current project's subprojects to the queue
		set activeProj to active project document
		set groupList to root group of activeProj as list
		repeat until (count items of groupList) = 0
			set groupList to groupList & groups of first item of groupList
			set projRefList to projRefList & (every file reference of (first item of groupList) whose file kind is "wrapper.pb-project")
			if (count items of groupList) = 1 then exit repeat
			set groupList to items 2 thru -1 of groupList
		end repeat
	end tell
	
	repeat with subProject in projRefList
		-- Upgrade subproject if necessary
		if (path of subProject does not end with ".xcodeproj") then
			tell application "Xcode" to set subProjectPath to full path of subProject
			set subProjectAlias to POSIX file subProjectPath
			tell application "Xcode"
				-- Rename the upgraded project's project reference
				set revDel to AppleScript's text item delimiters
				set AppleScript's text item delimiters to {"/"}
				set fName to last text item of (full path of subProject as string)
				set AppleScript's text item delimiters to revDel
				set upgradeName to (fName) & "proj"
				set name of subProject to upgradeName
			end tell
			
			try -- to upgrade target project... if already upgraded, this will error
				tell application "Xcode" to upgrade project file subProjectAlias as upgradeName
				delay 1
				updateCurrentProjectRefs()
				delay 1
				-- Move on to the next project in the queue.
				tell application "Xcode"
					close active project document
					set path of subProject to ((get path of subProject) & "proj")
				end tell
			on error upgErr
				display dialog fName & "could not be upgraded."
			end try
			
		end if
		
	end repeat
	
end updateCurrentProjectRefs
