/*
	File:		AddressBookCarb.c
	
	Contains:	Basic AddressBook api usage

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

                Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

// This code will run on Mac OS X 10.2 (or later) ONLY!!!

#include <Carbon/Carbon.h>
#include <AddressBook/AddressBook.h>

pascal OSStatus DoCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
void AddElbaToAddressBook(void);
void FindElbaInAddressBook(WindowRef window);
Boolean FindFirstMatch(ABMutableMultiValueRef multiValue, CFStringRef label, int *index);
void DrawMyControl(WindowRef window, ControlID *cntlID, CFStringRef thisText);

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		window;
	EventTypeSpec   eventType = {kEventClassCommand, kEventProcessCommand};
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. 
    // This name is set in InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );
    
    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );

    InstallWindowEventHandler(window, NewEventHandlerUPP(DoCommandProcess), 1, &eventType, (void *)window, NULL );

    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}
    
// Handle command-process events at the application level
pascal OSStatus DoCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler)
	HICommand  aCommand;
	OSStatus   result;

	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
      
	switch (aCommand.commandID)
	{
        case 'abad':
        {
            // Add Able Elba's info to the address book
            AddElbaToAddressBook();
            result = noErr;
            break;
        } 

		case 'abfd':
         {
            // Find some of Able Elba's info in the address book
            FindElbaInAddressBook((WindowRef) userData);
            result = noErr; 
            break;
        }

		case kHICommandQuit:
			QuitApplicationEventLoop();
			result = noErr;
			break;
            
		default:
			result = eventNotHandledErr;
			break;
	}
      HiliteMenu(0);
      return result;
}

void AddElbaToAddressBook(void)
{
	 // Get the address book - there is only one.
	ABAddressBookRef ab = ABGetSharedAddressBook();    
                     
    // Create a record.
    ABPersonRef person = ABPersonCreate();
    
    // Set value in record for first name property.
    ABRecordSetValue(person, kABFirstNameProperty, CFSTR("Able"));
    // Set value in record for last name property.
    ABRecordSetValue(person, kABLastNameProperty, CFSTR("Elba"));
    
    // kABAddressProperty is a multiValue.
    // It's values, such as kABAddressHomeLabel, have in turn keys, 
    // such as kABAddressStreetKey.
    // Create and populate a CFDictionary with some kABAddressHomeLabel keys.
    CFMutableDictionaryRef homeAddr = CFDictionaryCreateMutable(NULL, 5, NULL, NULL);
    CFDictionaryAddValue(homeAddr, kABAddressStreetKey, CFSTR("123 Home Dr."));
    CFDictionaryAddValue(homeAddr, kABAddressCityKey, CFSTR("Home City"));
    CFDictionaryAddValue(homeAddr, kABAddressStateKey, CFSTR("CA"));
    CFDictionaryAddValue(homeAddr, kABAddressZIPKey, CFSTR("94110"));
    CFDictionaryAddValue(homeAddr, kABAddressCountryKey, CFSTR("United States"));

    // Create and populate a CFDictionary with some kABAddressWorkLabel keys.
    CFMutableDictionaryRef workAddr = CFDictionaryCreateMutable(NULL, 5, NULL, NULL);
    CFDictionaryAddValue(workAddr, kABAddressStreetKey, CFSTR("123 Work Dr."));
    CFDictionaryAddValue(workAddr, kABAddressCityKey, CFSTR("Work City"));
    CFDictionaryAddValue(workAddr, kABAddressStateKey, CFSTR("CA"));
    CFDictionaryAddValue(workAddr, kABAddressZIPKey, CFSTR("94110"));
    CFDictionaryAddValue(workAddr, kABAddressCountryKey, CFSTR("United States"));

    // Create an ABMultivalue and add the kABAddressHomeLabel and 
    // kABAddressWorkLabel CFDictionaries
    ABMutableMultiValueRef multiValue = ABMultiValueCreateMutable();
    ABMultiValueAdd(multiValue, homeAddr, kABAddressHomeLabel, NULL);
    ABMultiValueAdd(multiValue, workAddr, kABAddressWorkLabel, NULL);

    // Set value in record for kABAddressProperty.
    ABRecordSetValue(person, kABAddressProperty, multiValue);

    CFRelease(homeAddr);
    CFRelease(workAddr);
    CFRelease(multiValue);

    // kABPhoneProperty is a multivalue.
    // Create and populate a multiValue.
    multiValue = ABMultiValueCreateMutable();
    ABMultiValueAdd(multiValue, CFSTR("408-974-0000"), kABPhoneWorkLabel, NULL);
    ABMultiValueAdd(multiValue, CFSTR("408-974-1111"), kABPhoneHomeLabel, NULL);
    ABMultiValueAdd(multiValue, CFSTR("408-974-2222"), kABPhoneMobileLabel, NULL);
    ABMultiValueAdd(multiValue, CFSTR("408-974-3333"), kABPhoneMainLabel, NULL);
    ABMultiValueAdd(multiValue, CFSTR("408-974-4444"), kABPhoneHomeFAXLabel, NULL);
    ABMultiValueAdd(multiValue, CFSTR("408-974-5555"), kABPhoneWorkFAXLabel, NULL);
    ABMultiValueAdd(multiValue, CFSTR("408-974-6666"), kABPhonePagerLabel, NULL);

    // Set value in record for kABPhoneProperty.
    ABRecordSetValue(person, kABPhoneProperty, multiValue);

    CFRelease(multiValue);

    // Add record to the Address Book
    if (ABAddRecord(ab, person))
        // Save the Address Book
        if (ABSave(ab))
            printf("success");
            
    CFRelease(person);
}

void FindElbaInAddressBook(WindowRef window)
{
    ControlID	    		recCount = {'abtx', 0}, firstName = {'abtx', 1};
    ControlID	    		homeStName = {'abtx', 2}, workFax = {'abtx', 3};
    CFStringRef				text;
    ABMutableMultiValueRef 	multiValue;
    int						index = 0;
                
    // Get the address book; there is only one.
    ABAddressBookRef ab = ABGetSharedAddressBook();
        
    // Create a search element
    ABSearchElementRef find = ABPersonCreateSearchElement(kABLastNameProperty, 
                                                            NULL, 
                                                            NULL, 
                                                            CFSTR("Elba"), 
                                                            kABEqual);  
    
    // Run a search
    CFArrayRef array = ABCopyArrayOfMatchingRecords(ab, find);
    CFRelease(find);
    
    // How many records matched?
    CFIndex count = CFArrayGetCount(array);
    if (count > 0)
    {
        // Fill in the matching records UI
        text = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), count);
        DrawMyControl(window, &recCount, text);
        CFRelease(text);
        
        // Get the first record
        CFArrayRef firstRecord = CFArrayGetValueAtIndex(array,0);
        
        // Get the entry for the kABFirstNameProperty
        text = ABRecordCopyValue((ABRecordRef)firstRecord, kABFirstNameProperty);
        
        // Fill in the first name UI
        DrawMyControl(window, &firstName, text);  
        CFRelease(text);
        
        // Create a multiValue and populate it with the items
        // in the kABAddressProperty
        multiValue = ABMultiValueCreateMutable();
        multiValue = (ABMutableMultiValueRef)ABRecordCopyValue((ABRecordRef)firstRecord, kABAddressProperty);
        
        // Get an index into a multiValue value for the kABAddressHomeLabel label
        if (FindFirstMatch(multiValue, kABAddressHomeLabel, &index))
        {
            // kABAddressHomeLabel is a CFDictionary
            CFDictionaryRef dict = ABMultiValueCopyValueAtIndex(multiValue, index);
            text = CFDictionaryGetValue(dict, kABAddressStreetKey);
            CFRelease(dict);
            
            // Fill in the Home street address UI
            DrawMyControl(window, &homeStName, text);
        }

        CFRelease(multiValue);
        
        // Create a multiValue and populate it with the items in the kABPhoneProperty
        multiValue = ABMultiValueCreateMutable();
        multiValue = (ABMutableMultiValueRef)ABRecordCopyValue((ABRecordRef)firstRecord, kABPhoneProperty);
        
        // Get an index into a multiValue value for the kABPhoneWorkFAXLabel label
        if (FindFirstMatch(multiValue, kABPhoneWorkFAXLabel, &index))
        {
            text = ABMultiValueCopyValueAtIndex(multiValue, index);
            
            // Fill in the Home street address UI
            DrawMyControl(window, &workFax, text);
            CFRelease(text);
        }               

        CFRelease(multiValue);
    }
    CFRelease(array);
}

Boolean FindFirstMatch(ABMutableMultiValueRef multiValue, CFStringRef label, int* index)
{
    unsigned int		mvCount = 0;
    int					x;
    
    mvCount = ABMultiValueCount(multiValue);
	if (mvCount > 0)
	{
        for (x = 0; x < mvCount; x++)
        {
            CFStringRef text = ABMultiValueCopyLabelAtIndex(multiValue, x);
            CFComparisonResult result = CFStringCompare(text, label, NULL);
            CFRelease(text);

            if (result == kCFCompareEqualTo)
            {
                *index = x;
                return true;
            }
        }
	}
    return false;
}

void DrawMyControl(WindowRef window, ControlID *cntlID, CFStringRef thisText)
{
    ControlHandle 			field;
    
    GetControlByID(window, cntlID, &field);
    SetControlData(field, 0, kControlStaticTextCFStringTag, sizeof(thisText), &thisText);
    DrawOneControl(field);
}