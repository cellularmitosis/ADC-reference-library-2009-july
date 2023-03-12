SimpleCalendar is an application that uses Calendar Store to fetch iCal events and display them on a custom calendar. 

The application uses both the read and write Calendar Store APIs. It initially fetches a range of event objects from Calendar Store. It uses Cocoa Bindings to display events on individual days of a monthly calendar. It observes iCal notifications and updates local records if they are changed externally. It also observes user changes to local records and saves them to Calendar Store.

Follow these steps to run SimpleCalendar:

1) Create multiple calendars and events in iCal for the current month
2) Launch SimpleCalendar
   A calendar view appears displaying the events you created in iCal.
3) Click the Previous and Next button to view the previous and next months
4) Add, change, or delete an event in iCal
   After a few moments, verify that the changes appear in SimpleCalendar.
5) Select Window > Events to open the events list window
6) Add, change, or delete an event using the Events window
   Again, verify that the changes appear in iCal.

* You can also change the title of an event by editing it in the calendar view.


Core SimpleCalendar Classes:

AppDelegate--the Cocoa application and window delegate. Creates the calendar views after its nib file, MyDocument.nib, is loaded.

Calendar--the "model" object of a calendar that manages CalEvent objects. The Calendar object fetches CalEvent objects, applies changes from iCal to local CalEvent objects, and saves user changes to CalEvent objects.

CalendarController and DayController--used to implement the calendar views.


Summary of Nib Files:

MainMenu.nib--contains the Cocoa main menu.

MyDocument.nib--contains the table view window and the core models and controllers. AppDelegate is the file's owner.

Calendar.nib--contains the calendar window and its views. CalendarController is the file's owner.

Day.nib--contains the controls that display a day on a month view. DayController is the file's owner.
