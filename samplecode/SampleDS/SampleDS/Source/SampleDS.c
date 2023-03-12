/*
	File:		SampleDS.c
	
	Description:	Sample Data Source (DS)

	Author:	Image Capture Engineering

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):

	   <1>	 	6/17/03	WN			first file

*/

// ===========================================================================
//
//	Example code for TWAIN Data Source.

#include <TWAIN/TWAIN.h>
#include "TWDefs.h"
#include "TWSystem.h"
#include "TWUtilities.h"

#include <QuickTime/QuickTime.h>		// for GraphicsImporter

#include <string.h>
#include "SampleDS.h"
#define	DEBUGGING		1

static TW_IDENTITY		Identity;

static TW_INT16			DSIsAlive=FALSE;
static TW_INT16			gCurrentState=STATE_DSLOADED;
static TW_INT16			gConditionCode = TWCC_SUCCESS;

static	TW_STR32		Manufacturer = "\pApple";
static 	TW_STR32		ProductFamily = "\pApple sample ds";
static 	TW_STR32		ProductName = "\pSample DS";
static 	TW_STR32		VersionInfo = "\p1.23";

static PicHandle		hPicture=NULL;
static TW_INT16			gTransferReady = FALSE;
static TW_INT16			TWMessage=-1;
static TW_IMAGEINFO		ImageInfo;
static TW_IMAGELAYOUT 	ImageLayout;

static TW_SETUPFILEXFER	GlobalSetupFile;
static TW_STR32			FileString="\pClasp.tmp";

static TW_INT16			PixelTypes[]={TWPT_BW,TWPT_GRAY,TWPT_RGB,TWPT_PALETTE};
static TW_INT16			NumPixelTypes=4;
static TW_INT16			CurPixelType=TWPT_RGB;
static TW_INT16			DefPixelType=TWPT_RGB;

static TW_INT32			XResMinValue=50;
static TW_INT32			XResMaxValue=300;
static TW_INT32			XResStepSize=10;
static TW_INT32			XResDefaultValue=300;
static TW_INT32			XResCurrentValue=300;

static TW_INT32			YResNumItems=4;
static TW_INT32			YResCurrentIndex=3;
static TW_INT32			YResDefaultIndex=3;
static TW_FIX32			YResItemList[4]={{50,0},{75,0},{150,0},{300,0}};

static	FSSpec			gFileSpec = {0, 0, "\p"};
static	bool			gInfoLoaded = false;
static	GWorldPtr		gGWorld = nil;
static	UInt32			gRowsTransfered = 0;
static	TW_INT16		gTransferMode = TWSX_NATIVE;

static PicHandle 		gPicture = NULL;

static ControlRef 		gScanButton;
static ControlRef 		gCancelButton;
static DialogRef  		gDialog;
static Boolean    		gDebugInitialized = false;
static CFDictionaryRef	gDebugDictionary = NULL;

#define kScanButton     1
#define kCancelButton   2
#define kReturn		    0x0D
#define kEnter		    0x03
// --------------------------------------------------------------------------------


#pragma mark -
#pragma mark ---------------------- Debugging

// ---------------------------------------------------------------------------
//	• DS_LogText
// ---------------------------------------------------------------------------
//	Log a string

void DS_LogText ( const char* pFormat, ... )
{
#if ICA_DEBUG
    va_list 		pArg;
    char 			outString[1024];
    int				len;

    va_start(pArg, pFormat);
    len = vsnprintf(outString, 1020, pFormat, pArg);
    va_end( pArg );

    printf(outString);    
#endif
}

// ---------------------------------------------------------------------------
//	• DS_LogValue
// ---------------------------------------------------------------------------
//
void DS_LogValue (char* label,
                   char* labelKey,
                   char* format,
                   UInt32 value)
{
    char 	key[64];
    Boolean found = false;

    sprintf(key, format, value);
    if (gDebugDictionary)
    {
        CFStringRef		keyRef;
        CFStringRef		strRef;
        CFDictionaryRef	dictRef = NULL;

        // find the sub-dictionary
        keyRef = CFStringCreateWithCString(NULL, labelKey, 0);
        if (keyRef)
        {
            dictRef = (CFDictionaryRef)CFDictionaryGetValue(gDebugDictionary, keyRef);
            CFRelease(keyRef);
        }

        if (dictRef)
        {
            // find the value
            keyRef = CFStringCreateWithCString(NULL, key, 0);
            if (keyRef)
            {
                strRef = (CFStringRef)CFDictionaryGetValue(dictRef, keyRef);
                if (strRef)
                {
                    DS_LogText ( "%s [%s] - %s\n", label, key, CFStringGetCStringPtr(strRef,0));
                    found = true;
                }
                CFRelease(keyRef);
            }
        }
    }
    if (!found)
        DS_LogText ( "%s [%s]\n", label, key );

}

// ---------------------------------------------------------------------------
//	• DS_LogResult
// ---------------------------------------------------------------------------
//	Log errors

void DS_LogResult ( UInt16 result,
                     UInt16 condition )
{
    if ( result != TWRC_SUCCESS
         && result != TWRC_NOTDSEVENT
         && result != TWRC_DSEVENT )
    {

        DS_LogValue("    result : ", "Result", "%d", result);

        if ( condition != TWCC_SUCCESS )
        {
            DS_LogValue(" Condition : ", "Condition", "%d", condition);
        }
        DS_LogText ( "\n" );
    }
}

// ---------------------------------------------------------------------------
//	• DS_LogMessage
// ---------------------------------------------------------------------------
void DS_LogMessage(UInt16 message)
{
    DS_LogValue("   message : ", "Messages",	"0x%04x", 	message);
}

// ---------------------------------------------------------------------------
//	• DS_LogTriplet
// ---------------------------------------------------------------------------
//	Log the triplet passed to DS_Entry().

void DS_LogTriplet ( UInt32 group,
                      UInt16 attribute,
                      UInt16 message,
                      Ptr data )
{
    // don't log the event loop triplets
    if ( message != MSG_PROCESSEVENT )
    {
        DS_LogValue("     group : ", "DataGroup", 	"%d", 		group);
        DS_LogValue(" attribute : ", "Attribute",	"0x%04x", 	attribute);
        DS_LogValue("   message : ", "Messages",	"0x%04x", 	message);

        if ( attribute == DAT_CAPABILITY )
            DS_LogValue("capability : ", "Capabilities", "0x%04x", ((pTW_CAPABILITY) data)->Cap  );
        DS_LogText ( "\n" );
    }
}

// ---------------------------------------------------------------------------
//	• DS_DebugInitialize
// ---------------------------------------------------------------------------
//
void DS_DebugInitialize()
{
    // write a header of sorts
    // Date format: YYYY '-' MM '-' DD ' ' hh ':' mm ':' ss.fff
    CFTimeZoneRef tz = CFTimeZoneCopySystem();	// specifically choose system time zone for logs
    CFGregorianDate gdate = CFAbsoluteTimeGetGregorianDate(CFAbsoluteTimeGetCurrent(), tz);
    CFRelease(tz);
    gdate.second = gdate.second + 0.0005;

    DS_LogText (  "*****************************************************************\n");
    DS_LogText (  "TWAIN mach-o DS Debug Log - %02d:%02d:%06.3f  \n", gdate.hour, gdate.minute, gdate.second);
    DS_LogText (  "*****************************************************************\n");
    
    // load the gDebugDictionary...
    CFURLRef			fileURL = NULL;
    Boolean				status;
    CFDataRef 			resourceData = NULL;
    SInt32				errorCode;
    CFStringRef			errorString = NULL;
    CFBundleRef			bundle;

    bundle = CFBundleGetBundleWithIdentifier ( CFSTR( "com.apple.sampleds" ) );
    fileURL = CFBundleCopyResourceURL(bundle, CFSTR("DSDebug"), CFSTR("plist"), NULL);
    if (fileURL)
    {
        status = CFURLCreateDataAndPropertiesFromResource(NULL, fileURL, &resourceData, NULL, NULL, &errorCode);
        if (status && resourceData)
        {
            gDebugDictionary = (CFDictionaryRef)CFPropertyListCreateFromXMLData( NULL, resourceData, kCFPropertyListImmutable, &errorString);
            CFRelease(resourceData);
        }
        CFRelease(fileURL);
    }
}

