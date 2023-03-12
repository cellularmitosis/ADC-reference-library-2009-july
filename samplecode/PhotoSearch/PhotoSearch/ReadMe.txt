PhotoSearch
AppKit Sample Application for the WWDC "Beyond Buttons and Sliders - Advanced Controls in Cocoa" talk.

Requirements:
Leopard

This sample application uses Spotlight to perform a search for images on your computer. The results are displayed in a custom cell, ImagePreviewCell. A custom NSOutlineView subclass is used to automatically add tracking areas to perform automatic rollover highlighting effects inside the cells. A custom DateCell class is implemented that demonstrates how to properly truncate dates based on the cell size. The NSPredicateEditor (introduced in Leopard) is used to create a search rule. The NSPathControl (introduced in Leopard) is used to display a selected path. This demo also has a special menu under the main application named "What's This?" that demonstrates setting custom views to an NSMenuItem.

* CellTrackingRect.h: Defines an abstract NSCell category for implementing tracking areas.
* DateCell.h/.m: A custom cell that can automatically truncate the date shown based on the available size.
* ImagePreviewCell.h/.m: A custom cell that demonstrates custom drawing, hit testing, custom tracking and custom editing.
* MainWindowController.h/.m: The main controller that glues the user interface to the data model.
* SearchItem.h/.m: The data model representation for a search result.
* SearchQuery.h/.m: The data model representation for a search query, which contains an array of SearchItem children.
* TrackableOutlineView.h/.m: A custom NSOutlineView subclass that automatically adds tracking areas for a cell that implements the category defined in CellTrackingRect.h
* CaseInsensitivePredicateTemplate.h/.m: A custom predicate template used by the NSPredicateEditor to have a case insensitive Spotlight search.
* MenuController.h/.m: Hooks up custom views to menu items.
* GameView.h/.m: A custom game view used in the menu.

