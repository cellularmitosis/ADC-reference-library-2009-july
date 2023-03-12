/*
 * File: SOAPBuilder.java
 *
 * Description: A class that builds the SOAP envelope and body.
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
import java.util.Enumeration;
import org.w3c.dom.*;
import org.apache.xerces.dom.DocumentImpl;
import org.apache.xerces.dom.DOMImplementationImpl;
import org.w3c.dom.Document;
import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.Serializer;
import org.apache.xml.serialize.SerializerFactory;
import org.apache.xml.serialize.XMLSerializer;
import java.io.*;

public class SOAPBuilder extends Object {

    public static String makeOrderDocument(NSArray orderList, String customerID) {
        String billMethod = "PO";
        Enumeration enumeration = orderList.objectEnumerator();
        
        try {
            Document doc = new DocumentImpl();
        
            /* Create a SOAP Envelope */
            Element envelope = doc.createElement("SOAP-ENV:Envelope");
                envelope.setAttribute("SOAP-ENV:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
                envelope.setAttribute("xmlns:SOAP-ENV","http://schemas.xmlsoap.org/soap/envelope/");
            
            Element body = doc.createElement("SOAP-ENV:Body");
                envelope.appendChild(body);
                               
            Element orderElement = doc.createElement("m:Order");
                orderElement.setAttribute("xmlns:m","http://wwdc-demo.apple.com/soap/request/");
                body.appendChild(orderElement);

            Element itemElement = doc.createElement("CustomerID");
                itemElement.appendChild(doc.createTextNode(customerID));                              
                orderElement.appendChild(itemElement);            
            
            itemElement = doc.createElement("BillMethod");
                itemElement.appendChild(doc.createTextNode(billMethod));
                orderElement.appendChild(itemElement);
                
            Element array = doc.createElement("SOAP-ENC:Array");
                array.setAttribute("SOAP-ENC:Array","xsd:orderType["+ String.valueOf(orderList.count()) +"]");
                orderElement.appendChild(array);
            
            while (enumeration.hasMoreElements()) {    
                Order orderItem = (Order)enumeration.nextElement();

                Element arrayItem = doc.createElement("OrderItems");
                    array.appendChild(arrayItem);

                itemElement = doc.createElement("ItemNumber");                              
                    arrayItem.appendChild(itemElement);            
                    itemElement.appendChild(doc.createTextNode(orderItem.itemNumber()));
                
                itemElement = doc.createElement("Quantity");                              
                    arrayItem.appendChild(itemElement);            
                    itemElement.appendChild(doc.createTextNode(String.valueOf(orderItem.quantity())));    
            }
            
            doc.appendChild(envelope);

            StringWriter stringOut = new StringWriter();
            XMLSerializer serial = new XMLSerializer(stringOut, new OutputFormat(doc));
        
            serial.asDOMSerializer(); 
            serial.serialize(doc.getDocumentElement());

            System.out.println( "\n\n\nXML SOAP Document:\n\n" + stringOut.toString() + "\n");
            return stringOut.toString();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

}
