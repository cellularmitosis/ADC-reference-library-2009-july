File: readme.txt

Abstract: ReadMe.txt for SimpleScriptingObjects sample code project

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved. 



This sample is a follow-on to the SimpleScripting and SimpleScriptingProperties samples.  After completing the steps defined in the SimpleScripting sample to set up and create a scriptable application and acquainting yourself with the concepts in the SimpleScriptingProperties sample, you can continue with the steps in this sample to add some objects to an application.  



Step 1:  Setting up

Create a new .sdef file that includes the standard scripting suite and add it to your Xcode project.  Usually, this file will have the same name as your application.  Going along with that convention, the .sdef file in this sample has been named "SimpleScriptingObjects.sdef".  In the contents of that file, enter an empty dictionary that includes the standard AppleScript suite as follows:


<dictionary xmlns:xi="http://www.w3.org/2003/XInclude">

	<xi:include href="file:///System/Library/ScriptingDefinitions/CocoaStandard.sdef" xpointer="xpointer(/dictionary/suite)"/>

		<!-- add your own suite definitions here -->

</dictionary>

The important parts of this definition are as follows:

1. The 'xi' namespace declaration in the opening dictionary element declares that enclosed elements using the 'xi' namespace will follow conventions defined by the XInclude standard.  This will allow us to include the standard definitions.

2. The 'xi:include' element includes the Standard Suite in the .sdef.

