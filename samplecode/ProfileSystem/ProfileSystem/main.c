/*
	File:		main.c

	Abstract:	Source file for ProfileSystem sample application.
        ProfileSystem demonstrates the use of the system_profiler shell command and how it can be called 
        by a Core Foundation application to retrieve the same information that is displayed in the System Profiler utility.  
        The sample uses the UNIX popen call to open a stream and read the results of the system_profiler command.  
        The resultant data is then read into a buffer and converted to a CFArray using the CFPropertyListCreateFromXMLData call.  
        For this sample, the CFArray is then parsed for specific information about PCI and USB devices.  
				
	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple's
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright ¬© 2005 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

/* Prototypes */
static CFDictionaryRef FindDictionaryForDataType (const CFArrayRef inArray, CFStringRef inDataType);
static CFArrayRef GetItemsArrayFromDictionary (const CFDictionaryRef inDictionary);

int main (int argc, const char * argv[]) 
{
    FILE *sys_profile;
    size_t	bytesRead = 0;
    char	streamBuffer[1024*512];
    UInt8       i = 0;
    CFDataRef       xmlData;
    CFDictionaryRef pciInfoDict;
    CFDictionaryRef usbInfoDict;
    CFArrayRef  itemsArray;
    CFIndex     arrayCount;
    
    
    // popen will fork and invoke the system_profiler command and return a stream reference with its result data
    // See the Darwin man page for system_profiler for options.
    sys_profile = popen("system_profiler SPPCIDataType SPUSBDataType -xml", "r");
    require (sys_profile != NULL, BAIL);
    
    // Read the stream into a memory buffer
    bytesRead = fread(streamBuffer, sizeof(char), sizeof(streamBuffer), sys_profile);
    
    // Close the stream
    pclose (sys_profile);
    
    // DEBUGGING: Output the raw data
//    printf ("Read %d bytes\n", bytesRead);
//    printf ("%s\n", buf);
    
    // Create a CFDataRef with the xml data
    xmlData = CFDataCreate (kCFAllocatorDefault, streamBuffer, bytesRead);
    
    // CFPropertyListCreateFromXMLData reads in the XML data and will parse it into a CFArrayRef for us. 
    CFStringRef errorString;
    CFArrayRef propertyArray = CFPropertyListCreateFromXMLData (kCFAllocatorDefault, xmlData, kCFPropertyListImmutable, &errorString);
    require_action (errorString == NULL, BAIL, CFShow (errorString));

    // DEBUGGING: Output the CFArray
//    CFShow (propertyArray);

    // Find the CFDictionary with the key/data pair of "_dataType"/"SPPCIDataType"
    // This will be the dictionary that contains all the information regarding 
    // PCI devices that system_profiler knows about.
    pciInfoDict = FindDictionaryForDataType (propertyArray, CFSTR("SPPCIDataType"));
    if (pciInfoDict != NULL)
    {        
        itemsArray = GetItemsArrayFromDictionary (pciInfoDict);
        if (itemsArray != NULL)
        {
            // Find out how many items in this category - each one is a dictionary
            arrayCount = CFArrayGetCount (itemsArray);
            
            // For each PCI device, let's output the name and type fields
            for (i=0; i < arrayCount; i++)
            {
                CFMutableStringRef  outputString;
                
                // Create a mutable CFStringRef with the dictionary value found with key "_name"
                // This is the name associated with the PCI device.
                outputString = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFDictionaryGetValue (CFArrayGetValueAtIndex(itemsArray, i), CFSTR("_name")));

                // Add some padding...
                CFStringAppend (outputString, CFSTR("        "));
                
                // Add on the type of PCI device
                CFStringAppend (outputString, CFDictionaryGetValue (CFArrayGetValueAtIndex(itemsArray, i), CFSTR("sppci_device_type")));

                CFShow (outputString);
                CFRelease (outputString);
            }
            CFRelease (itemsArray);
        }
        
        CFRelease (pciInfoDict);
    }   
    
    // Find the CFDictionary with the key/data pair of "_dataType"/"SPUSBDataType"
    // This will be the dictionary that contains all the information regarding 
    // USB devices that system_profiler knows about.
    usbInfoDict = FindDictionaryForDataType (propertyArray, CFSTR("SPUSBDataType"));
    if (usbInfoDict != NULL)
    {
        itemsArray = GetItemsArrayFromDictionary (usbInfoDict);
        if (itemsArray != NULL)
        {
            // Find out how many items in this category.  At this level, each one is a dictionary
            // describing the attributes of an individual USB bus.
            arrayCount = CFArrayGetCount (itemsArray);
            
            // For each USB bus, let's output the name field.
            // If any of the USB busses has devices attached, they are detailed in an array of dictionaries
            // located in the individual bus' dictionary.  See "system_profiler SPUSBDataType -xml" for a 
            // visual representation.
            for (i=0; i < arrayCount; i++)
            {

                // Output the name associated with this USB bus.
                CFShow (CFDictionaryGetValue (CFArrayGetValueAtIndex(itemsArray, i), CFSTR("_name")));
                
                // If this USB bus dictionary contains a CFArray called "_items", then we know there are devices present.
                // Let's show the device name and vendor name/id for the root level of each bus.  If we encounter a hub we
                // should iterate down through its devices (left as an excercise for the user...).
                CFArrayRef  usbDevicesArray = CFDictionaryGetValue (CFArrayGetValueAtIndex(itemsArray, i), CFSTR("_items"));
                if (usbDevicesArray)
                {
                    UInt8 i=0;
                    
                    // We have a CFArray of USB devices.  
                    // Get the dictionary for each one and print out two of the entries: the product name and the vendor info.
                    for (;i<CFArrayGetCount(usbDevicesArray);i++)
                    {
                        CFMutableStringRef  outputString;
                        
                        // Obtain the CFDictionary describing this device.
                        CFDictionaryRef deviceDictionary = CFArrayGetValueAtIndex (usbDevicesArray, i);
                        
                        // Create a mutable string starting with the the device name...
                        outputString = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, CFDictionaryGetValue (deviceDictionary, CFSTR("_name")));
                        
                        // ...add some padding...
                        CFStringAppend (outputString, CFSTR("        "));
                        
                        // ...and tack on the vendor info.
                        // If the vendor name is known, system_profiler will provide it,
                        // otherwise we'll just get the USB-IF assigned vendor id number.
                        if (CFDictionaryContainsKey (deviceDictionary, CFSTR("vendor_name")))
                            CFStringAppend (outputString, CFDictionaryGetValue (deviceDictionary, CFSTR("vendor_name")));                        
                        else
                            CFStringAppend (outputString, CFDictionaryGetValue (deviceDictionary, CFSTR("vendor_id")));

                        CFShow (outputString);
                        CFRelease (outputString);
                    }
                }
            }
            CFRelease (itemsArray);
        }
        CFRelease (usbInfoDict);
    }

