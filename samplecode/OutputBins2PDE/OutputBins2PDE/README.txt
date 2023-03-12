OutputBins2PDE demonstrates how to write a Cocoa PDE that supports all of the features of Mac OS X 10.5, while also running on Mac OS X 10.4. It demonstrates how to write a complex Cocoa Printer PDE, demonstrating the following features that are new in Mac OS X 10.5:

1. Resizing the printer dialog
2. Overriding the Help and OK buttons
3. Automatic PPD Constraint handling

When running on Mac OS X 10.4 the PDE automatically falls back to a reduced feature set.

To be fully compatible with applications that take advantage of all features of Mac OS X 10.5, you must build your PDE 4 way universal, that is for the ppc, ppc64, i386 and x86_64 architectures. Additionally Printer PDEs need to support running in a Garbage Collected environment. We highly recommend that you test your PDEs thoroughly on Mac OS X 10.4 to ensure that this does not cause your PDE to malfunction, as the any GC setting other than "unsupported" will change the generated code in ways that may not be compatible with Mac OS X 10.4 under certain situations. In general however, this should not be a problem.

To install the OutputBins2PDE you need to do the following:
1) Copy the PPD (OutputBins2Test.ppd) to /Library/Printers/PPDs/Contents/Resources/ (on Mac OS X 10.5) or /Library/Printers/PPDs/Contents/Resources/en.lproj/ (on Mac OS X 10.4)
2) Create the folder /Library/Printers/Sample/
3) Copy the build result OutputBins2PDE.bundle to /Library/Printers/Sample/
4) Create a printer that uses the OutputBins2Test.ppd, it will be named "OutputBins 2 PDE test" in the PPD list.

Now when you print to that printer you should see OutputBins and Password PDEs in the list of PDEs (on Mac OS X 10.5 they will be listed in the Vendor PDE section).

<Garbage Collection Programming Guide> http://developer.apple.com/documentation/Cocoa/Conceptual/GarbageCollection/Introduction.html

Project Configuration Details

Configuring the Targets
Much of the configuration for the OutputBins2PDE target is in the OutputBins2Config.xcconfig config file. If you open the Target Info for each of these targets, goto the Build tab and check the Based On popup menu at the bottom, you will see that both Debug & Release builds are based on this config file.
This structure allows most of the build settings for each configurations to be kept in sync automatically. Editing the xcconfig files to set a build setting automatically sets the build setting both configurations. Any settings you wish to keep specific to a particular target or configuration can still be set in the Build settings for that target.
The biggest reason for using a config file is that it allows you to specify settings that are not exposed at the GUI level. We take advantage of  architecture specific build settings to set the deployment target for our 32-bit targets to Mac OS X 10.4, while our 64-bit targets use a deployment target of Mac OS X 10.5.
You can easily extend the xcconfig files by using copy & paste to edit the xcconfig. Any setting from the Build tab can be exported to an xcconfig by copying the setting from the Target Info window and pasting it into your xcconfig file.