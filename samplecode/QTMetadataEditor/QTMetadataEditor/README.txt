README:

This sample code is leopard only.

The goal of this sample application are to illustrate how to read and write QT Metadata at various levels of a QT Movie, and to provide a handy tool with which to inspect metadata on QT movies.

To learn about how to read/write the QT metadata, look at MyDocument.h/m which specifies how to read/write the metadata from/to the file, and how to iterate through QT Movie containers.

The application as 2 windows per QT Movie: a browser, and an inspector. The browser looks at each "document" opened, and provides and interface to look at multiple pieces of metadata at once. The Inspector provides a break-out location to more closely inspect or modify a single piece of metadata.

Since we use a run-time Core Data database to back our application for each 'document' (QTMovie), we really have 2 document types that we're dealing with:the run-time database document, and the QTMovie. Since we don't want to open or save off the run-time database, we instead open and save to the movie document, essentially by doing an import of the metadata in the QTMovie into the run-time database, then export back to the QTMovie. These two document formats are MyDocument.h/m and MovieDocument.h/mm* respectively, with the MyDocument class being the core support behind the browser window. (The MovieDocument class starts with the sample sample code from QTAudioContextInsert but removes all controllers or UI.)

EditMetdataController.h/m describe and control the inspector panel, with MySplitViewWithDropSupport handling some special drag/drop functionality in that panel.

MyDocument.scdatamodel and MetadataAttributeMO.h/m describe the Metadata Attributes in the Core Data database.
