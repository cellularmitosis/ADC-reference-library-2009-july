/*
 * iCal.h
 */

#import <AppKit/AppKit.h>
#import <ScriptingBridge/ScriptingBridge.h>


@class iCalText, iCalAttachment, iCalParagraph, iCalWord, iCalCharacter, iCalAttributeRun, iCalItem, iCalApplication, iCalColor, iCalDocument, iCalWindow, iCalApplication, iCalCalendar, iCalDisplayAlarm, iCalMailAlarm, iCalSoundAlarm, iCalOpenFileAlarm, iCalAttendee, iCalTodo, iCalEvent;

typedef enum {
	iCalSavoYes = 'yes ' /* Save the file. */,
	iCalSavoNo = 'no  ' /* Do not save the file. */,
	iCalSavoAsk = 'ask ' /* Ask the user whether or not to save the file. */
} iCalSavo;

typedef enum {
	iCalWre6Unknown = 'E6na' /* No anwser yet */,
	iCalWre6Accepted = 'E6ap' /* Invitation has been accepted */,
	iCalWre6Declined = 'E6dp' /* Invitation has been declined */,
	iCalWre6Undecided = 'E6tp' /* Invitation has been tentatively accepted */
} iCalWre6;

typedef enum {
	iCalWre4Cancelled = 'E4ca' /* A cancelled event */,
	iCalWre4Confirmed = 'E4cn' /* A confirmed event */,
	iCalWre4None = 'E4no' /* An event without status */,
	iCalWre4Tentative = 'E4te' /* A tentative event */
} iCalWre4;

typedef enum {
	iCalWrp1NoPriority = 'tdp0' /* No priority */,
	iCalWrp1LowPriority = 'tdp9' /* Low priority */,
	iCalWrp1MediumPriority = 'tdp5' /* Medium priority */,
	iCalWrp1HighPriority = 'tdp1' /* High priority */
} iCalWrp1;

typedef enum {
	iCalWre5DayView = 'E5da' /* The iCal day view */,
	iCalWre5WeekView = 'E5we' /* The iCal week view */,
	iCalWre5MonthView = 'E5mo' /* The iCal month view */
} iCalWre5;



/*
 * Text Suite
 */

// Rich (styled) text
@interface iCalText : SBObject

- (SBElementArray *) paragraphs;
- (SBElementArray *) words;
- (SBElementArray *) characters;
- (SBElementArray *) attributeRuns;
- (SBElementArray *) attachments;

@property (copy) NSColor *color;  // The color of the first character.
@property (copy) NSString *font;  // The name of the font of the first character.
@property (copy) NSNumber *size;  // The size in points of the first character.

- (void) closeSaving:(iCalSavo)saving savingIn:(NSURL *)savingIn;  // Close an object.
- (void) delete;  // Delete an object.
- (void) duplicateTo:(SBObject *)to withProperties:(NSDictionary *)withProperties;  // Copy object(s) and put the copies at a new location.
- (BOOL) exists;  // Verify if an object exists.
- (void) moveTo:(SBObject *)to;  // Move object(s) to a new location.
- (void) saveIn:(NSURL *)in_ as:(NSString *)as;  // Save an object.
- (void) GetURL;  // Subscribe to a remote calendar through a webcal or http URL
- (void) show;  // Show the event or to-do in the calendar window

@end

// Represents an inline text attachment.  This class is used mainly for make commands.
@interface iCalAttachment : iCalText

@property (copy) NSString *fileName;  // The path to the file for the attachment


@end

// This subdivides the text into paragraphs.
@interface iCalParagraph : iCalText


@end

// This subdivides the text into words.
@interface iCalWord : iCalText


@end

// This subdivides the text into characters.
@interface iCalCharacter : iCalText


@end

// This subdivides the text into chunks that all have the same attributes.
@interface iCalAttributeRun : iCalText


@end



/*
 * Standard Suite
 */

// A scriptable object.
@interface iCalItem : SBObject

@property (copy) NSDictionary *properties;  // All of the object's properties.

- (void) closeSaving:(iCalSavo)saving savingIn:(NSURL *)savingIn;  // Close an object.
- (void) delete;  // Delete an object.
- (void) duplicateTo:(SBObject *)to withProperties:(NSDictionary *)withProperties;  // Copy object(s) and put the copies at a new location.
- (BOOL) exists;  // Verify if an object exists.
- (void) moveTo:(SBObject *)to;  // Move object(s) to a new location.
- (void) saveIn:(NSURL *)in_ as:(NSString *)as;  // Save an object.
- (void) show;  // Show the event or to-do in the calendar window

