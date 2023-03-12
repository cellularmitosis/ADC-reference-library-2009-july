/*
© Copyright 2006-2007 Apple Inc.  All rights reserved.

IMPORTANT:  This Apple software ("Apple Software") is supplied to you in consideration of your agreement to the following terms. Your use, installation and/or redistribution of this Apple Software constitutes acceptance of these terms. If you do not agree with these terms, please do not use, install, or redistribute this Apple Software.

Provided you comply with all of the following terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in the Apple Software, to use, reproduce, and redistribute the Apple Software for the sole purpose of creating Dashboard widgets for Mac OS X. If you redistribute the Apple Software, you must retain this entire notice in all such redistributions.

You may not use the name, trademarks, service marks or logos of Apple to endorse or promote products that include the Apple Software without the prior written permission of Apple. Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your products that incorporate the Apple Software or by other works in which the Apple Software may be incorporated.

The Apple Software is provided on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

function CreateIndicator(indicatorID, spec)
{
	var indicatorElement = document.getElementById(indicatorID);
	if (!indicatorElement.loaded) 
	{
		indicatorElement.loaded = true;
		indicatorElement.object = new Indicator(indicatorElement, spec.value || 0, spec.onValue || 0, spec.warningValue || 0, spec.criticalValue || 0, spec.imageOff || null, spec.imageOn || null, spec.imageWarning || null, spec.imageCritical || null);
		return indicatorElement.object;
	}
}

function Indicator(indicator, value, onValue, warningValue, criticalValue, imageOff, imageOn, imageWarning, imageCritical)
{
	/* Objects */
	this._indicator = indicator;
	
	/* public properties */
	// These are read-only. Use the setter functions to set them.
	this.value = value;
	
	/* Internal objects */
	this._image = null;
		
	this.imageOffPath = imageOff == null ? "Images/IndicatorOff.png" : imageOff;
	this.imageOnPath = imageOn == null ? "Images/IndicatorOn.png" : imageOn;
	this.imageWarningPath = imageWarning == null ? "Images/IndicatorWarning.png" : imageWarning;
	this.imageCriticalPath = imageCritical == null ? "Images/IndicatorCritical.png" : imageCritical;
		
	this._init(value, onValue, warningValue, criticalValue, imageOn, imageOff, imageWarning, imageCritical);
}

Indicator.prototype._init = function(value, onValue, warningValue, criticalValue, imageOn, imageOff, imageWarning, imageCritical)
{
	// For JavaScript event handlers
	var _self = this;
	
	this.onValue = onValue;
	this.warningValue = warningValue;
	this.criticalValue = criticalValue;
	
	var style = null;
	var element = null;

	// Level Indicator Track
	element = document.createElement("img");
	style = element.style;
	style.height = "100%";
	style.width = "100%";
	this._indicator.appendChild(element);
	this._image = element;
	
	this.refresh();
}


Indicator.prototype.remove = function()
{
	var parent = this._image.parentNode;
	parent.removeChild(this._image);
}

/*
 * refresh() member function
 * Refresh the current level indicator position.
 * Call this to make the level indicator appear after the widget has loaded and 
 * the Indicator object has been instantiated.
 */
Indicator.prototype.refresh = function()
{	
	this._computedIndicatorStyle = document.defaultView.getComputedStyle(this._indicator, '');
	this._setValueTo(this.value);
}

Indicator.prototype._setValueTo = function(newValue)
{	
	this.value = newValue;

	var imagePath = null;

	if (this.value >= this.criticalValue)
		imagePath = this.imageCriticalPath;
	else if (this.value >= this.warningValue)
		imagePath = this.imageWarningPath;
	else if (this.value >= this.onValue)
		imagePath = this.imageOnPath;
	else
		imagePath = this.imageOffPath;
		
	this._image.src = imagePath;
}

// Capture events that we don't handle but also don't want getting through
Indicator.prototype._captureEvent = function(event)
{
	event.stopPropagation();
	event.preventDefault();
}

Indicator.prototype.setValue = function(newValue)
{
	this.value = newValue;
	this.refresh();
}

Indicator.prototype.setOnValue = function(newValue)
{
	this.onValue = newValue;
	this.refresh();
}

Indicator.prototype.setWarningValue = function(newValue)
{
	this.warningValue = newValue;
	this.refresh();
}

Indicator.prototype.setCriticalValue = function(newValue)
{
	this.criticalValue = newValue;
	this.refresh();
}

Indicator.prototype.setImageOff = function(newValue)
{
	this.imageOffPath = newValue;	
	this.refresh();
}

Indicator.prototype.setImageOn = function(newValue)
{
	this.imageOnPath = newValue;	
	this.refresh();
}

Indicator.prototype.setImageWarning = function(newValue)
{
	this.imageWarningPath = newValue;	
	this.refresh();
}

Indicator.prototype.setImageCritical = function(newValue)
{
	this.imageCriticalPath = newValue;	
	this.refresh();
}
