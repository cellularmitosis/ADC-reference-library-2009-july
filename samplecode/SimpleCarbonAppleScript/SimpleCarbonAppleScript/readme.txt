File: readme.txt

Abstract: readme file

Version: 1.0

(c) Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

SimpleCarbonAppleScript employs the Cocoa scripting runtime to add scriptability to a Carbon application.  This sample illustrates the simplest AppleScript implementation for an application by implementing several properties on the main application.  Carbon programmers new to AppleScript will find this sample is a useful starting point for beginning their exploration of making their application scriptable.  

The following are important considerations for carbon programmers using the techniques illustrated herein.

1. Add the flag "NSAppleScriptEnabled" to your info.plist as a boolean set to true so your application will be listed in the Script Editor's 'open dictionary' window.  

2. The class 'scriptingCommands' defined in ScriptingCommands.h is where you put the routines that implement your scripting operations.  the name of this class must be the same as the name of the class specified in the .scriptSuite file.  

3. This is a Carbon application so some attention needs to be given to how the files are laid out.  The files have been organized to accomodate both Objective-C and C/C++ compilation.  The organization of the files is described as follows:

- main.c is the carbon application code.  It is compiled with the C compiler.  It does not contain any Objective-C code.

- Scriptability.h/Scriptability.m contains the initialization routine for the Cocoa based scripting runtime.   Scriptability.h is written in plain C so it can be included in main.c as we call the initialization routine from there.  We would have put the initialization code in the ScriptingCommands.h/ScriptingCommands.m along with the other scripting support code, but ScriptingCommands.h contains Objective-C code so we couldn't have included that file in main.c where we need to call the initialization routine.

- ScriptingCommands.h/ScriptingCommands.m contain the Objective-C implementation for the scripting.

- scriptLog.h is and Objective-C include file that defines a logging facility for debugging your scripting commands.  

4. attribute names defined in the .scriptSuite file map to method names in the 'scriptingCommands' class defined in ScriptingCommands.h/ScriptingCommands.m.  It is important that the name of the scripting implementation class match the name of the scripting class specified in the .scriptSuite and .scriptTerminology files.






