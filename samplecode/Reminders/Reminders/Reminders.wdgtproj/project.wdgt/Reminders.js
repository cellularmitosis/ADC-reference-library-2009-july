/*

File: Reminders.js

Abstract: Defines JavaScript functionality for the Reminders sample.
		  Uses the Cocoa RemindersPlugIn plug-in to fetch iCal events and to do items.
		  Displays iCal events and to do items on the widget when users 
		  click the "Events" and "To Do Items" buttons, respectively.

Version: <1.0>

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

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

/* Indicates which button was pressed last on the widget */
var lastControl = null;

/* Error messages */
var PLUGIN_ERR = "The Reminders plug-in could not be found.";
var EVENT_ERR = "No upcoming events in ";
var TODOITEMS_ERR = "No upcoming tasks in ";
var CALENDAR_ERR = "No available Calendars.";
var preferredCalendars = "";

/*
	Called by HTML body element's onload event when the widget is ready to start.
    Sets up the widget and calls initialize to update the widget.
*/
function load()
{
    setupParts();
	showEvents();
}

/*
	Called by the plug-in when users added, removed, or updated events or to do items.  
	Reloads the widget's front with to do items if toDoList is true and events otherwise. 
	Resets the preference key so all the new changes can be displayed.
*/
function reloadEventsOrToDoItems(toDoItems)
{
	/* Reset the preference key so the widget can display the changes */
	widget.setPreferenceForKey(null,"preferredCalendars");
	
	/* Display to do items if toDoItems is set to true and events, otherwise */
	if (toDoItems)
	{
		showToDoItems();	
	}
	else 
	{
		showEvents();
	}
}

/*
	Displays an error message on the widget. It receives four parameters:
	error is an error message, calendars contains the preferredCalendars's value that
	contains the uids of the user's preferred calendars, content is the content area in 
	the front or back of the widget, and scrollArea is the scroll area in the widget's front or back . 
*/
function showErrorMessage(error,calendars,content,scrollArea)
{
	/* Clear the scroll area */
	clearResults(content,scrollArea); 
	var errorDiv = createAnHTMLElement("div","errorDiv","","");
	var message;
	
	/*
		If calendars is empty (the preferredCalendars is empty when the widget first loads), 
		then all calendars must have be selected in the widget's back.
	*/
	if (calendars=="")
	{
		message = ((error==PLUGIN_ERR) || (error==CALENDAR_ERR)) ? error : error+"all iCal calendars.";
	}
	else
	{
		/*
		   If calendars is not empty, then it must contain the uids of calendars that have been chosen.
		   Call the plug-in to get the names of these chosen calendars.  
		*/
		var calendarArray = RemindersPlugIn.nameForCalendarWithUIDs(calendars);
	    message = (calendarArray) ? error+" the "+buildErrorMessage(calendarArray) : "No available Calendars in iCal.";
	}
	errorDiv.appendChild(document.createTextNode(message));
	
	document.getElementById(content).appendChild(errorDiv);
	document.getElementById(scrollArea).object.refresh();
}


/*
	Receives an array that contains one or more calendars'names. Joins all names
	in the array and appends " calendar." to the resulting string. If data contains
	one name, adds " calendar." to the name and returns it. If data contains two names, 
	joins them with a "and". If data contains more than two names, joins them with a ", ", 
	then inserts "and" after the last comma and before the next name.
*/
function buildErrorMessage(data)
{
	if (data.length==1)
	{
		/* If data contains only one name */
		return data[0]+" calendar.";
	}
	else if (data.length>1)
	{
		/* If data contains two names, concatenate them with "and" */
		if (data.length==2)
		{
			return data.join(" and ")+" calendars.";
		}
		else
		{ 
			/* If data contains three or more names, concatenate all items with a ", " and put the result in a string.
			   Look for the position of the last comma in that string and insert "and" after that comma and before.
		   */
			var dataString = data.join(", ");
			var lastCommaPosition = dataString.lastIndexOf(", ");
			return dataString.substring(0, lastCommaPosition+1)+" and "+dataString.substring(lastCommaPosition+2,dataString.length)+" calendars.";
		}
	}
	else
	{
		return "No calendar";
	}
	return null;
}


/*
	Removes everything from the scroll area.
*/
function clearResults(content,scrollArea)
{
	var resultsDiv = document.getElementById(content);
	while (resultsDiv.hasChildNodes())
	{
		resultsDiv.removeChild(resultsDiv.firstChild);
	}
	document.getElementById(scrollArea).object.refresh();
}


