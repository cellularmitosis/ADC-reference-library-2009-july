MyMovieFilter v1.1

This sample code is Mac OS X 10.5 Leopard only.

MyMovieFilter is a simple application which demonstrates how to play a movie into a layer-backed QTMovieView and apply a Core Image filter while the movie is playing.

Background

Layer-backed views use Core Animation layers as their backing store. Enabling layer-backing for the view (and its descendants) in this particular sample is done via Interface Builder by setting the Wants Core Animation Layer check box on the Movie View in the Movie View Effects Inspector Panel.

When layer backing is enabled for a view, the view and all its subviews are mirrored by a Core Animation layer tree.  The view and its subviews still take part in the responder-chain, still receive events, and act as any other view. However, when redrawing needs to be done and the content has not changed, the render tree handles the redraw rather than the application.

In addition to providing cached redrawing, layer-backed views expose a number of the advanced visual properties of Core Animation layer properties, including:

	- Control over the view’s opacity
	- An optional shadow, specified using an NSShadow object
	- An optional array of Core Image filters that are applied to the content behind a view before its content is composited
	- A Core Image filter used to composite the view’s contents with the background
	- An optional array of Core Image filters that are applied to the contents of the view and its subviews

This sample takes advantage of the advanced Core Animation layer visual properties and applies a Core Image filter to the contents of the QTMovieView to achieve a nice visual effect.

Running the Application

Launch the application and a document window is automatically displayed showing a preselected movie. Press the Play button and the movie starts playing. Press the Pause button and the movie will stop. A popup menu is provided to allow you to select a Core Image filter to apply to the movie. After launch, the filter default is set to No Filter. Select a filter of your choice while the movie is playing or stopped and the filter will automatically be applied to the movie. You can even change the filter while the movie is playing.

