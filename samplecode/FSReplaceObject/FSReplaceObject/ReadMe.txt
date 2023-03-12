/*

File: ReadMe.txt

Abstract: ReadMe file for FSReplaceObject sample

Version: 1.1

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved.

*/

About FSReplaceObject

FSReplaceObject is an example command line tool which shows how to exercise
the FSReplaceObject and FSPathReplaceObject related APIs available in Mac OS X
version 10.5 and later. These APIs are provided to assist in properly preserving
metadata during "safe save" operations. For detailed information, see the
comments in the Files.h header file.

usage: FSReplaceObject [-n newName] [-t temporaryName] [-d temporaryDirectoryPath]
    [-o replaceOptions] [-p] [-v] [-h] replacementObjectPath originalObjectPath

FSReplaceObject replaces the original object at originalObjectPath with the
replacement object at replacementObjectPath. The options for FSReplaceObject are:
    -n newName: utf8 string specifying the new name for the replaced object.
    -t temporaryName: utf-8 string specifying the name of a temporary object
       should the operation require one.
    -d temporaryDirectoryPath: utf-8 string specifying the directory path to
       create the temporary object in.
    -o Add the specified options where replaceOptions are the characters:
       m  add the kFSReplaceObjectReplaceMetadata option.
       s  add the kFSReplaceObjectSaveOriginalAsABackup option.
       r  add the kFSReplaceObjectReplacePermissionInfo option.
       p  add the kFSReplaceObjectPreservePermissionInfo option.
    -p Use FSPathReplaceObject API. Default is to use FSReplaceObject API.
    -v Verbose mode -- print more status info.
    -h Help for this command.

________________________________________________________________________________

Examples

-----

Replace the file "originalFile" with the file "replacementFile" using default
options. 

# FSReplaceObject replacementFile originalFile

If successful, the result:
• is named "originalFile" in the same parent directory as the original file.
• has the creation date and backup date of the original file.
• has the data from the replacement file.
• has merged metadata from both the replacement file and the
  original file. If both the replacement file and the original file had metadata
  with the same name, the replacement file's metadata is used.
• has ACL and mode of the original file.

-----

Replace the file "originalFile" with the file "replacementFile" using the
kFSReplaceObjectReplaceMetadata and kFSReplaceObjectReplacePermissionInfo options. 

# FSReplaceObject -o mr replacementFile originalFile

If successful, the result:
• is named "originalFile" in the same parent directory as the original file.
• has the creation date and backup date of the original file.
• has the data from the replacement file.
• has metadata from only the replacement file. Any metadata the original file
  may have had is not kept.
• has ACL and mode of the replacement file.

-----

Replace the file "originalFile" with the file "replacementFile" keeping a backup
of the original file.

# FSReplaceObject -t backupName -d backupDirectory -o s replacementFile originalFile

If successful, the result:
• is named "originalFile" in the same parent directory as the original file.
• has the creation date and backup date of the original file.
• has the data from the replacement file.
• has merged metadata from both the replacement file and the
  original file. If both the replacement file and the original file had metadata
  with the same name, the replacement file's metadata is used.
• has ACL and mode of the original file.

The backup file is exactly the same as the original file, but is named
"backupName" and is left in the "backupDirectory" (which could be the original
file's parent directory if that's what was specified with the -d option).

-----

Replace the file "originalFile" with the package "replacementPackage" using the
kFSReplaceObjectReplacePermissionInfo option and renaming the result to "resultPackage".

# FSReplaceObject -o r -n resultPackage replacementPackage originalFile

If successful, the result:
• is named "resultPackage" in the same parent directory as the original file.
• has the creation date and backup date of the original file.
• has the data from the replacement package.
• has merged metadata from both the replacement package and the
  original file. If both the replacement package and the original file had metadata
  with the same name, the replacement package's metadata is used.
• has ACL and modes based from the replacement package.

________________________________________________________________________________

Release Notes

v1.0  First Release.
v1.1  Updated sample code disclaimers. Build requirements updated to Xcode 3.0.
      Noted runtime requirements are Mac OS X 10.5.

________________________________________________________________________________
