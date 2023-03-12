/*
 * File: Administration.java
 *
 * Description: Java file associated with Administration.wo
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

public class Administration extends WOComponent {
    /** @TypeInfo java.lang.String */
    protected NSMutableArray orderList = new NSMutableArray();
    protected Order orderItem;

    /** @TypeInfo java.lang.String */        
    protected NSMutableArray productList = new NSMutableArray();
    protected Product product;
    protected Product productSelected;

    /** @TypeInfo java.lang.String */
    protected NSMutableArray vendorList = new NSMutableArray();
    protected String vendor;
    
    /** @TypeInfo java.lang.String */
    protected NSMutableArray departmentList = new NSMutableArray();
    protected String department;
    
    protected int quantity;
    protected double total;
    protected boolean error = false;
    protected String orderCount = "0";

    public Administration(WOContext context) {
        super(context);
        
        /* TODO: Put these items in a table */
          
        /* Populate the list with values represent preferred Vendors */
        
        vendorList.addObject("The Little MacDealer");
        vendorList.addObject("Barry's Office Supplies");
        vendorList.addObject("Red Ball Sporting Goods");
        vendorList.addObject("Andersons Office Furniture");
        
        /* School Department list */
        departmentList.addObject("Math Department");
        departmentList.addObject("English Department");
        departmentList.addObject("Social Sciences");
        departmentList.addObject("History Department");
        departmentList.addObject("Art Department");
        departmentList.addObject("Athletic Department");
        departmentList.addObject("Janitorial Department");
        departmentList.addObject("Administration Department");
        
        /* Product List */
        productList.addObject(new Product("iBook 500-Mhz", "M7710LL/A", 1299.00));
        productList.addObject(new Product("iMac Special Edition 600-Mhz", "M145345/LLA", 1499.00));
        productList.addObject(new Product("iMac 500-Mhz", "M145345/LLA", 1199.00));
        productList.addObject(new Product("iMac 400-Mhz", "M145345/LLA", 899.00));
        productList.addObject(new Product("PowerBook G4 500-Mhz", "M7710LL/A", 3997.00));
        productList.addObject(new Product("PowerMac G4 733-Mhz", "M8451LL/A", 3499.00));
        productList.addObject(new Product("PowerMac G4 533-Mhz", "M7688LL/A", 2199.00));
        productList.addObject(new Product("PowerMac G4 Dual 533-Mhz", "M7688LL/A", 2599.00));
        productList.addObject(new Product("PowerMac G4 466-Mhz", "M7627LL/A", 1699.00));
    }
    
    public Administration addItem()
    {
        error = false;
        
        if (quantity == 0) {
            error = true;
            return null;
        }
        
        Order currentOrderItem = new Order(productSelected, quantity);
        orderList.addObject(currentOrderItem);
        orderCount = String.valueOf(orderList.count());
        return null;
    }

    public Results submitOrder()
    {
        Results nextPage = (Results)pageWithName("Results");
        SOAPTransport soapBus = new SOAPTransport();
        NSArray args = new NSArray("ConfirmationNumber");
 
        boolean status = false;
        
        /* This is where we send our message to the SOAP server. */
        try {
            if (soapBus.sendMessage(SOAPBuilder.makeOrderDocument(orderList, ((Session)session()).pin()))) {
            
                NSDictionary results = SOAPParser.parseOrderResponseDocument(soapBus.output(), args);
                String confirmationNumber = (String)results.objectForKey("ConfirmationNumber");
                if (confirmationNumber != null && !("".equals(confirmationNumber))) {
                    nextPage.setConfirmationNumber(confirmationNumber);
                    status = true;
                }
            }
        }
        catch (Exception e) {
            status = false;
        }
        
        nextPage.setStatus(status);
                
        return nextPage;
    }
    
    public double total() {
        return total += quantity * productSelected.price();
    }
}
