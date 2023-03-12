//
//  File:       SampleCMPlugIn.c of SampleCMPlugIn
// 
//  Created:    Thu Jan 22 2004
//
//  Contains:   Sample source for a contextual menu plugin for the Finder
//
//  Notes:      This version adds support for menu attributes and modifiers to the Command_AddToAEDescList
//              routine. It also installs an application event handler for the kEventClassMenu, kEventMenuPopulate
//              event. This handler is used to add icons to the "Subcommand" menu items.
//              Sample code was also added to disable the CMM when the current app isn't the Finder.
//  
//  Copyright:  Copyright © 2005 Apple Computer, Inc. ( DTS ), All Rights Reserved
// 
//  Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
//              ( "Apple" ) in consideration of your agreement to the following terms, and your
//              use, installation, modification or redistribution of this Apple software
//              constitutes acceptance of these terms.  If you do not agree with these terms, 
//              please do not use, install, modify or redistribute this Apple software.
//
//              In consideration of your agreement to abide by the following terms, and subject
//              to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
//              copyrights in this original Apple software ( the "Apple Software" ), to use, 
//              reproduce, modify and redistribute the Apple Software, with or without
//              modifications, in source and/or binary forms; provided that if you redistribute
//              the Apple Software in its entirety and without modifications, you must retain
//              this notice and the following text and disclaimers in all such redistributions of
//              the Apple Software.  Neither the name, trademarks, service marks or logos of
//              Apple Computer, Inc. may be used to endorse or promote products derived from the
//              Apple Software without specific prior written permission from Apple.  Except as
//              expressly stated in this notice, no other rights or licenses, express or implied, 
//              are granted by Apple herein, including but not limited to any patent rights that
//              may be infringed by your derivative works or by other works in which the Apple
//              Software may be incorporated.
//
//              The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//              WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//              WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//              PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
//              COMBINATION WITH YOUR PRODUCTS.
//
//              IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//              CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//              GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION )
//              ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
//              OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
//              ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
//              ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ****************************************************
#pragma mark -
#pragma mark * complation directives *
// ----------------------------------------------------

#define LOG_ENTRY_POINTS            TRUE    // set this TRUE to log routine entry points to the console
#define FINDER_ONLY                 TRUE    // set this TRUE if you want your CMM to only appear in the Finder

// ****************************************************
#pragma mark -
#pragma mark * includes & imports *
// ----------------------------------------------------

#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

// ****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------

#define kMyBundleIdentifier CFSTR( "com.apple.SampleCMPlugIn" )

// "C52C2566-3DC1-11D5-BBA3-003065B300BC"
#define kSampleCMPlugIn_FactoryID   \
            ( CFUUIDGetConstantUUIDWithBytes( NULL, \
            0xC5, 0x2C, 0x25, 0x66, 0x3D, 0xC1, 0x11, 0xD5, \
            0xBB, 0xA3, 0x00, 0x30, 0x65, 0xB3, 0x00, 0xBC ) )

// The layout for an instance of SampleCMPlugInType.
typedef struct SampleCMPlugIn_struct {
    ContextualMenuInterfaceStruct * cmInterface;
    CFUUIDRef                       factoryID;
    UInt32                          refCount;
} SampleCMPlugIn_rec, *SampleCMPlugIn_ptr;

// ****************************************************
#pragma mark -
#pragma mark * interface function prototypes *
// ----------------------------------------------------

static HRESULT              SampleCMPlugIn_QueryInterface( void * thisInstance, REFIID iid, LPVOID * ppv );
static ULONG                SampleCMPlugIn_AddRef( void *thisInstance );
static ULONG                SampleCMPlugIn_Release( void *thisInstance );

static OSStatus             SampleCMPlugIn_ExamineContext( void * thisInstance, const AEDesc* inContext, AEDescList* outCommandPairs );
static OSStatus             SampleCMPlugIn_HandleSelection( void * thisInstance, AEDesc* inContext, SInt32 inCommandID );
static void                 SampleCMPlugIn_PostMenuCleanup( void *thisInstance );

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static SampleCMPlugIn_ptr   SampleCMPlugIn_Alloc( CFUUIDRef inFactoryID );
static void SampleCMPlugIn_Dealloc( SampleCMPlugIn_ptr thisInstance );

static char * Copy_CFStringRefToCString( CFStringRef pCFStringRef );

static OSStatus Command_AddToAEDescList( CFStringRef inCommandCFStringRef, SInt32 inCommandID, MenuItemAttributes inAttributes, UInt32 inModifiers, AEDescList* ioCommandList );

static OSStatus FileSubMenu_Create( const AEDesc* inContext, AEDescList* ioCommandList );
static OSStatus FileSubMenu_Handle( const AEDesc* inContext, SInt32 inCommandID );

static OSStatus SampleSubMenu_Create( AEDescList* ioCommandList );
static OSStatus SampleSubMenu_Handle( const AEDesc* inContext, SInt32 inCommandID );

static pascal OSStatus MenuEvent_Handle( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void * inUserData );

// ****************************************************
#pragma mark -
#pragma mark * exported globals *
// ----------------------------------------------------

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) globals *
// ----------------------------------------------------

static SInt32   gNumCommandIDs = 0;
static Boolean  gHasAttributeAndModifierKeys = FALSE;

static EventHandlerUPP gMenuEventHandlerUPP = NULL;
static EventHandlerRef gMenuEventHandlerRef = NULL;

static const char * gSubCommandCStrings[] = {"First Subcommand", "Second Subcommand", "Third Subcommand", "Fourth Subcommand"};

// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

// ----------------------------------------------------
//  SampleCMPlugIn_Factory
// ----------------------------------------------------
//  Implementation of the factory function for this type.
//
extern void * SampleCMPlugIn_Factory( CFAllocatorRef allocator, CFUUIDRef typeID );
void * SampleCMPlugIn_Factory( CFAllocatorRef allocator, CFUUIDRef typeID )
{
#pragma unused ( allocator )
    
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_Factory( %p, %p )\n", allocator, typeID );
    fflush( stdout );
#endif LOG_ENTRY_POINTS

    void * result = NULL;

    // If correct type is being requested, allocate an
    // instance of TestType and return the IUnknown interface.
    if ( CFEqual( typeID, kContextualMenuTypeID ) ) {
        result = ( void * ) SampleCMPlugIn_Alloc( kSampleCMPlugIn_FactoryID );
    }
    return result;
}

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