// ---------------------------------------------------------------------------
//	• DS_DebugTerminate
// ---------------------------------------------------------------------------
//
void DS_DebugTerminate()
{
    DS_LogText ( "--------------------------------------------------------------------------------\n" );
}

#pragma mark -
#pragma mark ---------------------- DS_Entry
// ---------------------------------------------------------------------------
//	DS_Entry												  
// ---------------------------------------------------------------------------
//	Entry point for Data Source.

TW_UINT16 DS_Entry ( pTW_IDENTITY 	pSource, 
					 TW_UINT32 		DG,
					 TW_UINT16 		Dat, 
					 TW_UINT16 		Msg, 
					 TW_MEMREF 		pData )
{
	TW_INT16			result = TWRC_SUCCESS;

    if ( !gDebugInitialized )
    {
        DS_DebugInitialize();
        gDebugInitialized = true;
    }

    DS_LogTriplet ( DG, Dat, Msg, pData );

    if (DSIsAlive == FALSE) /* initialize on first call */
	{
		Identity.Id					=1;
		Identity.Version.MajorNum	=1;
		Identity.Version.MinorNum	=9;
		Identity.Version.Language	=1; /* jjg fill in later */
		Identity.Version.Country	=01; 
		
		pstrcopy((pTW_UINT8)Identity.Version.Info,(pTW_UINT8)VersionInfo);
		
		Identity.ProtocolMajor		=1;
		Identity.ProtocolMinor		=9;
		Identity.SupportedGroups	=DG_CONTROL|DG_IMAGE;
		
		pstrcopy((pTW_UINT8)Identity.Manufacturer,(pTW_UINT8)Manufacturer);
		pstrcopy((pTW_UINT8)Identity.ProductFamily,(pTW_UINT8)ProductFamily);
		pstrcopy((pTW_UINT8)Identity.ProductName,(pTW_UINT8)ProductName);

		GlobalSetupFile.Format		=TWFF_TIFF;
		GlobalSetupFile.VRefNum		=0;
		pstrcopy((pTW_UINT8)GlobalSetupFile.FileName,(pTW_UINT8)FileString);
		
		GetDefaultImageLayout(&ImageLayout);
		
		// init QuickTime
		EnterMovies();
		
		DSIsAlive=TRUE;
	}
	
	switch (DG) 
	{
		case DG_CONTROL:
			result = DS_Control(pSource,Dat,Msg,pData);
			break;
		case DG_IMAGE:
			result = DS_Image(pSource,Dat,Msg,pData);
			break;
		case DG_AUDIO:
		    result = TWRC_SUCCESS;
		    break;
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	
	if (result == TWRC_SUCCESS) /* clear the condition */
	{
		gConditionCode = TWCC_SUCCESS;
	} else
	{
        DS_LogResult ( result, gConditionCode );
	}
	return(result);
}


#pragma mark -
#pragma mark ---------------------- Control Data Group

// ---------------------------------------------------------------------------
//	DS_Control												  
// ---------------------------------------------------------------------------
//	Process Data Argument types for the DG_CONTROL Data Group.

TW_INT16 	DS_Control(pTW_IDENTITY pSource, TW_UINT16 Dat, TW_UINT16 Msg, TW_MEMREF pData)
{
	TW_INT16		result = TWRC_SUCCESS;

	switch (Dat) 
	{
		case DAT_STATUS:
			result = DS_Control_Status(pSource,Msg,(pTW_STATUS)pData);
			break;
            
		case DAT_IDENTITY:
			result = DS_Control_Identity(pSource,Msg,(pTW_IDENTITY)pData);
			break;
            
		case DAT_EVENT:
			result = DS_Control_Event(pSource,Msg,(pTW_EVENT)pData);
			break;
            
		case DAT_CAPABILITY:
			result = DS_Control_Capability(pSource,Msg,(pTW_CAPABILITY)pData);
			break;
            
		case DAT_PENDINGXFERS:
			result = DS_Control_PendingXfers(pSource,Msg,(pTW_UINT16)pData);
			break;
            
		case DAT_SETUPMEMXFER:
			result = DS_Control_SetupMemXfer(pSource,Msg,(pTW_SETUPMEMXFER)pData);
			break;
	/*
		case DAT_SETUPFILEXFER:
			result = DS_Control_SetupFileXfer(pSource,Msg,(pTW_SETUPFILEXFER)pData);
			break;
	*/
		case DAT_USERINTERFACE:
			result = DS_Control_UserInterface(pSource,Msg,(pTW_USERINTERFACE)pData);
			break;
            
		case DAT_XFERGROUP:
			result = DS_Control_XferGroup(pSource,Msg,(pTW_UINT16)pData);
			break;
            
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}

	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_Status												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_Status ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_STATUS pStatus )
{	
	TW_INT16		result = TWRC_SUCCESS;

	switch (Msg)
	{
		case MSG_GET:
			pStatus->ConditionCode = gConditionCode;
			pStatus->Reserved=0;
			gConditionCode = TWCC_SUCCESS;
			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_Identity												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_Identity ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_IDENTITY pIdentity)
{
	TW_INT16 result = TWRC_SUCCESS;

	switch (Msg)
	{
		case MSG_GET: /* get the DS's identity */
			if (pIdentity!=NULL)
				*pIdentity=Identity;
			break;
			
		case MSG_OPENDS: /* open the Data source for transfers */
			if (gCurrentState==STATE_DSLOADED)
			{
				if (pIdentity!=NULL) *pIdentity=Identity;
				gCurrentState=STATE_DSOPEN;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		case MSG_CLOSEDS: /* do any house cleaning needed before unload */
			if (gCurrentState==STATE_DSOPEN)
			{		
				gCurrentState=STATE_DSLOADED;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_Event												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_Event ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_EVENT pEvent )
{
	TW_INT16 result = TWRC_NOTDSEVENT;
	
    pEvent->TWMessage=MSG_NULL;
	return result;
	
	switch (Msg)
	{
		case MSG_PROCESSEVENT: /* Process an event record */

			if ((gCurrentState>=STATE_DSENABLED)&&(gCurrentState<=STATE_XFERREADY))
			{
				WindowRef 		windowRef;
				ControlRef		controlRef;
				EventRecord * 	event;
                int 			partCode;
                int 			controlPartCode;
				
				if (pEvent && pEvent->pEvent)
				{
					event = (EventRecord *)pEvent->pEvent;

					switch (event->what)
					{
					case mouseDown:
                        DS_LogText("event->what = mouseDown (%08X)\n", event->what);
						
						partCode = FindWindow(event->where, &windowRef);
						switch (partCode)
						{
						case inMenuBar:
							break;
                            
						case inContent:
							SetPortWindowPort ( windowRef );
							GlobalToLocal(&event->where);
							controlPartCode = FindControl(event->where, windowRef, &controlRef);
							switch (controlPartCode)
							{
							case kControlButtonPart:
								if (TrackControl(controlRef, event->where, nil))
								{
									if (controlRef == gCancelButton)
									{
										DS_LogText("     cancel button pressed\n");
										TWMessage = MSG_CLOSEDSREQ;
									} else if (controlRef == gScanButton)
									{
										DS_LogText("     scan button pressed\n");
										gTransferReady = TRUE;
										TWMessage 	  = MSG_XFERREADY;
										gCurrentState  = STATE_XFERREADY;
										DS_LogText("     gTransferReady = TRUE\n");
										DS_LogText("     TWMessage     = MSG_XFERREADY\n");
										DS_LogText("     gCurrentState  = STATE_XFERREADY\n");
									}
								}
								break;
                                
							default:
								DS_LogText("     controlPartCode = %d\n", controlPartCode);
								break;
							}
							break;
                            
						case inGoAway:
							break;
                            
						default:
							break;
						}
						break;
					case mouseUp:
						DS_LogText("event->what = mouseUp (%08X)\n", event->what);
						break;
                        
					case keyDown:
						DS_LogText("event->what = keyDown (%08X)\n", event->what);
						break;
                        
					case keyUp:
						DS_LogText("event->what = keyUp (%08X)\n", event->what);
						break;
                        
					case autoKey:
						DS_LogText("event->what = autoKey (%08X)\n", event->what);
						break;
                       
					case updateEvt: 
						DS_LogText("event->what = updateEvt (%08X)\n", event->what);
						windowRef = (WindowRef)event->message;
						SetPortWindowPort ( windowRef );
						BeginUpdate ( windowRef );
						DrawControls(windowRef);
						EndUpdate ( windowRef );
						break;
                       
					case diskEvt:
						DS_LogText("event->what = diskEvt (%08X)\n", event->what);
						break;
                        
					case activateEvt:
						DS_LogText("event->what = activateEvt (%08X)\n", event->what);
						break;
                        
					case kHighLevelEvent:
						DS_LogText("event->what = kHighLevelEvent (%08X)\n", event->what);
						break;
                        
					default:
						break;
					}
				}
				
				if (TWMessage == -1) 
				{
					pEvent->TWMessage=MSG_NULL;
				} else
				{
					if (DEBUGGING)
					{
						switch (TWMessage)
						{
						case MSG_CLOSEDSREQ:
							DS_LogText("DS_Control_Event:  MSG_CLOSEDSREQ\n");
							break;
						case MSG_XFERREADY:
							DS_LogText("DS_Control_Event:  MSG_XFERREADY\n");
							break;
						}
					}					
					
					pEvent->TWMessage=TWMessage;
					TWMessage=-1;
				}
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
            
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_Capability												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_Capability ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_CAPABILITY pCapability)
{
	TW_INT16		CurrentIndex;
	TW_INT16		DefaultIndex;
	TW_INT32		TempValue;
	TW_INT16		ValueType;
	TW_INT16		Count;
	TW_INT16		result = TWRC_SUCCESS;
	TW_UINT16		TempArray[16];
	
	if ((Msg==MSG_SET)||(Msg==MSG_RESET))
	{
		if (gCurrentState!=STATE_DSOPEN)
		{
			result = TWRC_FAILURE;
			gConditionCode = TWCC_SEQERROR;
			return(result);
		}
	}
	else
	{
		if (gCurrentState==STATE_DSLOADED)
		{
			result = TWRC_FAILURE;
			gConditionCode = TWCC_SEQERROR;
			return(result);
		}
	}
	
	switch (Msg)
	{
		case MSG_GET: /* get the current and possible values for a capability */
			switch (pCapability->Cap)
			{
				case CAP_XFERCOUNT:
					if (gTransferReady == TRUE)
						SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)1);
					else
						SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)0);
					break;
					
				case CAP_SUPPORTEDCAPS:
					{
						TW_UINT16	capList[] = {CAP_XFERCOUNT, ICAP_XFERMECH, ICAP_COMPRESSION, 
							ICAP_PIXELTYPE, ICAP_UNITS, ICAP_XRESOLUTION, ICAP_YRESOLUTION};
						SetupArray(pCapability, TWTY_UINT16, 7, (TW_UINT8*) capList);
					}
					break;
					
				case CAP_UICONTROLLABLE:
					SetupOneValue(pCapability,TWTY_BOOL,(TW_INT32) true);
					break;

				case ICAP_PLANARCHUNKY:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWPC_CHUNKY);
					break;

				case ICAP_XFERMECH:
					{
						TW_UINT16	xferList[] = {TWSX_NATIVE, TWSX_MEMORY};
						SetupEnumeration(pCapability, TWTY_UINT16, 2,
							FindIndex(gTransferMode, xferList, 2),
							FindIndex(TWSX_NATIVE, xferList, 2),
							(pTW_UINT8) xferList);
					}
					break;

				case ICAP_COMPRESSION:
					SetupOneValue(pCapability, TWTY_UINT16, TWCP_NONE);
					break;

				case ICAP_PIXELTYPE:
					SetupEnumeration(pCapability, TWTY_UINT16, NumPixelTypes,
						FindIndex(CurPixelType, PixelTypes, NumPixelTypes),
						FindIndex(TWPT_RGB, PixelTypes, NumPixelTypes),
						(pTW_UINT8) PixelTypes);
					break;

				case ICAP_PIXELFLAVOR:
					SetupOneValue(pCapability, TWTY_UINT16, TWPF_CHOCOLATE);
					break;
					
				case ICAP_UNITS:
					SetupOneValue(pCapability, TWTY_UINT16, TWUN_PIXELS);
					break;
					
				case ICAP_PHYSICALHEIGHT:
				case ICAP_PHYSICALWIDTH:
					SetupOneValue(pCapability, TWTY_FIX32, 0);
					break;
				
				case ICAP_BITORDER:
					SetupOneValue(pCapability, TWTY_UINT16, TWBO_MSBFIRST);
					break;
					
				case ICAP_XRESOLUTION: /* provided as an example of a range */
					SetupRange(pCapability,TWTY_FIX32, XResMinValue, XResMaxValue,
						XResStepSize,XResDefaultValue,XResCurrentValue);
					break;
				
				case ICAP_YRESOLUTION: /* provided as an example of an enumeration */
					SetupEnumeration(pCapability, TWTY_FIX32, YResNumItems,
						YResCurrentIndex,YResDefaultIndex,(TW_UINT8 *)YResItemList);
					break;
					
				default:
					result = TWRC_FAILURE;
					gConditionCode = TWCC_BADCAP;
					break;
			}
			break;
			
		case MSG_GETCURRENT:
			switch (pCapability->Cap)
			{
				case CAP_XFERCOUNT: /* same as get */
					if (gTransferReady == TRUE)
						SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)1);
					else
						SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)0);
					break;

				case ICAP_PLANARCHUNKY:  /* same as get */
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWPC_CHUNKY);
					break;

				case ICAP_XFERMECH:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32) gTransferMode);
					break;

				case ICAP_COMPRESSION: /* same as get */
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWCP_NONE);
					break;

				case ICAP_PIXELTYPE:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)CurPixelType);
					break;

				case ICAP_PIXELFLAVOR:
					SetupOneValue(pCapability, TWTY_UINT16, (TW_INT32) TWPF_CHOCOLATE);
					break;
					
				case ICAP_UNITS: /* same as get */
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32) TWUN_PIXELS);
					break;

				case ICAP_PHYSICALHEIGHT:
				case ICAP_PHYSICALWIDTH:
					SetupOneValue(pCapability, TWTY_FIX32, (TW_INT32) 0);
					break;
					
				case ICAP_BITORDER:
					SetupOneValue(pCapability, TWTY_UINT16, (TW_INT32) TWBO_MSBFIRST);
					break;
					
				case ICAP_XRESOLUTION: /* provided as an example of a range */
					SetupOneValue(pCapability,TWTY_FIX32,XResCurrentValue);
					break;
					
				default:
					result = TWRC_FAILURE;
					gConditionCode = TWCC_BADCAP;
					break;
			}
			break;
			
		case MSG_GETDEFAULT: /* get the default value for a capability */
			switch (pCapability->Cap)
			{
				case CAP_XFERCOUNT:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)0);
					break;

				case ICAP_PLANARCHUNKY:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWPC_CHUNKY);
					break;

				case ICAP_XFERMECH:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWSX_NATIVE);
					break;

				case ICAP_COMPRESSION:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWCP_NONE);
					break;

				case ICAP_PIXELTYPE:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWPT_RGB);
					break;

				case ICAP_PIXELFLAVOR:
					SetupOneValue(pCapability, TWTY_UINT16, (TW_INT32) TWPF_CHOCOLATE);
					break;
					
				case ICAP_UNITS:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWUN_PIXELS);
					break;

				case ICAP_PHYSICALHEIGHT:
				case ICAP_PHYSICALWIDTH:
					SetupOneValue(pCapability, TWTY_FIX32, (TW_INT32) 0);
					break;
					
				case ICAP_BITORDER:
					SetupOneValue(pCapability, TWTY_UINT16, (TW_INT32) TWBO_MSBFIRST);
					break;
					
				case ICAP_XRESOLUTION: /* provided as an example of a range */
					SetupOneValue(pCapability,TWTY_FIX32,XResDefaultValue);
					break;
					
				case ICAP_YRESOLUTION: /* provided as an example of a enumeration */
					TempValue=YResItemList[YResDefaultIndex].Whole;
					SetupOneValue(pCapability,TWTY_FIX32,(TW_INT32)TempValue);
					break;
					
				default:
					result = TWRC_FAILURE;
					gConditionCode = TWCC_BADCAP;
					break;
			}
			break;
			
		case MSG_RESET:
			switch (pCapability->Cap)
			{
				case CAP_XFERCOUNT:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)0);
					break;

				case ICAP_XFERMECH:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWSX_NATIVE);
					gTransferMode = TWSX_NATIVE;
					break;

				case ICAP_COMPRESSION:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWCP_NONE);
					break;

				case ICAP_PIXELTYPE:
					NumPixelTypes=4;
					CurPixelType=2;
					PixelTypes[0]=TWPT_BW;
					PixelTypes[1]=TWPT_GRAY;
					PixelTypes[2]=TWPT_RGB;
					PixelTypes[3]=TWPT_PALETTE;			
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWPT_RGB);
					break;

				case ICAP_PIXELFLAVOR:
					SetupOneValue(pCapability, TWTY_UINT16, (TW_INT32) TWPF_CHOCOLATE);
					break;
					
				case ICAP_UNITS:
					SetupOneValue(pCapability,TWTY_UINT16,(TW_INT32)TWUN_PIXELS);
					break;

				case ICAP_PHYSICALHEIGHT:
				case ICAP_PHYSICALWIDTH:
					SetupOneValue(pCapability, TWTY_FIX32, (TW_INT32) 0);
					break;
					
				case ICAP_BITORDER:
					SetupOneValue(pCapability, TWTY_UINT16, (TW_INT32) TWBO_MSBFIRST);
					break;
					
				case ICAP_XRESOLUTION: /* provided as an example of a range */
					XResMinValue=50;
					XResMaxValue=300;
					XResStepSize=10;
					XResDefaultValue=300;
					XResCurrentValue=300;
					
					SetupRange(pCapability,TWTY_FIX32,XResMinValue,XResMaxValue,
						XResStepSize,XResDefaultValue,XResCurrentValue);
					break;
					
				default:
					result = TWRC_FAILURE;
					gConditionCode = TWCC_BADCAP;
					break;
			}
			break;

		case MSG_SET: /* set the current value for a capability */
			switch (pCapability->Cap)
			{
				case CAP_XFERCOUNT:
					GetOneValue(pCapability,NULL,&TempValue);
					if (TempValue!=0)
					{
						result = TWRC_FAILURE;
						gConditionCode = TWCC_BADVALUE;
					}
					break;

				case ICAP_XFERMECH:
					TempValue = TWSX_NATIVE;
					switch (pCapability->ConType)
					{
						case TWON_ONEVALUE:
							result = GetOneValue(pCapability,NULL,&TempValue);
							break;
						
						case TWON_ENUMERATION:
							result = GetEnumeration(pCapability, &ValueType, &Count,
								&CurrentIndex, &DefaultIndex, (pTW_UINT8) TempArray, 32);
							if (result == TWRC_SUCCESS)
							{
								if (ValueType == TWTY_UINT16 || ValueType == TWTY_INT16)
								{
									TempValue = TempArray[CurrentIndex];
									if (TempValue != TWSX_NATIVE && TempValue != TWSX_MEMORY)
										TempValue = TempArray[DefaultIndex];
									
								}
								else if (ValueType == TWTY_UINT8 || ValueType == TWTY_INT8)
								{
									TempValue = ((TW_INT8*) TempArray)[CurrentIndex];
									if (TempValue != TWSX_NATIVE && TempValue != TWSX_MEMORY)
										TempValue = ((TW_INT8*) TempArray)[DefaultIndex];
								}
								else
								{
									result = TWRC_FAILURE;
									gConditionCode = TWCC_BADVALUE;
								}
							}
							break;
						
						default:
							
							break;
					}
					
					if (result != TWRC_SUCCESS || 
						(TempValue != TWSX_NATIVE && TempValue != TWSX_MEMORY))
					{
						result = TWRC_FAILURE;
						gConditionCode = TWCC_BADVALUE;
					}
					else
					{
						gTransferMode = TempValue;
					}
					break;
					
				case ICAP_COMPRESSION:
					GetOneValue(pCapability,NULL,&TempValue);
					if (TempValue!=TWCP_NONE)
					{
						result = TWRC_FAILURE;
						gConditionCode = TWCC_BADVALUE;
					}
					break;

				case ICAP_PIXELTYPE:
					if (pCapability->ConType==TWON_ONEVALUE)
					{
						GetOneValue(pCapability,NULL,&TempValue);
						CurPixelType=(TW_INT16)TempValue;
					}
					else
					{
						GetEnumeration(pCapability,NULL,&NumPixelTypes,
							&CurrentIndex,&DefaultIndex,(pTW_UINT8)&PixelTypes,8);
						
						CurPixelType=PixelTypes[CurrentIndex];
						DefPixelType=PixelTypes[DefaultIndex];
					}
					break;

				case ICAP_PIXELFLAVOR:
					// Do nothing... this is an example DS, not an example
					// of how to write Mac imaging code... :-)
					
					break;

				case ICAP_UNITS:
					GetOneValue(pCapability,NULL,&TempValue);
					if (TempValue!=TWUN_PIXELS)
					{
						result = TWRC_FAILURE;
						gConditionCode = TWCC_BADVALUE;
					}
					break;

				case ICAP_XRESOLUTION: /* provided as an example of a range */
					if (pCapability->ConType==TWON_ONEVALUE)
					{
						GetOneValue(pCapability,NULL,&XResCurrentValue);
					}
					else
					{
						GetRange(pCapability,NULL,&XResMinValue,&XResMaxValue,
							&XResStepSize,&XResDefaultValue,&XResCurrentValue);
					}
					break;
					
				default:
					result = TWRC_FAILURE;
					gConditionCode = TWCC_BADCAP;
					break;
			}
			break;
			
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_PendingXfers												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_PendingXfers ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_UINT16 pPendingXfers )
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_GET: /* get the number of pending transfers */
			if (gCurrentState!=STATE_DSLOADED)
			{
				if (gTransferReady == TRUE)
					*pPendingXfers = 1;
				else
					*pPendingXfers = 0;
			}
			else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		case MSG_ENDXFER: /* terminate the current transfer */
			if (gCurrentState==STATE_XFER) /* this will never happen since we only do natives */
			{
				if (gTransferReady == TRUE)
				{
					if (hPicture!=NULL)
						DisposeHandle((Handle) hPicture);
					hPicture=NULL;
					gTransferReady = FALSE;
				}
				*pPendingXfers=0;
				gCurrentState=STATE_DSENABLED;
				TWMessage=MSG_CLOSEDSREQ;
			}
			else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		case MSG_RESET:
			if (gCurrentState==STATE_XFERREADY)
			{
				if (gTransferReady == TRUE)
				{
					if (hPicture!=NULL)
						DisposeHandle((Handle) hPicture);
					hPicture=NULL;
					gTransferReady = FALSE;
					gCurrentState=STATE_DSENABLED;
				}
				*pPendingXfers=0;
			}
			else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_SetupMemXfer												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_SetupMemXfer ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_SETUPMEMXFER pSetupMem )
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_GET:
			if ((gCurrentState>=STATE_DSOPEN)&&(gCurrentState<=STATE_XFERREADY))
			{
				if (!gInfoLoaded)
					result = GetImageInfo();
				
				if (result == TWRC_SUCCESS)
				{
					pSetupMem->MinBufSize = 32 * 1024;

					// Width * Samples rounded to the next 16 byte boundry * height
					pSetupMem->MaxBufSize = 
					pSetupMem->Preferred = 
						((((ImageInfo.ImageWidth *
						(ImageInfo.BitsPerPixel / 8)) + 15) / 16) * 16) * 
						ImageInfo.ImageLength;
				}
			}
			else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_SetupFileXfer												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_SetupFileXfer ( pTW_IDENTITY pSource,
                                    TW_UINT16 Msg,
                                    pTW_SETUPFILEXFER pSetupFile )
{
	TW_INT16 result = TWRC_SUCCESS;
	
	if ((gCurrentState<STATE_DSOPEN)||(gCurrentState>STATE_XFERREADY))
	{
		result = TWRC_FAILURE;
		gConditionCode = TWCC_SEQERROR;
		return(result);
	}
	
	switch (Msg)
	{
		case MSG_GET:
			*pSetupFile=GlobalSetupFile;
			break;

		case MSG_GETDEFAULT:
			pstrcopy((pTW_UINT8)pSetupFile->FileName,(pTW_UINT8)FileString);
			pSetupFile->Format=TWFF_TIFF;
			pSetupFile->VRefNum=0;
			break;

		case MSG_SET:
			GlobalSetupFile=*pSetupFile;
			break;

		case MSG_RESET:
			pstrcopy((pTW_UINT8)GlobalSetupFile.FileName,(pTW_UINT8)FileString);
			GlobalSetupFile.Format=TWFF_TIFF;
			GlobalSetupFile.VRefNum=0;

			*pSetupFile=GlobalSetupFile;

			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}

// ---------------------------------------------------------------------------
//	DS_Control_UserInterface												  
// ---------------------------------------------------------------------------
//	
TW_INT16 DS_Control_UserInterface ( pTW_IDENTITY pSource,
                                    TW_UINT16 Msg,
                                    pTW_USERINTERFACE pUserInterface)
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_ENABLEDS:	/* show the UI and arm for possible transfers */
			if (gCurrentState==STATE_DSOPEN)
			{
				gCurrentState=STATE_DSENABLED;
				
				if (pUserInterface->ShowUI)
				{
                    DisplayUserInterface();
				}
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
			
		case MSG_DISABLEDS:	/* hide the UI and disarm */
			DisposeDialog(gDialog);
			
			if (gCurrentState==STATE_DSENABLED)
			{
				gCurrentState=STATE_DSOPEN;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
			
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Control_XferGroup												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Control_XferGroup ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_UINT16 pXferGroup)
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_GET:
			if ((gCurrentState>=STATE_DSOPEN)&&(gCurrentState<=STATE_XFERREADY))
			{
				*pXferGroup=DG_IMAGE;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}

#pragma mark -
#pragma mark ---------------------- Event handling

// ---------------------------------------------------------------------------
//	doDrawContent
// ---------------------------------------------------------------------------
//
void doDrawContent(WindowRef windowRef)
{
    GrafPtr oldPort;

    GetPort(&oldPort);
    SetPortWindowPort(windowRef);
    BeginUpdate ( windowRef );
    DrawControls(windowRef);
    EndUpdate ( windowRef );
    SetPort(oldPort);
}

// ---------------------------------------------------------------------------
//	doCloseCommand
// ---------------------------------------------------------------------------
//
void doCloseCommand()
{
}

// ---------------------------------------------------------------------------
//	doAdjustCursor
// ---------------------------------------------------------------------------
//
void  doAdjustCursor(WindowRef windowRef)
{
#pragma unused (windowRef)
    SetThemeCursor(kThemePlusCursor);
}

// ---------------------------------------------------------------------------
//	windowEventHandler
// ---------------------------------------------------------------------------
//
OSStatus  windowEventHandler(EventHandlerCallRef eventHandlerCallRef,
                             EventRef   eventRef,
                             void*      userData)
{
#pragma unused (eventHandlerCallRef, userData)
    OSStatus		result = eventNotHandledErr;
    UInt32			eventClass;
    UInt32			eventKind;
    EventRecord	    eventRecord;
    SInt16			itemHit;
    SInt8		    charCode;
    ControlRef	    controlRef;
    UInt32 			finalTicks;

    eventClass = GetEventClass(eventRef);
    eventKind  = GetEventKind(eventRef);

    //DS_LogText("windowEventHandler eventClass - '%4.4s'\n", &eventClass);
    //DS_LogText("                   eventKind  - '%d'\n", eventKind);
    switch(eventClass)
    {
        case kEventClassWindow:																// event class window
            ConvertEventRefToEventRecord(eventRef,&eventRecord);
            switch(eventKind)
            {
                case kEventWindowUpdate:
                    doDrawContent(GetDialogWindow(gDialog));
                    break;

                case kEventWindowDrawContent:
                    DrawControls(GetDialogWindow(gDialog));
                    result = noErr;
                    break;

                case kEventWindowActivated:
                    DialogSelect(&eventRecord,&gDialog,&itemHit);
                    result = noErr;
                    break;

                case kEventWindowClose:
                    QuitApplicationEventLoop();
                    result = noErr;
                    break;
            }

            case kEventClassMouse:																// event class mouse
                ConvertEventRefToEventRecord(eventRef,&eventRecord);
                switch(eventKind)
                {
                    case kEventMouseDown:
                        if(IsDialogEvent(&eventRecord))
                        {
                            if(DialogSelect(&eventRecord,&gDialog,&itemHit))
                            {
                                TW_CALLBACK callback = {};

                                if (itemHit == kScanButton)
                                {
                                    DS_LogText("kScanButton pressed\n");

                                    gTransferReady = true;
                                    gCurrentState = STATE_XFERREADY;

                                    //  now call into the DSM...

                                    callback.Message = MSG_XFERREADY;

                                    result = DSM_Entry((pTW_IDENTITY)&Identity,
                                                       (pTW_IDENTITY)NULL,
                                                       (TW_UINT32)DG_CONTROL,
                                                       (TW_UINT16)DAT_CALLBACK,
                                                       (TW_UINT16)MSG_INVOKE_CALLBACK,
                                                       (TW_MEMREF) &callback);

                                } else if(itemHit == kCancelButton)
                                {
                                    DS_LogText("kCancelButton pressed\n");

                                    //  now call into the DSM...

                                    callback.Message = MSG_CLOSEDSREQ;

                                    result = DSM_Entry((pTW_IDENTITY)&Identity,
                                                       (pTW_IDENTITY)NULL,
                                                       (TW_UINT32)DG_CONTROL,
                                                       (TW_UINT16)DAT_CALLBACK,
                                                       (TW_UINT16)MSG_INVOKE_CALLBACK,
                                                       (TW_MEMREF) &callback);
                                }
                            }
                        }
                        doAdjustCursor(GetDialogWindow(gDialog));
                        break;

                    case kEventMouseMoved:
                    {
                        Rect 			pictRect;
                        DialogItemType	itemType;
                        Handle          item;
                        Point			mouseLocation;

                        SetPortWindowPort ( GetDialogWindow(gDialog) );
                        GetDialogItem(gDialog, 3, &itemType, &item, &pictRect);
                        GetMouse(&mouseLocation);

                        if (PtInRect(mouseLocation, &pictRect))
                            SetThemeCursor(kThemePlusCursor);
                        else
                            SetThemeCursor(kThemeArrowCursor);
                    }
                        break;
                }
                    break;

            case kEventClassKeyboard:															 // event class keyboard
                switch(eventKind)
                {
                    case kEventRawKeyDown:
                        ConvertEventRefToEventRecord(eventRef,&eventRecord);
                        GetEventParameter(eventRef,kEventParamKeyMacCharCodes,typeChar,NULL,
                                          sizeof(charCode),NULL,&charCode);
                        if((charCode == kReturn) || (charCode == kEnter))
                        {
                            GetDialogItemAsControl(gDialog,kScanButton,&controlRef);
                            HiliteControl(controlRef,kControlButtonPart);
                            Delay(8,&finalTicks);
                            HiliteControl(controlRef,kControlEntireControl);
                            return noErr;
                        }
                            break;
                }
                break;
    }

    return result;
}

#pragma mark -
#pragma mark ---------------------- Image Data Group

// ---------------------------------------------------------------------------
//	DS_Image												  
// ---------------------------------------------------------------------------
//	Process Data Argument types for the DG_IMAGE Data Group.

TW_INT16 DS_Image ( pTW_IDENTITY pSource, TW_UINT16 Dat, TW_UINT16 Msg, TW_MEMREF pData)
/*
**	Parameters:		None.
**	Globals:		None.
**	Operation:		
**	Return:
**		status		TWRC_SUCCESS
**					TWRC_FAILURE
**	History:		
*/
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Dat) 
	{
		case DAT_IMAGEINFO:
			result = DS_Image_ImageInfo(pSource,Msg,(pTW_IMAGEINFO)pData);
			break;
            
		case DAT_IMAGENATIVEXFER:
			result = DS_Image_NativeXfer(pSource,Msg,(PicHandle *)pData);
			break;
            
		case DAT_IMAGEFILEXFER:
			result = DS_Image_FileXfer(pSource,Msg,pData);
			break;
            
		case DAT_IMAGEMEMXFER:
			result = DS_Image_MemXfer(pSource,Msg,(pTW_IMAGEMEMXFER)pData);
			break;
            
		case DAT_IMAGELAYOUT:
			result = DS_Image_ImageLayout(pSource,Msg,(pTW_IMAGELAYOUT)pData);
			break;
            
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Image_ImageInfo												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Image_ImageInfo ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_IMAGEINFO pImageInfo )
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
	case MSG_GET: /* get the current transfers image information */
		if (gCurrentState==STATE_XFERREADY)
		{
			if (!gInfoLoaded)
				result = GetImageInfo();
			
			if (result == TWRC_SUCCESS)
				*pImageInfo=ImageInfo;
		} else
		{
			result = TWRC_FAILURE;
			gConditionCode = TWCC_SEQERROR;
		}
		break;
        
	default:
		result = TWRC_FAILURE;
		gConditionCode = TWCC_BADPROTOCOL;
		break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Image_ImageLayout												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Image_ImageLayout ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_IMAGELAYOUT pImageLayout)
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_GET: /* get the current transfers image information */
			if ((gCurrentState>=STATE_DSOPEN)&&(gCurrentState<=STATE_XFERREADY))
			{
				*pImageLayout=ImageLayout;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
            
		case MSG_GETDEFAULT: /* get the current transfers image information */
			if ((gCurrentState>=STATE_DSOPEN)&&(gCurrentState<=STATE_XFERREADY))
			{
				GetDefaultImageLayout(pImageLayout);
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
                
		case MSG_RESET: /* get the current transfers image information */
			if (gCurrentState==STATE_DSOPEN)
			{
				GetDefaultImageLayout(pImageLayout);
				ImageLayout=*pImageLayout;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
                    
		case MSG_SET: /* get the current transfers image information */
			if (gCurrentState==STATE_DSOPEN)
			{
				ImageLayout=*pImageLayout;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
                        
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Image_NativeXfer												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Image_NativeXfer ( pTW_IDENTITY pSource, TW_UINT16 Msg, PicHandle *pHandle )
{
	TW_INT16 result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_GET:
			if (gCurrentState==STATE_XFERREADY)
			{
				if (gTransferReady)
				{
					if (hPicture == nil)
						result = LoadImageAsPict();
					
					if (result == TWRC_SUCCESS)
					{
						*pHandle=hPicture;
						hPicture = NULL;
						gTransferReady = FALSE;
						gCurrentState = STATE_XFER;
					} else
					{
						*pHandle = nil;
						hPicture = nil;
						gTransferReady = FALSE;
						gCurrentState = STATE_XFERREADY;
					}
				}
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
            
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Image_MemXfer												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Image_MemXfer ( pTW_IDENTITY pSource, TW_UINT16 Msg, pTW_IMAGEMEMXFER pImageXfer )
{
	TW_INT16		result = TWRC_SUCCESS;
	
	switch (Msg)
	{
		case MSG_GET: /* get a block of image information */
			if ((gCurrentState==STATE_XFERREADY) || (gCurrentState==STATE_XFER))
			{
				if (gTransferReady)
				{
					if (gGWorld == nil)
						result = LoadImageAsGWorld();
					
					if (result == TWRC_SUCCESS)
					{
						// Clear the image buffer
						char*	buffer = pImageXfer->Memory.TheMem;
                        UInt32	rowsThisTime;
                        
						if (pImageXfer->Memory.Flags & TWMF_HANDLE)
							buffer = (char*) * (Handle) buffer;
						memset(buffer, 128, pImageXfer->Memory.Length);
						
						// Figure how many rows to transfer on this pass
                        rowsThisTime = pImageXfer->Memory.Length / (ImageInfo.ImageWidth * 3);
						if (gRowsTransfered + rowsThisTime > ImageInfo.ImageLength)
							rowsThisTime = ImageInfo.ImageLength - gRowsTransfered;
						
						// Fill in the info
						pImageXfer->Compression = TWCP_NONE;
						pImageXfer->BytesPerRow = ImageInfo.ImageWidth * 3;
						pImageXfer->Columns = ImageInfo.ImageWidth;
						pImageXfer->Rows = rowsThisTime;
						pImageXfer->XOffset = 0;
						pImageXfer->YOffset = gRowsTransfered;
						pImageXfer->BytesWritten = pImageXfer->Memory.Length;
						
						// Move pixels from the GWorld to the buffer
						(void) LockPixels(GetGWorldPixMap(gGWorld));
						{
							UInt32		gworldRowBytes = (*GetGWorldPixMap(gGWorld))->rowBytes & 0x3FFE;
							UInt32		col, row;
							UInt8*		gworldPixel;
							UInt8*		bitmapPixel;
							
							for (row = 0; row < rowsThisTime; row++)
							{
								gworldPixel = (UInt8*) GetPixBaseAddr(
														GetGWorldPixMap(gGWorld)) + 
														((row + gRowsTransfered) * gworldRowBytes);
								bitmapPixel = (UInt8*) (buffer + (row * pImageXfer->BytesPerRow));

								for (col = 0; col < ImageInfo.ImageWidth; col++)
								{
									bitmapPixel[0] = gworldPixel[1];
									bitmapPixel[1] = gworldPixel[2];
									bitmapPixel[2] = gworldPixel[3];

									bitmapPixel += 3;
									gworldPixel += 4;
								}
							}
						}
						UnlockPixels(GetGWorldPixMap(gGWorld));
						gCurrentState=STATE_XFER;
						
						gRowsTransfered += rowsThisTime;
						
						// See if we are done
						if (gRowsTransfered == ImageInfo.ImageLength)
						{
							gRowsTransfered = 0;
							gTransferReady = FALSE;
							gCurrentState=STATE_XFER;
							result = TWRC_XFERDONE;
						}
					} else
					{
						
					}
				}
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;

		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	
	return(result);
}



// ---------------------------------------------------------------------------
//	DS_Image_FileXfer												  
// ---------------------------------------------------------------------------
//	

TW_INT16 DS_Image_FileXfer ( pTW_IDENTITY 	pSource,
                             TW_UINT16 		Msg,
                             TW_MEMREF  	pNULL  )
{
	TW_INT16		result = TWRC_SUCCESS;
	
	switch (Msg)
	{
	#if 0
		case MSG_GET: /* get a block of image information */
			DS_LogText("DS_Image_FileXfer:  MSG_GET\n");

			if (gCurrentState==STATE_XFERREADY)
			{
				/* do file transfer */

				gCurrentState=STATE_XFER;
			} else
			{
				result = TWRC_FAILURE;
				gConditionCode = TWCC_SEQERROR;
			}
			break;
            
	#endif
		default:
			result = TWRC_FAILURE;
			gConditionCode = TWCC_BADPROTOCOL;
			break;
	}
	return(result);
}


#pragma mark -
#pragma mark ---------------------- Internal
// ---------------------------------------------------------------------------
// FSPathMakeFSSpec
// ---------------------------------------------------------------------------
OSErr FSPathMakeFSSpec( const UInt8 *	inPath,
                        FSSpec *		outFSSpec,
                        Boolean *		outIsDirectory )
{
    OSStatus        err;
    FSRef           ref;

    err = FSPathMakeRef( inPath, &ref, outIsDirectory );
    require_noerr( err, exit );

    err = FSGetCatalogInfo( &ref, kFSCatInfoNone, NULL, NULL, outFSSpec,  NULL );
    require_noerr( err, exit );

exit:
        return( err );
}

// ---------------------------------------------------------------------------
//	DisplayUserInterface
// ---------------------------------------------------------------------------
//

void DisplayUserInterface()
{
    SInt16 		savedResRefNum;
    SInt16 		resRefNum = 0;
    CFBundleRef selfBundleRef;
    OSStatus 	status = noErr;
    ControlRef 	pictureControlRef;

    EventTypeSpec	windowEvents[] = { { kEventClassWindow,	kEventWindowDrawContent  },
    { kEventClassWindow, kEventWindowUpdate       },
    { kEventClassWindow, kEventWindowClose        },
    { kEventClassWindow, kEventWindowClickDragRgn },
    { kEventClassMouse,  kEventMouseDown          },
    { kEventClassMouse,  kEventMouseMoved    } };

    selfBundleRef = CFBundleGetBundleWithIdentifier ( CFSTR ( "com.apple.sampleds" ) );

    if (selfBundleRef)
        resRefNum = CFBundleOpenBundleResourceMap ( selfBundleRef );
    else
        DS_LogText("ERROR: selfBundleRef is NULL");
    savedResRefNum = CurResFile();
    UseResFile ( resRefNum );

    gDialog = GetNewDialog(128, nil, (WindowRef)-1);

    UseResFile ( savedResRefNum );

    ChangeWindowAttributes(GetDialogWindow(gDialog),kWindowStandardHandlerAttribute |
                           kWindowCloseBoxAttribute,
                           kWindowCollapseBoxAttribute);
    GetDialogItemAsControl(gDialog, kScanButton,   &gScanButton);
    DS_LogText("gScanButton   = %08X\n", gScanButton);
    GetDialogItemAsControl(gDialog, kCancelButton, &gCancelButton);
    DS_LogText("gCancelButton = %08X\n", gCancelButton);

    InstallStandardEventHandler(GetWindowEventTarget(GetDialogWindow(gDialog)));
    InstallWindowEventHandler(GetDialogWindow(gDialog),
                              NewEventHandlerUPP((EventHandlerProcPtr) windowEventHandler),
                              GetEventTypeCount(windowEvents), windowEvents,
                              0,NULL);
    DS_LogText("InstallWindowEventHandler done\n");

    ShowWindow(GetDialogWindow(gDialog));

    // drop the file selection sheet
//    gImageFileInfo.specified = FilteredChooseOneFile ( &gImageFileInfo.spec, GetDialogWindow ( gDialog ) );

//    LoadImageAsPICT();

    status = GetDialogItemAsControl ( gDialog, 3, &pictureControlRef );
    require_noerr ( status, BAIL );

    gPicture = GetPicture(130);
    status = SetControlData ( pictureControlRef, kControlPicturePart, kControlPictureHandleTag, sizeof ( PicHandle ), &gPicture );
    require_noerr ( status, BAIL );

    DrawOneControl ( pictureControlRef );

BAIL:
        return;
}

// --------------------------------------------------------------------------------
//	SelectFile
//
//	Ask the user to choose a file
// --------------------------------------------------------------------------------

TW_INT16 SelectFile(void)
{
	TW_INT16 		result = TWRC_FAILURE;
	OSErr 			err = noErr;
    NavReplyRecord 	navReplyRec;
    NavDialogOptions navDialogOptions;
    NavTypeListHandle fileTypeListHandle;
    
	err = NavLoad();
	
	navReplyRec.validRecord = false;
	
	NavGetDefaultDialogOptions ( &navDialogOptions );
	navDialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;

    fileTypeListHandle = (NavTypeListHandle)Get1Resource ( 'open', 128 );
	err = ResError();
	
	err = NavChooseFile ( nil, &navReplyRec, &navDialogOptions, nil, nil, nil, fileTypeListHandle, nil );
	
	if ( ( err != noErr ) || navReplyRec.validRecord == false )
	{
		result = TWRC_CANCEL;
	} else
	{
		AEDesc specDesc;
		AEDescList	selectedItems = navReplyRec.selection;
		err = AECoerceDesc ( &selectedItems, typeFSS, &specDesc );

		gFileSpec = **(FSSpec**) specDesc.dataHandle;
		
		AEDisposeDesc ( &specDesc );

		gTransferReady = true;
		TWMessage = MSG_XFERREADY;
		gCurrentState = STATE_XFERREADY;
		result = TWRC_SUCCESS;
	}

	// cleanup
	if ( navReplyRec.validRecord )
		NavDisposeReply ( &navReplyRec );
	ReleaseResource ( (Handle)fileTypeListHandle );
	NavUnload();
	
	return result;
}


// --------------------------------------------------------------------------------
//	LoadImageAsPict
//
//	Load the selected image as a PICT
// --------------------------------------------------------------------------------

TW_INT16 LoadImageAsPict(void)
{
	TW_INT16				result = TWRC_FAILURE;
	GraphicsImportComponent importer = nil;

	if (GetGraphicsImporterForFile(&gFileSpec, &importer) == noErr)
	{
#ifdef USING_QUICKTIME_4
        if (GraphicsImportSetFlags(importer, kGraphicsImporterDontDoGammaCorrection) == noErr)
#endif
		{
			PicHandle	pictHandle = nil;
			if (GraphicsImportGetAsPicture(importer, &pictHandle) == noErr)
			{
				// Whew! We now have a PicHandle. Unfortunately, the caller
				// expects a normal Handle. Copy the data and kill the picture.
				UInt32	pictSize = GetHandleSize((Handle) pictHandle);
				Handle	dataHandle = NewHandle(pictSize);
				
				if (dataHandle && MemError() == noErr)
				{
					BlockMoveData(*pictHandle, *dataHandle, pictSize);
					hPicture = (PicHandle) dataHandle;
					result = TWRC_SUCCESS;
				}
				KillPicture(pictHandle);
			}
		}
		
		// Clean up
		if (importer)
			CloseComponent(importer);
	}
	
	return (result);
}

// --------------------------------------------------------------------------------
//	LoadImageAsGWorld
//
//	Load the selected image as a GWorld
// --------------------------------------------------------------------------------

TW_INT16 LoadImageAsGWorld(void)
{
	TW_INT16				result = TWRC_FAILURE;
	GraphicsImportComponent importer = nil;

	if (GetGraphicsImporterForFile(&gFileSpec, &importer) == noErr)
	{
#ifdef USING_QUICKTIME_4
        if (GraphicsImportSetFlags(importer, kGraphicsImporterDontDoGammaCorrection) == noErr)
#endif
		{
			// Create a GWorld to hold the image
			Rect	rect = {0, 0, ImageInfo.ImageLength, ImageInfo.ImageWidth};
			UInt16	depth = ImageInfo.BitsPerPixel;
			
			if (depth == 24)
				depth = 32;
			
			if (NewGWorld(&gGWorld, depth, &rect, nil, nil, useTempMem) == noErr)
			{
				// Erase the GWorld so the picture loads correctly
				CGrafPtr	savePort;
				GDHandle	saveDevice;
				RGBColor	white = {65535, 65535, 65535};
				RGBColor	black = {0, 0, 0};
				GetGWorld(&savePort, &saveDevice);
				SetGWorld(gGWorld, nil);
				
				RGBForeColor(&white);
				RGBBackColor(&black);
				
				EraseRect(&rect);
				
				// Load the image
				if (GraphicsImportSetGWorld(importer, gGWorld, nil) == noErr)
				{
					if (GraphicsImportDraw(importer) == noErr)
					{
						result = TWRC_SUCCESS;
					}
				}
				SetGWorld(savePort, saveDevice);
			}
		}
		
		// Clean up
		if (importer)
			CloseComponent(importer);
	}
	
	if (result != TWRC_SUCCESS)
	{
		if (gGWorld != nil)
		{
			DisposeGWorld(gGWorld);
			gGWorld = nil;
		}
	}
	return (result);
}

// --------------------------------------------------------------------------------
//	GetImageInfo
//
//	Loads ImageInfo with information about the image
// --------------------------------------------------------------------------------

TW_INT16 GetImageInfo(void)
{
	TW_INT16			result = TWRC_FAILURE;
    CFURLRef			fileURL = NULL;
    CFBundleRef			bundle;
    FSRef 				fsRef;
    
    gFileSpec.name[0] = 0;

    // find the sample.pict in our bundle...
    bundle = CFBundleGetBundleWithIdentifier ( CFSTR( "com.apple.sampleds" ) );
    fileURL = CFBundleCopyResourceURL(bundle, CFSTR("sample"), CFSTR("pict"), NULL);
    if (fileURL)
    {
        if ( CFURLGetFSRef(fileURL, &fsRef) )
        {
            char buffer[256];
            FSRefMakePath(&fsRef, buffer, 256);
            DS_LogText ( "found sample pict: '%s'\n", buffer );
            FSPathMakeFSSpec(buffer, &gFileSpec, 0);
        }

        CFStringRef pathStr = CFURLCopyPath(fileURL);
        if (pathStr)
        {

            CFRelease(pathStr);
        }
        CFRelease(fileURL);
    }
    
	if (gFileSpec.name[0])
	{
		GraphicsImportComponent importer = nil;
		ImageDescriptionHandle	imageDescHandle = nil;
		ImageDescriptionPtr		imageDescPtr = nil;
		
		if (GetGraphicsImporterForFile(&gFileSpec, &importer) == noErr)
		{
#ifdef USING_QUICKTIME_4
			if (GraphicsImportSetFlags(importer, kGraphicsImporterDontDoGammaCorrection) == noErr)
#endif
			{
				if (GraphicsImportGetImageDescription(importer, &imageDescHandle) == noErr)
				{
					HLock ( (Handle)imageDescHandle );
					{
						imageDescPtr = *imageDescHandle;
						
						ImageInfo.XResolution.Whole			= Fix2Long(imageDescPtr->hRes);
						ImageInfo.XResolution.Frac			= 0;
						ImageInfo.YResolution.Whole			= Fix2Long(imageDescPtr->vRes);
						ImageInfo.YResolution.Frac			= 0;
						ImageInfo.ImageWidth				= imageDescPtr->width;
						ImageInfo.ImageLength				= imageDescPtr->height;
						ImageInfo.SamplesPerPixel			= 3;
						ImageInfo.BitsPerPixel				= imageDescPtr->depth;
						// Clip to 24 bits, since Mac GWorlds are usually 32 bits
						if (ImageInfo.BitsPerPixel > 24)
							ImageInfo.BitsPerPixel = 24;
						ImageInfo.BitsPerSample[0]			= ImageInfo.BitsPerPixel / ImageInfo.SamplesPerPixel;
						ImageInfo.BitsPerSample[1]			= ImageInfo.BitsPerPixel / ImageInfo.SamplesPerPixel;
						ImageInfo.BitsPerSample[2]			= ImageInfo.BitsPerPixel / ImageInfo.SamplesPerPixel;
						ImageInfo.Planar					= FALSE;
						ImageInfo.PixelType					= TWPT_RGB;
						ImageInfo.Compression				= TWCP_NONE;
						
						// Always return that the image is 24 bits for this sample
						// Handling lower bit depth images is left as an exercise for the
						// reader...
						ImageInfo.BitsPerPixel = 24;
						
						
						gInfoLoaded = true;
						result = TWRC_SUCCESS;
					}
					HUnlock ( (Handle)imageDescHandle );
				}
			}
		}
		
		// Clean up
		if (importer)
			CloseComponent(importer);
		if (imageDescHandle)
			DisposeHandle((Handle) imageDescHandle);
	}
	else
	{
		TWMessage=MSG_CLOSEDSREQ;
		return(TWRC_CANCEL);
	}
	
	return (result);
}



// ---------------------------------------------------------------------------
//	FindIndex												  
// ---------------------------------------------------------------------------
//	Retruns the index of the requested Item.

TW_INT16 FindIndex(TW_INT16 	Item,
                   pTW_INT16 	pList,
                   TW_INT16 	Number)
{
	TW_INT16		i;
	
	for (i=0;i<Number;i++)
	{
		if (pList[i]==Item)
            return(i);
	}
	return(TWERR);
}

// ---------------------------------------------------------------------------
//	GetDefaultImageLayout												  
// ---------------------------------------------------------------------------
//	

TW_INT16 GetDefaultImageLayout(pTW_IMAGELAYOUT pImageLayout)
{
	pImageLayout->Frame.Top.Whole	=0;
	pImageLayout->Frame.Top.Frac	=0;
	pImageLayout->Frame.Bottom.Whole=11;
	pImageLayout->Frame.Bottom.Frac	=0;
	pImageLayout->Frame.Left.Whole	=0;
	pImageLayout->Frame.Left.Frac	=0;
	pImageLayout->Frame.Right.Whole	=8;
	pImageLayout->Frame.Right.Frac	=65536/2;

	pImageLayout->DocumentNumber	=0;
	pImageLayout->PageNumber		=0;
	pImageLayout->FrameNumber		=0;

	return(OKAY);
}