# postprocessing.sh
# ScriptBuildPhases sample code
#
# Author: MCF
#
# Copyright (c) 2002, Apple Computer, Inc., all rights reserved.
#
#
# IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
# consideration of your agreement to the following terms, and your use, installation,
# modification or redistribution of this Apple software constitutes acceptance of these
# terms.  If you do not agree with these terms, please do not use, install, modify or
# redistribute this Apple software.
#
# In consideration of your agreement to abide by the following terms, and subject to these
# terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in
# this original Apple software (the "Apple Software"), to use, reproduce, modify and
# redistribute the Apple Software, with or without modifications, in source and/or binary
# forms; provided that if you redistribute the Apple Software in its entirety and without
# modifications, you must retain this notice and the following text and disclaimers in all
# such redistributions of the Apple Software.  Neither the name, trademarks, service marks
# or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
# the Apple Software without specific prior written permission from Apple. Except as expressly
# stated in this notice, no other rights or licenses, express or implied, are granted by Apple
# herein, including but not limited to any patent rights that may be infringed by your
# derivative works or by other works in which the Apple Software may be incorporated.
#
# The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
# USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
#
# IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
# REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
# WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
# OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# This shell script runs at the end the build process each time we build.  Note that no
# dependency analysis is done; it will just happily run each time, even if everything is
# already up to date.  What we do here is post-process the java .class file that Project
# Builder (using javac) has built for us, creating the proper PalmOS executables from it.
# You must have the open source superwaba SDK downloaded (http://www.superwaba.org) and the 
# VM installed on your 
# PalmOS/handheld device in order to run the final executables, and the superwaba classes
# must be on your system (it'll look for them at "/" on your boot volume by default)
# in order for this postprocessing script to work.

# Our current working directory by default is the directory containing the project, so we change to the
# proper directory for built products, so we can put our output there. Note the use of build settings here for the
# TARGET_BUILD_DIR and product name.  These variables are expanded by the shell at compiletime
# into their correct values (with proper workarounds for the December 2001 Dev Tools where TARGET_BUILD_DIR did not yet exist).
# A complete list of build settings that Project Builder defines
# can be found on your hard drive at /Developer/Documentation/ReleaseNotes/PBBuildSettings.html. Note that at
# the time of this writing, PRODUCT_NAME was not yet documented (but its meaning is obvious).

if [ "z${TARGET_BUILD_DIR}" = "z" ] ; then
         TARGET_BUILD_DIR=${SYMROOT}
fi
     
cd "$TARGET_BUILD_DIR/$PRODUCT_NAME"

# Now we execute the Warp and Exegen superwaba java apps to build PalmOS .pdb and .prc
# executable files for our application.  Wherever you put the superwaba packages on your
# system, make sure that you provide a proper superwaba classpath
# here and in the Project Builder Targets pane (look for "Java Classes" or "Pure Java-Specific"
# depending upon your Project Builder version) so that all the proper Java classes/apps can be found and used.
java -classpath /superwaba/palm/bin Warp c /! "$PRODUCT_NAME" palmJava.class
java -classpath /superwaba/palm/bin Exegen /! "$PRODUCT_NAME" palmJava "$PRODUCT_NAME"



