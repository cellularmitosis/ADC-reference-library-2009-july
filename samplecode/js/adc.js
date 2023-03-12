<!--
function placeWatermark() {
    if (document.layers) {
        document.watermark.pageX = (window.innerWidth - document.watermark.document.myImage.width)/2;
        document.watermark.pageY = (window.innerHeight - document.watermark.document.myImage.height)/2;
        document.watermark.visibility = 'visible';
    }
}

function closeWatermark() {

	if(document.all){
		watermark.style.visibility = "hidden";
	} else if(document.layers) {
		document.watermark.visibility = "hidden";
	} else if(document.getElementById && !document.all) {
		document.getElementById("watermark").style.visibility = "hidden";
	}

}

function visByClass(className, state)
{
	if (document.getElementsByTagName) { //check for obj
		 var nodes = document.getElementsByTagName("DIV");
		 for (var i = 0;i < nodes.length;i++) {
			var nodeObj = nodes.item(i);
			var attrMax = nodeObj.attributes.length
			for (var j = 0; j < attrMax; j++) {
				if (nodeObj.attributes.item(j).nodeName == 'class') {
					if (nodeObj.attributes.item(j).nodeValue == className) {
						vista = (state) ? 'block'	: 'none';
						nodeObj.style.display = vista;
					 }
				}
			 }
		}
	}
	var nodes = document.getElementsByTagName("SPAN");
		 
	var max = nodes.length
	for (var i = 0;i < max;i++) {
		var nodeObj = nodes.item(i);
		for (var j = 0; j < nodeObj.attributes.length; j++)  {
			if (nodeObj.attributes.item(j).nodeName == 'class') {
				if (nodeObj.attributes.item(j).nodeValue == className) {
					vista = (state) ? 'block'	: 'none';
					nodeObj.style.display = vista;
			 	}
			}
		}
	}
}

// Modify the display state of the CSS style with the given class name 
var foundStylesheet;
function visByClass2(className, state)
{
    var stylesheets = document.styleSheets;
    if( stylesheets && stylesheets.length > 0 ) {
        // Safari sometimes fails to return any stylesheets here when called
        // before the page finishes loading.  Flag that we did find a style sheet
        // so that we can tell if we should repeat the test after the page loads.
        foundStylesheet = 1;
        var rules = stylesheets[0].cssRules;
        if( rules == null ) {
            // IE6/Win uses rules, not cssRules to get style sheet rules
            rules = stylesheets[0].rules;
        }
        for( var i=0; i<rules.length; i++ ) {
            if( rules[i].selectorText.indexOf( className ) != -1 ) {
                var style = rules[i].style;
                vista = (state) ? 'block' : 'none';
                style.display = vista;
                return;
            }
        }
    } else {
        // Falling back to old code
        visByClass(className, state);
    }
}


// Pop A Window
function MM_openBrWindow(theURL,winName,features) { //v2.0
	window.open(theURL,winName,features);
}


// Redirect user with the item selected with a drop-down menu
function redirect(categoryName, selectedItem)
{	
	//window.location.href = "http://www.apple.com";
	//alert(categoryName);
	var q = selectedItem.indexOf("WONoSelectionString"); 
	//alert(q);
	if ( q == 0 ) {
		//user selected the first item, so take them back to the page listing all docs
		//you have to somehow pass in the name of the category
		//alert("../" + categoryName + "/" + categoryName + "-date.html");
		window.location.href = "../" + categoryName + "/" + categoryName + "-date.html";
	} else {
		window.location.href = selectedItem;
	}
}

function busNav(newPage,defaultIndex) {
    newLoc = newPage.options[newPage.selectedIndex].value
    if (newLoc != "") {
            window.location.href = newLoc;
    }
    if( typeof defaultIndex != "undefined" && defaultIndex >= 0 ) {
        newPage.selectedIndex = defaultIndex;
    }
}

// Check whether the descriptions should be hidden based on a cookie
function testDescriptionFlag() {
    var value = getCookie("RefLib_Descriptions");
    if( value && value == 'off' ) {
        //  The default setting is to show descriptions, so we only need to
        //    act if the cookie is set to 'off'
        hideDescriptions(0);
    }
}

