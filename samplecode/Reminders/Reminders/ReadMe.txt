Reminders illustrates how you can use a WebKit plug-in to extend the functionality of a Dashboard widget. It also shows how to share complex data from Objective-C to JavaScript, update and retrieve widget preferences, and call JavaScript from a WebKit plug-in and vice versa.

It is a Dashboard widget that displays upcoming iCal events and to do items occurring within the year. Users can see all events (starting now) and incomplete tasks ending before the end of the year without launching iCal. All iCal changes are instantly added to the widget. The widget shows the date and title of each event or to do item, the start and end time of each event, and the priority of each to do item. The "all-day" tag is used in lieu of start and end time for events lasting all day long. Users are informed when there are no available events, calendars, or to do items. 

Reminders displays data from multiple calendars at once. However, you can choose to restrict your widget to some specific calendars. It contains a menu bar that enables users to toggle between events and to do items. A set of checkboxes, in the back of the widget, allows users to choose which calendars should be seen in the widget. The widget saves and remembers the users' calendar preferences.  

Reminders uses the RemindersPlugIn plug-in to query the CalendarStore framework for iCal events, to do items, and calendars. RemindersPlugIn is a Cocoa bundle that provides data used by the widget through JavaScript callbacks; the JavaScript enabled widget can access specific Objective-C methods [in the plug-in] while the plug-in can call the widget's JavaScript functions. 


Using the sample 
1. Add some calendars, events, or to do items to iCal. For testing purposes,  
Reminders add a fictitious calendar with an event and to do item if your iCal is empty. 

2. Click the Reminders.wdgt to install the widget and open it in Dashboard.

3. Toggle between the "Events" and "To Do Items" buttons to view events and to 
do items. Click the info button to switch to the back of the widget. Set your calendar preferences by checking one or more calendars. Use the Done 
button to return to the front of the widget. 


Building the sample
This sample consists of a widget (Reminders) and plug-in (RemindersPlugIn). Thus, building the sample consists of implementing RemindersPlugIn in Xcode and creating Reminders in Dashcode.


Creating the plug-in RemindersPlugIn
In the steps below, we are going to create and set up RemindersPlugIn in Xcode, then add classes to it.

Setting up the Project
This section shows how to create and set up RemindersPlugIn. 

1. Create a new Project
RemindersPlugIn is a Cocoa bundle. 
Open Xcode, choose File > New Project and select Cocoa Bundle under Bundle in the New Project Assistant window. Click Next and save the project as RemindersPlugIn.


2. Add the WebKit and CalendarStore frameworks
Open the Frameworks and Libraries group in the Groups & Files list of the project window and then right-click or control-click the Linked Frameworks subgroup. 
Choose Add > Existing Frameworks from the pop-up menu and navigate to the 
/System/Library/Frameworks/ folder to select and add the WebKit.framework and CalendarStore.framework to the project.


3. Add the minimum system version Info.plist key
This project uses the CalendarStore framework that is only available in Mac OS X 10.5 and later. Set the LSMinimumSystemVersion to 10.5 so users can receive notices informing them about the minimum system requirement when they attempt to run the application on a system earlier than Mac OS X 10.5. 
Double-click the Info.plist file in the resource area of the project and copy and paste the following lines into the file. 
	<key>LSMinimumSystemVersion</key>
	<string>10.5</string>


4. Set the Target SDK to Mac OS X 10.5
Double-click the RemindersPlugIn project icon in the Groups & Files list  to open the project info window. Go to the General tab and set the "Cross-Develop Using Target SDK" to Mac OS X 10.5. 


5. Set the bundle identifier and principal class
The bundler identifier uniquely identifies your plug-in. The principal class identifies the main class used to define your plug-in's functionality; its name should be the same as the name of your project. 

Choose Project > Edit Active Target "RemindersPlugin" and click the Properties tab.
Fill out the identifier and principal class text fields with a bundle identifier of your choice and RemindersPlugin, respectively.

Read the Code Loading Programming Topics for Cocoa > Creating Loadable Bundles and Code Loading Programming Topics for Cocoa > Preventing Name Conflicts sections to learn more about bundle identifiers.


