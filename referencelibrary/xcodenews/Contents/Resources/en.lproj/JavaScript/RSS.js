// Use these feeds for deployment purposes.
var blast_feed = {url:"http://developer.apple.com/rss/xcodehotnews.rss"};	
//var news_feed = {url:"#"};
var news_feed = {url:"http://developer.apple.com/rss/xcodenews.rss"};
	
var scrollArea, scrollbar;
var blast_request = null;
var news_request = null;

function load ()
{
	scrollbar = new AppleVerticalScrollbar(document.getElementById("myScrollBar"));
	scrollArea = new AppleScrollArea(document.getElementById("contents"), scrollbar);
	scrollArea.scrollsHorizontally = false;
	scrollArea.singlepressScrollPixels = 15;
	
	scrollArea.focus(); // for key control when first loading in Safari
	
	window.onfocus = function () { scrollArea.focus(); }
	window.onblur = function () { scrollArea.blur(); }
	
	new_request(news_request, news_feed, news_loaded);

}

function load_blast ()
{
	scrollbar = new AppleVerticalScrollbar(document.getElementById("myScrollBar_blast"));
	scrollArea = new AppleScrollArea(document.getElementById("contents_blast"), scrollbar);
	scrollArea.scrollsHorizontally = false;
	scrollArea.singlepressScrollPixels = 15;
	
	scrollArea.focus(); // for key control when first loading in Safari
	
	window.onfocus = function () { scrollArea.focus(); }
	window.onblur = function () { scrollArea.blur(); }
	
	new_request(blast_request, blast_feed, blast_loaded);

}

// new_request sends a request to feed.url, calls xml_callback when done, and stores itself in xml_request
function new_request(xml_request, feed, xml_callback)
{
	if (xml_request != null)
	{
		xml_request.abort();
		xml_request = null;
	}
	xml_request = new XMLHttpRequest();

	xml_request.onload = function(e) {xml_callback(e, xml_request);}
	xml_request.overrideMimeType("text/xml");
	xml_request.open("GET", feed.url);
	xml_request.setRequestHeader("Cache-Control", "no-cache");
	xml_request.send(null);	
}


//---------------------------------------------------------------------------------------------
//
// findChild - scan the children of a given DOM element for a node matching nodeName; much more
//				efficient than the standard DOM methods (getElementsByTagName, etc) if you know
//				what you're looking for
//
//----------------------------------------------------------------------------------------------
function findChild (element, nodeName)
{
	var child;
	
	for (child = element.firstChild; child != null; child = child.nextSibling)
	{
		if (child.nodeName == nodeName)
			return child;
	}
	
	return null;
}

//---------------------------------------------------------------------------------------------
//
// xml_loaded - extract the content of Apple RSS Hot News feed and place the items data into a
//			    results array: extract the title, link and publication date for each item. 
//
//----------------------------------------------------------------------------------------------

function news_loaded (e, request) 
{
	news_request = null;
	if (request.responseXML)
	{	
			
		var results = parse_rss(request.responseXML, document.getElementById("ohnoes"));
		// copy title and date into rows for display. Store link so it can be used when user
		// clicks on title
		nItems = results.length;
		var even = true;		
				
		for (var i = 0; i < nItems; ++i)
		{
			var item = results[i];
			//if (i==0){update_blast(item);}
			var row = createRow (item.title, item.link, item.date, even);
			even = !even;
			
			contents.appendChild (row);
		}

		// update the scrollbar so scrollbar matches new data
		scrollArea.refresh();
	} else {
        document.getElementById("ohnoes").style.display = "block";
	}
}