// -----------------------------------------------------------------------------
//  SampleCMInterface   definition
// -----------------------------------------------------------------------------
//  The Contextual Menu Interface function table.
//
static ContextualMenuInterfaceStruct gSampleCMInterface =
{
    // Required padding for COM
    NULL, 
    
    // These three are the required COM functions
    SampleCMPlugIn_QueryInterface, 
    SampleCMPlugIn_AddRef, 
    SampleCMPlugIn_Release, 
    
    // Interface implementation
    SampleCMPlugIn_ExamineContext, 
    SampleCMPlugIn_HandleSelection, 
    SampleCMPlugIn_PostMenuCleanup
};

// ****************************************************
#pragma mark -
#pragma mark * interface function implementations *
// ----------------------------------------------------

// -----------------------------------------------------------------------------
//  SampleCMPlugIn_QueryInterface
// -----------------------------------------------------------------------------
//  Implementation of the IUnknown QueryInterface function.
//
static HRESULT SampleCMPlugIn_QueryInterface( void * thisInstance, REFIID iid, LPVOID * ppv )
{
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_QueryInterface( %p, %p, %p )\n", thisInstance, &iid, ppv  );
    fflush( stdout );
#endif

    HRESULT result = S_OK;  // assume success
    
    // Create a CoreFoundation UUIDRef for the requested interface.
    CFUUIDRef   interfaceID = CFUUIDCreateFromUUIDBytes( NULL, iid );

    // Test the requested ID against the valid interfaces.
    if ( CFEqual( interfaceID, kContextualMenuInterfaceID ) ) {
        // If the TestInterface was requested, bump the ref count, 
        // set the ppv parameter equal to the instance, and
        // return good status.
        SampleCMPlugIn_AddRef( thisInstance );

        *ppv = thisInstance;
        CFRelease( interfaceID );
    } else if ( CFEqual( interfaceID, IUnknownUUID ) ) {
        // If the IUnknown interface was requested, same as above.
        SampleCMPlugIn_AddRef( thisInstance );

        *ppv = thisInstance;
        CFRelease( interfaceID );
    } else {
        // Requested interface unknown, bail with result.
        *ppv = NULL;
        CFRelease( interfaceID );
        result = E_NOINTERFACE;
    }
    return result;
}

// -----------------------------------------------------------------------------
//  SampleCMPlugIn_AddRef
// -----------------------------------------------------------------------------
//  Implementation of reference counting for this type. Whenever an interface
//  is requested, bump the refCount for the instance. NOTE: returning the
//  refcount is a convention but is not required so don't rely on it.
//
static ULONG SampleCMPlugIn_AddRef( void * thisInstance )
{
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_AddRef( %p )\n",  thisInstance  );
    fflush( stdout );
#endif
    
    ( ( SampleCMPlugIn_ptr ) thisInstance )->refCount += 1;
    return ( ( SampleCMPlugIn_ptr ) thisInstance )->refCount;
}

// -----------------------------------------------------------------------------
// SampleCMPlugIn_Release
// -----------------------------------------------------------------------------
//  When an interface is released, decrement the refCount.
//  If the refCount goes to zero, deallocate the instance.
//
static ULONG SampleCMPlugIn_Release( void * thisInstance )
{
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_Release( %p )\n",  thisInstance  );
    fflush( stdout );
#endif

    ULONG   result = 0;
    
    ( (SampleCMPlugIn_ptr ) thisInstance )->refCount -= 1;
    if ( (( SampleCMPlugIn_ptr ) thisInstance )->refCount == 0 ) {
        SampleCMPlugIn_Dealloc( (SampleCMPlugIn_ptr ) thisInstance );
    } else {
        result = ( (SampleCMPlugIn_ptr ) thisInstance )->refCount;
    }
    return result;
}

// -----------------------------------------------------------------------------
//  SampleCMPlugIn_ExamineContext
// -----------------------------------------------------------------------------
//  The implementation of the ExamineContext test interface function.
//