@end

// An application's top level scripting object.
@interface iCalApplication : SBApplication
+ (iCalApplication *) application;

- (SBElementArray *) documents;
- (SBElementArray *) windows;

@property (copy, readonly) NSString *name;  // The name of the application.
@property (readonly) BOOL frontmost;  // Is this the frontmost (active) application?
@property (copy, readonly) NSString *version;  // The version of the application.

- (void) open:(NSURL *)x;  // Open an object.
- (void) print:(NSURL *)x;  // Print an object.
- (void) quitSaving:(iCalSavo)saving;  // Quit an application.
- (void) createCalendarWithName:(NSString *)withName;  // Creates a new calendar (obsolete, will be removed in the future, use make calendar syntax)
- (void) reloadCalendars;  // Tell the application to reload all calendar files contents
- (void) switchViewTo:(iCalWre5)to;  // Show calendar on the given view
- (void) viewCalendarAt:(NSDate *)at;  // Show calendar on the given date
- (void) GetURL:(NSString *)x;  // Subscribe to a remote calendar through a webcal or http URL

@end

// A color.
@interface iCalColor : SBObject

- (void) closeSaving:(iCalSavo)saving savingIn:(NSURL *)savingIn;  // Close an object.
- (void) delete;  // Delete an object.
- (void) duplicateTo:(SBObject *)to withProperties:(NSDictionary *)withProperties;  // Copy object(s) and put the copies at a new location.
- (BOOL) exists;  // Verify if an object exists.
- (void) moveTo:(SBObject *)to;  // Move object(s) to a new location.
- (void) saveIn:(NSURL *)in_ as:(NSString *)as;  // Save an object.
- (void) show;  // Show the event or to-do in the calendar window

@end

// A document.
@interface iCalDocument : SBObject

@property (copy) NSString *path;  // The document's path.
@property (readonly) BOOL modified;  // Has the document been modified since the last save?
@property (copy) NSString *name;  // The document's name.

- (void) closeSaving:(iCalSavo)saving savingIn:(NSURL *)savingIn;  // Close an object.
- (void) delete;  // Delete an object.
- (void) duplicateTo:(SBObject *)to withProperties:(NSDictionary *)withProperties;  // Copy object(s) and put the copies at a new location.
- (BOOL) exists;  // Verify if an object exists.
- (void) moveTo:(SBObject *)to;  // Move object(s) to a new location.
- (void) saveIn:(NSURL *)in_ as:(NSString *)as;  // Save an object.
- (void) show;  // Show the event or to-do in the calendar window

@end

// A window.
@interface iCalWindow : SBObject

@property (copy) NSString *name;  // The full title of the window.
- (NSNumber *) id;  // The unique identifier of the window.
@property NSRect bounds;  // The bounding rectangle of the window.
@property (copy, readonly) iCalDocument *document;  // The document whose contents are being displayed in the window.
@property (readonly) BOOL closeable;  // Whether the window has a close box.
@property (readonly) BOOL titled;  // Whether the window has a title bar.
@property (copy) NSNumber *index;  // The index of the window in the back-to-front window ordering.
@property (readonly) BOOL floating;  // Whether the window floats.
@property (readonly) BOOL miniaturizable;  // Whether the window can be miniaturized.
@property BOOL miniaturized;  // Whether the window is currently miniaturized.
@property (readonly) BOOL modal;  // Whether the window is the application's current modal window.
@property (readonly) BOOL resizable;  // Whether the window can be resized.
@property BOOL visible;  // Whether the window is currently visible.
@property (readonly) BOOL zoomable;  // Whether the window can be zoomed.
@property BOOL zoomed;  // Whether the window is currently zoomed.

- (void) closeSaving:(iCalSavo)saving savingIn:(NSURL *)savingIn;  // Close an object.
- (void) delete;  // Delete an object.
- (void) duplicateTo:(SBObject *)to withProperties:(NSDictionary *)withProperties;  // Copy object(s) and put the copies at a new location.
- (BOOL) exists;  // Verify if an object exists.
- (void) moveTo:(SBObject *)to;  // Move object(s) to a new location.
- (void) saveIn:(NSURL *)in_ as:(NSString *)as;  // Save an object.
- (void) show;  // Show the event or to-do in the calendar window

@end



