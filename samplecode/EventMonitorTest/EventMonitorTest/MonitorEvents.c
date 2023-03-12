/*
 *  MonitorEvents.c
 *  EventMonitorTest

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved

 *  Created 10/28/05.
 *
 */

#include "MonitorEvents.h"

static EventHandlerRef		sHandler, fHandler;
static Boolean				gApplicationActive = TRUE;
static Boolean				gProcessForegroundEvents = FALSE;

/*
	MonitorHandler - called whenever the Event monitor target detects a registered event happens.
	determine the type of event and send a message to the system log as to the event type
 */
static OSStatus MonitorHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	char* className = NULL;
	char* kindName = NULL;
	Point	pt;
	UInt32	ch;
	
	// get the event class
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassKeyboard:
			className = "Keyboard";
			// get the keyboard event type
			switch ( GetEventKind( inEvent ) )
			{
				case kEventRawKeyDown:
					kindName = "KeyDown";
					GetEventParameter(inEvent, kEventParamKeyCode, 
									  typeUInt32, NULL, sizeof(UInt32), NULL, &ch);
					fprintf(stderr, "keycode - %d - ", (int)ch);
					break;
					
				case kEventRawKeyUp:
					kindName = "KeyUp";
					GetEventParameter(inEvent, kEventParamKeyCode, 
									  typeUInt32, NULL, sizeof(UInt32), NULL, &ch);
					fprintf(stderr, "keycode - %d - ", (int)ch);
					break;
					
				case kEventRawKeyRepeat:
					kindName = "KeyRepeat";
					GetEventParameter(inEvent, kEventParamKeyCode, 
									  typeUInt32, NULL, sizeof(UInt32), NULL, &ch);
					fprintf(stderr, "keycode - %d - ", (int)ch);
					break;
					
				case kEventRawKeyModifiersChanged:
					GetEventParameter(inEvent, kEventParamKeyModifiers, 
									  typeUInt32, NULL, sizeof(UInt32), NULL, &ch);
					fprintf(stderr, "0x%x - ", (unsigned int)ch);
					kindName = "ModifiersChanged";
					break;
					
				default:
					break;
			}
				break;
			
		case kEventClassMouse:
			className = "Mouse";
			// get the type or mouse event
			switch ( GetEventKind( inEvent ) )
			{
				case kEventMouseDown:
					kindName = "MouseDown";
					break;
					
				case kEventMouseUp:
					kindName = "MouseUp";
					break;
					
				case kEventMouseMoved:
					kindName = "MouseMoved";
					GetEventParameter(inEvent, kEventParamMouseLocation, 
									  typeQDPoint, NULL, sizeof(Point), NULL, &pt);
					fprintf(stderr, "location - %d, %d - ", pt.v, pt.h);
					break;
					
				case kEventMouseDragged:
					kindName = "MouseDragged";
					break;
					
				case kEventMouseWheelMoved:
					kindName = "MouseWheel";
					break;
					
				default:
					break;
			}
				break;
			
		case kEventClassTablet:
			className = "Tablet";
			// get the kind of tablet event
			switch ( GetEventKind( inEvent ) )
			{
				case kEventTabletPoint:
					kindName = "TabletPoint";
					break;
					
				case kEventTabletProximity:
					kindName = "TabletProximity";
					break;
					
				default:
					break;
			}
				break;
			
		default:
			break;
	}
	
	fprintf( stderr, "event class = %s, kind = %s\n",
			 className != NULL ? className : "<unknown>",
			 kindName != NULL ? kindName : "<unknown>" );
			 
	return noErr;
}