NOTE: Prior to Mac OS X 10.5 developers would copy the standard suite from the ScriptingDefinitions sample (http://developer.apple.com/samplecode/ScriptingDefinitions/index.html) directly into their .sdef file.  For backwards compatibility with Mac OS X 10.4, you may continue to use that technique, but moving forward the XInclude technique described above is recommended.




Step 2: Add a starting suite

Add a new suite to your .sdef file at the end of the dictionary just before the closing </dictionary> tag:


	<suite name="Simple Scripting Objects Suite" code="ScOb"
			description="Simple Scripting Objects application specific scripting facilities.">
		<!-- put your application specific scripting suite information here -->
		 
	</suite>

It is inside of this suite definition where you will put your application specific scripting information.  You can add additional suites if you like and use them to group related scripting functionality, but for most purposes one suite should be sufficient.




Step 3: Add an application class to your new script suite.

Here is our new suite with the application class added in:

	<suite name="Simple Scripting Objects Suite" code="ScOb"
			description="Simple Scripting Objects application specific scripting facilities.">
		
		<!-- put your application specific scripting suite information here -->
		
		<class name="application" code="capp" 
					description="Our simple application class." inherits="application">

			<cocoa class="NSApplication"/>

			<property name="weight" code="ScWt" type="real" access="r"
			        description="the weight of all of the items in the application"/>
					
			<property name="value" code="valL" type="real" access="r"
			        description="the value of all of the treasure in the application"/>
					
			<element type="treasure"/>
			
			<element type="trinket"/>
			
			<element type="bucket"/>
			
			<element type="strong box">
				<cocoa key="strongBoxes"/>
			</element>
			
		</class>

		<!-- add your special classes (object definitions) here -->


	</suite>

Note, the application class we have added inherits from the application class defined in the Standard Suite in Skeleton.sdef.  

The application class is the root container class for an scriptable application.  All of the root functionality provided by your application will be contained in this class and the other classes and objects that it contains.

Important points to note here:
  - the suite has a unique four-character code associated with it 'ScOb'. 
  - when selecting four character codes to pair together with terms you would
  like to use in AppleScript, you should always consult the AppleScript Terminology and Apple Event Codes table here:
  http://developer.apple.com/releasenotes/AppleScript/ASTerminology_AppleEventCodes/TermsAndCodes.html
  
  The four character codes you use for your terms you are using should always agree with the four character codes in that table.  If you are introducing new terms, then you should make sure the four character code you are using to represent that term isn't already in use by consulting that table.


  - the <cocoa class="NSApplication"/> part names the Objective-C class where Cocoa will look for the property accessor methods for the properties we define for our custom application AppleScript class.  For every property we define in the scripting definition for our application class, we will define appropriate accessor methods on the category of NSApplication defined in the SimpleApplication.h and SimpleApplication.m files.

  - we have added two properties to the application class, weight and value.  
  
  - we have added four elements to the application class and we have identified them by their type:
  	<element type="treasure"/>
	<element type="trinket"/>
	<element type="bucket"/>
	<element type="strong box">
		<cocoa key="strongBoxes"/>
	</element>
  These definitions define the objects (elements) that our application container will contain.  On the AppleScript side, Cocoa will display the plural form of the elements we are using here when displaying the elements to the Scripter.  For example, the scripter will see that the application contains 'treasures', 'trinkets', 'buckets', and 'strong boxes'.  
  
  - Cocoa will also used the plural name when generating kvc names for calling accessors for these collections.
  
  - 'strong box' is an irregular plural so we have overridden the default name generation provided by Cocoa and specified a name for Cocoa to use when generating KVC compliant names for calling accessors for these collections.




Step 4: Add some additional class definitions to the scripting suite.


Here are the classes added to the application suite for this example:


(a) the 'trinket' class:

		<class name="trinket" code="ScOT"
			description="A leaf object that does not contain other objects.">
			
			<cocoa class="Trinket"/>
			
			<!-- properties -->
			
			<property name="id" code="ID  " type="text" access="r"
				description="The unique identifier.">
				<cocoa key="uniqueID"/>
			</property>
			
			<property name="name" code="pnam" type="text" access="rw"
			        description="the item's name"/>

			<property name="shiny" code="TrSh" type="boolean" access="rw"
			        description="shiny side up!"/>
					
			<property name="weight" code="ScWt" type="real" access="rw"
			        description="the item's weight"/>
					
			<property name="description" code="desc" type="text" access="rw"
			        description="a description of the item"/>
			
		</class>
			
The trinket class is a simple class that has a number of properties.  The id and name properties are necessary for the Cocoa runtime to function properly, and the other properties are there for illustration.  We have identified the Cocoa class implementing this AppleScript class in the '<cocoa class="Trinket"/>' element of this definition.  Cocoa will use this Objective-C class for implementing the operations for this AppleScript class.




(b) the 'treasure' class:

		<class name="treasure" code="ScOR" inherits="trinket"
			description="A leaf object that does not contain other objects.">
			
			<cocoa class="Treasure"/>
			
			
			<!-- properties -->
			
			<property name="value" code="valL" type="real" access="rw"
			        description="the item's value"/>
					
			<property name="metal" code="SMtl" type="preciousmetal" access="rw"
			        description="main constituent metal"/>
			
		</class>

The treasure class is a subclass of the trinket class and it adds two new properties to that definition.  For illustration, we have used a custom enumeration ('preciousmetal') for one of the properties of this class.  We have identified the Cocoa class implementing this AppleScript class in the '<cocoa class="Treasure"/>' element of this definition.  Cocoa will use this Objective-C class for implementing the operations for this AppleScript class.




(c) the 'bucket' class:

		<class name="bucket" code="ScOB"
			description="An object that can contain other objects.">
			
			<cocoa class="Bucket"/>
			
			
			<!-- properties -->
			
			<property name="id" code="ID  " type="text" access="r"
				description="The unique identifier.">
				<cocoa key="uniqueID"/>
			</property>
			
			<property name="name" code="pnam" type="text" access="rw"
			        description="the item's name"/>
			
			<property name="weight" code="ScWt" type="real" access="r"
			        description="the weight of all of the items in the bucket"/>
					
			<property name="value" code="valL" type="real" access="r"
			        description="the value of all of the treasure in the bucket"/>
			
			
			<!-- elements -->
			
			<element type="treasure"/>

			<element type="trinket"/>

			
		</class>

The bucket class is a container class that can contain both trinket and treasure objects.  The id and name properties are necessary for the Cocoa runtime to function properly, and the other properties are there for illustration.  Note that we have declared both the value and the weight properties as 'read only' (access="r").  For illustration, in our implementation those values are calculated as the sum of the weights and values of the trinket and treasure objects contained in our bucket object.  We have identified the Cocoa class implementing this AppleScript class in the '<cocoa class="Bucket"/>' element of this definition.  Cocoa will use this Objective-C class for implementing the operations for this AppleScript class.




(d) the 'strong box' class:

		<class name="strong box" plural="strong boxes" code="ScOS" inherits="bucket"
			description="An object that can contain other objects.">
			
			<cocoa class="StrongBox"/>
			
			<!-- properties -->
								
			<property name="label" code="clbl" type="text" access="rw"
			        description="the label on the outside of the box"/>
			
			<property name="weight" code="ScWt" type="real" access="r"
			        description="the weight of all of the items in the strong box"/>
					
			<property name="value" code="valL" type="real" access="r"
			        description="the value of all of the treasure in the strong box"/>

			<!-- elements -->
			
			<element type="bucket"/>

			
		</class>

The strong box class is a container class and it is a subclass of bucket has all of the properties of bucket, but adds an extra property and it can contain elements of type bucket.  Note that the strong box class provides specialized methods (implemented in Strongbox.m) for the weight and value properties so we can add in the totals from all of the buckets contained in a strong box.  even though those properties have already been defined in the bucket superclass, we have included them in this class definition so we can provide specialized description values for them (important: we have used the same four letter codes for those properties in this definition as the ones we used in the Bucket class definition).  We have identified the Cocoa class implementing this AppleScript class in the '<cocoa class="StrongBox"/>' element of this definition.  Cocoa will use this Objective-C class for implementing the operations for this AppleScript class.






Step 5: Implement the setter and getter methods on a Category of NSApplication.


In this example we have provided the implementation for all of the necessary setter and getter methods for maintaining our application's properties and collections in the SimpleApplication.m file.  Comments in that file provide more information about the implementation.  See the files SimpleApplication.h and SimpleApplication.m for more information.


Step 6: Implement the objects defined by our scripting definition file.


In this example we have built all of our objects on top of the root class 'Element' defined in the files Element.h and Element.m.  This class maintains the id and name properties and it does most of the necessary house keeping operations for integrating with AppleScript and Cocoa.  Comments in that file explain the implementation in greater detail.


Other classes we have defined for this sample are defined in the files Trinket.h/m, Treasure.h/m, Bucket.h/m, and StrongBox.h/m.  Comments in those files provide detailed explanations of those implementations.



Step 7:  Where to next?

Well, now that you have the very basics in hand, you're all ready to start adding scriptability to your application.  But, careful planning before you start adding in scripting features will be well worth your while.  So, please consider reading the following documentation.

- The items listed in the section "Making Your Application Scriptable" on this page are essential reading.  Everyone new to scripting should read through these documents and familiarize themselves with the topics discussed.
http://developer.apple.com/referencelibrary/GettingStarted/GS_AppleScript/index.html

- "Designing for Scriptability in Cocoa Scripting Guide provides a high-level checklist of design issues and tactics:
http://developer.apple.com/documentation/Cocoa/Conceptual/ScriptableCocoaApplications/index.html

- This Scripting Interface Guidelines document provides more detailed information you should consider when adding scriptability to your application: 
http://developer.apple.com/technotes/tn2002/tn2106.html

- The AppleScript terminology and Apple Event Codes document provides a listing of four character codes that area already defined for use with specific terms.  As you are adding terminology to your application you should always check there to see if a four character code has already been defined for a term you would like to use AND to make sure a four character code you would like to use is not already being used by some other terminology. 
http://developer.apple.com/releasenotes/AppleScript/ASTerminology_AppleEventCodes/TermsAndCodes.html

- NSScriptCommand class is the one you use for implementing verbs (aka commands) 
http://developer.apple.com/documentation/Cocoa/Reference/Foundation/Classes/NSScriptCommand_Class/Reference/Reference.html



Step 8:  And after that?

This sample is part of a suite of samples is structured as an incremental tutorial with concepts illustrated in one sample leading to the next in the order they are listed below.

SimpleScripting
	http://developer.apple.com/samplecode/SimpleScripting/
	
SimpleScriptingProperties
	http://developer.apple.com/samplecode/SimpleScriptingProperties/
	
SimpleScriptingObjects (you are here)
	http://developer.apple.com/samplecode/SimpleScriptingObjects/
	
SimpleScriptingVerbs
	http://developer.apple.com/samplecode/SimpleScriptingVerbs/