6. Set the Wrapper Extension.
Double-click the "RemindersPlugIn" target, select the Build pane, and search for the Wrapper Extension setting. Set the Wrapper Extension's value to widgetplugin. 



Adding Classes 
We just set up our Xcode project. In the steps below, we are going to add the 
NSColorHexadecimalValue, CalendarItems, and RemindersPlugIn classes to our project. These classes are needed for querying the CalendarStore framework, parsing and processing data, and exposing Objective-C methods and instance variables to JavaScript and vice versa. 


7. Create the NSColorHexadecimalValue's class files
Select File > New File and click Objective-C class under Cocoa in the New File Assistant window. Name the file NSColorHexadecimalValue. 


8. Create the NSColorHexadecimalValue class
NSColorHexadecimalValue is an Objective-C category that adds the "hexadecimalStringValue" method to the NSColor class. "hexadecimalStringValue" returns the hexadecimal value of a color.
The declaration of the NSColorHexadecimalValue interface (in the NSColorHexadecimalValue.h file) is shown below:

@interface NSColor(NSColorHexadecimalValue) 
-(NSString *)hexadecimalStringValue;

The NSColorHexadecimalValue.m file, which contains the category implementation, describes how to implement "hexadecimalStringValue."


9. Create the CalendarItemsÕs class files
Proceed as done in step 7 and name the file CalendarItems.
CalendarItems is an Objective-C class that stores specific information about a calendar (uid, title, color), event (start date, title, color, start and end time), and to do item (start date, title, color, start and end time).
See below for its interface declaration:

extern NSString * const kJSSelectorPrefix;
@interface CalendarItems : NSObject {
	NSString *js_uid;
	NSString *js_startDate;
	NSString *js_title;
	NSString *js_color;
	NSString *js_timeOrPriority;	
}

@property(copy, readwrite) NSString *js_uid;
@property(copy, readwrite) NSString *js_startDate;
@property(copy, readwrite) NSString *js_title;
@property(copy, readwrite) NSString *js_color;
@property(copy, readwrite) NSString *js_timeOrPriority;

-(id)initWithStartDate:(NSString *)date 
				 title:(NSString *)aTitle 
				 color:(NSString *)aColor 
		        timeOrPriority:(NSString *)aValue;

-(id)initWithUID:(NSString *)aUid title:(NSString *)aTitle color:(NSString *)aColor; 
 

CalendarItems.m implements the WebKitÕs isKeyExcludedFromWebScript and webScriptNameForKey methods, which allows its exposure to JavaScript.
Note that step b is optional. Follow that step if you want to use custom names for your instance variables.


a) Identify all instance variables that will be shared with JavaScript
 The isKeyExcludedFromWebScript method determines which Objective-C properties should be available to JavaScript. In this sample, all CalendarItems instance variables are exposed to JavaScript. See below for an implementation of isKeyExcludedFromWebScript : 

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
    NSString* keyName = [[[NSString alloc] initWithUTF8String: name] autorelease];
    return !([ keyName hasPrefix:kJSSelectorPrefix]);
}


b) Provides mapping between Objective-C and JavaScript instance variable names
The plug-in adds a "js" to the beginning of any property that is to be exposed to JavaScript. The webScriptNameForKey method returns friendly names for the exposed Objective-C instance variables. In this sample, all properties are exposed to JavaScript without the "js" prefix. See below for an implementation of webScriptNameForKey:


+ (NSString *)webScriptNameForKey:(const char *)name
{
    NSString* keyName = [[[NSString alloc] initWithUTF8String: name] autorelease];
    if ([ keyName hasPrefix:kJSSelectorPrefix] && 
       ([ keyName length] > [kJSSelectorPrefix length]))
    {
        return [keyName substringFromIndex:[kJSSelectorPrefix length]];
    }
    return nil;
}


10. Create the RemindersPlugIn class' s files
Proceed as done in step 7 and name the file RemindersPlugIn.


