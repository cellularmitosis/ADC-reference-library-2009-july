/* main.c */

#include <Carbon/Carbon.h>

#include "main.h"
#include "ServerInfo.h"

void Initialize(void);	/* function prototypes */
void EventLoop(void);
void MakeWindow(void);
void MakeMenu(void);
void DoEvent(EventRecord *event);
void DoMenuCommand(long menuResult);
void DoAboutBox(void);
void DrawWindow(WindowRef window);
void StopServer(void);
OSStatus DoBindEndpoint(void);
OSStatus StartServer(void);
void DoReceiveData(EndpointRef myEp);
static OSErr PassServerInfoEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon );
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
static OSStatus HandleOTLookErr(EndpointRef myEp);
static void EventHandler(void* contextPtr, OTEventCode event, OTResult result, void* cookie);
static Boolean IsOSSupported(void);

enum {
    kOTActive = 0x01,
    kOTEndpointCreated = 0x02,
    kOTEndpointBound = 0x04,
    kOTLastFlag = 0x8000
};

enum {
    kInListenLoop = 0
};	// atomic bits for gBitFlags


Boolean			gQuitFlag;	/* global */
EndpointRef		gEp;
EndpointRef		gHandoffEp;
UInt32			gStatus = 0;
OTNotifyUPP		gOTNotifier = NULL;
TCall			gCall;
struct InetAddress 	gRcvsin;
struct InetAddress 	gServerAddr;
UInt8			gBitFlags = 0;
InetInterfaceInfo	gInetInfo;


int main(int argc, char *argv[])
{
    OSStatus	status;    

    // This simply launches the 'Console.app' to show the output from the printf statements used below.
    LSOpenCFURLRef(CFURLCreateWithString(NULL, CFSTR("/var/tmp/console.log"), NULL), NULL);

    if (IsOSSupported() == false)	// check if a supported version of the OS is present.
	{
		printf("You must have either Mac OS X 10.0.x or Mac OS X 10.1.5 or greater present for this sample to work \n");
        return 0;
	}
        
    Initialize();
    MakeWindow();
    MakeMenu();
    status = StartServer();
    if (status == kOTNoError)
    {
        EventLoop();
        StopServer();
    }
    return 0;
}
 
void Initialize()	/* Initialize the cursor and set up AppleEvent quit handler */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
    
    err = AEInstallEventHandler( kPassServerInfoClass, kPassServerInfoEvent, NewAEEventHandlerUPP((AEEventHandlerProcPtr)PassServerInfoEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();    
        
}

static OSErr PassServerInfoEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    OSErr	err = noErr;
    DescType	typeCode;
    UInt32	dummyLong;
    Size	size;
    
    err = AEGetParamPtr(appleEvt, keyDirectObject, typeLongInteger, 
                        &typeCode, &dummyLong, sizeof(dummyLong), &size);
    if (err == noErr)
    {
        if ((gStatus & kOTEndpointBound) == 0)
        {
            // if the server is not ready, then return an event not handled error
            err = errAEEventNotHandled;
            printf("LocalServer is not ready yet - returning errAEEventNotHandled\n");
        }
        else
        {
            // note that we realy don't care about what the caller is sending us, except to
            // verify that the correct call was made.  We could just look at the reply structure
            // and return the desired information
            
            err = AEPutParamPtr(reply, keyDirectObject, kPassServerType, &gServerAddr, 
                                sizeof(struct InetAddress));
            if (err != noErr)
            {
                printf("PassServerInfoEventHandler::AEPutParamPtr returned error %d\n", err);
            }
        }
    }
    else
        printf("PassServerInfoEventHandler::AEGetParamPtr returned error %d\n", err);
                        
    return err;
}

static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}

void ShowEndpointState(EndpointRef ep, OTResult	state)
{
    switch (state)
    {
        case T_UNBND:
            printf("endpoint in UNBOUND state\n");
            break;
        
        case T_IDLE:
            printf("endpoint in IDLE state\n");
            break;
        
        case T_DATAXFER:
            printf("endpoint in DATAXFER state\n");
            break;
        
    }
}

void EventLoop()
{
    Boolean	gotEvent;
    EventRecord	event;
    OTResult	state1 = 0, state2 = 0;
     
    gQuitFlag = false;
    do
    {
        gotEvent = WaitNextEvent(everyEvent, &event, kSleepTime, nil);

        if (gotEvent)
            DoEvent(&event);
            
        state2 = OTGetEndpointState(gHandoffEp);
        if (state1 != state2)
        {
            state1 = state2;
            ShowEndpointState(gHandoffEp, state2);
        }
    } while (!gQuitFlag);
    
}

