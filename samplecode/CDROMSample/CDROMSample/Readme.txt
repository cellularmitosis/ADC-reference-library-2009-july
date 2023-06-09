CDROMSample
Command-line tool demonstrating how to use IOKitLib to find CD-ROM media mounted on the system. It also shows how to open, read raw sectors from, and close the drive.

Version: 1.3 10/17/2002

Techniques shown are:
- Finding ejectable CD-ROM media
- Locating the BSD /dev/rdisk* node name corresponding to that media 
- Opening the /dev/rdisk* node
- Retrieving the media's preferred block size
- Reading a sector from the media 
- Closing the device 

On versions of Mac OS X prior to 10.1 this sample must be run with root privileges. This is because the /dev/rdisk* nodes were owned by root in those releases. Starting with Mac OS X 10.1 /dev/*disk* nodes for removable media are owned by the currently logged in user. Note that nodes for non-removable media are still owned by root. 

Includes Project Builder 2.1 and Xcode 2.1 projects.

Version: 1.4 8/17/2005

- Updated to produce a universal binary.
- Use kIOMasterPortDefault instead of older IOMasterPort function.
