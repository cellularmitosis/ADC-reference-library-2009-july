ReadMe.txt

IKImageBrowserViewWithCoreData, as the title suggests, shows one approach to using the browser view from the Image Kit framework with content managed by Core Data. There are several ways this could be accomplished, but the actual objects shown in the browser must implement the required methods in the IKImageBrowserItem informal protocol (http://developer.apple.com/documentation/GraphicsImaging/Reference/IKImageBrowserItem_Protocol/IKImageBrowserItem_Reference.html).

This sample also shows how to subclass NSArrayController to overcome certain limitations in how the browser observes its content objects.

Overview:

The application is a very simple form of an address book, in which a gallery of images is used to browse to locate a person, and the fields below show the details about the person selected in the browser. "+" and "-" buttons all the user to add and remove people from the application. The image for the person must be set using the image well in the bottom right corner. Also, a slider control allows the user to manipulate the zoom factor of the browser.

Core Data:

The Core Data model for the project contains a single entity, "Person". The person has a number of string attributes which are not particularly important to the application. The imageRepresentation attribute, of type binary data, is the important attribute. The name of the attribute matches the IKImageBrowserItem method exactly.

Code:

The Person entity has a custom NSManagedObject subclass in order to implement the other required methods in the IKImageBrowserItem protocol. Since we are returning binary data for the image, the imageRepresentationType is "IKImageBrowserNSDataRepresentationType" - if you chose to return a different representation type, you will need to update this method as well. Finally, the imageUID must be a unique string. In Core Data, one common approach for getting a unique identifier for an entity is to use the NSManagedObjectID of the entity, and retrieve a string description of its URI representation. This could be achieved in other ways, but this is a simple and relatively common pattern for this kind of problem.

The AppDelegate class is mostly the boilerplate provided by the Xcode template for Core Data Application. The additions are provided to enable the slider in the user interface to control the zoom factor in the browser. This is mostly achieved through bindings, but the browserZoom property provides the necessary model and the value is initialized in awakeFromNib.

ImageBrowserViewArrayController is a subclass of NSArrayController that provided Key-Value Observing functionality necessary to ensure that the browser view properly updates when the imageRepresentation for any of its content objects is changed. By itself, the browser will update when the content itself changes, but does not observe any of properties of the items themselves. This could be implemented in a number of different ways - the essential thing is that reloadData is invoked on the browser following any relevant updates.

Interface Builder:

The user interface is contained in the window provided with the MainMenu nib in the project template. Important additions include the array controller, the browser view, add/remove buttons, a slider, a form, and an image view. The interface is a type of master-detail design; the browser, the buttons, and the slider are the "master", and the form and the image view are the "detail". For each major element, only alterations from its default state (when dragged from IB's library) are described.

ImageBrowserViewArrayController
    Attributes
        Mode:                   Entity
        Entity Name:            Person
        Prepares Content:       Checked
    Bindings
        Managed Object Context: AppDelegate.managedObjectContext
    Identity
        Class:                  ImageBrowserViewArrayController
    Connections
        browser:                IKImageBrowserView
    
IKImageBrowserView
    Bindings
        Content:                ImageBrowserViewArrayController.arrangedObjects
        Selection Indexes:      ImageBrowserViewArrayController.selectionIndexes
        Zoom Factor:            AppDelegate.zoomFactor

NSButtons
    Attributes
        Title:                  (blank)
        Image:                  NSAddTemplate and NSRemoveTemplate
    Connections
        Action:                 ImageBrowserViewArrayController.add and remove

NSSlider:
    Attributes
        State-Continuous:       Checked
    Bindings
        Value:                  AppDelegate.zoomFactor
        
NSForm
    Attributes
        Rows:                   4 (labels renamed, resize, etc.)
    Bindings of Cells
        Value:                  ImageBrowserViewArrayController.selection.firstName, etc.
        
NSImageView
    Attributes
        Editable:               Checked
    Bindings
        Data:                   ImageBrowserViewArrayController.selection.imageRepresentation
        
        
Copyright (C) 2008, Apple Inc.