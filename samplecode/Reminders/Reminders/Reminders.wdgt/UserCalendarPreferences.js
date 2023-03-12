/*

File: UserCalendarPreferences.js

Abstract: Defines JavaScript functionalities for the back of the 
		  Reminders widget. Calls the Cocoa RemindersPlugIn plug-in 
		  to fetch all iCal calendars. Updates or retrieves the 
		  widget's custom preferredCalendars key.

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
/* Indicate whether the back of the widget is showing some calendars */
var noCalendars = false;

/*
	Returns the value of the preferredCalendarKey key.
*/
function retrievePreferredCalendarKey()
{
	var retrievedKey = widget.preferenceForKey("preferredCalendars");
	var value = (retrievedKey && retrievedKey.length > 0) ? retrievedKey : "";
	return value;
}


/*
	Retrieves the user's calendar preferences. Returns an array that contains 
	the uids of calendars selected by users; the array will be empty if the 
	user never chooses any calendars.
*/
function userCalendarPreferences()
{
	/* Get the user's preferred calendars */ 
	preferredCalendars = retrievePreferredCalendarKey();	
	/* calendarUIDs is empty if the user did not previously select any calendars on the widget's back */
	/* All preferred calendars'uids are separated by a "&", break and place them into an array */
	var calendarUIDs = (preferredCalendars.length > 0) ? preferredCalendars.split(" & ") : new Array();
	return calendarUIDs;
}


/*
	Lists all available calendars on the back of the widget. By default all calendars are selected. 
	Calls the plug-in's calendars method to fetch these calendars. 
*/
function displayUserCalendarPreferences()
{	
	/* Clear the widget back's scroll area */
	clearResults("backContent","backScrollArea");

	/* Call the plug-in, check for available calendars if RemindersPlugIn exists and show an error message otherwise */
	if (RemindersPlugIn)
	{
		/* Get all available calendars */
		var calendars = RemindersPlugIn.calendars("");
		/* Get the user's preferred calendars */
		var calendarUIDs = userCalendarPreferences();

		/* Display all iCal calendars if calendars is not empty and an error message otherwise */
		if (calendars && calendars.length!=0) 
		{
			var scrollAreaDiv = document.getElementById("backContent");

			/* Iterate through calendars, which is an array of CalendarItems objects */
			for(var i=0; i<calendars.length; i++)
			{
				
				/* Create a checkbox */
				var color = createAnHTMLElement("input","input"+i,"","");
				color.setAttribute("type","checkbox");
				color.setAttribute("name","calendars");
				color.setAttribute("size","1");
				color.setAttribute("value",calendars[i].uid);
				
				/* Check all checkboxes on the widget's back if the user did not select any */
				if (calendarUIDs.length==0)
				{
					color.checked = true;
				}
				else if ((calendarUIDs.length >0) & (contains(calendarUIDs,calendars[i].uid)))
				{
					/* Check this box if its associated calendar was previuously selected */
					color.checked = true;
				}
				else if ((calendarUIDs.length >0) & (!contains(calendarUIDs,calendars[i].uid)))
				{
					/* Uncheck this box if its associated calendar was not previuously selected */
					color.checked = false;
				}
				var container = createAnHTMLElement("div","","container","");
				container.appendChild(color);

				var title = document.createTextNode(calendars[i].title);
				container.appendChild(title);
				scrollAreaDiv.appendChild(container);
			}
			document.getElementById("backScrollArea").object.refresh();			
		}	
		else
		{
	        /* Indicate iCal does not contain any calendars */	
			noCalendars = true;
			showErrorMessage(CALENDAR_ERR,"","backContent","backScrollArea");
		}
	}
	else
	{
		showErrorMessage(PLUGIN_ERR,"","backContent","backScrollArea");
	}
}


/*
	Fetches all calendars selected by the user. It returns false and shows a warning message and icon if
	the user left all calendar preferences unchecked. It updates the preferredCalendars key and returns 
	true if the user checked at least one calendar preference on the back of the widget.
*/
function retrieveSelectedCalendars()
{
	var preferences = new Array();
	
	/* Warning message and icon are shown whenever all calendar preferences are left unchecked on the widget's back */
	var warning = document.getElementById("warning");
	var indicator = document.getElementById("indicator");
	
	/* Reference to all checkboxes */
	var checkboxes = document.getElementsByName("calendars");
	var j = 0;
	/* Loop through all checkboxes */
	for(i=0; i<checkboxes.length; i++)
	{
		/* If this checkbox is selected, get its associated uid */
		if(checkboxes[i].checked==true)
		{
			preferences[j] = checkboxes[i].value;
			j++;
		}
	}

	/* Update the preferredCalendars key and return true if users selected some calendars.
	   Show a warning message and icon and return false if users left all options unchecked. 
	   Users must choose at least one of the calendar options before switching back to the front
	   of the widget.
	*/
	if (preferences.length>0)
	{
		/* Join all selected calendars'uids using a "&" */
		var preferredCalendarValues = preferences.join(" & ");
		if(window.widget)
		{
			/* Update the preferredCalendars key */
			widget.setPreferenceForKey(preferredCalendarValues,"preferredCalendars");
			
			/* Hide the warning message and icon if they are visible */
			warning.style.display = "none";
			indicator.style.display = "none";
			return true;
		}
	}
	else
	{
		/* Show the warning message and icon, users did not select any calendars (all checkboxes were left unchecked) */
		warning.style.display = "block";
		indicator.style.display = "block";
	}
	return false;
}


/*
	Determines whether an array structure contains a given parameter.
	Returns true if the array contains the specified element and false otherwise.
*/
function contains(array,element)
{
	/* Iterate through all objects in array */
	for (var index in array)
	{
	    /* Compare each array's object with the specified element and return true if they are equal */
		if (array[index]==element)
		{
			return true;
		}
	}
	return false;
}


/*
	Called when the done button is clicked from the back of the widget.
	Calls the flipToFront function to take users to the front of the widget.
*/
function showFront(event)
{
	/* If noCalendars is true, then there are no calendars on the back of the widget.
	   So, do not show the warning message and icon and let users go to the front of the widget. */
	if (noCalendars)
	{
		flipToFront();
	}
	else
	{
	     /* noCalendars is set to false, so the back of the widget must be showing one or more calendars. 
		    Let's make sure that users did not leave all options unchecked. */
		if (retrieveSelectedCalendars())
		{
			flipToFront();
		}
	}
}

/*
	Takes users from the back of the widget to its front.
	Calls the retrieveSelectedCalendars function to check whether users
	chose at least one option from the back of the widget. 
*/
function flipToFront()
{
	var front = document.getElementById("front");
    var back = document.getElementById("back");
	if (window.widget) 
	{
		widget.prepareForTransition("ToFront");
	}	
	/* Show the front of the widget and hide its back */	
	front.style.display = "block";
    back.style.display = "none";	 
		
	if (window.widget) 
	{	
		/* Show events when the widget switches to its front */
		showEvents();
		setTimeout('widget.performTransition();', 0);
	}	
}