void MakeWindow()	/* Put up a window */
{
    Rect	wRect;
    WindowRef	myWindow;
    
    SetRect(&wRect,50,50,600,200); /* left, top, right, bottom */
    myWindow = NewCWindow(nil, &wRect, "\pHello", true, zoomNoGrow, (WindowRef) -1, true, 0);
    
    if (myWindow != nil)
        SetPort(GetWindowPort(myWindow));  /* set port to new window */
    else
	ExitToShell();					
}

void MakeMenu()		/* Put up a menu */
{
    Handle	menuBar;
    MenuRef	menu;
    long	response;
    OSErr	err;
	
    menuBar = GetNewMBar(rMenuBar);	/* read menus into menu bar */
    if ( menuBar != nil )
    {
        SetMenuBar(menuBar);	/* install menus */
        
        err = Gestalt(gestaltMenuMgrAttr, &response);
	if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
        {
            menu = GetMenuHandle( mFile );
            DeleteMenuItem( menu, iQuit );
            DeleteMenuItem( menu, iQuitSeparator );
        }
        
        DrawMenuBar();
    }
    else
    	ExitToShell();
}

void DoEvent(EventRecord *event)
{
    short	part;
    Boolean	hit;
    char	key;
    Rect	tempRect;
    WindowRef	whichWindow;
        
    switch (event->what) 
    {
        case mouseDown:
            part = FindWindow(event->where, &whichWindow);
            switch (part)
            {
                case inMenuBar:  /* process a moused menu command */
                    DoMenuCommand(MenuSelect(event->where));
                    break;
                    
                case inSysWindow:
                    break;
                
                case inContent:
                    if (whichWindow != FrontWindow()) 
                        SelectWindow(whichWindow);
                    break;
                
                case inDrag:	/* pass screenBits.bounds */
                    GetRegionBounds(GetGrayRgn(), &tempRect);
                    DragWindow(whichWindow, event->where, &tempRect);
                    break;
                    
                case inGrow:
                    break;
                    
                case inGoAway:
                    if (TrackGoAway(whichWindow, event->where))
                      DisposeWindow(whichWindow);
                    break;
                    
                case inZoomIn:
                case inZoomOut:
                    hit = TrackBox(whichWindow, event->where, part);
                    if (hit) 
                    {
                        SetPort(GetWindowPort(whichWindow));   // window must be current port
                        EraseRect(GetWindowPortBounds(whichWindow, &tempRect));   // inval/erase because of ZoomWindow bug
                        ZoomWindow(whichWindow, part, true);
                        InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &tempRect));	
                    }
                    break;
                }
                break;
		
                case keyDown:
		case autoKey:
                    key = event->message & charCodeMask;
                    if (event->modifiers & cmdKey)
                        if (event->what == keyDown)
                            DoMenuCommand(MenuKey(key));
		case activateEvt:	       /* if you needed to do something special */
                    break;
                    
                case updateEvt:
			DrawWindow((WindowRef) event->message);
			break;
                        
                case kHighLevelEvent:
			AEProcessAppleEvent( event );
			break;
		
                case diskEvt:
			break;
	}
}

void DoMenuCommand(long menuResult)
{
    short	menuID;		/* the resource ID of the selected menu */
    short	menuItem;	/* the item number of the selected menu */
	
    menuID = HiWord(menuResult);    /* use macros to get item & menu number */
    menuItem = LoWord(menuResult);
	
    switch (menuID) 
    {
        case mApple:
            switch (menuItem) 
            {
                case iAbout:
                    DoAboutBox();
                    break;
                    
                case iQuit:
                    ExitToShell();
                    break;
				
                default:
                    break;
            }
            break;
        
        case mFile:
            break;
		
        case mEdit:
            break;
    }
    HiliteMenu(0);	/* unhighlight what MenuSelect (or MenuKey) hilited */
}

void DrawWindow(WindowRef window)
{
    Rect		tempRect;
    GrafPtr		curPort;
	
    GetPort(&curPort);
    SetPort(GetWindowPort(window));
    BeginUpdate(window);
    EraseRect(GetWindowPortBounds(window, &tempRect));
    DrawControls(window);
    DrawGrowIcon(window);
    EndUpdate(window);
    SetPort(curPort);
}

void DoAboutBox(void)
{
   (void) Alert(kAboutBox, nil);  // simple alert dialog box
}