/*
	Puts or removes focus on "Events" or "To Do Items" buttons. 
*/
function updateControl(newControl) 
{
	/* Remove focus from the previous pressed button */
	if (lastControl) 
	{
		/* _setPressed is an AppleButton handler, false indicates the user
		   pressed a button other than the one referenced by lastControl. 	
		*/
		lastControl.object._setPressed(false);
		
		/* Dim the text's color on the button referenced by lastControl */
		lastControl.style.color = "rgb(102, 102, 102)";
	}
	
	/* Put focus on newly pressed button */
	newControl.object._setPressed(true);
	
	/* Set the text on the pressed control to white */
	newControl.style.color = "white";
	/* Keep a reference to this newly pressed button */
	lastControl = newControl;
}

/*
	Creates and defines an HTML element according to four values: element, idReference, classReference, and innerText. 
	element is an html element to be built such as div or td, idReference is the element's unique identifier,
	classReference is a class  for element , and innerText is the text to be added to element. idReference, classReference,
	and innerText are optional values. 	
*/
function createAnHTMLElement(element, idReference, classReference, innerText)
{
    /* Generate the HTML object specified by element */
	var createdElement = document.createElement(element);

	/* Set up an id for the created HTML object if idReference is not empty */
	if(idReference.length!=0)
	{
		createdElement.setAttribute("id", idReference);
	}
	
	/* Add a style to the created HTML object if classReference is not empty */
	if(classReference.length!=0)
	{
		createdElement.setAttribute("class", classReference);
	}
	
	/* Append a text to the created HTML object if innerText is not empty */
	if(innerText.length!=0)
	{
		createdElement.appendChild(document.createTextNode(innerText));
	}
	return createdElement;
}



/* 
	Adds iCal events to the widget if the received toDoList variable is false and 
	to do items otherwise. 
	toDoList is a boolean variable that indicates whether calendar contains events or to do items objects. 
	calendar is an array of CalendarItems objects, which store either events or to do items.
*/
function displayResults(calendar,toDoList)
{
	/* Clear the scroll area */
	clearResults("frontContent","frontScrollArea"); 
	var scrollAreaDiv = document.getElementById("frontContent");
	
	/* Keep a reference of the current date so it can be compared to the next one */
	var previousDate = "";
	
	/* Iterate through calendar */
	for (var i=0; i<calendar.length; i++)
	{
	
		/* Create a div for the start date or due date */
		var startDateDiv = createAnHTMLElement("div","startDateDiv","",calendar[i].startDate);
		
		/*
			All entries are grouped according to their start date or due date. Thus,
			appends this date to the widget if and only if it is different from the previous date.
		*/
		if(previousDate !=calendar[i].startDate)
		{
			scrollAreaDiv.appendChild(startDateDiv);	
		}

		/*
			Create a div to hold the associated calendar's color. This statement only
			creates a 10x10 box whose color will be set below. At this point, the box 
			does not exist yet on the widget (it has not been added to the widget).
		*/
		var color = createAnHTMLElement("div","colorDiv"+i,"colorDiv","");
		
		/* leftColumn is a inner div that contains the color div */
		var leftColumn = createAnHTMLElement("div","leftColumn","","");
		leftColumn.appendChild(color);

		/* title is a div that contains an event or to do item's title */
		var title = createAnHTMLElement("div","title","",calendar[i].title);
		var timeorPriority = "";
        
		if (toDoList)
		{
			/* Append a div containing a to do item's priority if toDoList is true */
			timeorPriority = createAnHTMLElement("div",calendar[i].timeOrPriority,"timeOrPriority","Priority: "+calendar[i].timeOrPriority);
		}
		else
		{
			/* Append a div containing an event's time if toDoList is false */
			timeorPriority = createAnHTMLElement("div",calendar[i].timeOrPriority,"timeOrPriority",calendar[i].timeOrPriority);
		}

		/* rightColumn is a inner div that contains the title and timeorPriority divs */
		var rightColumn = createAnHTMLElement("div","rightColumn","","");
		rightColumn.appendChild(title);
		rightColumn.appendChild(timeorPriority);

		/* container is a div that contains the leftColumn and rightColumn divs */
		var container = createAnHTMLElement("div","","container","");
		container.appendChild(leftColumn);
		container.appendChild(rightColumn);
		
		/* Append the container's content such as title, 10x10 color box, and time to the scroll area */
		scrollAreaDiv.appendChild(container);

		/* Add a gray line on the widget to group all information by date */ 
		var startDateHr = createAnHTMLElement("div","hrDiv","","");
		
		/*
			At this point, the 10x10 color box has been added to the widget as mentionned above 
			So let's set its color. 
		*/
		document.getElementById("colorDiv"+i).style.backgroundColor = calendar[i].color;
		
		/* Get the next index */
		var nextIndex = i+1;
		
		/* Check if nextIndex is less than the number of objects in calendar */
		if (nextIndex<calendar.length)
		{
			/* Get the next element's start date or due date in calendar */
			var nextDate = calendar[nextIndex].timeOrPrioritycalendar;
			/*
				if the next element's start date is different from the current one,
				append a gray line to the widget to separate this group of information 
				from the next one.
			*/
			if(nextDate !=calendar[i].startDate)
			{	
				scrollAreaDiv.appendChild(startDateHr);
			}
		}
		/* Assign the current date to previousDate */
		previousDate = calendar[i].startDate;
	}
	
	/*
		IMPORTANT: Need to refresh the scroll area once we are done appending data to it.
		Problems that you may encounter if this line is not provided: 
		  -sliders might not be visible when the content is too large for the scroll area. 
		  -If you move the slider up or down while the widget displays either events or to do items data, 
		   then switches to the other view, the slider will stay at its previous position instead of jumping to the top of
		   the scroll area. 
	*/
	document.getElementById("frontScrollArea").object.refresh();
}

