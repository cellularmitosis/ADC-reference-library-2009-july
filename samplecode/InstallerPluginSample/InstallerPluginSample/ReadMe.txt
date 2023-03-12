1. What is an Installer Plugin?
	An Installer Plugin extends the user experience of the Installation process by allowing developers to insert additional custom steps in the installation process
	of their software packages.  In the sample code provided here, a registration pane is built that asks users for registration information before installing.
	
	
2. Using the Sample Installer Plugins
	(1) Take a pre-existing package (or create a new one with PackageMaker.) The sample package "InstallerPluginsTestPackage.pkg" is provided for your test use.
	(2) Create a "Plugins" directory in that Package bundle: InstallerPluginsTestPackage.pkg/Contents/Plugins
	(3) Build the SamplePlugins project
	(4) Copy the bundle created by the SamplePlugins project into the Plugins directory of your package.
	(5) Copy the "InstallerSections.plist" file (it can be found in the same directory as this ReadMe file) into the Plugins directory of your package. The "InstallerSections.plist" defines the order that the Installer section pane will be presented to the user, and where the Registration pane will appear (following the "License" pane.)
	(6) Open the package -- it will run with the additional custom Registration plugin.
	
	
3. Creating Your Own Custom Installer Plugin
3A. Getting Started
	- Use the provided plugin as your starting point for development.
	- Custom Installer Plugins must be written in Objective-C Cocoa.  They may not be written in Carbon or Java.
	- Review the InstallerPlugins.framework APIs.  These can be found at /System/Library/Frameworks/InstallerPlugins.framework/Headers/.
	- Copy and update the provided InstallerSections.plist file to include your plugin(s), and place them in the appropriate location in the Install sequence.
	- Contact the Mac OS X Installation Technology Engineering team with questions by subscribing to the installer-dev mailing list (email contact information
	  is provided in the 'Notes' section.)
	
3B. Code Flow in the Plugin
	- The plugin's entry point to is the "- (void)didEnterPane:(InstallerSectionDirection)dir" method.
	- When the user clicks the "Continue" button, the "- (BOOL)shouldExitPane:(InstallerSectionDirection)dir" method is called.  Returning "NO" from this method
	  prevents the user from leaving your plugin page.
	
	
4. Notes
	Plugins are only supported in Mac OS X Tiger (v10.4) and later.
	
	This project was built using Xcode 2.2.1. Xcode 2.3 will also open the project file. The Xcode Tools v2.3 can be downloaded from the Apple Developer
	Connection site <http://developer.apple.com>, once you have logged in.
	
	Contact:	Subscribe to the installer-dev mailing list at the Apple Mailing Lists web page <http://lists.apple.com/mailman/listinfo>.
			Select the "Installer-dev" list item and follow the steps on "Subscribing to Installer-dev".
	
	Documentation:	http://developer.apple.com/documentation/DeveloperTools/Conceptual/SoftwareDistribution/index.html


[Document revision 1.0.2  Last revised 13 June, 2006]
