README - QTFlattenToHandle

QTFlattenToHandle.c defines functions that illustrate how to use the handle data handler.
A data handler is a component (of type DataHandlerType) that is responsible for reading
and writing a media's data. In other words, a data handler provides data input and output
services to the media's media handler. Originally, QuickTime included a file data handler.
QuickTime version 2.0 introduced the handle data handler (component subtype HandleDataHandlerSubType),
which allows you to play movie data stored in memory rather than in a file.
This sample code shows how to work with the handle data handler.

Here, we will open a movie file and then flatten the movie data into a handle.
Then we will play the movie from the handle. The essential step is to create a
data reference record describing the handle and then pass that record, instead
of an FSSpec record, to FlattenMovieData. To do this, set the flattenFSSpecPtrIsDataRefRecordPtr
flag when calling FlattenMovieData.

Enjoy,
QuickTime Team

