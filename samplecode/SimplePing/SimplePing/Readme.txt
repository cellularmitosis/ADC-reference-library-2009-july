Read Me About SimplePing

1.0.1

*Description: This sample gives developers an simple API which allows developers to ping a remote host for testing connectivity and responses from the host.  Note this sample require MacOSX 10.2.x since only then did ping become available to non-root programs.  In MacOSX 10.2.x a ping can be sent using a datagram socket rather than a raw socket which is what we do here.

*What the sample does by default: This sample will ping www.google.com by default and print out the replies and round trip times for the pings.  Passing a second argument to the tool will cause the tool to ping the host which is typed in rather than the default google.com.

*Packing List:
• SimplePing.h — Header to the SimplePing API. 
• SimplePing.c — Source to the SimplePing.  This does all the work of looking up host, creating the ping packet, sending the packet, waiting for a response and then printing out the results of the ping(s).
• main.c — The main file in the program and a demonstration file showing how to use SimplePing to ping a remote host.       
• SimplePing.pbproj — The project builder project file.
• Readme.txt — This file.

*Sample Requirements:

For ProjectBuilder users: This project was built with ProjectBuilder Jaguar version as a standard tool.  This project relies on no frameworks and uses only the underlying BSD subsystem.  This project requires ProjectBuilder version 2.1 for building the project.

*Building the Sample:

Using Project Builder:  To build the sample simply open the Project builder file and hit the 'build' button.  Similarly the sample can be run simply by clicking the 'run' button.  This project requires ProjectBuilder version 2.1 for building the project.*Credits and Version History:

If you find any problems with this sample or have any suggestions, mail <dts@apple.com> with “Attn: Chad Jones” as the first line of your mail.

Version 1.0.1 updated sample and readme with comment that sample requires Project Builder 2.1 or later build tools.

Version 1.0 is the first release.

Chad Jones
Apple Developer Technical Support
Networking, Communications, Hardware

October 3, 2003

---