static OSStatus SampleCMPlugIn_ExamineContext(  void *          thisInstance,
                                                const AEDesc *  inContext,
                                                AEDescList *        outCommandPairs )
{
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_ExamineContext( %p, %p, %p )\n", thisInstance, inContext, outCommandPairs   );
    fflush( stdout );
#endif
    OSStatus result = noErr;
    
    CFStringRef bundleCFStringRef = CFSTR( "com.apple.SampleCMPlugIn" );
    CFBundleRef myCFBundleRef = CFBundleGetBundleWithIdentifier( bundleCFStringRef );
    
#if FINDER_ONLY
    ProcessInfoRec tPIR;
    
    ProcessSerialNumber tPSN = {0, kCurrentProcess};
    
    tPIR.processInfoLength = sizeof( ProcessInfoRec );
    tPIR.processName = nil;
    tPIR.processAppSpec = nil;
    
    OSStatus status = GetProcessInformation( &tPSN, &tPIR );
    if ( noErr == status ) {
        if ( tPIR.processSignature != 'MACS' || 
             tPIR.processType != 'FNDR' ) {
            fprintf( stderr, "\nNot the Finder!" ); fflush( stderr );
            return noErr;
        }
    } else {
        fprintf( stderr, "\nGetProcessInformation error: %ld.", status ); fflush( stderr );
    }
#endif
    
    // Install Carbon ( menu ) event handler
    const EventTypeSpec menuEvents[] = {
        {kEventClassMenu, kEventMenuPopulate},
    };

    if ( !gMenuEventHandlerUPP ) {
        gMenuEventHandlerUPP = NewEventHandlerUPP( MenuEvent_Handle );
    }
    InstallApplicationEventHandler( gMenuEventHandlerUPP, GetEventTypeCount( menuEvents ), menuEvents, NULL, &gMenuEventHandlerRef );

    // initilize the command id sequence
    gNumCommandIDs = 0;

    // Verify that we've got an up-to-date CMM
    verify_noerr( Gestalt( gestaltContextualMenuAttr, &result ) );
    if ( ( result & ( 1 << gestaltContextualMenuHasAttributeAndModifierKeys ) ) != 0 ) {
        printf( "\nSampleCMPlugIn: CMM supports Attributes and Modifiers keys." );
        gHasAttributeAndModifierKeys = TRUE;
    } else {
        printf( "\nSampleCMPlugIn: CMM DOES NOT support Attributes and Modifiers keys" );
        gHasAttributeAndModifierKeys = FALSE;
    }
    
    // this is a quick sample that looks for text in the context descriptor

    // make sure the descriptor isn't null
    if ( inContext ) {
        AEDesc theAEDesc = { typeNull, NULL };
        CFStringRef tempCFStringRef;
        Str32 tStr32;

        Command_AddToAEDescList( CFSTR( "-" ), ++gNumCommandIDs, kMenuItemAttrSeparator, kMenuNoModifiers, outCommandPairs );

        if ( myCFBundleRef ) {
            tempCFStringRef = CFBundleGetValueForInfoDictionaryKey( myCFBundleRef, CFSTR( "CFBundleName" ));
            tempCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "Inside '%@'" ), tempCFStringRef );
            if ( tempCFStringRef ) {
                Command_AddToAEDescList( tempCFStringRef, ++gNumCommandIDs, 0, kMenuNoModifiers, outCommandPairs );
                CFRelease( tempCFStringRef );
            }
        }

        // This shows how to use the keyContextualMenuAttributes & keyContextualMenuModifiers
        if ( gHasAttributeAndModifierKeys ) {
            Command_AddToAEDescList( CFSTR( "a disabled item" ), ++gNumCommandIDs, kMenuItemAttrDisabled, kMenuNoModifiers, outCommandPairs );
            
            Command_AddToAEDescList( CFSTR( "Option key up" ), ++gNumCommandIDs, kMenuItemAttrDynamic, kMenuNoModifiers, outCommandPairs );
            Command_AddToAEDescList( CFSTR( "Option key down" ), ++gNumCommandIDs, kMenuItemAttrDynamic, kMenuOptionModifier, outCommandPairs );
            
            Command_AddToAEDescList( CFSTR( "Another Option key up" ), ++gNumCommandIDs, kMenuItemAttrNotPreviousAlternate | kMenuItemAttrDynamic, kMenuNoModifiers, outCommandPairs );
            Command_AddToAEDescList( CFSTR( "Another Option key down" ), ++gNumCommandIDs, kMenuItemAttrNotPreviousAlternate | kMenuItemAttrDynamic, kMenuOptionModifier, outCommandPairs );
            
            Command_AddToAEDescList( CFSTR( "This should be hidden!" ), 0, kMenuItemAttrHidden, kMenuNoModifiers, outCommandPairs );
            
            Command_AddToAEDescList( CFSTR( "a separator item" ), 0, kMenuItemAttrSeparator, kMenuNoModifiers, outCommandPairs );
            Command_AddToAEDescList( CFSTR( "a section header" ), 0, kMenuItemAttrSectionHeader, kMenuNoModifiers, outCommandPairs );
            
            Command_AddToAEDescList( CFSTR( "-A meta-ignored item" ), ++gNumCommandIDs, kMenuItemAttrIgnoreMeta, kMenuNoModifiers, outCommandPairs );
            Command_AddToAEDescList( CFSTR( "-A meta separator item" ), 0, 0, kMenuNoModifiers, outCommandPairs );

            Command_AddToAEDescList( CFSTR( "a modified item\\I" ), ++gNumCommandIDs, 0, kMenuOptionModifier, outCommandPairs );
            Command_AddToAEDescList( CFSTR( "an unmodified item\\I" ), ++gNumCommandIDs, 0, kMenuNoModifiers, outCommandPairs );
        }

        // tell the raw type of the descriptor
        sprintf( ( char * ) tStr32, "raw type: '%4.4s'", ( Ptr ) &inContext->descriptorType );
        tempCFStringRef = CFStringCreateWithCString( kCFAllocatorDefault, ( Ptr ) tStr32, kCFStringEncodingASCII );
        Command_AddToAEDescList( tempCFStringRef, 0, kMenuNoModifiers, ++gNumCommandIDs, outCommandPairs );
        CFRelease( tempCFStringRef );

        if ( typeAEList == inContext->descriptorType )
            FileSubMenu_Create( inContext, outCommandPairs );
        else if ( typeAlias == inContext->descriptorType ) {
            if ( AECoerceDesc( inContext, typeAEList, &theAEDesc ) == noErr ) {
                FileSubMenu_Create( &theAEDesc, outCommandPairs );
                AEDisposeDesc( &theAEDesc );
            } else {
                fprintf( stderr, "\nSampleCMPlugIn_ExamineContext: Unable to coerce to list." ); fflush( stderr );
                // add a text only command to our command list
                Command_AddToAEDescList( CFSTR( "Can't Coerce alias to list." ), ++gNumCommandIDs, 0, kMenuNoModifiers, outCommandPairs );
            }
        } else {
            // try to get text out of the context descriptor; make sure to
            // coerce it, cuz the app may have passed an object specifier or
            // styled text, etc.
            if ( AECoerceDesc( inContext, typeUTF8Text, &theAEDesc ) == noErr ) {
                // add a text only command to our command list
                Command_AddToAEDescList( CFSTR( "We got text!" ), ++gNumCommandIDs, 0, kMenuNoModifiers, outCommandPairs );
                AEDisposeDesc( &theAEDesc );
            } else {
                fprintf( stderr, "\nSampleCMPlugIn_ExamineContext: Unable to coerce to text." ); fflush( stderr );
                // add a text only command to our command list
                Command_AddToAEDescList( CFSTR( "Can't Coerce." ), ++gNumCommandIDs, 0, kMenuNoModifiers, outCommandPairs );
            }
        }

        // Just for kicks, lets create a SubMenu for our plugin
        SampleSubMenu_Create( outCommandPairs );
        Command_AddToAEDescList( CFSTR( "bottom separator item" ), 0, kMenuItemAttrSeparator, kMenuNoModifiers, outCommandPairs );

    } else {
        fprintf( stderr, "\nSampleCMPlugIn_ExamineContext: NULL descriptor?" ); fflush( stderr );
        // we have a null descriptor
        Command_AddToAEDescList( CFSTR( "NULL Descriptor" ), ++gNumCommandIDs, 0, kMenuNoModifiers, outCommandPairs );
    }
    
