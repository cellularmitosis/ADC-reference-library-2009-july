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
static void DoUnbindHandoffEp(void);
static OSStatus DoRcvDisconnect(EndpointRef myEp);
static OSStatus DoRcvOrderlyDisconnect(EndpointRef myEp);
static OSStatus HandleOTLookErr(EndpointRef myEp);
static void DoListenAccept(void);
static void EventHandler(void* contextPtr, OTEventCode event, OTResult result, void* cookie);
static OTResult SetFourByteOption(EndpointRef ep, OTXTILevel level, OTXTIName  name, UInt32   value);
static Boolean IsOSSupported(void);

enum {
    kOTActive = 0x01,
    kOTEndpointCreated = 0x02,
    kOTEndpointBound = 0x04,
    kOTEndpointListening = 0x08,
    kOTEndpointServing = 0x10,
    kOTHandoffEndpointCreated = 0x20,
    kOTHandoffEndpointBound = 0x40,
    kRebindEndpoint = 0x1000,
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
        if ((gStatus & kOTEndpointListening) == 0)
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
        
        case T_OUTCON:
            printf("endpoint in OUTCON state\n");
            break;
        
        case T_INCON:
            printf("endpoint in INCON state\n");
            break;
        
        case T_DATAXFER:
            printf("endpoint in DATAXFER state\n");
            break;
        
        case T_OUTREL:
            printf("endpoint in OUTREL state\n");
            break;
        
        case T_INREL:
            printf("endpoint in INREL state\n");
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
        gStatus &= (-1 ^ kOTEndpointListening);
        OTSetSynchronous(gEp);
        OTUnbind(gEp);
        gStatus &= (-1 ^ kOTEndpointBound);
        OTCloseProvider(gEp);
        gStatus &= (-1 ^ kOTEndpointCreated);
    }

    if ((gStatus & kOTHandoffEndpointCreated) != 0)
    {
        OTSetSynchronous(gHandoffEp);
        if (OTGetEndpointState(gHandoffEp) >= T_IDLE)
        {
            OTUnbind(gHandoffEp);
            printf("Handoff ep unbound.\n");
        }
        gStatus &= (-1 ^ kOTHandoffEndpointBound);
        OTCloseProvider(gHandoffEp);
        printf("Handoff ep closed.\n");
        gStatus &= (-1 ^ kOTHandoffEndpointCreated);
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
        gStatus |= kOTEndpointListening;       
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

    gEp = OTOpenEndpointInContext(OTCreateConfiguration(kTCPName), 0, NULL, &status, NULL);
    if ((gEp == NULL) || (status != kOTNoError))
    {
        printf("OTOpenEndpointInContext failed with error %ld.\n", status);
        return status;
    }
    
        // indicate that we've created the synchronous endpoint
    gStatus |= kOTEndpointCreated;

    gHandoffEp = OTOpenEndpointInContext(OTCreateConfiguration(kTCPName), 0, NULL, &status, NULL);
    if ((gHandoffEp == NULL) || (status != kOTNoError))
    {
        printf("OTOpenEndpointInContext on handoff ep failed with error %ld.\n", status);
        return status;
    }
        // set the NO_DELAY option for the handoff endpoint so that it can make small send requests
        // as fast as possible, otherwise the Nagle algorithm to delay sending multiple small
        // packets kicks in an delays sending these packets for many milliseconds.
    status = SetFourByteOption(gHandoffEp, INET_TCP, TCP_NODELAY, 1);
    if (status)
        printf("SetFourByteOption returned error %ld on gHandoffEp\n", status);

        // indicate that we've created the synchronous endpoint
    gStatus |= kOTHandoffEndpointCreated;
    
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

        // set up the notifier for the handoff endpoint so that we can go into asynchronous operations
    status = OTInstallNotifier(gHandoffEp, gOTNotifier, &gHandoffEp);
    if (status != kOTNoError)
    {
        printf("OTInstallNotifier for handoff ep failed with error %ld.\n", status);
        return status;
    }
    
        // set the endpoint into async mode
    status = OTSetAsynchronous(gEp);
    if (status != kOTNoError)
    {
        printf("OTSetAsynchronous failed with error %ld.\n", status);
        return status;
    }
    
        // set the handoff endpoint into async mode
    status = OTSetAsynchronous(gHandoffEp);
    if (status != kOTNoError)
    {
        printf("OTSetAsynchronous on handoff ep failed with error %ld.\n", status);
        return status;
    }
    
    return status;
}

void DoUnbindHandoffEp(void)
{
    if (OTGetEndpointState(gHandoffEp) >= T_IDLE)
    {
        OTUnbind(gHandoffEp);
    }
}

