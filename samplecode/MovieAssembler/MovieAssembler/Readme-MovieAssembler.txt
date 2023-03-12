MovieAssembler

Requires: Mac OS X 10.4.  Final Cut Pro 5.1.2 or later. Xcode 2.1 or later.

The MovieAssembler sample application demonstrates the use of several new capabilities in Final Cut Pro 5.1.2:

- AppleEvent commands to communicate with and control Final Cut Pro. 
- QuickTime metadata to identify and process movie files. 
- Version 3 of the Final Cut Pro XML Interchange Format to access and modify the contents of sequences in Final Cut Pro project files.  

Once configured properly, MovieAssembler monitors a watch folder for newly copied media files. It uses ID tag(s) stored as metadata to specify if and where a particular media file should be inserted into a selected sequence.

To use the application, first launch Final Cut Pro, version 5.1.2 or later. Then launch MovieAssembler.  Next, use MovieAssembler to open a Final Cut Pro project. (Final Cut Pro opens the same project file automatically.) In the MovieAssembler document window, confirm that the media and watch folder paths are correct. Then choose the sequence in the Final Cut Pro project that should be updated and click the START button to initiate monitoring of the watch folder.  

While MovieAssembler is monitoring the watch folder, it copies any new file placed in that folder into the media folder. It then examines the new file, checking to see if it is a QuickTime movie file and contains the required metadata.  If the file is a QuickTime movie file and if it has a clip ID stored as metadata, MovieAssembler searches the selected sequence for clip(s) with the same ID.  If it finds a match, it updates those clips to point to the new media file in the media folder.  If no match is found, it leaves the sequence unmodified and the new file in the media folder.

The log field in the document window displays entries for relevant MovieAssembler actions.

Note: An example project and a few small media files are included in this package under the ExampleFiles subdirectory. To exercise the application using this media, launch the software, open the project file, and drag one of the movie files in ExampleFiles/AdditionalMedia into ExampleFiles/Project/Incoming.


Notable Methods/Code Snippets:

openProjectInFinalCutPro: This method illustrates sending a simple Apple event to Final Cut Pro, complete with a project URL as an argument.

refreshCachedProjectData: This method requests an XML representation of the specified project. It then uses the support for XQuery in the NSXML classes to enumerate the sequences present, as well as to try to infer a folder where media files are stored.

readClipIDFromFile: This method is a basic implementation of the search and retrieval of metadata in a QuickTime movie file.  This method exercises the standard QuickTime metadata and properties APIs introduced in Quicktime 7.0, and supported in Final Cut Pro 5.1.2 and later.

processFileForProject: This method is called when a new file is detected in the watch folder.  After requesting the clip ID (using the readClipIDFromFile: method), it requests the latest XML representation of the selected sequence, searches it for clips that match the clip ID, and (when a match is found) replaces their media file references with the new file. This method also has code for dealing with the disappearance of shared file references, when more than one clip in the sequence uses an existing media file.

