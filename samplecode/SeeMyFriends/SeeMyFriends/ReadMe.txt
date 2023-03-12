The SeeMyFriends XCode project contains an example application that shows how to integrate SyncServices in a Carbon based application:

SeeMyFriends displays all user's Contact data in a very simple way (first name, last name, picture if present) in a custom HIView, which is updated when the contacts are changed by another sync client. SeeMyFriends itself does not allow modification of data, and is a read-only sync client. Therefore, it does not have an explicit sync button but instead is expected to be triggered through the mechanism of sync alert.  This application registers with the sync server with an alert handler, which is called when other clients syncing contact data begin a sync session. It will however trigger a sync at launch to update its content.

A way to test SeeMyFriends is to change some data (like a first name or last name attribute) in Address Book and wait for the change to be synchronized. It will automatically appear in SeeMyFriends.
	
SeeMyFriends sync data are archived between two launches of the app in a subfolder of ~/Library/Application Support.The archiving is done using the HIArchive mechanism.

SeeMyFriend is using the Model-View-Controller paradigm in a Carbon application and through the use of an Controller object written in Objective-C (SMFWindowController). This object is managing the content of the main window, as well as interaction with SyncServices.

This sample was built using XCode 2.4 running under Mac OS X 10.4.8. The sample has been tested under Mac OS X 10.4 and later. Open the project file and choose Make under the Project menu. This will automatically build the application sample.

Please send all feedback about this sample to 
http://developer.apple.com/contact/feedback.html