#if FALSE   // Set this TRUE to printf the outCommandPairs
    Handle strHdl;
    result = AEPrintDescToHandle( outCommandPairs, &strHdl );
    if ( noErr == result ) {
        char    nul = '\0';
        PtrAndHand( &nul, strHdl, 1 );
        printf( "\nSampleCMPlugIn_ExamineContext: outCommandPairs: \"%s\".", *strHdl );
        fflush( stdout );
        DisposeHandle( strHdl );
    }
#endif

    fflush( stdout );

    return noErr;
}   // SampleCMPlugIn_ExamineContext

// -----------------------------------------------------------------------------
//  HandleSelection
// -----------------------------------------------------------------------------
//  The implementation of the HandleSelection test interface function.
//
static OSStatus SampleCMPlugIn_HandleSelection( void *  thisInstance,
                                               AEDesc * inContext,
                                               SInt32   inCommandID )
{
#if LOG_ENTRY_POINTS
    printf( "\nSampleCMPlugIn_->SampleCMPlugIn_HandleSelection( instance: %p, inContext: %p, inCommandID: 0x%08lX )",
            thisInstance, inContext, inCommandID );
#endif LOG_ENTRY_POINTS
    OSStatus status = noErr;
    
    // Sequence the command ids
    gNumCommandIDs = 0;

    // this is a quick sample that looks for text in the context descriptor

    // make sure the descriptor isn't null
    if ( inContext ) {
        AEDesc theAEDesc = { typeNull, NULL };

        if ( typeAEList == inContext->descriptorType )
            status = FileSubMenu_Handle( inContext, inCommandID );
        else if ( typeAlias == inContext->descriptorType ) {
            status = AECoerceDesc( inContext, typeAEList, &theAEDesc );
            if ( noErr == status ) {
                status = FileSubMenu_Handle( &theAEDesc, inCommandID );
                AEDisposeDesc( &theAEDesc );
            } else {
                fprintf( stderr, "\nSampleCMPlugIn_HandleSelection: Unable to coerce to list." );
            }
        } else {
            // try to get text out of the context descriptor; make sure to coerce it
            // because the app may have passed an object specifier or styled text, etc.
            if ( AECoerceDesc( inContext, typeUTF8Text, &theAEDesc ) == noErr ) {
                ++gNumCommandIDs;
                if ( inCommandID == gNumCommandIDs ) {
                    Size tSize = AEGetDescDataSize( &theAEDesc );
                    Ptr tPtr = NewPtrClear( tSize );
                    if ( tPtr ) {
                        AEGetDescData( &theAEDesc, tPtr, tSize );
                        if ( noErr == status ) {
                            printf( "\nSampleCMPlugIn_HandleSelection( command: %ld, text: \"%s\" )", inCommandID, tPtr );
                        }
                        DisposePtr( tPtr );
                    }
                }
                AEDisposeDesc( &theAEDesc );
            } else {
                fprintf( stderr, "\nSampleCMPlugIn_HandleSelection: Unable to coerce to text." );
                ++gNumCommandIDs;
            }
        }
        
        // handle the SubMenu for our plugin
        SampleSubMenu_Handle( inContext, inCommandID );
    } else {
        fprintf( stderr, "\nSampleCMPlugIn_HandleSelection: NULL descriptor" );
        ++gNumCommandIDs;
    }

    return noErr;
}   // SampleCMPlugIn_HandleSelection

// -----------------------------------------------------------------------------
//  PostMenuCleanup
// -----------------------------------------------------------------------------
//  The implementation of the PostMenuCleanup test interface function.
//
static void SampleCMPlugIn_PostMenuCleanup( void * thisInstance )
{
#if LOG_ENTRY_POINTS
    printf( "\nSampleCMPlugIn_PostMenuCleanup( instance: %p )", thisInstance );
#endif LOG_ENTRY_POINTS
    if ( gMenuEventHandlerRef )
        RemoveEventHandler( gMenuEventHandlerRef );
    if ( gMenuEventHandlerUPP )
        DisposeEventHandlerUPP( gMenuEventHandlerUPP );
}   // SampleCMPlugIn_PostMenuCleanup

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

// -----------------------------------------------------------------------------
//  Copy_CFStringRefToCString
// -----------------------------------------------------------------------------
// Note: If not NULL the results have to be DisposePtr'ed.
static char * Copy_CFStringRefToCString( CFStringRef pCFStringRef )
{
    char * results = NULL;
    if ( pCFStringRef ) {
        CFIndex length = sizeof( UniChar ) * CFStringGetLength( pCFStringRef ) + 1;
        results = ( char * ) NewPtrClear( length );
        if ( !CFStringGetCString( pCFStringRef, results, length, kCFStringEncodingASCII ) ) {
            if ( !CFStringGetCString( pCFStringRef, results, length, kCFStringEncodingUTF8 ) ) {
                DisposePtr( results );
                results = NULL;
            }
        }
    }
    return results;
}   // Copy_CFStringRefToCString

// -----------------------------------------------------------------------------
//  SampleCMPlugIn_Alloc
// -----------------------------------------------------------------------------
//  Utility function that allocates a new instance.
//
static SampleCMPlugIn_ptr SampleCMPlugIn_Alloc( CFUUIDRef       inFactoryID )
{
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_Alloc( %p )\n", inFactoryID );
#endif
    // Allocate memory for the new instance.
    SampleCMPlugIn_ptr theNewInstance;

    theNewInstance = ( SampleCMPlugIn_ptr ) malloc( sizeof( SampleCMPlugIn_rec ) );

    // Point to the function table
    theNewInstance->cmInterface = &gSampleCMInterface;

    // Retain and keep an open instance refcount for each factory.
    theNewInstance->factoryID = CFRetain( inFactoryID );
    CFPlugInAddInstanceForFactory( inFactoryID );

    // This function returns the IUnknown interface
    // so set the refCount to one.
    theNewInstance->refCount = 1;
    return theNewInstance;
}   // SampleCMPlugIn_Alloc

// -----------------------------------------------------------------------------
//  SampleCMPlugIn_Dealloc
// -----------------------------------------------------------------------------
//  Utility function that deallocates the instance when
//  the refCount goes to zero.
//
static void SampleCMPlugIn_Dealloc( SampleCMPlugIn_ptr thisInstance )
{
#if LOG_ENTRY_POINTS
    printf( "SampleCMPlugIn_Dealloc( %p )\n", thisInstance );
#endif LOG_ENTRY_POINTS
    CFUUIDRef   theFactoryID = thisInstance->factoryID;

    free( thisInstance );
    if ( theFactoryID ) {
        CFPlugInRemoveInstanceForFactory( theFactoryID );
        CFRelease( theFactoryID );
    }
}   // SampleCMPlugIn_Dealloc

