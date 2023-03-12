/*
 * File: Main.java
 *
 * Description: Java file associated with Main.wo
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
import com.webobjects.appserver.*;
import com.webobjects.eocontrol.*;
import com.webobjects.eoaccess.*;

import org.w3c.dom.*;
import java.util.Random;
import java.lang.Math;

public class Main extends SOAPComponent {
    private final int FIRST_NODE = 0;
    private final int ORDER_ITEM = 0;
    private final int QUANTITY = 1;
    protected String confirmationNumber = null;
    protected String channel = "WebService fullfilment";
    protected String faultCode = null;
    protected String faultString = null;
    protected Customer customer = null;
    protected Order order = null;


    public Main(WOContext context) {
        super(context);
        
        /* Create a unique confirmation number. */
        Random rand = new Random();
        confirmationNumber = "E" + String.valueOf(Math.abs(rand.nextInt()));
    }

    public void awake() {
        super.awake();
        
        Customer customer = null;
        Order newOrder = null;
        
        /* Pull out arguments from the DOM Document */ 
        NodeList orderList = doc.getElementsByTagName("OrderItems");
        String customerID = doc.getElementsByTagName("CustomerID").item(FIRST_NODE).getFirstChild().getNodeValue();
        String billMethod = doc.getElementsByTagName("BillMethod").item(FIRST_NODE).getFirstChild().getNodeValue();

        try {
            /* Check if that customer exists. */
            NSDictionary dict = new NSDictionary(customerID, "customerID");
            customer = (Customer)EOUtilities.objectMatchingValues(getEC(), "Customer", dict);
        }
        catch (EOObjectNotAvailableException e) {
            /* Customer does not exist. */
            faultCode = "1000";
            faultString = "Unknown Customer";
            return;
        }
                
        /* Create a new order */
        order = (Order)EOUtilities.createAndInsertInstance(getEC(), "Order");
        order.setToCustomer(customer);
        order.setOrderNumber(confirmationNumber);
        order.setMarketChannel(channel);
       
        for (int i = 0; i < orderList.getLength(); i++) {
            OrderItems orderItems = (OrderItems)EOUtilities.createAndInsertInstance(getEC(), "OrderItems");
            NodeList itemList = orderList.item(i).getChildNodes();

            orderItems.setItemNumber(itemList.item(ORDER_ITEM).getFirstChild().getNodeValue());
            orderItems.setQuantity(new Integer(itemList.item(QUANTITY).getFirstChild().getNodeValue()));            
            orderItems.setToOrder(order);
        }
        
        getEC().saveChanges();
    } 
    
    public String getFaultCode() { return faultCode; }
    public String getFaultString() { return faultString; }
}
