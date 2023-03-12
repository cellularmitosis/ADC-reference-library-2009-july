// setup

var currentTab = "tab_splash";

function setupNewsWindow() {
    var tabName = get_xcodenews_cookie("tabname");
    if(tabName) {
        loadTab(tabName);
    }
}

// tab switching 

function loadTab(tabName) {

    var tabToDisplay;

    if (document.getElementById(tabName)) {
	    tabToDisplay = tabName;
	}else {
        document.getElementById("tabBox").style.display = "block";

        //grab the name of the content tab we want from the request
	    if(document.getElementById(tabName)) {
	        tabToDisplay = tabName;
	    } else {
            // can't find this tab
            tabToDisplay = "tab_splash";
	    }
    }

    
    var newTab = document.getElementById(tabToDisplay);
	
	//////////////// UNIVERSAL FOR ALL TABS /////////////////////////////
	
	var tabBox = document.getElementById("tabBox");
	var tabs = tabBox.getElementsByTagName("li");
	for (var i=0; i<tabs.length; i++) {
		tabs[i].className = "inactive";				
	    if(tabs[i].id == "select_" + tabName) {
            tabs[i].className = "active";
	    }
	}
	
	//hide the content tabs
	var contentBox = document.getElementById("tabContents");
	var contentTabs = getDivsByClassName("tab_content"); 
	for (var i=0; i<contentTabs.length; i++) {
			contentTabs[i].style.display = "none";
	
	}

	//show only the content tab we want
	newTab.style.display = "block";
	
	//////////////// SPECIAL HANDLING /////////////////////////////
	
	//clear contents of news feed at each click of the tab to prevent cumulative loading of content.
	document.getElementById("contents").innerHTML = "";
	document.getElementById("myScrollBar").innerHTML = "";	


	
	if(tabName == "tab_xcode_news") {
		load_blast();
		load();
	}
	
	   //set current tab in cookie
    currentTab = tabName;
	set_xcodenews_cookie("tabname", currentTab);
	
}

// conveniences

function getDivsByClassName (className) {
  var elements = new Array ();
  var children = document.getElementsByTagName( "div" );
  for ( var i = 0; i < children.length; i++ )
  {
    if ( children[i].getAttribute("class") == className ) {
    	elements.push ( children[i] );
    }
  }
  return elements;
}

// cookie handling

function set_xcodenews_cookie(cookieName, cookieValue) {
     var now = new Date();
    now.setTime(new Date(now.getTime() + 1000 * 60 * 60 * 24 * 365));
    document.cookie = cookieName+"="+escape(cookieValue)+";expires="+now.toGMTString();
}

function get_xcodenews_cookie(cookie_name) {
  var results = document.cookie.match ( '(^|;) ?' + cookie_name + '=([^;]*)(;|$)' );
  if (results)
    return (unescape(results[2]));
  else
    return null;
}

