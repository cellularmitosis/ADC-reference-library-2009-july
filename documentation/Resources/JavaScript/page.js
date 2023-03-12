////////////////// JS DOCSET PROTOCOL

function ContentFrameName() {
    return "content";
}

/////////////////// COOKIE HANDLING

function get_absolute_path(url) {
    var loc = window.location.pathname; 
    loc = loc.substring(0, loc.lastIndexOf('/'));
    while (/^\.\./.test(url)){
        loc = loc.substring(0, loc.lastIndexOf('/'));
        url = url.substring(3);
    }
    if (url) {
        return loc + "/" +url;
    } else {
        return loc;
    }
}

function set_toc_cookie(cookieName, cookieValue) {
    var cookiePath = toc_cookie_path();
    if(cookiePath) {
        var expiry = new Date();
        expiry.setFullYear(expiry.getFullYear()+1);
        document.cookie = cookieName+"="+escape(cookieValue) +
            "; expires=" + expiry.toGMTString() + 
            "; path=" + window.location.pathname;
    }
}

function get_toc_cookie(cookie_name) {
    var results = document.cookie.match ( '(^|;) ?' + cookie_name + '=([^;]*)(;|$)' );
    if (results) {
    return (unescape(results[2]));
    } else {
        return null;
    }
}

function delete_toc_cookie ( cookie_name ) {
    var cookie_date = new Date();
    cookie_date.setTime (cookie_date.getTime() - 1);
    document.cookie = cookie_name += "=;path="+window.location.pathname+";expires=" + cookie_date.toGMTString();
}

function toc_cookie_path() {
    var cookiePath;
    var tocMetaElt = document.getElementById("toc-file");
    if(!tocMetaElt) {
    tocMetaElt = parent.document.getElementById("toc-file");
    }
    if(tocMetaElt) {
        cookiePath = get_absolute_path(tocMetaElt.content);
    }    
    return cookiePath;
}

///////////////////////////////////////

function initialize_index() {

    var frameURL = document.getElementById("refresh").content.split("URL=")[1];
    
    var xcodeVersion;
    var versionRegexp = new RegExp("Xcode\/([0-9.]+)");
    var xcodeVersionArray = versionRegexp.exec(navigator.appVersion);
    if(xcodeVersionArray) {
        xcodeVersion = parseFloat(xcodeVersionArray[1]);
    }
    
    if(xcodeVersion && xcodeVersion < 1000.0 && !parent.frames["content"]) {
        window.webKitWorkaround = document.getElementsByName(location.hash.substr(1))[0];
        document.write('<iframe width="100%" height="100%" align="left" frameborder="0" id="content" name="content" src="' + frameURL + '">This document set is best viewed in a browser that supports iFrames.</iframe>');
        document.body.style.margin = "0";
        return;
    }
}

function initialize_page() {
    
    var xcodeVersion;
    var versionRegexp = new RegExp("Xcode\/([0-9.]+)");
    var xcodeVersionArray = versionRegexp.exec(navigator.appVersion);
    if(xcodeVersionArray) {
        xcodeVersion = parseFloat(xcodeVersionArray[1]);
    }
       
    if(xcodeVersion && xcodeVersion < 1000.0 && !parent.frames["content"]) {
        window.webKitWorkaround = document.getElementsByName(location.hash.substr(1))[0];
        document.body.innerHTML = '<iframe width="100%" height="100%" align="left" frameborder="0" id="content" name="content" src="' + self.location + '">This document set is best viewed in a browser that supports iFrames.</iframe>';
        return;
    }
    
    // dynamically load TOC
    var tocFileElt = document.getElementById("toc-file");
    if(tocFileElt) {        
        var filePath = tocFileElt.content;
        var tocElt = document.createElement("div");
        tocElt.id="tocMenu";
        tocElt.innerHTML = '<iframe id="toc_content" name="toc_content" SRC="' + filePath + '" width="210" height="100%" align="left" frameborder="0">This document set is best viewed in a browser that supports iFrames.</iframe>';
        document.body.appendChild(tocElt);
    }
    
    // create tooltip
    var newDiv = document.createElement('div');
    newDiv.setAttribute('id', 'tooltip');
    document.body.appendChild(newDiv);
    
    // determine TOC cookie state and set TOC appropriately
    if(get_toc_cookie("tocHideState") && get_toc_cookie("tocHideState") == 'hide') {
        showHideTOC('hide');
    }

    //Tell pedia it is ok to load
    loadPedia();
}

