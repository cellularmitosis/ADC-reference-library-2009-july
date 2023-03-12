SampleUSBAudioPlugin

This project is a  simple USB audio plug-in that performs a lowpass filtering operation on audio streamed
through an audio device.

To specify a specific audio device for this sample code project to use, you will need to modify the project's Info.plist file and pluginInit() routine in SampleUSBAudioPlugin.cpp.
You must modify the IOPropertyMatch key in Info.plist. These values must be changed to match to your device.  Modify the Info.plist entry IOPropertyMatch to match to the hex values of the idVendor and idProduct of your device.  Do the same for vendorID & productID in SampleUSBAudioPlugin.cpp.

You can discover these values by using developer applications like IORegistryExplorer or USBProber.
You will also need to change these values in  SampleUSBAudioPlugin::pluginInit(..).

The target produces SampleUSBAudioPlugin.kext which must be copied to the 
/System/Library/Extensions folder of the boot volume by an administrator using a 
command like (from the project folder):

sudo cp -rf ./build/SampleUSBAudioPlugin.kext /System/Library/Extensions

Your plugin must have the correct ownership (root , wheel):
sudo chown -R root:wheel SampleUSBAudioPlugin.kext/

Then you must notify the operating system that the contents of the kernel extension directory has 
changed by "touching the directory":

sudo touch /System/Library/Extensions

Restart the system.
 
After restarting the system, the plug-in should automatically load when the device
specified in the Info.plist (accessible from the Targets->SampleUSBAudioPlugin ->Get Info -> Properties) 
is attached. Plug-in logging can be observed using Console in /Applications/Utilities 
on the boot volume.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
NOTE:
This project uses the open source header file "AppleUSBAudioPlugin.h" from the AppleUSBAudio project in the Darwin sources.  The open source darwin project is periodically updated. To obtain the most recent version of the AppleUSBAudioPlugin.h header, please visit :

http://www.opensource.apple.com/darwinsource/Current/AppleUSBAudio/AppleUSBAudioPlugin.h

Please note that agreeing to the Apple Public Source License is necessary to access the Darwin sources.

http://www.opensource.apple.com/apsl/
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
