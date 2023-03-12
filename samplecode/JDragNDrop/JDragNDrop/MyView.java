/*
 MyView.java
 JDragNDrop

 Author: Sandeep Parikh (parikhs)
 Created July 26, 2001

 Copyright (c) 2001 by Apple Computer, Inc., all rights reserved.

 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


import com.apple.cocoa.foundation.*;
import com.apple.cocoa.application.*;

public class MyView extends NSView {
    
    // these are the Interface elements to feed information to...
    private NSTextField filename;
    private NSImageView iconwell;
    
    // override constructor to add dragging registration
    public MyView(NSRect frame) {
        super(frame);
        
        // register for drag and drop type - filenames...
        NSMutableArray dragTypesArray = new NSMutableArray();
        dragTypesArray.addObject(NSPasteboard.FilenamesPboardType);
        
        registerForDraggedTypes((NSArray) dragTypesArray);
    }
    
    // draw background and fill in color
    public void drawRect(NSRect rect) {
        NSColor myColor = new NSColor();
        NSBezierPath myBezierPath = new NSBezierPath();
        
        myColor.windowBackgroundColor().set();
        myBezierPath.fillRect(this.bounds());
    }
    
    public String validateDrag(NSDraggingInfo sender) {
        // if the source of the drag operation does not originate from this view...
        if(sender.draggingSource() != this) {
            // get the pasteboard and get the type of item dropped
            NSPasteboard pb = sender.draggingPasteboard();
            String type = pb.availableTypeFromArray(new NSArray(NSPasteboard.FilenamesPboardType));
            
            // if type exists return the string for it else return null
            if(type != null) {
                return pb.stringForType(type);
            }
        }
        return null;
    }

    public int draggingEntered(NSDraggingInfo sender) {
        // if drag operation is valid, let the pasteboard know
        // the app is only copying the type (filename in this case)
        if(validateDrag(sender) != null) {
            return NSDraggingInfo.DragOperationCopy;
        }
        // else return no operation
        return NSDraggingInfo.DragOperationNone;
    }
    
    public int draggingUpdated(NSDraggingInfo sender) {
        // if drag operation is valid, let the pasteboard know
        // the app is only copying the type (filename in this case)
        if(validateDrag(sender) != null) {
            return NSDraggingInfo.DragOperationCopy;
        }
        // else return no operation
        return NSDraggingInfo.DragOperationNone;
    }

    public void draggingExited(NSDraggingInfo sender) {
        // once drag operation exits window, set display to view again...
        this.setNeedsDisplay(true);
    }
    
    public boolean prepareForDragOperation(NSDraggingInfo sender) {
        // if dragging is valid, let the pasteboard know the app is prepared
        if(validateDrag(sender) != null) {
            return true;
        }
        
        return false;
    }
    
    public boolean performDragOperation(NSDraggingInfo sender) {
        // if dragging is valid, let the pasteboard know the app is performing operation
        if(validateDrag(sender) != null) {
            return true;
        }
        return false;
    }
    
    public void concludeDragOperation(NSDraggingInfo sender) {
        
        // create a new NSPasteboard object to confirm type of dropped item
        NSPasteboard pboard = sender.draggingPasteboard();
        NSMutableArray pArray = new NSMutableArray();

        pArray.addObject(NSPasteboard.FilenamesPboardType);

        // check type to make sure it conforms with type you registered
        String type = pboard.availableTypeFromArray((NSArray) pArray);
        
        // if type of item is dropped is File/Folder
        if (type.equalsIgnoreCase(NSPasteboard.FilenamesPboardType)) {
        
            // get the list of all files/folders dropped
            NSArray filenames = (NSArray) pboard.propertyListForType(NSPasteboard.FilenamesPboardType);
            
            // NSFileWrapper is useful for getting the icon of the dropped item
            NSFileWrapper droppedFile = new NSFileWrapper((String)filenames.lastObject(), false);

            // this will set filename to the full path of the dropped item
            filename.setStringValue((String)filenames.objectAtIndex(0));
            
            // this will set the icon to the icon of the file/folder dropped
            iconwell.setImage(droppedFile.icon());
        }
        else {
            NSAlertPanel alert = new NSAlertPanel();
            alert.runAlert("Incorrect Type", "The view has not registered for this drag type", null, null, null);
        }
    }
}
