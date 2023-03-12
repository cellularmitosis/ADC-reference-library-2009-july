Read Me - XML Transport

Example code from WWDC 2001, session 607 - Practical Uses of XML with WebObjects.

This sample performs object serialization and archiving using the WOXMLCoder and WOXMLDecoder classes along with a mapping file.

How to Use the Application:

1. Create a database call 'XMLDemo' using OpenBase Manager. Do not create any tables nor data. Just start the database.

2. Create the schema using EOModeler by opening 'XMLDemo.eomodeld' and executing the SQL generated. Beware, the execution is silent. After a few seconds, inspect the schema using OpenBase Manager again.

3. Create an EO by uploading "data.xml" found in the project directory. Click on "Convert to EO" button. View the created data in OpenBase Manager.

4. Test the creation of new XML file by clicking on "Convert to XML". Use Terminal or TextEdit to look at the file '/tmp/PersonNew.xml'.

Project Requirements

Mac OS X
WebObjects 5 Developer Tools
OpenBase (included with WebObjects 5)
