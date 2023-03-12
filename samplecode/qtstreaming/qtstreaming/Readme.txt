GETTING STARTED

	Projects in this folder expect to find the QuickTime Mac
	OS SDK and QuickTime Win32 SDK (that is, the QuickTime
	header files and libraries) in the folders QTDevMac and
	QTDevWin, respectively, both of which are expected to be in
	the parent of the parent of the parent of the folder
	containing the .mcp file. If you have placed the required
	header files and libraries elsewhere, you will need to change
	the access paths of the projects.
	

QUICKTIME STREAMING SAMPLE CODE

	This folder contains sample code for QuickTime Streaming
	RTPMediaPacketizer components and RTPReassembler components.
	These components packetize or reassemble multimedia data that
	is streamed over RTP.


Sample Code Organization

	The sample code is organized into two Metrowerks CodeWarrior
	Pro 3 projects, one for Component Video RTP components and
	one for IMA Audio RTP components.  Each project defines four
	targets:
	
		- a Mac OS packetizer component ('thng') file;
		
		- a Mac OS reassembler component file;
		
		- a Win32 packetizer DLL; and,
		
		- a Win32 reassembler DLL.


Building Win32 Component (.qtx) Files

	The .dll files need to be converted into QuickTime component (.qtx)
	files. In general, you do this by first creating the .dll file, then
	building the QTML resource (.qtr) file using the Rez tool, and then
	combining the .dll file and the .qtr file into a QuickTime component
	(.qtx) file using the RezWack tool. 
	
	If you are using CodeWarrior on the Windows platform, you can build
	the .dll file using CodeWarrior and then build the .qtr and the
	.qtx files using the batch file (.bat) provided (for example,
	RTPMPIMAAudio.rez.bat).
	
	If you are using CodeWarrior on the MacOS platform, you can build
	the .dll file using CodeWarrior, but you must build the .qtr and the
	.qtx files on Windows, using the batch file (.bat) provided. Here's
	what you need to do:
	
		(1) Build the .dll file using CodeWarrior on the MacOS.
		(2) Copy the .dll file, the Headers folder, and all the .r files
		in the Sources folder to a Windows machine. Put them into a folder
		that is a child of a sibling of the QTDevWin folder. (In other words,
		put those files into a folder in the WinSampleCode folder that is a 
		sibling of the QTDevWin folder.)
		(3) Execute the batch files (for example, RTPMPIMAAudio.rez.bat). The
		batch file will build the .qtr and .qtx files.