// *****************************************************
//
// Routine: Command_AddToAEDescList( inCommandCFStringRef, inCommandID, inAttributes, inModifiers, ioCommandList )
//
// Purpose: Create the file sub menu item commnds
//
// Inputs:  inCommandCFStringRef    - the text for the command
//          inCommandID             - its command ID
//          inAttributes            - its attributes
//          inModifiers             - its modifiers
//          ioCommandList           - AEDescList of commands to append
//
// Returns: OSStatus            - error code
//

static OSStatus Command_AddToAEDescList( CFStringRef inCommandCFStringRef, SInt32 inCommandID, MenuItemAttributes inAttributes, UInt32 inModifiers, AEDescList* ioCommandList )
{
    OSStatus result = noErr;
    AERecord theCommandRecord = { typeNull, NULL };
    CFStringRef tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%@-[0x%08lX]" ), inCommandCFStringRef, inCommandID );

#if LOG_ENTRY_POINTS
    if ( tCFStringRef ) {
        char buffer[256];
        if ( CFStringGetCString( tCFStringRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8) ) {
            printf( "\nSampleCMPlugIn->Command_AddToAEDescList( \"%s\", 0x%08lX, 0x%08lX, 0x%08lX, %p );", buffer, inCommandID, inAttributes, inModifiers, ioCommandList );
        }
        CFShow( tCFStringRef );
    }
#endif LOG_ENTRY_POINTS
    
    // create an AEDesc list for our command
    result = AECreateList( NULL, 0, TRUE, &theCommandRecord );
    require_noerr( result, Command_AddToAEDescList_fail );
    
    // stick the command text into the aerecord
    CFRetain( tCFStringRef );
    result = AEPutKeyPtr( &theCommandRecord, keyContextualMenuName, typeCFStringRef, &tCFStringRef, sizeof( CFStringRef ) );
    require_noerr( result, Command_AddToAEDescList_fail );
    
    // stick the command ID into the AERecord
    result = AEPutKeyPtr( &theCommandRecord, keyContextualMenuCommandID, typeSInt32, &inCommandID, sizeof( inCommandID ) );
    require_noerr( result, Command_AddToAEDescList_fail );
    
    if ( gHasAttributeAndModifierKeys ) {
        // stick the attributes into the AERecord
        if ( inAttributes != 0 ) {
            result = AEPutKeyPtr( &theCommandRecord, keyContextualMenuAttributes, typeSInt32, &inAttributes, sizeof( inAttributes ) );
            require_noerr( result, Command_AddToAEDescList_fail );
        }
        
        // stick the modifiers into the AERecord
        if ( inModifiers != 0 ) {
            result = AEPutKeyPtr( &theCommandRecord, keyContextualMenuModifiers, typeSInt32, &inModifiers, sizeof( inModifiers ) );
            require_noerr( result, Command_AddToAEDescList_fail );
        }
    }
    
#if FALSE   // Set this TRUE to printf the command record
    Handle strHdl;
    result = AEPrintDescToHandle( &theCommandRecord, &strHdl );
    if ( noErr == result ) {
        char    nul = '\0';
        PtrAndHand( &nul, strHdl, 1 );
        printf( "\nSampleCMPlugIn->Command_AddToAEDescList: theCommandRecord:\"%s\".", *strHdl );
        fflush( stdout );
        DisposeHandle( strHdl );
    }
#endif

    // stick this record into the list of commands that we are
    // passing back to the CMM
    result = AEPutDesc( ioCommandList, 0, &theCommandRecord );
    
Command_AddToAEDescList_fail: ;
    // clean up after ourself; dispose of the AERecord
    AEDisposeDesc( &theCommandRecord );

    if ( tCFStringRef ) {
        CFRelease( tCFStringRef );
    }
    return result;
} // Command_AddToAEDescList

/*****************************************************
*
* Routine:  FileSubMenu_Create( ioCommandList )
*
* Purpose:  Create the file sub menu item commnds
*
* Inputs:   ioCommandList       - AEDescList of commands to which we append our file sub menu commands
*
* Returns:  OSStatus            - error code
*/
static OSStatus FileSubMenu_Create( const AEDesc * inContext, AEDescList * ioCommandList )
{
    OSStatus    result = noErr;
    
    AEDescList  theSubMenuCommands = { typeNull, NULL };
    AERecord    theSubMenuCommand = { typeNull, NULL };
    Str255      theSubMenuCommandText = "\pSubMenu Here";
    
    // the first thing we should do is create an AEDescList of subcommands
    
    // set up the AEDescList
    result = AECreateList( NULL, 0, FALSE, &theSubMenuCommands );
    require_noerr( result, FileSubMenu_Create_Complete_fail );
    
    long    idx, numItems;
    result = AECountItems( inContext, &numItems );
    if ( noErr == result ) {
        sprintf( ( char * ) &theSubMenuCommandText[1], "%ld item(s)!", numItems );
        theSubMenuCommandText[0] = strlen( ( char * ) &theSubMenuCommandText[1] );
        
        for ( idx = 1; idx <= numItems; idx++ ) {
            AEKeyword   theAEKeyword;
            AEDesc      theAEDesc;
            
            result = AEGetNthDesc( inContext, idx, typeAlias, &theAEKeyword, &theAEDesc );
            if ( noErr != result ) continue;
            
            if ( typeAlias == theAEDesc.descriptorType ) {
                FSRef tFSRef;
                Size dataSize = AEGetDescDataSize( &theAEDesc );
                AliasHandle tAliasHdl = ( AliasHandle ) NewHandle( dataSize );
                
                if ( tAliasHdl ) {
                    Boolean wasChanged;
                    
                    result = AEGetDescData( &theAEDesc, *tAliasHdl, dataSize );
                    if ( noErr == result ) {
                        result = FSResolveAlias( NULL, tAliasHdl, &tFSRef, &wasChanged );
                        if ( noErr == result ) {
                            CFURLRef tCFURLRef = CFURLCreateFromFSRef( kCFAllocatorDefault, &tFSRef );
                            CFStringRef nameCFStringRef = CFURLCopyLastPathComponent( tCFURLRef );
                            
                            CFRelease( tCFURLRef ); // don't need this anymore
                            
                            Command_AddToAEDescList( nameCFStringRef, 0x00010000 + idx, 0, kMenuNoModifiers, &theSubMenuCommands );
                            
                            CFRelease( nameCFStringRef );   // don't need this anymore
                        }
                    }
                    DisposeHandle( ( Handle ) tAliasHdl );
                }
            }
            AEDisposeDesc( &theAEDesc );
        }
    }
    
    // now, we need to create the supercommand which will "own" the
    // subcommands.  The supercommand lives in the root command list.
    // this looks very much like the Command_AddToAEDescList function,
    // except that instead of putting a command ID in the record,
    // we put in the subcommand list.
    
    // create an apple event record for our supercommand
    result = AECreateList( NULL, 0, TRUE, &theSubMenuCommand );
    require_noerr( result, FileSubMenu_Create_fail );
    
    // stick the command text into the aerecord
    result = AEPutKeyPtr( &theSubMenuCommand, keyContextualMenuName, typeUTF8Text, &theSubMenuCommandText[1], theSubMenuCommandText[0] );
    require_noerr( result, FileSubMenu_Create_fail );
    
    // stick the subcommands into into the AERecord
    result = AEPutKeyDesc( &theSubMenuCommand, keyContextualMenuSubmenu, &theSubMenuCommands );
    require_noerr( result, FileSubMenu_Create_fail );
    
    // stick the supercommand into the list of commands that we are
    // passing back to the CMM
    result = AEPutDesc( ioCommandList,          // the list we're putting our command into
                        0,                      // stick this command onto the end of our list
                        &theSubMenuCommand );   // the command I'm putting into the list
    
    // clean up after ourself
FileSubMenu_Create_fail:    ;
    AEDisposeDesc( &theSubMenuCommands );
    AEDisposeDesc( &theSubMenuCommand );
    
FileSubMenu_Create_Complete_fail:
        return result;
    
} // FileSubMenu_Create