void StopServer(void)
{
        // make sure that both endpoints are in synchronous mode
    if ((gStatus & kOTEndpointCreated) != 0)
    {
        gStatus &= (-1 ^ kOTEndpointBound);
        OTSetSynchronous(gEp);
        OTUnbind(gEp);
        OTCloseProvider(gEp);
        gStatus &= (-1 ^ kOTEndpointCreated);
    }

    if (gOTNotifier)
    {
        DisposeOTNotifyUPP(gOTNotifier);
        gOTNotifier = NULL;
    }

    if ((gStatus & kOTActive) != 0)
    {
        CloseOpenTransportInContext(NULL);
        gStatus &= (-1 ^ kOTActive);
    }
}

OSStatus DoBindEndpoint(void)
{
    OSStatus 		status;
    struct InetAddress 	reqAddr;
    TBind		req, ret;
    char		inetStr[32];

    if ((gStatus & kOTEndpointBound) != 0)
    {
        // first have to unbind the endpoint
        // make sure that the endpoint is in synchronous mode
        OTSetSynchronous(gEp);
        
        status = OTUnbind(gEp);
        if (status != kOTNoError)
        {
            printf("Error unbinding the endpoint - %ld\n", status);
            return status;
        }
        else
            gStatus &= (-1 ^ kOTEndpointBound);
        
    }
        // get the primary interface inet info
    status = OTInetGetInterfaceInfo(&gInetInfo, kDefaultInetInterface);
    if (status != kOTNoError)
    {
        printf("OTInetGetInterfaceInfo failed with error %ld.\n", status);
        return status;
    }
    
    reqAddr.fAddressType = AF_INET;
    reqAddr.fPort = 0;	// have OT assign us a port
    reqAddr.fHost = gInetInfo.fAddress;
    memset(&req, 0, sizeof(TBind));
    req.addr.len = sizeof (struct InetAddress);
    req.addr.buf = (UInt8*)&reqAddr;


        // specify a qlen value of 1 so that we can receiving incoming requests
    req.qlen = 1;

    memset(&gServerAddr, 0, sizeof(struct InetAddress));
    memset(&ret, 0, sizeof(TBind));
    ret.addr.maxlen = sizeof(struct InetAddress);
    ret.addr.buf = (unsigned char *) &gServerAddr;

        // bind the endpoint - we don't care what the system port we bind to, just so we know what the address
        // and port are.
    status = OTBind(gEp, &req, &ret);

    if (status != kOTNoError)
    {
        printf("OTBind failed with error %ld.\n", status);
        return status;
    }
    else
    {
        OTInetHostToString(gServerAddr.fHost, inetStr);
        printf("Endpoint bound to address %s, port %d.\n", inetStr, gServerAddr.fPort); 
    }
    
    gStatus |= kOTEndpointBound;
    
    return status;
}

OSStatus StartServer(void)
{
    OSStatus		status;
    
    status = InitOpenTransportInContext(kInitOTForApplicationMask, NULL);
    if (status != kOTNoError)
    {
        printf("InitOpenTransportInContext failed with error %ld.\n", status);
        return status;
    }
        // indicate that we've opened OT successfully
    gStatus |= kOTActive;

    gEp = OTOpenEndpointInContext(OTCreateConfiguration(kUDPName), 0, NULL, &status, NULL);
    if ((gEp == NULL) || (status != kOTNoError))
    {
        printf("OTOpenEndpointInContext failed with error %ld.\n", status);
        return status;
    }
    
        // indicate that we've created the synchronous endpoint
    gStatus |= kOTEndpointCreated;
    
    status = DoBindEndpoint();
    if (status != kOTNoError)
        return status;    
    
        // set up the notifier for the main server endpoint so that we can go into asynchronous operations
    gOTNotifier = NewOTNotifyUPP(EventHandler);
    if (gOTNotifier == NULL)
    {
        printf("NewOTNotifyUPP failed .\n");
        return -1;
    }
        
    status = OTInstallNotifier(gEp, gOTNotifier, &gEp);
    if (status != kOTNoError)
    {
        printf("OTInstallNotifier failed with error %ld.\n", status);
        return status;
    }
    
        // set the endpoint into async mode
    status = OTSetAsynchronous(gEp);
    if (status != kOTNoError)
    {
        printf("OTSetAsynchronous failed with error %ld.\n", status);
        return status;
    }
        
    return status;
}

