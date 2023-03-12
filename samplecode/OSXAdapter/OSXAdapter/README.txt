OSXAdapter
v 2.0

This sample uses a reflection Proxy model to hook existing preferences, about, and quit functionality into handlers for the Mac OS X application menu.  This functionality is easily adopted by implementing the com.apple.eawt.ApplicationListener interface, but this sample's dynamic implementation will only be triggered on platforms that actually support the Apple APIs (e.g. Java 1.4 or later on Mac OS X), avoiding any compatibility concerns.

The sample also supports document handing from the Finder by implementing the handleOpenFile method and registering for supported file types in its Info.plist file.  This is how Mac OS X knows to allow drags of certain file types over a given application.  Try opening an image from the file menu, then drag another image over the application's icon in the Dock.  This feature will only work with a bundled Mac OS X application.

This sample is for developers looking to support multiple platforms, including support for Mac OS X-specific features, with a single codebase.  It is written for and should build and run on any J2SE 1.4 or later implementation without any stub or placeholder libraries.

To run the included pre-built sample from a command line, type

java -jar OSXAdapter.app/Contents/Resources/Java/OSXAdapter.jar

The sample is built around an ant project which can be reused on any platform with ant installed.  Be sure to note the "package" subtask which produces a working Mac OS X application bundle and can be integrated into an existing script.

[05/29/2007]