11. Create the RemindersPlugIn class
Add the "#import <WebKit/WebKit.h>" to the beginning of the RemindersPlugIn.h file.
Add both "#import <CalendarStore/CalendarStore.h>" and "#import "NSColorHexadecimalValue.h" to the beginning of the RemindersPlugIn.m file.
This process involves several steps that are described from number 12 to 15.


12. Add code for exporting Objective-C methods to JavaScript
The plug-in defines a WebScriptObject object and implements some functions that allows communication between the widget and itself.
Note that step b is optional. Follow that step if you want to use custom names for your methods.

a) Identify the object that contains methods and properties to be exported to JavaScript
This step consists of providing a name to the plug-in and using the Web KitÕs
windowScriptObjectAvailable method to bind this name to the above mentioned WebScriptObject object.See below for an implementation of windowScriptObjectAvailable:

- (void)windowScriptObjectAvailable:(WebScriptObject*)scriptObject
{
    // our plug-in will be called as RemindersPlugIn from JavaScript
	[scriptObject setValue:self forKey:@"RemindersPlugIn"];
	webScriptObject = scriptObject;
	[webScriptObject retain];
}


b) Provide mapping between Objective-C and JavaScript method names
The plug-in adds a "js" to the beginning of any method that will be exposed to JavaScript. The Web Kit's webScriptNameForSelector method returns friendly names for these exposed Objective-C methods.  In this sample, all Objective-C methods will be used in JavaScript without the "js" prefix. For instance, the Objective-C  "js_calendarToDoItems" method will be exposed as "calendarToDoItems" in JavaScript. See below for webScriptNameForSelector's implementation: 

NSString * const kJSSelectorPrefix = @"js_";

+ (NSString *)webScriptNameForSelector:(SEL)aSelector 
{
	NSString* selectorName = NSStringFromSelector(aSelector);
	if ([selectorName hasPrefix:kJSSelectorPrefix] && ([selectorName length] > [kJSSelectorPrefix length]))
	{
		return [[[selectorName substringFromIndex:[kJSSelectorPrefix length]] componentsSeparatedByString: @":"] objectAtIndex: 0];
	}
	return nil;
}


c) Allow JavaScript to access Objective-C methods and properties
This step consists of identifying all methods and instance variables that will be shared with JavaScript.

i) Identify all methods that will be shared with JavaScript
The Web Kit's isSelectorExcludedFromWebScript determines which Objective-C methods should be exposed to JavaScript. In this sample, all methods with a "js" prefix  are shared with JavaScript. Below is an implementation of isSelectorExcludedFromWebScript: 

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)aSelector 
{
	return !([NSStringFromSelector(aSelector) hasPrefix:kJSSelectorPrefix]);
}


13. Fetching and parsing data
The "fetchCalendarEvents" and "fetchToDoItems" are used to retrieve events and to do items, respectively. They both receive a list of calendars' uids. They return data from specific calendars if the list is not empty and all available calendars otherwise. Both methods use the "fetchCalendarsWithUIDs" to get all calendars corresponding to the specified uids. 

Data parsing is accomplished through the "parseCalendarEvents," "parseCalendars," and "parseCalendarToDoItems" methods that respectively parse events, calendars, and to do items information. All three methods build an array of CalendarItems objects. 


14. Calling the plug-in from JavaScript
JavaScript can access the plug-in's " js_calendarEvents,"  "js_calendarToDoItems," "js_calendars," and "js_nameForCalendarWithUIDs" methods. They are rendered in JavaScript without the "js" prefix. For instance, the "js_nameForCalendarWithUIDs" will be accessed in JavaScript as follows:
	RemindersPlugIn.nameForCalendarWithUIDs


15. Calling JavaScript from the plug-in
Both "eventsChanged" and "tasksChanged" methods call the JavaScript "reloadEventsOrToDoItems" from the plug-in whenever users add, remove, or update events or to do items. This is accomplished using the WebScriptObject object and the evaluateWebScript method as seen in the code below: 

- (void)eventsChanged:(NSNotification *)notification
{
    [webScriptObject evaluateWebScript:@"reloadEventsOrToDoItems(false)"];
}


