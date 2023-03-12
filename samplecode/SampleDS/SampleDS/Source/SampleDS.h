// ===========================================================================
//	SampleDS.h				TWAIN 1.9			©1991-2002 TWAIN Working Group
// ===========================================================================
//

// --------------------------------------------------------------------------------

/* Control Group Prototypes */

TW_INT16	DS_Control(pTW_IDENTITY pSource,TW_UINT16 Dat, TW_UINT16 Msg,TW_MEMREF pData);
TW_INT16	DS_Control_Status(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_STATUS pStatus);
TW_INT16	DS_Control_Identity(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_IDENTITY pDSIdentity);
TW_INT16	DS_Control_Event(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_EVENT pEvent);
TW_INT16	DS_Control_Capability(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_CAPABILITY pCapability);
TW_INT16	DS_Control_PendingXfers(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_UINT16 pPendingXfers);
TW_INT16	DS_Control_SetupMemXfer(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_SETUPMEMXFER pSetupMem);
TW_INT16	DS_Control_SetupFileXfer(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_SETUPFILEXFER pSetupFile);
TW_INT16	DS_Control_UserInterface(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_USERINTERFACE pUserInterface);
TW_INT16 DS_Control_XferGroup(pTW_IDENTITY pSource, TW_UINT16 Msg,pTW_UINT16 pXferGroup);
					
/* Image Group Prototypes */

TW_INT16	DS_Image(pTW_IDENTITY pSource,TW_UINT16 Dat,TW_UINT16 Msg,TW_MEMREF pData);
TW_INT16	DS_Image_ImageInfo(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_IMAGEINFO pImageInfo);
TW_INT16	DS_Image_ImageLayout(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_IMAGELAYOUT pImageLayout);
TW_INT16	DS_Image_NativeXfer(pTW_IDENTITY pSource,TW_UINT16 Msg,PicHandle *pHandle);
TW_INT16	DS_Image_MemXfer(pTW_IDENTITY pSource,TW_UINT16 Msg,pTW_IMAGEMEMXFER pImageXfer);
TW_INT16	DS_Image_FileXfer(pTW_IDENTITY pSource,TW_UINT16 Msg,TW_MEMREF pNULL); /* pData is null */

/* Internal Utility Routines */

TW_INT16 SelectFile(void);
TW_INT16 GetImageInfo(void);
TW_INT16 LoadImageAsPict(void);
TW_INT16 LoadImageAsGWorld(void);
void DisplayUserInterface();

TW_INT16		FindIndex(TW_INT16 Item,pTW_INT16 pList,TW_INT16 Number);
//TW_UINT16	FindIndex(TW_INT16 Item,pTW_UINT16 pList,TW_INT16 Number);

TW_INT16	GetDefaultImageLayout(pTW_IMAGELAYOUT pImageLayout);


void doDrawContent(WindowRef windowRef);
void doCloseCommand();
void  doAdjustCursor(WindowRef windowRef);
OSStatus  windowEventHandler(EventHandlerCallRef eventHandlerCallRef,
                             EventRef   eventRef,
                             void*      userData);

/* debugging */
void DS_LogText ( const char* pFormat, ... );
void DS_LogValue (char* label,
                  char* labelKey,
                  char* format,
                  UInt32 value);
void DS_LogResult ( UInt16 result,
                    UInt16 condition );
void DS_LogMessage(UInt16 message);
void DS_LogTriplet ( UInt32 group,
                     UInt16 attribute,
                     UInt16 message,
                     Ptr data );