//*****************************************************
//
// Routine: FileSubMenu_Handle( inContext, inCommandID )
//
// Purpose: This is where I handle the file sub menu item commnds
//
// Inputs:  inContext           - EventHandlerCallRef ( not being used )
//          inCommandID         - EventRef
//
// Returns: OSStatus            - error code
//

static OSStatus FileSubMenu_Handle( const AEDesc * inContext, SInt32 inCommandID )
{
    long    idx, numItems;
    OSStatus    result = noErr;

    if ( ( inCommandID >> 16 ) != 0x0001 )
        return result;

#if LOG_ENTRY_POINTS
    printf( "\nSampleCMPlugIn->FileSubMenu_Handle( inContext: %p, inCommandID: 0x%08lX )", inContext, inCommandID );
#endif LOG_ENTRY_POINTS

    result = AECountItems( inContext, &numItems );
    if ( noErr == result ) {
        for ( idx = 1; idx <= numItems; idx++ ) {
            AEKeyword   theAEKeyword;
            AEDesc      theAEDesc;

            result = AEGetNthDesc( inContext, idx, typeAlias, &theAEKeyword, &theAEDesc );
            if ( noErr != result ) continue;

            if ( typeAlias == theAEDesc.descriptorType ) {
                FSRef tFSRef;
                Size dataSize = AEGetDescDataSize( &theAEDesc );
                AliasHandle tAliasHdl = ( AliasHandle ) NewHandle( dataSize );

                if ( tAliasHdl ) {
                    Boolean wasChanged;

                    result = AEGetDescData( &theAEDesc, *tAliasHdl, dataSize );
                    if ( noErr == result ) {
                        result = FSResolveAlias( NULL, tAliasHdl, &tFSRef, &wasChanged );
                        if ( noErr == result ) {
                            CFURLRef tCFURLRef = CFURLCreateFromFSRef( kCFAllocatorDefault, &tFSRef );
                            CFStringRef nameCFStringRef = CFURLCopyLastPathComponent( tCFURLRef );
                            CFRelease( tCFURLRef ); // don't need this anymore

                            if ( nameCFStringRef ) {
                                if ( inCommandID == ( 0x00010000 + idx ) ) {
                                    char * tPtr = Copy_CFStringRefToCString( nameCFStringRef );

                                    if ( tPtr ) {
                                        printf( "\nSampleCMPlugIn->FileSubMenu_Handle( file: \"%s\" )", tPtr );
                                        DisposePtr( tPtr );
                                    }
                                }
                                CFRelease( nameCFStringRef );   // don't need this anymore
                            }
                        }
                    }
                    DisposeHandle( ( Handle ) tAliasHdl );
                }
            }
            AEDisposeDesc( &theAEDesc );
        }
    }
    return result;
} // FileSubMenu_Handle

//*****************************************************
//
// Routine: SampleSubMenu_Create( ioCommandList )
//
// Purpose: Create some sub menu item commnds
//
// Inputs:  ioCommandList       - AEDescList of commands to which we append our sub menu commands
//
// Returns: OSStatus            - error code
//