// If we failed to find any stylesheet while the page was loading, repeat the test
function retestDescriptionFlag() {
    if( foundStylesheet ) {
        return;
    }
    testDescriptionFlag();
}

//  Turn on the descriptions.  If the needCookie flag is true, set a cookie
//  to remember this flag.
function showDescriptions( needCookie ) {
    visByClass2('results_description', 1);
    if( needCookie ) {
        // Set expiration date to 20 years in the future
        // (Maximum date is ~2038)
        var expires = new Date();
        expires.setTime( expires.getTime() + 1000*60*60*24*365*20 );
        setCookie('RefLib_Descriptions', 'on', expires, '/');
    }
    var onButton = document.getElementById( 'DescriptionsOn' );
    if( onButton ) {
        onButton.checked = 1;
    }
}

//  Turn off the descriptions.  If the needCookie flag is true, set a cookie
//  to remember this flag.
function hideDescriptions( needCookie ) {
    visByClass2('results_description', 0);
    if( needCookie ) {
        // Set expiration date to 20 years in the future
        // (Maximum date is ~2038)
        var expires = new Date();
        expires.setTime( expires.getTime() + 1000*60*60*24*365*20 );
        setCookie('RefLib_Descriptions', 'off', expires, '/');
    }
    var offButton = document.getElementById( 'DescriptionsOff' );
    if( offButton ) {
        offButton.checked = 1;
    }
}


// onLoad event handler to search for links that point to pages that
// have multiple variations each sorted in a different way.  When
// linking to one of those pages, we need to check if the user has a
// cookie set that identifies a preferred page sorting.  fixNavigationLinks
// searches for those links and converts them to javascript calls to
// loadSortedPage (or sortByColumn for page sorting links).
function fixNavigationLinks() {

    // Set action for all navigation links.
    var allLinks = document.links;
    for (var i = 0; i < allLinks.length; i++) {
        
        // Get the current link and check whether it has a sorting attribute.
        var current_link = allLinks[i];
        var sorting_list = current_link.getAttribute("sortings");
        var sorted_att = current_link.getAttribute("sorted");
        if( sorting_list ) {
            
            // This is the kind of link we're looking for, so convert
            // the href to a javascript call.
            var current_href = current_link.getAttribute("href");
            var newJSLink = "javascript:loadSortedPage(\"" 
                                    + current_href + "\",\"" 
                                    + sorting_list + "\")";
            current_link.href = newJSLink;
            
        } else if ( sorted_att ) {
            
            //  This link changes the sorting preference
            var current_href = current_link.getAttribute("href");
            var newJSLink = "javascript:sortByColumn(\"" 
                                    + sorted_att + "\",\"" 
                                    + current_href + "\")";
            current_link.href = newJSLink;
            
        }
        
    }
}

//  Get the user's sort preference, either from a cookie or by looking at
//  the current page's URL and trying to identify it's sort order.
function getSortPreference( sortList ) {
    // Default behavior is to maintain the current sort column, so see
    // if the current page is sorted the same as one of the destination's columns
    var href = window.location + '';  // Convert location into a string object
    var availableSorts = sortList.split(',');
    for( var i=0; i<availableSorts.length; i++ ) {
        var sortStringIndex = href.indexOf( '-' + availableSorts[i] );
        if( sortStringIndex > 0 ) {
            return availableSorts[i];
        }
    }
    
    // Destination page does not have a column that matches the current page's
    // sort order, so check for a cookie identifying a prefered sort order.
    var preferred_sort = getCookie('sortByPreference');
    if( preferred_sort ) {
        return preferred_sort;
    }
    
    return '';
}