BAIL:
    return 0;
}


// FindDictionaryForDataType
// 
// Returns the CFDictionary that contains the system profiler data type described in inDataType.
//
CFDictionaryRef FindDictionaryForDataType (const CFArrayRef inArray, CFStringRef inDataType)
{
    UInt8   i;
    CFDictionaryRef theDictionary;
    
    // Search the array of dictionaries for a CFDictionary that matches
    for (i = 0; i<CFArrayGetCount(inArray); i++)
    {
        theDictionary = CFArrayGetValueAtIndex(inArray, i);
        
        // If the CFDictionary at this index has a key/value pair with the value equal to inDataType, retain and return it.
        if (CFDictionaryContainsValue (theDictionary, inDataType))
        {
            // Retain the dictionary.  Caller is responsible for releasing it.
            CFRetain (theDictionary);
            return (theDictionary);
        }        
    }
    
    return (NULL);
}


// GetItemsArrayFromDictionary
// 
// Returns the CFArray of "item" dictionaries.
//
CFArrayRef GetItemsArrayFromDictionary (const CFDictionaryRef inDictionary)
{
    CFArrayRef  itemsArray;
    
    // Retrieve the CFDictionary that has a key/value pair with the key equal to "_items".
    itemsArray = CFDictionaryGetValue (inDictionary, CFSTR("_items"));
    if (itemsArray != NULL)
        CFRetain (itemsArray);
        
    return (itemsArray);    
}