static OSStatus SampleSubMenu_Create( AEDescList *  ioCommandList )
{
    OSStatus    result = noErr;
    
    AEDescList  theSubMenuCommands = { typeNull, NULL };
    AERecord    theSubMenuCommand = { typeNull, NULL };
    Str255      theSubMenuCommandText = "\pSubMenu Here";
    Str255      theSubMenuCommandPlusText = "\pSubMenu Here-[0x20000]";
    long        commandID = 0x00020000;
    
#if LOG_ENTRY_POINTS
    printf( "\nSampleCMPlugIn->SampleSubMenu_Create( )." );
#endif LOG_ENTRY_POINTS
    
    // the first thing we should do is create an AEDescList of subcommands
    
    // set up the AEDescList
    result = AECreateList( NULL, 0, FALSE, &theSubMenuCommands );
    require_noerr( result, SampleSubMenu_Create_Complete_fail );

    int idx, count = sizeof( gSubCommandCStrings ) / sizeof( char * );
    for ( idx = 0; idx < count; idx++ ) {
        CFStringRef itemNameCFStringRef = CFStringCreateWithCString( kCFAllocatorDefault, gSubCommandCStrings[idx], kCFStringEncodingUTF8 );
        if ( itemNameCFStringRef ) {
            result = Command_AddToAEDescList( itemNameCFStringRef, 0x00020001 + idx, 0, kMenuNoModifiers, &theSubMenuCommands );
            require_noerr( result, SampleSubMenu_Create_CreateDesc_fail );
            CFRelease( itemNameCFStringRef );
        }
    }

    // now, we need to create the supercommand which will "own" the
    // subcommands.  The supercommand lives in the root command list.
    // this looks very much like the Command_AddToAEDescList function, 
    // except that instead of putting a command ID in the record, 
    // we put in the subcommand list.
    
    // create an apple event record for our supercommand
    result = AECreateList( NULL, 0, TRUE, &theSubMenuCommand );
    require_noerr( result, SampleSubMenu_Create_fail );
    
    // stick the command text into the aerecord
    if ( gHasAttributeAndModifierKeys ) {
        result = AEPutKeyPtr( &theSubMenuCommand, keyContextualMenuName, typeUTF8Text, &theSubMenuCommandPlusText[1], theSubMenuCommandPlusText[0] );
        require_noerr( result, SampleSubMenu_Create_fail );
        
        // stick the command ID into the AERecord
        result = AEPutKeyPtr( &theSubMenuCommand, keyContextualMenuCommandID, typeSInt32, &commandID, sizeof( commandID ) );
        require_noerr( result, SampleSubMenu_Create_fail );
        
        if ( gHasAttributeAndModifierKeys ) {
            const MenuItemAttributes tMenuItemAttributes = kMenuItemAttrSubmenuParentChoosable;
            result = AEPutKeyPtr( &theSubMenuCommand, keyContextualMenuAttributes, typeSInt32, &tMenuItemAttributes, sizeof( tMenuItemAttributes ) );
            require_noerr( result, SampleSubMenu_Create_fail );
        }
    } else {
        result = AEPutKeyPtr( &theSubMenuCommand, keyContextualMenuName, typeUTF8Text, &theSubMenuCommandText[1], theSubMenuCommandText[0] );
        require_noerr( result, SampleSubMenu_Create_fail );
    }
    
    // stick the subcommands into into the AERecord
    result = AEPutKeyDesc( &theSubMenuCommand, keyContextualMenuSubmenu, &theSubMenuCommands );
    require_noerr( result, SampleSubMenu_Create_fail );
    
    // stick the supercommand into the list of commands that we are passing back to the CMM
    result = AEPutDesc( ioCommandList,      // the list we're putting our command into
                       0,                   // stick this command onto the end of our list
                       &theSubMenuCommand );    // the command I'm putting into the list
    
    // clean up after ourself
SampleSubMenu_Create_fail:  ;
    AEDisposeDesc( &theSubMenuCommands );
    
SampleSubMenu_Create_CreateDesc_fail:   ;
    AEDisposeDesc( &theSubMenuCommand );
    
SampleSubMenu_Create_Complete_fail: ;
    return result;
} // SampleSubMenu_Create

// *****************************************************
//
// Routine: SampleSubMenu_Handle( inContext, inCommandID )
//
// Purpose: This is where I handle the sub menu item commnds
//
// Inputs:  inContext           - EventHandlerCallRef ( not being used )
//          inCommandID             - EventRef
//
// Returns: OSStatus            - error code
//
static OSStatus SampleSubMenu_Handle( const AEDesc * inContext, SInt32 inCommandID )
{
    OSStatus    result = noErr;
    
#pragma unused ( inContext )
    
    switch ( inCommandID ) {
        case 0x00020001: {
            printf( "\n\"First Subcommand\"" );
            break;
        }
        case 0x00020002: {
            printf( "\n\"Second Subcommand\"" );
            break;
        }
        case 0x00020003: {
            printf( "\n\"Third Subcommand\"" );
            break;
        }
        case 0x00020004: {
            printf( "\n\"Fourth Subcommand\"" );
            break;
        }
        default: {
            if ( 0x0002 == ( inCommandID >> 16 )) {
                printf( "\nSampleSubMenu_Handle:( inCommandID: %.8lX )", inCommandID );
            }
            break;
        }
    }
    
    return result;
} // SampleSubMenu_Handle

