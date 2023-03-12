InkSample 1.1 READ ME
------------------------------------

This is the InkSample sample code.  It is designed to demonstrate some usage scenarios for the Ink.Framework APIs.

The application contains a window with a tabbed control in which there are four panes.  Each pane demonstrates a different task using the Ink.Framework APIs.

The panes are as follows:

InkTextPane:

Task:

Handle kEventClassInk events.  Input text from kEventInkText to the text view.  Handle gesture types from kEventInkGesture appropriately for the text view.

Note that most  gesture commands like copy, paste, select all, etc. do not have to be handled.  If they are not handled, Ink.Framework will redispatch them as menu key commands (example: select all will be dispatched as "Command A" ).  The 'Join' gesture is not handled completely at this time.

InkTextAlternatesPane:

Handle kEventClassInk events.  Input text from kEventInkText to the text view.  Retain the InkTextRef for each word input in the text view.  Maintain a data structure that associates the correct InkTextRef with the position of the word in the text view.  When the user clicks on a word, retrieve the InkTextRef for that word and use it to display a contextual menu with alternate choices for recognition of the original ink.

Note that the text view in this pane restricts editing, in order to keep the maintenance task of synchronizing the InkTextRefs with word offsets more straightforward.

InkPhraseTerminationPane:

Tell Ink.Framework not to automatically terminate an ink session.  Manage termination of the ink input session by application logic (in this app, simply detect the click on a button control.)

StrokeAccumulationPane:

Tell Ink.Framework not to perform any handling of tablet generated events.  Ink.Framework will not draw ink on the screen, and it will not recognize handwriting input automatically.  Register for mouse / tablet events in the application, and build up data structures from tablet events that will constitute a single stroke (mouse down to mouse up) of the stylus on the tablet.  When a stroke is complete, build an array of InkPoints and tell Ink.Framework to add the array as a single stroke.  Manage termination of the current Ink Phrase by application logic (again, in this app, simply detect the click on a button control.)

If you find any bugs in the sample code, please report them to dts@apple.com, or use <http://bugreport.apple.com>.

October 13, 2003