OSStatus HandleOTLookErr(EndpointRef myEp)
{
    OSStatus		err = kOTNoError;
    OTResult		result;
    
    result = OTLook(myEp);
    
    switch (result)
    {
        
        default:
                    // The use of this function in this instance is to handle
                    // T_DISCONNECT events that can happen.  This call will
                    // need to be customized for other use, like handling 
                    // T_DATA events which is not implemented in this sample.
                    // Pass a debugStr call to show what event was not handled here
            printf("HandleOTLookErr passed unprocessed event\n");
            break;
        
    }
    
    return err;
}

void EventHandler(void* contextPtr, OTEventCode event, OTResult result, void* cookie)
{
    EndpointRef	*ep = (EndpointRef *) contextPtr;
    
    switch (event)
    {
            
        case T_DATA:		/* Standard data is available			*/
                                // check that we are in the DATAXFER state
            printf("T_DATA event occurred\n");
            DoReceiveData(*ep);
            break;

        case T_UNBINDCOMPLETE:	                                                                                                                                                         	    printf("T_UNBINDCOMPLETE event occurred\n");
            if (*ep == gEp)
			{
                gStatus &= (-1 ^ kOTEndpointBound);
			}
            break;

        default:
            printf("EventHandler got unexpected event %ld\n", event);
            break;
    }
    return;
}

Boolean ReceiveOnePacket(EndpointRef myEp)
{
	TUnitData			unitdata;
	struct InetAddress  source;
	OTResult 			result, bytesSent;
	UInt32				flag;
	Boolean				done = false;
	UInt8				buffer[2048];
	UInt16				i;
			
	unitdata.addr.maxlen = sizeof(struct InetAddress);
	unitdata.opt.maxlen = 0;
	unitdata.opt.buf = 0;
	unitdata.udata.maxlen = sizeof(buffer);
	unitdata.udata.buf = buffer;
	unitdata.addr.buf = (UInt8*) &source;
	result = OTRcvUData(myEp, &unitdata, &flag);
	if (result < 0)
	{
            if (result == kOTLookErr)
            {
                HandleOTLookErr(myEp);
                done = TRUE;
                    
            }
            else if (result == kOTOutStateErr)
            {
                printf("OTRcv returned kOTOutStateErr\n");
                // client could have issued a disconnect call while this call was about to
                // to be processed
                // need to handle out of state error
                done = TRUE;
            }
            
            else if (result == kOTNoDataErr)
            {
                done = TRUE;
            }
            else 
            {
                printf("error occurred on OTRcvUData call %ld\n", result);
            }
	}
	else if (result == kOTNoError)
	{
		// data recevied
		// verify that the sender is from this system
		if (gInetInfo.fAddress == source.fHost)
		{
			// show the bytes which were received.
			printf("%ld bytes received -  ", unitdata.udata.len);
			for (i = 0; i < unitdata.udata.len; i++)
				printf("%x ", buffer[i]);
			printf("\n");

			result = OTSndUData(myEp, &unitdata);
			// indicate how many bytes were received.
			if (result < noErr)
			{
				printf("error occurred on OTSnd call %ld\n", bytesSent);
			}
			else
			{
				printf("sent %ld bytes to sender\n", unitdata.udata.len);
			}
		}
		else
		{
			// do nothing as the incoming packet is from a different system
		}
	}
	
	return done;
		
}

void DoReceiveData(EndpointRef myEp)
{
    Boolean		done = false;
    
    while (done == false)
    {
        done = ReceiveOnePacket(myEp);
            
    }
}

#define kMACOSX1010 0x1010
#define kMACOSX1014	0x1014

/*
    This server sample will not communicate properly with a client running under Classic on the same
    system if Mac OS 10.1 thru 10.1.4 is in use.  This results from a problem with the IP Sharing module
    which is fixed in the 10.1.5 release.  The server works under 10.0.x as the problem is not present in these
    earlier releases.
    
    Note that for Mac OS X clients, there is no problem with communicating with the OS X server, however as this
    sample is to demonstrate a technique for communications across the OS X and Classic boundary, the 
    following function is used.
    
*/

Boolean IsOSSupported(void)
{
    long	response;
    OSErr	err;
    Boolean	result = false;
    
    err = Gestalt(gestaltSystemVersion, &response);
    if (err)
    {
        printf("Gestalt call failed to determine system version, error %d\n", err);
    }
    else if (response >= kMACOSX1010 && response <= kMACOSX1014)
    {
        printf("An unsupported version of Mac OS X was detected - quitting server\n");
    }
    else
        result = true;
        
    return result;
}