OSStatus DoRcvDisconnect(EndpointRef myEp)
{
    OSStatus		err = kOTNoError;
    
    err = OTRcvDisconnect(myEp, NULL);
    if (err != kOTNoError)
    {
        printf( "error occured on OTRcvDisconnect %ld\n", err);
    }
    gStatus &= (-1 ^ kOTEndpointServing);
    
    DoUnbindHandoffEp();

    return err;
}

OSStatus DoRcvOrderlyDisconnect(EndpointRef myEp)
{
    OSStatus		err;
    
    err = OTRcvOrderlyDisconnect(myEp);
    if (err < kOTNoError)
    {
        printf("error occured on OTRcvOrderlyDisconnect %ld\n", err);
    }
    else if (err > 0)
    {			
        printf("OTRcvOrderlyDisconnect returned positive result %ld\n", err);
    }
    else
    {
        // since we have no data to send, then xomplete the OrderlyDisconnect sequence
        err = OTSndOrderlyDisconnect(myEp);
        if (err < kOTNoError)
        {
            printf("error occured on OTSndOrderlyDisconnect from  DoRcvOrderlyDisconnect %ld\n", err);
        }
        else
        {
            printf("OTSndOrderlyDisconnect successful from  DoRcvOrderlyDisconnect\n");
        }
        gStatus &= (-1 ^ kOTEndpointServing);
        
        // unbind the handoff endpoint
        DoUnbindHandoffEp();
    }
        
    return err;
}