//  Based on a user's sorting preference (either from a Cookie or the current page's
//  sort order), load a new page with that sort order, if available.
function loadSortedPage( href, sorting_list ) {
    
    var preferred_sort = getSortPreference(sorting_list);
    if( preferred_sort && sorting_list ) {
        
        //  Both a sorting cookie and a sorting attribute are available.
        //  Check if the preferred sort is in the list of available sorts
        var sortForPage;
        var availableSorts = sorting_list.split(',');
        for( var j=0; j<availableSorts.length; j++ ) {
            if( availableSorts[j] == preferred_sort ) {
                sortForPage = preferred_sort;
                break;
            }
        }
        
        if( sortForPage ) {
            
            //  A sorting match was found.
            //  Check if the href already has the preferred sorting
            
            var sortStringIndex = href.indexOf( '-' + sortForPage );
            if( sortStringIndex == -1 ) {
                
                //  href string points to a different page than the preferred sort,
                //  so construct its URL and load that page.
                
                //  Find which sorting is specified in the href string
                for( var i=0; i<availableSorts.length; i++ ) {
                    
                    var availableSort = availableSorts[i];
                    sortStringIndex = href.indexOf( '-' + availableSort );
                    if( sortStringIndex != -1 ) {
                        
                        //  Found a match.  Swap it out for the preferred sorting.
                        
                        var tailIndex = sortStringIndex + 1 + availableSort.length;
                        var newHref = href.substring(0, sortStringIndex+1)
                                        + sortForPage
                                        + href.substring( tailIndex, href.length );
                        href = newHref;
                        
                    }
                    
                }
            }
            
        }
        
    }
    
    window.location = href;
}

//  Sort by one of the columns.
function sortByColumn(column, href) {
    var cookieName = 'sortByPreference';
    
    // Set expiration date to 20 years in the future
    // (Maximum date is ~2038)
    var expires = new Date();
    expires.setTime( expires.getTime() + 1000*60*60*24*365*20 );
    setCookie(cookieName, column, expires, '/');
    
    window.location.replace(href);
    return false;
}

//  Go to the page identified by the selected popup entry.  If a sortings attribute
//  exists for the entry, use it to load the page with the preferred sorting.
function gotoSelection(popup,defaultIndex) {
    var selectedOption = popup.options[popup.selectedIndex];
    if( selectedOption ) {
        newLoc = selectedOption.value;
        if (newLoc != "") {
            var sorting_list = selectedOption.getAttribute("sortings");
            if( sorting_list ) {
                loadSortedPage( newLoc, sorting_list );
            } else {
                window.location.href = newLoc;
            }
        }
    }
    //  Reset the popup to its original value
    if( typeof defaultIndex != "undefined" && defaultIndex >= 0 ) {
        popup.selectedIndex = defaultIndex;
    }
}

function category_hover(cell) {
	var links = cell.getElementsByTagName("a");
	links.item(0).style.color = "#FF6600";
	links.item(0).style.textDecoration = "underline";
	cell.style.cursor = "pointer";
}
function category_unhover(cell) {
	var links = cell.getElementsByTagName("a");
	links.item(0).style.color = "#0000FF";
	links.item(0).style.textDecoration = "none";
}
function category_descend(cell) {
	var links = cell.getElementsByTagName("a");
	var url = links.item(0).getAttribute("href");
	location.href = url;
}

// Get a cookie
// Code from: http://www.webreference.com/js/column8/functions.html
function getCookie(name) {
    var dc = document.cookie;
    var prefix = name + "=";
    var begin = dc.indexOf("; " + prefix);
    if (begin == -1) {
        begin = dc.indexOf(prefix);
        if (begin != 0) return null;
    } else {
        begin += 2;
    }
    var end = document.cookie.indexOf(";", begin);
    if (end == -1) {
        end = dc.length;
    }
    return unescape(dc.substring(begin + prefix.length, end));
}

// Set a cookie
// Code from: http://www.webreference.com/js/column8/functions.html
function setCookie(name, value, expires, path, domain, secure) {
    var curCookie = name + "=" + escape(value) +
        ((expires) ? "; expires=" + expires.toGMTString() : "") +
        ((path) ? "; path=" + path : "") +
        ((domain) ? "; domain=" + domain : "") +
        ((secure) ? "; secure" : "");
    document.cookie = curCookie;
}

//-->