/*
	CmdHandler was originall setup to process mouse clicks in the Main Window which presents the user with
	different event types to be tracked by the process. As each item is selected/unselected a call
	to AddEventTypesToHandler/RemoveEventTypesFromHandler to process the event.
	This function has since been updated to check whether to process foreground keyboard and mouse
	events in the same way that the events are processed while the application is in the 
	background.
*/
static OSStatus CmdHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	HICommandExtended	cmd;
	EventTypeSpec*		eventList = NULL;
	OSStatus			err;
	UInt32				class;
	
	class = GetEventClass( inEvent );
	
	if (class != kEventClassCommand) 
	{
		// if this is not an EventClassCommand, then it could be a mouse or keyboard event
		// see if we should log the event.
		if ((gApplicationActive == TRUE) && (gProcessForegroundEvents == TRUE))
		{
			err = MonitorHandler(inCaller, inEvent, inRefcon);
			return err;
		}
		return noErr;
	}
	
	// log the event ID information - which button on the selection dialog did the user click on
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );
	fprintf(stderr, "CmdHandler called - cmd.commandID is %c%c%c%c\n", (char)(((UInt32)cmd.commandID&0xff000000)>>24),
																	   (char)(((UInt32)cmd.commandID&0xff0000)>>16),
																	   (char)(((UInt32)cmd.commandID&0xff00)>>8),
																	   (char)((UInt32)cmd.commandID&0xff));
	switch ( cmd.commandID )
	{
		case kCmdKeyDown:
		{
			// KeyDown radio button setting toggled
			EventTypeSpec kEvent = { kEventClassKeyboard, kEventRawKeyDown };
			eventList = &kEvent;
			break;
		}
			
		case kCmdKeyUp:
		{
			// KeyUp radio button setting toggled
			EventTypeSpec kEvent = { kEventClassKeyboard, kEventRawKeyUp };
			eventList = &kEvent;
			break;
		}
			
		case kCmdKeyRepeat:
		{
			// KeyRepeat radio button setting toggled
			EventTypeSpec kEvent = { kEventClassKeyboard, kEventRawKeyRepeat };
			eventList = &kEvent;
			break;
		}
			
		case kCmdKeyModifiersChanged:
		{
			// ModifiersChanged radio button setting toggled
			EventTypeSpec kEvent = { kEventClassKeyboard, kEventRawKeyModifiersChanged };
			eventList = &kEvent;
			break;
		}
			
		case kCmdMouseDown:
		{
			// MouseDown radio button setting toggled
			EventTypeSpec kEvent = { kEventClassMouse, kEventMouseDown };
			eventList = &kEvent;
			break;
		}
			
		case kCmdMouseUp:
		{
			// MouseUp radio button setting toggled
			EventTypeSpec kEvent = { kEventClassMouse, kEventMouseUp };
			eventList = &kEvent;
			break;
		}
			
		case kCmdMouseMoved:
		{
			// MouseMoved radio button setting toggled
			EventTypeSpec kEvent = { kEventClassMouse, kEventMouseMoved };
			eventList = &kEvent;
			break;
		}
			
		case kCmdMouseDragged:
		{
			// MouseDragged radio button setting toggled
			EventTypeSpec kEvent = { kEventClassMouse, kEventMouseDragged };
			eventList = &kEvent;
			break;
		}
			
		case kCmdMouseWheel:
		{
			// MouseWheel radio button setting toggled
			EventTypeSpec kEvent = { kEventClassMouse, kEventMouseWheelMoved };
			eventList = &kEvent;
			break;
		}
		
		case kCmdTabletPoint:
		{
			// TabletPoint radio button setting toggled
			EventTypeSpec kEvent = { kEventClassTablet, kEventTabletPoint };
			eventList = &kEvent;
			break;
		}
		
		case kCmdTabletProximity:
		{
			// TabletProximity radio button setting toggled
			EventTypeSpec kEvent = { kEventClassTablet, kEventTabletProximity };
			eventList = &kEvent;
			break;
		}
			
		case kCmdForeground:
		{
			// The "process foreground events" radio button was toggled.
			// get the current setting of the control
			eventList = NULL;
			if ( GetControlValue( cmd.source.control ) != 0 )
				gProcessForegroundEvents = TRUE;
			else
				gProcessForegroundEvents = FALSE;
			break;
		}
		
		default:
			break;
	}
	
	if ( eventList != NULL )
	{
		// get the current setting of the control
		if ( GetControlValue( cmd.source.control ) != 0 )
		{
			OSStatus	err1;
			// control was toggled on so add event type to both handler lists - the one for the 
			// EventMonitor target as well as for the main event handler
			err = AddEventTypesToHandler( sHandler, 1, eventList );
			err1 = AddEventTypesToHandler( fHandler, 1, eventList );
			if ((err) || (err1))
			{
				fprintf(stderr, "error adding event type - error was %d, %d\n", (int)err, (int)err1);
			}
			else
			{
				fprintf(stderr, "ok adding event type\n");
			}
		}
		else
		{
			// control was toggled off so remove event from both handler lists.
			RemoveEventTypesFromHandler( sHandler, 1, eventList );
			RemoveEventTypesFromHandler( fHandler, 1, eventList );			
		}
		return noErr;
	}
	else
	{
		return eventNotHandledErr;
	}
}

static OSStatus SuspendResumeHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	switch(GetEventKind(inEvent))
	{
		case kEventAppActivated:
			gApplicationActive = TRUE;
			fprintf(stderr, "Application is now frontmost\n");
			break;
		
		case kEventAppDeactivated:  
			gApplicationActive = FALSE;
			fprintf(stderr, "Application is now in the background\n");
			break;
	}
	return noErr;
}

OSStatus InstallMyEventHandlers(void)
{
	EventTypeSpec	kEvents[] =
		{
			// use an event that isn't monitored just so we have a valid EventTypeSpec to install
			{ kEventClassCommand, kEventCommandUpdateStatus }
		};
		
	EventTypeSpec	kCmdEvents[] =
		{
			{ kEventClassCommand, kEventCommandProcess }						
		};

EventTypeSpec kSuspendResumeEvents[]={{kEventClassApplication,kEventAppActivated},
	{kEventClassApplication,kEventAppDeactivated}};

	// install an event handler to detect clicks on the main window to tell the application which 
	// events to track/stop tracking
	InstallApplicationEventHandler( CmdHandler, GetEventTypeCount( kCmdEvents ), kCmdEvents, 0, &fHandler );
	
	// the magic happens here with the call to GetEventMonitorTarget as described in CarbonEvents.h
	// The event monitor target is a special event target used to monitor user input events across all processes.
	// When the monitor target detects a matching event, then MonitorHandler is called.
	InstallEventHandler( GetEventMonitorTarget(), MonitorHandler, GetEventTypeCount( kEvents ),
						 kEvents, 0, &sHandler );
	
    // handle suspend and resume events to detect when these transitions occur so that we know when this test 
	// process is in the foreground or not
    InstallApplicationEventHandler(NewEventHandlerUPP(SuspendResumeHandler), GetEventTypeCount( kSuspendResumeEvents ),
								   kSuspendResumeEvents, 0, NULL);
	return noErr;
}