OSStatus HandleOTLookErr(EndpointRef myEp)
{
    OSStatus		err = kOTNoError;
    OTResult		result;
    
    result = OTLook(myEp);
    
    switch (result)
    {
        case T_DISCONNECT:
            printf("Handling T_DISCONNECT from HandleOTLookErr\n");
            err = DoRcvDisconnect(myEp);
            if (err != kOTNoError)
            {
                printf("Error calling DoRcvDisconnect from HandleOTLookErr %ld\n", err);
            }
            break;
        
        case T_ORDREL:
            printf("Handling T_ORDREL from HandleOTLookErr\n");
            err = DoRcvOrderlyDisconnect(myEp);

            if (err != kOTNoError)
            {
                printf("Error calling DoRcvOrderlyDisconnect from HandleOTLookErr %ld\n", err);
            }
            break;
        
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

void DoListenAccept(void)
{
    OSStatus		err;
    Boolean		done = false;
    
            // check we have already entered DoListenAccept from the main event loop and are 
            // trying to do so again from the secondary interrupt
    if (OTAtomicSetBit(&gBitFlags, kInListenLoop))
            return;	// if the bit was previous set, then we already have entered, but not exitted
                            // this proc from the main event loop

         // prepare for the listen call
    memset(&gCall, 0, sizeof(TCall));
    memset(&gRcvsin, 0, sizeof(struct InetAddress));

    gCall.addr.maxlen = sizeof(struct InetAddress);
    gCall.addr.len = sizeof(struct InetAddress);
    gCall.addr.buf = (unsigned char *) &gRcvsin;
    gCall.opt.maxlen = 0;
    gCall.opt.buf = 0;
    gCall.udata.maxlen = 0;

    err = OTListen(gEp, &gCall);
    if (err != kOTNoError)
    {
        printf("OTListen failed with error %ld.\n", err);
        return;
    }
        
    if (err == kOTNoDataErr)
    {
        printf("kOTNoDataErr returned by OTListen\n");
            // don't need to do anything
    }
    else if (err == kOTLookErr)
    {
            // step 2 for handling incoming connection requests
            // handle disconnect indications for a pending connection request
            // the only look indication allowed on an OTListen call is
            // T_DISCONNECT
        HandleOTLookErr(gEp);
        done = true;
                
    }
    else if (err != kOTNoError) // there was an unknown error in doing the listen
    {
        printf("error occured on OTListen %ld\n", err);
        done = true;

    }
    
    if (done == false)
    {
        if ((gStatus & kOTEndpointServing) != 0) // check whether we already have an active connection
        {
            printf("cannot accept a second client - sending disconnect");
            OTSndDisconnect(gEp, &gCall);
            done = true;
        }
        else
        {
            // verify that the request is coming from our own system
            // this example only supports connection requests from the same system
            if (gInetInfo.fAddress != gRcvsin.fHost)
            {
                printf("cannot accept a client froma foreign machine - sending disconnect");
                OTSndDisconnect(gEp, &gCall);
                done = true;
            }
        }
    }

    if (done == false)
    {
            
            // make the accept call and handoff the connection to the handoff endpoint
        err = OTAccept(gEp, gHandoffEp, &gCall);

        if (err == kOTLookErr)
        {
            HandleOTLookErr(gEp);
        }
        else if (err != kOTNoError)
        {
            printf("error occured on OTAccept %ld\n", err);
        }
    }
	

    OTAtomicClearBit(&gBitFlags, kInListenLoop);	// clear the bit that indicates we're in this loop
	
}

void EventHandler(void* contextPtr, OTEventCode event, OTResult result, void* cookie)
{
    EndpointRef	*ep = (EndpointRef *) contextPtr;
    
    switch (event)
    {
        case T_LISTEN:
            // this sample takes a shortcut that there will only be a single client
            // trying to connect with this server.  
            DoListenAccept();
            break;
            
        case T_DATA:		/* Standard data is available			*/
                                // check that we are in the DATAXFER state
            printf("T_DATA event occurred\n");
            DoReceiveData(*ep);
            break;

        case T_ORDREL:				/* An orderly release is available		*/
            printf("T_ORDREL event occurred\n");
            DoRcvOrderlyDisconnect(*ep);
            break;

        case T_DISCONNECT:	/* A disconnect is available			*/
            printf("T_DISCONNECT event occurred\n");
            DoRcvDisconnect(*ep);
            break;

        case T_ACCEPTCOMPLETE:		/* Accept call is complete				*/
            printf("T_ACCEPTCOMPLETE event occurred\n");
            
            if (result != kOTNoError)
            {
                printf("Error occured with T_ACCEPTCOMPLETE event\n");
                if (result == kOTLookErr )
                {
                    HandleOTLookErr(*ep);
                }
            }
            else
                gStatus |= kOTEndpointServing; 
            break;
        
        case T_PASSCON:
	    printf("T_PASSCON event occurred\n");
            break;
        

        case T_DISCONNECTCOMPLETE:	/* Disconnect call is complete			*/
	    printf("T_DISCONNECTCOMPLETE event occurred\n");
            gStatus &= (-1 ^ kOTEndpointServing);
            break;
        
        case T_UNBINDCOMPLETE:	                                                                                                                                                         	    printf("T_UNBINDCOMPLETE event occurred\n");
            if (*ep == gEp)
                gStatus &= (-1 ^ kOTEndpointBound);
            else
                gStatus &= (-1 ^ kOTHandoffEndpointBound);
            break;

        default:
            printf("EventHandler got unexpected event %ld\n", event);
            break;
    }
    return;
}

Boolean ReceiveOnePacket(EndpointRef myEp)
{
	OTResult 	result, bytesSent;
        UInt32		flag;
	Boolean		done = false;
        UInt8		buffer[2048];
        UInt16		i;
					
	result = OTRcv(myEp, &buffer, sizeof(buffer), &flag);
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
                printf("error occurred on OTRcv call %ld\n", result);
            }
	}
	else if (result == 0)
	{
            // no data was in the packet - but we can't return false just yet
            // if we were supporting EOM (End of Message) option we would check the 
            // flag for T_MORE bit set.
            if ((flag & T_MORE) == 0)
            {
                // there is no more data on the way
                // you still need to make one more OTRcv call to clear the 
                // data indication within OT
            }
            else
            {
                // there is more data coming
            }
 	}
	else
	{
            // data recevied
            bytesSent = OTSnd(myEp, &buffer, result, 0);
            // indicate how many bytes were received.
            printf("%ld bytes received -  ", result);
            for (i = 0; i < result; i++)
                printf("%x ", buffer[i]);
            printf("\n");
            if (bytesSent < noErr)
            {
                printf("error occurred on OTSnd call %ld\n", bytesSent);
            }
            else
            {
                printf("returned %ld bytes to sender\n", bytesSent);
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

OTResult SetFourByteOption(EndpointRef ep,
                           OTXTILevel level,
                           OTXTIName  name,
                           UInt32   value)
{
   OTResult err;
   TOption  option;
   TOptMgmt request;
   TOptMgmt result;
   
   /* Set up the option buffer to specify the option and value to
         set. */
   option.len  = kOTFourByteOptionSize;
   option.level= level;
   option.name = name;
   option.status = 0;
   option.value[0] = value;

   /* Set up request parameter for OTOptionManagement */
   request.opt.buf= (UInt8 *) &option;
   request.opt.len= sizeof(option);
   request.flags  = T_NEGOTIATE;

   /* Set up reply parameter for OTOptionManagement. */
   result.opt.buf  = (UInt8 *) &option;
   result.opt.maxlen  = sizeof(option);

   
   err = OTOptionManagement(ep, &request, &result);

   if (err == noErr) {
      if (option.status != T_SUCCESS) 
         err = option.status;
   }
            
   return (err);
}

#define kMACOSX1010 	0x1010
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