16. Build the sample
Click the Build toolbar item in the project window to compile the plug-in. 



Creating  the widget Reminders
We have just created the plug-in part, what follows are the steps for creating the widget that uses the plug-in. We are going to set up the widget, build its interface, and add functionalities for fetching and displaying data on its front and back. 


Note: The widget consists of several files; however, the most important files                                
are: Reminders.js, Reminders.html, Reminders.css, and UserCalendarPreferences.js. 

1. Create the widget 
Reminders was created using Dashcode and consists of HTML, CSS, and JavaScript files. Open Dashcode, select File > new Project, and choose the widget's custom template. Save the new project as Reminders.


2. Set up the widget's identifier 
Click the widget attributes pane in the window to access the widget's properties. Fill the widget identifier with the RemindersPlugIn's bundle identifier.  


3. Set up the widget's plugin
The widget plugin field holds our plug-in's name. Use the "choose" button in the widget's properties window to select the RemindersPlugIn.widgetplugin folder (RemindersPlugIn/build/Release/RemindersPlugIn.widgetplugin).


4. Add the widget's preferrredCalendar key
The widget uses the preferrredCalendar key to save the user's calendar preferences.
Click the "+" sign in the Localization area to add the preferredCalendar key.


5. Create the widget's interface
Our widget has front and back interfaces. Both interfaces were created using controls available in Dashcode's library. 
Choose Window > Show Library and click the Parts button to see all available Dashcode's controls. We built the widget by dragging each of its parts (buttons, scroll areas, rectangles) from the Parts Library to the widgetÕs body on the canvas.


6. Implement the front of the widget 
The Reminders.js file contains all required JavaScript functions for the front of the widget. Its "showEvents" and "showToDoItems" functions are activated when users respectively click the "Events" or "To Do Items" buttons on the widget's menu bar. 
Both functions call the RemindersPlugIn object to retrieve events and to do items belonging to all or specific calendars, respectively. "showEvents"  calls the plug-in using the following line: 
    RemindersPlugIn.calendarEvents(preferredCalendars)

Its "displayResults" function receives an array of CalendarItems objects and a boolean value; it shows events if the value is false and to do items otherwise. The "showErrorMessage" function adds specific error messages on the widget. Both "displayResults" and "showErrorMessage"  write data to the widget's scroll area. We need to refresh the scroll area once we are done appending data to it. Use the following line to refresh the scroll area: 
    document.getElementById(scrollArea).object.refresh()


Note: Below are problems that you may encounter if you do not refresh the scroll area after changing its content:
-Sliders might not be visible when the content is too large for the scroll area. 
 -If you move the slider up or down while the widget displays either events or to do items data, then switches to the other view, the slider will stay at its previous position instead of jumping to the top of the scroll area. 

Users can switch to the back of the widget using the "showBack" function. 


7. Implement the back of the widget 
The UserCalendarPreferences.js file defines all required JavaScript functions for the back of the widget. It uses the "displayUserCalendarPreferences" function to fetch and display all available calendars. It calls the "retrieveSelectedCalendars" function to get all calendars checked by the user and update the custom "preferredCalendars" key.

All calendars are selected by default when the widget first loads. "retrieveSelectedCalendars" shows a warning message and icon if users uncheck all of them. The following line shows how to update the "preferredCalendars" key: 
	widget.setPreferenceForKey(preferredCalendars,"preferredCalendars")

The "userCalendarPreferences" function checks the widget's "preferredCalendars" key using widget.preferenceForKey("preferredCalendars") and returns 
the user's calendar preferences. Use the "showFront" function to return to the front of the widget.


8. Build the sample
Click Run in the toolbar to test the widget.
Click File > Deploy Widget to Dashboard to install the widget on your computer.


Feedback and Bug Reports
Please send all feedback about this sample by using the Feedback form on the bottom of the sample's webpage.
Please submit any bug reports about this sample to the Bug Reporting <http://developer.apple.com/bugreporter> page.

Copyright © 2008 Apple Inc. All rights reserved.