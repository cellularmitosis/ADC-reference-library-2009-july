
Description:
file readme.txt

readme file for the EmbededAppleScripts sample.

a sample that shows how you can easily extend the functionality
of your application by adding precompiled AppleScripts.


Copyright: 	(c) Copyright 2003 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by
Apple Computer, Inc.  ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install,
modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the
Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its
entirety and without modifications, you must retain this notice and
the following text and disclaimers in all such redistributions of
the Apple Software.  Neither the name, trademarks, service marks or
logos of Apple Computer, Inc. may be used to endorse or promote
products derived from the Apple Software without specific prior
written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple
herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple
Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE,
EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.




We have added two new build phases that compile the AppleScript in the
file AddOnScripts.applescript to place a compiled version of the script
in the resources directory of the built product.  The application only
loads the precompiled script, it does not load the text version.


1. The first build phase executes this command:

    osacompile -d -o AddOnScripts.scpt AddOnScripts.applescript

This command compiles this source file 'AddOnScripts.applescript' saving the result
in the data fork of the file 'AddOnScripts.scpt'.


2. The second build phase simply copies both of the files 'AddOnScripts.scpt'
and 'AddOnScripts.applescript' into the final application's resources directory.


IMPORTANT:  I have noticed that you need to 'clean' the build
before it will copy the compiled versions of these files over
to the resources directory.  



Some interesting points to make about this sample include:

(a) if at any time you want to reconfigure your application so that the scripts
do different things you can do so by editing this file and recompiling it to the
.scpt file using this command:
    osacompile -d -o AddOnScripts.scpt AddOnScripts.applescript

(b) everything here is datafork based and does not require any resource forks.  As
such,  it's easily transportable to other file systems.

(c) Recompiling this script file does not require recompilation of your main
program, but it can significantly enhance the configurability of your application.
As well, it can defer some design and interoperability decisions until later in
the development cycle.  Want to swap in a different app for some special task?
Just rewrite the script, your main program doesn't have to know about it...

(d) recompiling this script is even something that daring advanced users
with special requirements may want to do.

(c) because the main program only loads the precompiled
'AddOnScripts.scpt' your application does not bear any of the runtime
compilation costs that are involved.  From the application's point of
view, it's just 'Load and go...'.


end of file readme.txt