/*
	Calls the plug-in to get all available events starting now and ending before or 
	at 12:59:59 PM on December 31 of this year.
*/
function showEvents()
{ 
	/* Put focus on the events button */
	updateControl(document.getElementById("events"));
	/* Get the uids of the user's preferred calendars */
	preferredCalendars = retrievePreferredCalendarKey();

	/* Call the plug-in, check for available iCal events if RemindersPlugIn exists and show an error message otherwise */
	if (RemindersPlugIn)
	{
		/* The plug-in should return all events belonging to calendars whose uid is contained in 
		   preferredCalendars if preferredCalendars is not empty.
		   If preferredCalendars is empty, the plug-in should return all events belonging to all
		   calendars.
		*/
		var events = RemindersPlugIn.calendarEvents(preferredCalendars);
		/* Display iCal events if events is not empty and an error message, otherwise */
		if (events && events.length!=0) 
        {
			/* false indicates that displayResults should only list events on the widget */
			displayResults(events,false);
		}	
		else
		{
			showErrorMessage(EVENT_ERR,preferredCalendars,"frontContent","frontScrollArea");
		}
    }
	else
	{
		showErrorMessage(PLUGIN_ERR,"","frontContent","frontScrollArea");
	} 
}


/*
	Calls the plug-in to get all incomplete to do items whose due date is set before or 
	at 12:59:59 PM on December 31 of this year.
*/
function showToDoItems()
{
	/* Put focus on the toDoItems button */
	updateControl(document.getElementById("toDoItems"));
	preferredCalendars = retrievePreferredCalendarKey();
	/* Call the plug-in, check for to do items if RemindersPlugIn exists and show an error message otherwise */
	if (RemindersPlugIn)
	{	
		var tasks = RemindersPlugIn.calendarToDoItems(preferredCalendars);

		/* Display all to do items if tasks is not empty and an error message, otherwise */
		if (tasks && tasks.length!=0) 
        {	
			/* true indicates that displayResults will list to do items on the widget */
			displayResults(tasks,true);
		}	
		else
		{
			showErrorMessage(TODOITEMS_ERR,preferredCalendars,"frontContent","frontScrollArea");
		}
    }
	else
	{
		showErrorMessage(PLUGIN_ERR,"","frontContent","frontScrollArea");
	}
}

/*
	Called when the info button is clicked to show the back of the widget.
	Calls the displayUserCalendarPreferences function to list the 
	user's calendar preferences. 
*/
function showBack(event)
{
    var front = document.getElementById("front");
    var back = document.getElementById("back");
 
    if (window.widget)
	{
        widget.prepareForTransition("ToBack");
    }
	/* Show the back of the widget and hide its front */
    front.style.display = "none";
    back.style.display = "block";

    if (window.widget) 
	{
        setTimeout('widget.performTransition();', 0);
		/* Show the user's calendar preferences */
		displayUserCalendarPreferences();
    }
}

function remove()
{
    /* Remove the key from the system */
	widget.setPreferenceForKey(null,"preferredCalendars");
}

if (window.widget)
{
    widget.onremove = remove;
}