function announce_page_loaded() {

    // If we're in a frameset, tell the TOC frame this page was loaded, so it can track it.
    if (document.getElementById && document.getElementById('toc_content')) {
        document.getElementById('toc_content').contentWindow.page_loaded(document.location);
    } else if (frames.length) {
        frames['toc_content'].page_loaded(document.location);
    }
}

function showtip(hovered,event){
// Makes the "tooltip" element visible and moves it to the 
    // (x,y) of the mouse event (plus some buffer zone)
    
    var agent = navigator.userAgent;
    if (agent.indexOf("MSIE") > 0 && agent.indexOf("Mac") > 0) { 
        // IE-Mac no longer supported, and the CSS functionality is not up to the par needed for this
        return;
    }
    
    var abstract_text = hovered.getElementsByTagName('img').item(0).getAttribute('abstract');
    if(!abstract_text) { 
        return; 
    } 
    
    // Event-handling code for cross-browser support
    var mouse_event;
    if(!event) { mouse_event = window.event; } else { mouse_event = event; }
    
    var tooltip = document.getElementById("tooltip");
    tooltip.innerHTML = abstract_text;
    
    tooltip.style.backgroundColor = "#FDFEC8";
    
    var xcoord = 0;
    var ycoord = 0;
    
    if(mouse_event.pageX || mouse_event.pageY) {
        xcoord = event.pageX;
        ycoord = event.pageY;
    } else if(mouse_event.clientX || mouse_event.clientY) {
        xcoord = mouse_event.clientX + (document.documentElement.scrollLeft ?  document.documentElement.scrollLeft : document.body.scrollLeft);
        ycoord = mouse_event.clientY;
    }
    
    tooltip.style.left = xcoord + 4 + "px";
    tooltip.style.top = ycoord + 10 + "px";
    tooltip.style.visibility="visible";
}

function hidetip() {
    var tooltip = getStyleObject("tooltip");
    tooltip.visibility = "hidden";
}

function placeWatermark() {
    if (document.layers) {
        document.watermark.pageX = (window.innerWidth - document.watermark.document.myImage.width)/2;
        document.watermark.pageY = (window.innerHeight - document.watermark.document.myImage.height)/2;
        document.watermark.visibility = 'visible';
    }
}

function closeWatermark() {
    var watermark = getStyleObject("watermark");
    watermark.visibility = "hidden";
}

// cross-browser function to get an object's style object given its
function getStyleObject(objectId) {
    if(document.getElementById && document.getElementById(objectId)) {
    // W3C DOM
    return document.getElementById(objectId).style;
    } else if (document.all && document.all(objectId)) {
    // MSIE 4 DOM
    return document.all(objectId).style;
    } else if (document.layers && document.layers[objectId]) {
    // NN 4 DOM.. note: this won't find nested layers
    return document.layers[objectId];
    } else {
    return false;
    }
}

var state = 'block';
var margin = 210;
// shows or hides the TOC as well as toggles the show/hide text (2 per page)
function showHideTOC(forceState) {
    var tocDiv     = getStyleObject("tocMenu");
    var bodyDiv    = getStyleObject("bodyText");
    var showHideTOCUpperSpan = document.getElementById("showHideTOCUpperSpan");
    var showHideTOCLowerSpan = document.getElementById("showHideTOCLowerSpan");
    var upperAnchor = showHideTOCUpperSpan.getElementsByTagName('a').item(1);
    var lowerAnchor = showHideTOCLowerSpan.getElementsByTagName('a').item(1);
    var showText = showHideTOCUpperSpan.getElementsByTagName('img').item(0).getAttribute('showText');
    var hideText = showHideTOCUpperSpan.getElementsByTagName('img').item(0).getAttribute('hideText');

    if (state == 'block' || forceState == 'hide') {
        //hide
        state = 'none'; 
        margin = 10;
        upperAnchor.innerHTML = showText;
        lowerAnchor.innerHTML = showText;
        set_toc_cookie("tocHideState", "hide");
    } else {
        //show
        state = 'block'; 
        margin = 210;
        upperAnchor.innerHTML = hideText;
        lowerAnchor.innerHTML = hideText;
        delete_toc_cookie("tocHideState");
    }
    
    tocDiv.display = state;
    if (document.layers) {
        bodyDiv.marginLeft = margin;
    } else {
        bodyDiv.marginLeft = margin + "px";
    }
    
    return false;
}

document.observe("dom:loaded", function() {
        if (navigator.userAgent.match('Xcode/')){
            var hideInXcode = $$('.hideInXcode');
            for (var i = 0; i<hideInXcode.length; i++){
                hideInXcode[i].hide();
            }
            var showInXcode = $$('.showInXcode');
            for (i = 0; i<showInXcode.length; i++){
                showInXcode[i].show();
            }
        }
});