/*
 * iCal
 */

// This class represents iCal.
@interface iCalApplication (ICal)

- (SBElementArray *) calendars;

@end

// This class represents a calendar.
@interface iCalCalendar : iCalItem

- (SBElementArray *) todos;
- (SBElementArray *) events;

@property (copy) NSString *name;  // This is the calendar title.
@property (copy) NSColor *color;  // The calendar color.
@property (copy, readonly) NSString *uid;  // A unique calendar key
@property (readonly) BOOL writable;  // indicates whether this calendar is modifiable on this machine.
@property (copy) NSString *objectDescription;  // This is the calendar description.


@end

// This class represents a message alarm.
@interface iCalDisplayAlarm : iCalItem

@property NSInteger triggerInterval;  // The interval in minutes between the event and the alarm: (positive for alarm that trigger after the event date or negative for alarms that trigger before).
@property (copy) NSDate *triggerDate;  // An absolute alarm date..


@end

// This class represents a mail alarm.
@interface iCalMailAlarm : iCalItem

@property NSInteger triggerInterval;  // The interval in minutes between the event and the alarm: (positive for alarm that trigger after the event date or negative for alarms that trigger before).
@property (copy) NSDate *triggerDate;  // An absolute alarm date..


@end

// This class represents a sound alarm.
@interface iCalSoundAlarm : iCalItem

@property NSInteger triggerInterval;  // The interval in minutes between the event and the alarm: (positive for alarm that trigger after the event date or negative for alarms that trigger before).
@property (copy) NSString *soundName;  // The system sound name to be used for the alarm
@property (copy) NSString *soundFile;  // The (POSIX) path to the sound file to be used for the alarm
@property (copy) NSDate *triggerDate;  // An absolute alarm date..


@end

// This class represents an 'open file' alarm.
@interface iCalOpenFileAlarm : iCalItem

@property NSInteger triggerInterval;  // The interval in minutes between the event and the alarm: (positive for alarm that trigger after the event date or negative for alarms that trigger before).
@property (copy) NSString *filepath;  // The (POSIX) path to be opened by the alarm
@property (copy) NSDate *triggerDate;  // An absolute alarm date..


@end

// This class represents a attendee.
@interface iCalAttendee : iCalItem

@property (copy, readonly) NSString *displayName;  // The first and last name of the attendee.
@property (copy, readonly) NSString *email;  // e-mail of the attendee.
@property iCalWre6 participationStatus;  // The invitation status for the attendee.


@end

// This class represents a task.
@interface iCalTodo : iCalItem

- (SBElementArray *) displayAlarms;
- (SBElementArray *) mailAlarms;
- (SBElementArray *) openFileAlarms;
- (SBElementArray *) soundAlarms;

@property (copy) NSDate *completionDate;  // The todo completion date.
@property (copy) NSDate *dueDate;  // The todo due date.
@property iCalWrp1 priority;  // The todo priority.
@property (readonly) NSInteger sequence;  // The todo version.
@property (copy, readonly) NSDate *stampDate;  // The todo modification date.
@property (copy) NSString *summary;  // This is the todo summary.
@property (copy) NSString *objectDescription;  // The todo notes.
@property (copy, readonly) NSString *uid;  // A unique todo key.
@property (copy) NSString *url;  // The URL associated to the todo.


@end

// This class represents an event.
@interface iCalEvent : iCalItem

- (SBElementArray *) attendees;
- (SBElementArray *) displayAlarms;
- (SBElementArray *) mailAlarms;
- (SBElementArray *) openFileAlarms;
- (SBElementArray *) soundAlarms;

@property (copy) NSString *objectDescription;  // The events notes.
@property (copy) NSDate *startDate;  // The event start date.
@property (copy) NSDate *endDate;  // The event end date.
@property BOOL alldayEvent;  // True if the event is an all-day event
@property (copy) NSString *recurrence;  // The iCalendar (RFC 2445) string describing the event recurrence, if defined
@property (readonly) NSInteger sequence;  // The event version.
@property (copy, readonly) NSDate *stampDate;  // The event modification date.
@property (copy) NSArray *excludedDates;  // The exception dates.
@property iCalWre4 status;  // The event status.
@property (copy) NSString *summary;  // This is the event summary.
@property (copy) NSString *location;  // This is the event location.
@property (copy, readonly) NSString *uid;  // A unique event key.
@property (copy) NSString *url;  // The URL associated to the event.


@end