/*****************************************************
*
* Routine:  MenuEvent_Handle( inEventHandlerCallRef, inEventRef, inUserData )
*
* Purpose:  I'm using this as a hook to modify menu items before the finder CMM code displays them
*           In this case I'm trying to add an Icon
*
* Inputs:   inEventHandlerCallRef   - EventHandlerCallRef ( not being used )
*           inEventRef              - EventRef
*           inUserData              - pointer to users data ( not being used )
*
* Returns:  OSStatus                - Always eventNotHandledErr since this is just a hook; we don't handle any events here
*/
static pascal OSStatus MenuEvent_Handle( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void * inUserData )
{
    OSStatus result = eventNotHandledErr;           // assume that we're not going to handle the event ( which is ALWAYS TRUE )
    EventClass theClass = GetEventClass( inEventRef );
    EventClass theKind = GetEventKind( inEventRef );
    
#if LOG_ENTRY_POINTS
    printf( "\nMenuEvent_Handle!" ); fflush( stdout );
#endif LOG_ENTRY_POINTS
    
    OSStatus status = noErr;
    
    CFBundleRef myCFBundleRef = CFBundleGetBundleWithIdentifier( kMyBundleIdentifier );
    CFURLRef tCFURLRef = CFBundleCopyResourceURL( myCFBundleRef, CFSTR( "Skull&Bones" ), CFSTR( "icns" ), NULL );
    IconRef iconRef = NULL;
    
    if ( tCFURLRef ) {
        FSRef tFSRef;
        status = ( CFURLGetFSRef( tCFURLRef, &tFSRef ) ? noErr : fnfErr );
        if ( noErr == status ) {
            status = RegisterIconRefFromFSRef( 'CMMs', 'skul', &tFSRef, &iconRef );
            if ( noErr != status )
                fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle->RegisterIconRefFromFSRef error: %ld ( 0x%08lX ).", status, status );
        } else {
            fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle->CFURLGetFSRef error: %ld ( 0x%08lX ).", status, status );
        }
        CFRelease( tCFURLRef );
    } else {
        fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle->CFBundleCopyResourceURL is NULL." );
    }
    fflush( stderr );
    
    switch ( theClass ) {
        case kEventClassMenu: {
            switch ( theKind ) {
                case kEventMenuPopulate: {
                    status = CallNextEventHandler( inEventHandlerCallRef, inEventRef );
                    
                    MenuRef myMenuRef;
                    status = GetEventParameter( inEventRef, kEventParamDirectObject, typeMenuRef, NULL, sizeof( MenuRef ), NULL, &myMenuRef );
                    
                    Boolean firstOpen;
                    status = GetEventParameter( inEventRef, kEventParamMenuFirstOpen, typeBoolean, NULL, sizeof( Boolean ), NULL, &firstOpen );
                    
                    UInt32 menuContext;
                    status = GetEventParameter( inEventRef, kEventParamMenuContext, typeUInt32, NULL, sizeof( UInt32 ), NULL, &menuContext );
#if LOG_ENTRY_POINTS
                    printf( "\nSampleCMPlugIn->MenuEvent_Handle: {ref: %8.8lX, first: %s, context: %8.8lX}.", 
                            ( UInt32 ) myMenuRef, firstOpen ? "Yes" : "No", menuContext );
                    
                    CFStringRef menuCFStringRef;
                    status = CopyMenuTitleAsCFString( myMenuRef, &menuCFStringRef );
                    if ( ( noErr == status ) && ( menuCFStringRef ) ) {
                        char * myPtr = Copy_CFStringRefToCString( menuCFStringRef );
                        if ( myPtr ) {
                            if ( strlen( myPtr ) ) {
                                printf( "\nSampleCMPlugIn->MenuEvent_Handle->CopyMenuTitleAsCFString: \"%s\".", myPtr );
                            }
                            DisposePtr( myPtr );
                        }
                        CFRelease( menuCFStringRef );
                    }
#endif LOG_ENTRY_POINTS
                    
                    //
                    // Try to match against the menu item text strings.
                    //
                    
                    short idx, count = CountMenuItems( myMenuRef );
                    
                    for ( idx = 1;idx <= count;idx++ ) {
                        char itemString[256];
                        
                        CFStringRef tCFStringRef;
                        status = CopyMenuItemTextAsCFString( myMenuRef, idx, &tCFStringRef );
                        if ( noErr != status ) {
                            continue;
                        }
                        
                        if ( !CFStringGetCString( tCFStringRef, itemString, sizeof( itemString ), kCFStringEncodingUTF8 ) ) {
                            continue;
                        }
                        
                        // Ok, here's our first string...
                        if ( 0 == strncmp( gSubCommandCStrings[0], itemString, strlen( gSubCommandCStrings[0] ) ) ) {
                            printf( "\nSampleCMPlugIn->MenuEvent_Handle: found first sub item!" );
                            //
                            // Try SetItemMark with this one...
                            //
                            SetItemMark( myMenuRef, idx, kMenuPencilGlyph );
                            //
                            // ...this works!
                            //
                        } else if ( 0 == strncmp( gSubCommandCStrings[1], itemString, strlen( gSubCommandCStrings[1] ) ) ) {
                            printf( "\nSampleCMPlugIn->MenuEvent_Handle: found second sub item!" );
                            //
                            // Try to use SetMenuItemIconHandle with kMenuIconResourceType
                            // to pass a CFStringRef naming a resource in the main bundle of the process
                            //
                            CFStringRef tIconCFStringRef = CFSTR( "Skull&Bones.icns" );
                            status = SetMenuItemIconHandle( myMenuRef, idx, kMenuIconResourceType, ( Handle ) tIconCFStringRef );
                            if ( noErr != status ) {
                                fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle->SetMenuItemIconHandle( #%d ) error: %ld ( 0x%08lX ).", idx, status, status );
                            }
                            //
                            // This doesn't work. Note that there's a space for the icon but it's blank. weird.
                            // Just guessing but it's probably because the finder doesn't have this icon file.
                            // it's looking in the "main bundle" ( the Finder ) not the CMM bundle for this file?
                            //
                        }
                        else if ( 0 == strncmp( gSubCommandCStrings[2], itemString, strlen( gSubCommandCStrings[2] ) ) ) {
                            printf( "\nSampleCMPlugIn->MenuEvent_Handle: found third sub item!" );
                            //
                            // This time we're going to try to use SetMenuItemIconHandle with kMenuIconRefType to pass an icon ref
                            //
                            if ( myMenuRef ) {
                                status = SetMenuItemIconHandle( myMenuRef, idx, kMenuIconRefType, ( Handle ) iconRef );
                                if ( noErr != status )
                                    fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle->SetMenuItemIconHandle( #%d ) error: %ld ( 0x%08lX ).", idx, status, status );
                                else
                                    printf( "\nSampleCMPlugIn->MenuEvent_Handle->SetMenuItemIconHandle( #%d ) success!", idx );
                            }
                            //
                            // This works!
                            //
                        }
                        else if ( 0 == strncmp( gSubCommandCStrings[3], itemString, strlen( gSubCommandCStrings[3] ) ) ) {
                            printf( "\nSampleCMPlugIn->MenuEvent_Handle: found fourth sub item!" );
                            //
                            // this is the same code as above, I just want to test DisableMenuItemIcon
                            //
                            if ( myMenuRef ) {
                                status = SetMenuItemIconHandle( myMenuRef, idx, kMenuIconRefType, ( Handle ) iconRef );
                                if ( noErr != status )
                                    fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle->SetMenuItemIconHandle( #%d ) error: %ld ( 0x%08lX ).", idx, status, status );
                                else
                                    printf( "\nSampleCMPlugIn->MenuEvent_Handle->SetMenuItemIconHandle( #%d ) success!", idx );
                            }
                            DisableMenuItemIcon( myMenuRef, idx );
                            //
                            // This works!
                            //
                        }
                    }
                    break;
                }
                default:
                    fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle: handled event class ( \"%.4s\" ) & unhandled kind ( \"%.4s\" ).", ( char * ) &theClass, ( char * ) &theKind );
                    break;
            }
            break;
        }
        default:
            fprintf( stderr, "\nSampleCMPlugIn->MenuEvent_Handle: Unhandled event class ( \"%.4s\" ) & kind ( \"%.4s\" ).", ( char * ) &theClass, ( char * ) &theKind );
            break;
    }
    return result;
}   // MenuEvent_Handle
