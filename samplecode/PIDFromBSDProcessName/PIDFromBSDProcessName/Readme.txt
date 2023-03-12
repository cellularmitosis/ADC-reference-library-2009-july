Read Me About PIDFromBSDProcessName

1.0

*Description: This sample gives developers an simple API which allows lookup of Process PID based on BSD process name.  This also allows developers at the UNIX level to determine if a process is running or not.

*What the sample does by default: Returns a list of PIDs for predefined processes like the Finder and init

*Packing List:
• GetPID.h — Header to the PID lookup API. 
• GetPID.c — Source to the PID lookup API.  This does all the work of looking up the process in the process list and returning the PID for the process.
• main.c — The main file in the program and a demonstration file showing how to lookup processes PIDs based on Process Name.       
• PIDFromBSDProcessName.pbproj — The project builder project file.
• Readme.txt — This file.

*Sample Requirements:

For ProjectBuilder users: This project was built with ProjectBuilder Jaguar version as a standard tool.  This project relies on no frameworks and only the underlying BSD subsystem.

*Building the Sample:

Using Project Builder:  To build the sample simply open the Project builder file and hit the 'build' button.  Similary the sample can be run simply by clicking the 'run' button.

*Credits and Version History:

If you find any problems with this sample or have any suggestions, mail <dts@apple.com> with “Attn: Chad Jones” as the first line of your mail.

Version 1.0 is the first release.

Chad Jones
Apple Developer Technical Support
Networking, Communications, Hardware

Feb 10, 2003