function blast_loaded (e, request) 
{
	blast_request = null;
	if (request.responseXML)
	{		
		var results = parse_rss(request.responseXML, document.getElementById("ohnoes_blast"));
		
		//document.getElementById("contents_blast").innerHTML = "<a href='" + results[0].link + "' target='_blank'>" + results[0].title + "</a>";
		
		var blast_headline = document.getElementById("contents_blast");
		
		blast_headline.innerText = results[0].title;
		if (results[0].link != null)
		{
			blast_headline.setAttribute('the_link', results[0].link);
			blast_headline.setAttribute('onclick', 'clickOnTitle (event, this);');
		}

	 }else {
        document.getElementById("ohnoes_blast").style.display = "block";
	}
}

function parse_rss(responseXML, error_display)
{

	// Get the top level <rss> element 
	var rss = findChild(responseXML, 'rss');
	if (!rss) {
		// alert("no <rss> element"); 
		error_display.style.display = "block";
		return;
	}
	
	// Get single subordinate channel element
	var channel = findChild( rss, 'channel');
	if (!channel) {
		alert("no <channel> element");
		error_display.style.display = "block";
		return;
	}
	
	error_display.style.display = "none";

	var results = new Array;
	
	// Get all item elements subordinate to the channel element
	// For each element, get title, link and publication date. 
	// Note that all elements of an item are optional. 
	for( var item = channel.firstChild; item != null; item = item.nextSibling)
	{
		if( item.nodeName == 'item' ) 
		{
			var title = findChild (item, 'title');
		
			// we have to have the title to include the item in the list 
			if( title != null ) 
			{
				var link = findChild (item, 'link');
				var pubDate = findChild (item, 'pubDate');
				results[results.length] = {title:title.firstChild.data, 
					link:(link != null ? link.firstChild.data : null), 
					date:new Date(Date.parse(pubDate.firstChild.data))
			   };
			}
		}
	}
	
	// sort by date
	results.sort (compFunc);	
	
	return results;
}


//---------------------------------------------------------------------------------------------
//
// sortFunc - compare function used for sorting dates
//
//----------------------------------------------------------------------------------------------

function compFunc (a, b)
{
	if (a.date < b.date)
		return 1;
	else if (a.date > b.date)
		return -1;
	else
		return 0;
}

//---------------------------------------------------------------------------------------------
//
// createRow - add data to the next row in the widget body. Rows have alternating (light and 
//			   dark backgound). The title and date as displayed for each item. The link is used
//			   when the user clicks on a RSS title. 
//6
//----------------------------------------------------------------------------------------------


function createRow (title, link, date, even)
{
	var row = document.createElement ('div');
	row.setAttribute ('class', 'row ' + (even ? 'light' : 'dark'));
	
	var title_div = document.createElement ('div');
	title_div.innerText = title;
	title_div.setAttribute ('class', 'title');
	if (link != null)
	{
		title_div.setAttribute ('the_link', link);
		title_div.setAttribute ('onclick', 'clickOnTitle (event, this);');
	}
	row.appendChild (title_div);
	
	if (date != null)
	{
		var date_div = document.createElement ('div');
		date_div.setAttribute ('class', 'date');
		date_div.innerText = createDateStr (date);
		
		row.appendChild (date_div);
	}
	
	return row;	
}

function createDateStr (date)
{
	var month;
	switch (date.getMonth())
	{
		case 0: month = 'Jan'; break;
		case 1: month = 'Feb'; break;
		case 2: month = 'Mar'; break;
		case 3: month = 'Apr'; break;
		case 4: month = 'May'; break;
		case 5: month = 'Jun'; break;
		case 6: month = 'Jul'; break;
		case 7: month = 'Aug'; break;
		case 8: month = 'Sep'; break;
		case 9: month = 'Oct'; break;
		case 10: month = 'Nov'; break;
		case 11: month = 'Dec'; break;
	}	
	return month + ' ' + date.getDate();
}

//---------------------------------------------------------------------------------------------
//
// clickOnTitle - take user to the RSS link when she clicks on an article's title.  
//
//----------------------------------------------------------------------------------------------

function clickOnTitle (event, div)
{
	 	window.open(div.getAttribute('the_link'));
}