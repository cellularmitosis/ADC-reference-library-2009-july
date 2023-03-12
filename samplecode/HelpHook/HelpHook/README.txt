HelpHook

This is a very simple example of integrating a J2SE application with the Apple Help Viewer application.  A simple Cocoa library calls [NSApplication showHelp], which allows HelpViewer to inspect the application bundle for help content.  Also critical are the CFBundleHelpBookFolder and CFBundleHelpBookName keys, added to the application's Info.plist dictionary.  (see the HelpHook target in Xcode).

The call to [NSApplication showHelp] is made asynchronously to prevent deadlocks between AWT and AppKit.  A badShowHelp function is also written that blocks AWT against this call which, under certain circumstances, can lead to a deadlock.  See the comments in HelpHook.m for more details.