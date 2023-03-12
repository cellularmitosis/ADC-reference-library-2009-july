// ======================================================================================================================
//  MyPDFView.m
// ======================================================================================================================


#import "MyPDFView.h"
#import "MyWindowController.h"


static NSRect RectPlusScale (NSRect aRect, float scale);


@implementation MyPDFView
// ============================================================================================================ MyPDFView
// ------------------------------------------------------------------------------------------------------------- drawPage

- (void) drawPage: (PDFPage *) pdfPage
{
	// Let PDFView do most of the hard work.
	[super drawPage: pdfPage];
	
	if ([(MyWindowController *)[[self window] windowController] linkMode] == kLinkEditMode)
	{
		NSArray		*allAnnotations;
		
		allAnnotations = [pdfPage annotations];
		if (allAnnotations)
		{
			unsigned int	count;
			unsigned int	i;
			BOOL			foundActive = NO;
			
			[self transformContextForPage: pdfPage];
			
			count = [allAnnotations count];
			for (i = 0; i < count; i++)
			{
				PDFAnnotation	*annotation;
				
				annotation = [allAnnotations objectAtIndex: i];
				if ([[annotation type] isEqualToString: @"Link"])
				{
					if (annotation == _activeAnnotation)
					{
						foundActive = YES;
					}
					else
					{
						NSRect			bounds;
						NSBezierPath	*path;
						
						bounds = [annotation bounds];
						
						path = [NSBezierPath bezierPathWithRect: bounds];
						[path setLineJoinStyle: NSRoundLineJoinStyle];
						[[NSColor colorWithDeviceWhite: 0.0 alpha: 0.1] set];
						[path fill];
						[[NSColor grayColor] set];
						[path stroke];
					}
				}
			}
			
			// Draw active annotation last so it is not "painted" over.
			if (foundActive)
			{
				NSRect			bounds;
				NSBezierPath	*path;
				
				bounds = [_activeAnnotation bounds];
				
				path = [NSBezierPath bezierPathWithRect: bounds];
				[path setLineJoinStyle: NSRoundLineJoinStyle];
				[[NSColor colorWithDeviceRed: 1.0 green: 0.0 blue: 0.0 alpha: 0.1] set];
				[path fill];
				[[NSColor redColor] set];
				[path stroke];
				
				// Draw resize handle.
				NSRectFill(NSIntegralRect([self resizeThumbForRect: bounds rotation: [pdfPage rotation]]));
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------- transformContextForPage

- (void) transformContextForPage: (PDFPage *) page
{
	NSAffineTransform	*transform;
	NSRect				boxRect;
	
	boxRect = [page boundsForBox: [self displayBox]];
	
	transform = [NSAffineTransform transform];
	[transform translateXBy: -boxRect.origin.x yBy: -boxRect.origin.y];
	[transform concat];
}

#pragma mark -------- event overrides
// ------------------------------------------------------------------------------------------- setCursorForAreaOfInterest

- (void) setCursorForAreaOfInterest: (PDFAreaOfInterest) area
{
	NSPoint		viewMouse;
	BOOL		overDocument;
	
	// Get mouse in document view coordinates.
	viewMouse = [[self documentView] convertPoint: [[NSApp currentEvent] locationInWindow] fromView: NULL];
	overDocument = [[self documentView] mouse: viewMouse inRect: [[self documentView] visibleRect]];
	if (overDocument == NO)
	{
		[[NSCursor arrowCursor] set];
		return;
	}
	
	// Handle link-edit mode.
	if ([(MyWindowController *)[[self window] windowController] linkMode] == kLinkEditMode)
		[[NSCursor arrowCursor] set];
	else
		[super setCursorForAreaOfInterest: area];
}

// ------------------------------------------------------------------------------------------------------------ mouseDown

- (void) mouseDown: (NSEvent *) theEvent
{
	// Defer to super for locked PDF.
	if ([[self document] isLocked])
	{
		[super mouseDown: theEvent];
		return;
	}
	
	// Handle link-edit mode.
	if ([(MyWindowController *)[[self window] windowController] linkMode] == kLinkEditMode)
	{
		PDFAnnotation	*newActiveAnnotation = NULL;
		PDFAnnotation	*wasActiveAnnotation;
		NSArray			*annotations;
		int				numAnnotations, i;
		NSPoint			pagePoint;
		BOOL			newActive;
		
		// Mouse in display view coordinates.
		_mouseDownLoc = [self convertPoint: [theEvent locationInWindow] fromView: NULL];
		
		// Page we're on.
		_activePage = [self pageForPoint: _mouseDownLoc nearest: YES];
		
		// Get mouse in "page space".
		pagePoint = [self convertPoint: _mouseDownLoc toPage: _activePage];
		
		// Hit test for annotation.
		annotations = [_activePage annotations];
		numAnnotations = [annotations count];
		for (i = 0; i < numAnnotations; i++)
		{
			NSRect		annotationBounds;
			
			// Hit test annotation.
			annotationBounds = [[annotations objectAtIndex: i] bounds];
			if (NSPointInRect(pagePoint, annotationBounds))
			{
				PDFAnnotation	*annotationHit;
				
				// A link annotation?
				annotationHit = [annotations objectAtIndex: i];
				if ([[annotationHit type] isEqualToString: @"Link"])
				{
					// We count this one.
					newActiveAnnotation = annotationHit;
					
					// Remember click point relative to annotation origin.
					_clickDelta.x = pagePoint.x - annotationBounds.origin.x;
					_clickDelta.y = pagePoint.y - annotationBounds.origin.y;
					break;
				}
			}
		}
		
		// Flag indicating if _activeAnnotation will change. 
		newActive = (_activeAnnotation != newActiveAnnotation);
		
		// Deselect old annotation when appropriate.
		if ((_activeAnnotation != NULL) && (newActive))
		{
			[self setNeedsDisplayInRect: RectPlusScale([self convertRect: [_activeAnnotation bounds]
					fromPage: [_activeAnnotation page]], [self scaleFactor])];
		}
		
		// Assign.
		wasActiveAnnotation = _activeAnnotation;
		_activeAnnotation = (PDFAnnotationLink *)newActiveAnnotation;
		
		if (newActive)
		{
			// Notification (MyWindowController listens for this).
			if ((wasActiveAnnotation != NULL) && (_activeAnnotation == NULL))
			{
				[[NSNotificationCenter defaultCenter] postNotificationName: @"newActiveAnnotation" object: self 
						userInfo: [NSDictionary dictionaryWithObjectsAndKeys: 
						wasActiveAnnotation, @"wasActiveAnnotation", NULL]];
			}
			else if ((_activeAnnotation != NULL) && (wasActiveAnnotation == NULL))
			{
				[[NSNotificationCenter defaultCenter] postNotificationName: @"newActiveAnnotation" object: self 
						userInfo: [NSDictionary dictionaryWithObjectsAndKeys: 
						_activeAnnotation, @"activeAnnotation", NULL]];
			}
			else
			{
				[[NSNotificationCenter defaultCenter] postNotificationName: @"newActiveAnnotation" object: self 
						userInfo: [NSDictionary dictionaryWithObjectsAndKeys: _activeAnnotation, @"activeAnnotation", 
						wasActiveAnnotation, @"wasActiveAnnotation", NULL]];
			}
		}
		
		if (_activeAnnotation == NULL)
		{
			[super mouseDown: theEvent];
		}
		else
		{
			// Old (current) annotation location.
			_wasBounds = [_activeAnnotation bounds];
			
			// Force redisplay.
			[self setNeedsDisplayInRect: RectPlusScale([self convertRect: [_activeAnnotation bounds] 
					fromPage: _activePage], [self scaleFactor])];
			_mouseDownInAnnotation = YES;
			
			// Hit-test for resize box.
			_resizing = NSPointInRect(pagePoint, [self resizeThumbForRect: _wasBounds rotation: [_activePage rotation]]);
		}
	}
	else
	{
		[super mouseDown: theEvent];
	}
}

// --------------------------------------------------------------------------------------------------------- mouseDragged

- (void) mouseDragged: (NSEvent *) theEvent
{
	// Defer to super for locked PDF.
	if ([[self document] isLocked])
	{
		[super mouseDown: theEvent];
		return;
	}
	
	_dragging = YES;
	
	// Handle link-edit mode.
	if (_mouseDownInAnnotation)
	{
		NSRect		newBounds;
		NSRect		currentBounds;
		NSRect		dirtyRect;
		NSPoint		mouseLoc;
		NSPoint		endPt;
		
		// Where is annotation now?
		currentBounds = [_activeAnnotation bounds];
		
		// Mouse in display view coordinates.
		mouseLoc = [self convertPoint: [theEvent locationInWindow] fromView: NULL];
		
		// Convert end point to page space.
		endPt = [self convertPoint: mouseLoc toPage: _activePage];
		
		if (_resizing)
		{
			NSPoint		startPoint;
			
			// Convert start point to page space.
			startPoint = [self convertPoint: _mouseDownLoc toPage: _activePage];
			
			// Resize the annotation.
			switch ([_activePage rotation])
			{
				case 0:
				newBounds.origin.x = _wasBounds.origin.x;
				newBounds.origin.y = _wasBounds.origin.y + (endPt.y - startPoint.y);
				newBounds.size.width = _wasBounds.size.width + (endPt.x - startPoint.x);
				newBounds.size.height = _wasBounds.size.height - (endPt.y - startPoint.y);
				break;
				
				case 90:
				newBounds.origin.x = _wasBounds.origin.x;
				newBounds.origin.y = _wasBounds.origin.y;
				newBounds.size.width = _wasBounds.size.width + (endPt.x - startPoint.x);
				newBounds.size.height = _wasBounds.size.height + (endPt.y - startPoint.y);
				break;
				
				case 180:
				newBounds.origin.x = _wasBounds.origin.x + (endPt.x - startPoint.x);
				newBounds.origin.y = _wasBounds.origin.y;
				newBounds.size.width = _wasBounds.size.width - (endPt.x - startPoint.x);
				newBounds.size.height = _wasBounds.size.height + (endPt.y - startPoint.y);
				break;
				
				case 270:
				newBounds.origin.x = _wasBounds.origin.x + (endPt.x - startPoint.x);
				newBounds.origin.y = _wasBounds.origin.y + (endPt.y - startPoint.y);
				newBounds.size.width = _wasBounds.size.width - (endPt.x - startPoint.x);
				newBounds.size.height = _wasBounds.size.height - (endPt.y - startPoint.y);
				break;
			}
			
			// Keep integer.
			newBounds = NSIntegralRect(newBounds);
		}
		else
		{
			// Move annotation.
			// Hit test, is mouse still within page bounds?
			if (NSPointInRect([self convertPoint: mouseLoc toPage: _activePage], 
					[_activePage boundsForBox: [self displayBox]]))
			{
				// Calculate new bounds for annotation.
				newBounds = currentBounds;
				newBounds.origin.x = roundf(endPt.x - _clickDelta.x);
				newBounds.origin.y = roundf(endPt.y - _clickDelta.y);
			}
			else
			{
				// Snap back to initial location.
				newBounds = _wasBounds;
			}
		}
		
		// Change annotation's location.
		[_activeAnnotation setBounds: newBounds];
		
		// Force redraw.
		dirtyRect = NSUnionRect(currentBounds, newBounds);
		[self setNeedsDisplayInRect: 
				RectPlusScale([self convertRect: dirtyRect fromPage: _activePage], [self scaleFactor])];
	}
	else
	{
		[super mouseDragged: theEvent];
	}
}

// -------------------------------------------------------------------------------------------------------------- mouseUp

- (void) mouseUp: (NSEvent *) theEvent
{
	// Defer to super for locked PDF.
	if ([[self document] isLocked])
	{
		[super mouseDown: theEvent];
		return;
	}
	
	_dragging = NO;
	
	// Handle link-edit mode.
	if (_mouseDownInAnnotation)
	{
		_mouseDownInAnnotation = NO;
	}
	else
	{
		[super mouseUp: theEvent];
	}
}

// -------------------------------------------------------------------------------------------------------------- keyDown

- (void) keyDown: (NSEvent *) theEvent
{
	unichar			oneChar;
	unsigned int	theModifiers;
	BOOL			noModifier;
	
	// Get the character from the keyDown event.
	oneChar = [[theEvent charactersIgnoringModifiers] characterAtIndex: 0];
	theModifiers = [theEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
	noModifier = ((theModifiers & (NSShiftKeyMask | NSControlKeyMask | NSAlternateKeyMask)) == 0);
	
	// Delete?
	if ((oneChar == NSDeleteCharacter) || (oneChar == NSDeleteFunctionKey))
		[self delete: self];
	else
		[super keyDown: theEvent];
}

// --------------------------------------------------------------------------------------------------------------- delete

- (void) delete: (id) sender
{
	if (_activeAnnotation != NULL)
	{
		PDFAnnotationLink	*wasAnnotation;
		
		wasAnnotation = _activeAnnotation;
		[self setActiveAnnotation: NULL];
		[[wasAnnotation page] removeAnnotation: wasAnnotation];
		
		// Set edited flag.
		[[self window] setDocumentEdited: YES];
	}
}

#pragma mark -------- menu actions
// -------------------------------------------------------------------------------------------------------- printDocument

- (void) printDocument: (id) sender
{
	// Let PDFView handle the printing.
	[super printWithInfo: [NSPrintInfo sharedPrintInfo] autoRotate: YES];
	
	return;
}

// ------------------------------------------------------------------------------------------------------- saveDocumentAs

- (void) copy: (id) sender
{
	// Put PDF and TIFF data on the Pasteboard if no text selected.
	if ([self currentSelection] == NULL)
	{
		NSData		*pageData;
		NSImage		*image;
		
		// Get PDF data for single (current) page.
		pageData = [[self currentPage] dataRepresentation];
		
		// Create NSImage from PDF data.
		image = [[[NSImage alloc] initWithData: pageData] autorelease];
		
		// Types to pasteboard.
		[[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObjects: NSPDFPboardType, NSTIFFPboardType, 
				NULL] owner: NULL];
		
		// Assign data.
		[[NSPasteboard generalPasteboard] setData: pageData forType: NSPDFPboardType];
		[[NSPasteboard generalPasteboard] setData: [image TIFFRepresentationUsingCompression: NSTIFFCompressionLZW 
				factor: 0 ] forType: NSTIFFPboardType];
	}
	else
	{
		// Default behavior (PDFView will handle the text case for free).
		[super copy: sender];
	}
}

#pragma mark -------- accessors
// ----------------------------------------------------------------------------------------------------- activeAnnotation

- (PDFAnnotationLink *) activeAnnotation
{
	return _activeAnnotation;
}

// -------------------------------------------------------------------------------------------------- setActiveAnnotation

- (void) setActiveAnnotation: (PDFAnnotationLink *) newLink;
{
	BOOL		linkChange;
	
	// Change?
	linkChange = newLink != _activeAnnotation;
	
	// Will need to redraw old active anotation.
	if (_activeAnnotation != NULL)
	{
		[self setNeedsDisplayInRect: RectPlusScale([self convertRect: [_activeAnnotation bounds] fromPage: 
				[_activeAnnotation page]], [self scaleFactor])];
	}
	
	// Assign.
	if (newLink)
	{
		_activeAnnotation = newLink;
		_activePage = [newLink page];
		
		// Force redisplay.
		[self setNeedsDisplayInRect: RectPlusScale([self convertRect: [_activeAnnotation bounds] fromPage: _activePage], 
				[self scaleFactor])];
	}
	else
	{
		_activeAnnotation = NULL;
		_activePage = NULL;
	}
	
	if (linkChange)
	{
		// Notification (MyWindowController listens for this).
		[[NSNotificationCenter defaultCenter] postNotificationName: @"newActiveAnnotation" object: self 
				userInfo: NULL];
	}
}

// --------------------------------------------------------------------------------------------------- defaultNewLinkSize

- (NSSize) defaultNewLinkSize
{
	return NSMakeSize(180.0, 16.0);
}

// --------------------------------------------------------------------------------------------------- resizeThumbForRect

- (NSRect) resizeThumbForRect: (NSRect) rect rotation: (int) rotation
{
	NSRect		thumb;
	
	// Start with rect.
	thumb = rect;
	
	// Use rotation to determine thumb origin.
	switch (rotation)
	{
		case 0:
		thumb.origin.x += rect.size.width - 8.0;
		break;
		
		case 90:
		thumb.origin.x += rect.size.width - 8.0;
		thumb.origin.y += rect.size.height - 8.0;
		break;
		
		case 180:
		thumb.origin.y += rect.size.height - 8.0;
		break;
	}
	
	thumb.size.width = 8.0;
	thumb.size.height = 8.0;
	
	return thumb;
}

@end

// -------------------------------------------------------------------------------------------------------- RectPlusScale

static NSRect RectPlusScale (NSRect aRect, float scale)
{
	float		maxX;
	float		maxY;
	NSPoint		origin;
	
	// Determine edges.
	maxX = ceilf(aRect.origin.x + aRect.size.width) + scale;
	maxY = ceilf(aRect.origin.y + aRect.size.height) + scale;
	origin.x = floorf(aRect.origin.x) - scale;
	origin.y = floorf(aRect.origin.y) - scale;
	
	return NSMakeRect(origin.x, origin.y, maxX - origin.x, maxY - origin.y);
}
