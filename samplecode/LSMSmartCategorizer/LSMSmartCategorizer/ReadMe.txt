NOTE: Some of the funcationalities requires an internet connection.

~~~~~~~~~~~~~~~~~~~~~~~~
1. What is this project?
~~~~~~~~~~~~~~~~~~~~~~~~
LSMSmartCategorizer uses Latent Semantic Mapping (LSM) framework to 
categorize news feeds. It instends to provide an example of basic usage 
of LSM framework.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
2. How should I study the source code?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
There are nineteen source files in this project. You should focus on 
five of them: 
LSMClassifier.h/.m 
LSMClassifierResult.h/.m
LSMClassifierResultPrivate.m. 

LSM framework is a Carbon framework. Above five files implement two 
Objective-C classes, which encapsulate all LSM functionalities required 
by this application. And those five files are thoroughly commented. 

Besides those five files, you might also want to look at three routines:
[TrainingWindowController doTrainAndSave]
[EvalWindowController doLoadMap:]
[EvalWindowController processFeedData:fromURL:]
Those three routines are users of LSMClassifier and LSMClassifierResult.

The rest of the source code files are here for the purpose of making
this application complete.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
3. How to use this application?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3.1 Introducation
------------------
Similar to most supervised machine learning technique, the usage of LSM
consists of two parts, training and evaluation. During training stage, 
you provide to the application some news feeds which have already been 
categorized. The LSM framework will create a map based on the data you 
provide. During evaluation stage, you can provide news feeds that the 
application has not seen before. LSM framework will use the trained map
to categorize those feeds for you. 

In this application, you train new maps in Training window, and evaluate
in Evaluation window. You can switch between the two windows using 
"Window" menu.

NOTE: The main purpose of this sample application is to demonstrate the
usage of LMS framework. It is not our best interest, in this application,
to optimize the classification accuracy.

3.2 Training
------------
In Training window, there are two ways you can provide training data,
using a directory hierarchy that contains the news feed files, or using
a property list file that contains URLs to the feeds.

A sample property list file, named "training_rss_categories.plist", is 
included in the project and the application bundle. You can follow its
format to create you own training property list.

If you want to use new feeds stored on your filesystem, you should make 
your training data hierarchy look like:
/my/training/data/directory/
+-- Category1/
    +-- feed1.xml
	+-- feed2.xml
	+-- ...
+-- Category2/
	+-- feed1.xml
	+-- feed2.xml
	+-- ...
+-- ...

You will provide path "/my/training/data/directory/" to the application
as the top level directory. Within that directory, each sub-directory
represents a category which contains all the feeds that belong to that
category.

Once you loaded training data, you can press "Train and Save Map..." button
to train the map and save it to you hard drive for later evaluation.

3.3 Evalutaion
---------------
Once you create a map in Training window, you may use it to categorize 
other feeds in Evaluation window. First thing you need to do is to press
"Load Map..." button to load a map. Once the map is loaded, you will all
existing categories in the outline view. Now you may press "Categorize
Feed File..." to read a feed on your filesystem, or press "Categorize Feed
URL..." to read a feed from a URL. The application will put the feed into
the category to which it thinks the feed belongs. You will see the result
in the outline view. 

If you press "Categorize Feed URL...", there are some pre-populated feed 
URLs to choose from.
