/*
 * File: SOAPParser.java
 *
 * Description: A class that parses the SOAP result from the server.
 *
 * Author: Apple Computer
 *
 * Copyright: © Copyright 2001 Apple Computer, Inc. All rights reserved.
 *
 * Disclaimer:
 *      IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 *      ("Apple") in consideration of your agreement to the following terms, and your
 *      use, installation, modification or redistribution of this Apple software
 *      constitutes acceptance of these terms.  If you do not agree with these terms,
 *      please do not use, install, modify or redistribute this Apple software.
 *
 *      In consideration of your agreement to abide by the following terms, and subject
 *      to these terms, Apple grants you a personal, non-exclusive license, under AppleUs
 *      copyrights in this original Apple software (the "Apple Software"), to use,
 *      reproduce, modify and redistribute the Apple Software, with or without
 *      modifications, in source and/or binary forms; provided that if you redistribute
 *      the Apple Software in its entirety and without modifications, you must retain
 *      this notice and the following text and disclaimers in all such redistributions of
 *      the Apple Software.  Neither the name, trademarks, service marks or logos of
 *      Apple Computer, Inc. may be used to endorse or promote products derived from the
 *      Apple Software without specific prior written permission from Apple.  Except as
 *      expressly stated in this notice, no other rights or licenses, express or implied,
 *      are granted by Apple herein, including but not limited to any patent rights that
 *      may be infringed by your derivative works or by other works in which the Apple
 *      Software may be incorporated.
 *
 *      The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 *      WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 *      WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *      PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 *      COMBINATION WITH YOUR PRODUCTS.
 *
 *      IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 *      CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *      GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *      ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 *      OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 *      (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 *      ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *                              
 * Change History (most recent first):
 *
 */
import com.webobjects.foundation.*;
import org.xml.sax.*;
import org.xml.sax.InputSource;
import org.xml.sax.helpers.DefaultHandler;
import org.apache.xerces.parsers.SAXParser; 
import java.io.StringReader;
import java.util.Enumeration;


public class SOAPParser extends DefaultHandler {

    protected NSMutableDictionary result = new NSMutableDictionary();
    protected NSArray args;
    protected String currentTag;
    
    public SOAPParser() {
    }

    public SOAPParser(NSArray args) { 
        this.args = args;
    }
    
    public NSDictionary results() { 
        return result; 
    }
    
    public static NSDictionary parseOrderResponseDocument(String xmlDocument, NSArray args) {
    
        SOAPParser parser = new SOAPParser(args);
        
        try {
            parser.parseDocument(xmlDocument);
        }
        catch(Exception e) {
        }
        
        return parser.results();
    } 
    
    private void parseDocument(String xmlDocument) throws Exception {

        SAXParser parser = new SAXParser();            
        InputSource source  = new InputSource(new StringReader(xmlDocument));
            
        parser.setContentHandler(this);
        parser.setFeature("http://xml.org/sax/features/namespaces", true);
        parser.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
        parser.setFeature("http://apache.org/xml/features/validation/schema", true);
        parser.parse(source);        
    }
        
    /* Receive notification of character data. */
    public void characters(char[] ch, int start, int length) {
        System.out.println("characters("+ new String(ch, start, length) + ")");
        
        Enumeration enumerator = args.objectEnumerator();
        
        while (enumerator.hasMoreElements()) {
            String item = (String)enumerator.nextElement();
            if (item.equals(currentTag)) {
                result.setObjectForKey(new String(ch, start, length), currentTag);
            }
        }
    }
    
    /* Receive notification of the end of a document. */
    public void endDocument() {
        System.out.println("endDocument()");        
    } 

	/* Receive notification of the end of an element. */
    public void endElement(java.lang.String namespaceURI, java.lang.String localName, java.lang.String qName) {
        System.out.println("endElement(" + namespaceURI + "," + localName + "," + qName + ")");
        System.out.println("-----------------------------------------------------------------------------------");
    }
    
    /* End the scope of a prefix-URI mapping. */
    public void endPrefixMapping(java.lang.String prefix) {
                System.out.println("endPrefixMapping(" + prefix + ")");
    }
    
    /* Receive notification of ignorable whitespace in element content. */
    public void ignorableWhitespace(char[] ch, int start, int length) {
            System.out.println("ignorableWhitespace("+ new String(ch, start, length) + ")");
    }
    
    /* Receive notification of a processing instruction. */
    public void processingInstruction(java.lang.String target, java.lang.String data) {
        System.out.println("processingInstruction(" + target + "," + data + ")");
    }

    /* Receive an object for locating the origin of SAX document events. */
    public void setDocumentLocator(Locator locator) {}

    /* Receive notification of a skipped entity. */
    public void skippedEntity(java.lang.String name) {
        System.out.println("skippedEntity(" + name + ")");
    }
    
    /* Receive notification of the beginning of a document. */
    public void startDocument() {
            System.out.println("startDocument()");
    }

    /* Receive notification of the beginning of an element. */
    public void startElement(java.lang.String namespaceURI, java.lang.String localName, java.lang.String qName, Attributes atts) {
        System.out.println("startElement(" + namespaceURI + "," + localName + "," + qName + ")");
        for (int index = 0; index < atts.getLength(); index++) {
            System.out.println("Attribute name: " + atts.getLocalName(index));
            System.out.println("Attribute type: " + atts.getType(index));
            System.out.println("Attribute value: " + atts.getValue(index));
            System.out.println("Attribute namespace: " + atts.getURI(index));
        }
        
        currentTag = localName;      
    }

    /* Begin the scope of a prefix-URI Namespace mapping. */
    public void startPrefixMapping(java.lang.String prefix, java.lang.String uri) {
    }
}
