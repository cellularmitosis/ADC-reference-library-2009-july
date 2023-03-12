// this is the JS function that parses the applet tag
function AppletTag (myAppletTag, myParamList) {
	var is = new Is();
	
	// parse the applet tags
	this.appletTag = ""
	this.ieWinobjectTag = ""
	this.ieParamTag = ""
	this.navParamTag = ""

	var startTag = "<PARAM "
	var name = "NAME=" 
	var val = "VALUE="
	var endTag = ">"
	
	if (myAppletTag["code"]) {
		this.appletTag += "CODE =\"" + myAppletTag.code + "\" "
		this.ieParamTag += startTag + name + "\"CODE\" " + val + "\"" + myAppletTag.code + "\"" +  endTag + " "
	}
	
	if ( myAppletTag["object"]) {
		this.appletTag += "OBJECT = \"" + myAppletTag.object + "\" "
		this.ieParamTag += startTag + name + "\"OBJECT\" " + val + "\"" + myAppletTag.object + "\"" + endTag + " "
	}
	
	if ( myAppletTag["archive"]) {
		this.appletTag += "ARCHIVE=\"" + myAppletTag.archive + "\" "
		this.ieParamTag += startTag + name + "\"ARCHIVE\" " + val + "\"" + myAppletTag.archive + "\"" + endTag	+ " "
	}
	
	if (myAppletTag["name"]) {
		this.appletTag += "NAME =\"" + myAppletTag.name + "\" "
		this.ieWinobjectTag += "ID=\"" + myAppletTag.name + "\" "
	}
		
	if (myAppletTag["width"]) {
		this.appletTag += "WIDTH =\"" + myAppletTag.width + "\" "
		this.ieWinobjectTag += "WIDTH=\"" + myAppletTag.width + "\" "
	}	
	
	if (myAppletTag["height"]) {
		this.appletTag += "HEIGHT =\"" + myAppletTag.height + "\" "
		this.ieWinobjectTag += "HEIGHT =\"" + myAppletTag.width + "\" "
	}
	
	if ( myAppletTag["alt"])
		this.appletTag += "ALT=\"" + myAppletTag.alt + "\" "
	
	if ( myAppletTag["align"]){
		this.appletTag += "ALIGN=\"" + myAppletTag.align + "\" "
		this.ieWinobjectTag += "ALIGN=\"" + myAppletTag.align + "\" "
	}
	
	if ( myAppletTag["vspace"]) {
		this.appletTag += "VSPACE=\"" + myAppletTag.vspace + "\" "
		this.ieWinobjectTag += "VSPACE=\"" + myAppletTag.vspace + "\" "
	}	
	
	if ( myAppletTag["hspace"]) {
		this.appletTag += "HSPACE=\"" + myAppletTag.hspace + "\" "
		this.ieWinobjectTag += "HSPACE=\"" + myAppletTag.hspace + "\" "
	}	

// Not supported because Win IE plugin uses codebase for the plugin page
	if (false) {
		if (myAppletTag["codebase"]) {
			this.appletTag += " codebase = " + myAppletTag.codebase
			this.ieWinobjectTag +=  " codebase" + myAppletTag.codebase
		}	
	}
	
	if (myParamList != null) {			
		for (i = 0; i < myParamList.length; i = i + 2) {
			x = i;
			n = myParamList[x];
			v = myParamList[x + 1]
			this.ieParamTag += startTag + name + "\"" + n + "\" " + val + "\"" + v + "\"" + endTag + " "
			this.navParamTag += n + "=\"" + v + "\" "
		}
	}
	printThis (this, is);
}


function printThis (jreTag, is) {
	if (is.win32) {
		if (is.nav4up){	// Netscape 4 and up
		     document.write("<EMBED type=\"application/x-java-applet;version=1.3\" ");
		     document.write("PLUGINSPAGE=\"http://java.sun.com/products/plugin/1.3/plugin-install.html\" ");
		     document.write(jreTag.appletTag + " " + jreTag.navParamTag + ">");
		     document.write("</EMBED>");
		 
		} else if (is.ie4up)	{ // IE4 and later 
		   document.write("<OBJECT classid=\"clsid:8AD9C840-044E-11D1-B3E9-00805F499D93\" ");
		   document.write(jreTag.ieWinobjectTag);
		   document.write("CODEBASE=\"http://java.sun.com/products/plugin/1.3/jinstall-13-win32.cab#Version=1,3,0,0\"> ");
		   document.write("<PARAM NAME=\"type\" VALUE=\"application/x-java-applet;version=1.3\"> ");
		   document.write(jreTag.ieParamTag);
		   document.write("</OBJECT>");
		} else {	//presume applet environment works with applet tag
		   document.write("<APPLET " + jreTag.appletTag + " >");
		   document.write(jreTag.ieParamTag);
		   document.write("</APPLET>");
		}		
	} else if (is.mac) {
		if (is.nav) {
			if (is.nav5up) {
			   	document.write("<APPLET " + jreTag.appletTag + " >");
			   	document.write(jreTag.ieParamTag);
			   	document.write("</APPLET>");
			} else {
				document.write ("<EMBED type=\"application/x-java-vm\" pluginspage=\"ftp://ftp.mozilla.org/pub/OJI/MRJPlugin.sit.hqx\" ");
				document.write (jreTag.appletTag + jreTag.navParamTag + ">");
				document.write ("</EMBED>")
			}
		} else if (is.ie) {
			if (is.ie4up) { // IE4 and up and later 
			   	document.write("<APPLET " + jreTag.appletTag + ">");
			   	document.write(jreTag.ieParamTag);
			 	document.write("</APPLET>");
			}
		} else {	//presumes supports MRJ and thus applet tag
		   document.write("<APPLET " + jreTag.appletTag + ">");
		   document.write(jreTag.ieParamTag);
		   document.write("</APPLET>");
		}		
	} else {
		document.write("<h2>QTJava Applet currently runs on MacOS and Windows32 OS only.</h2>");
	}
}


function Is () {
   // convert all characters to lowercase to simplify testing
    var agt=navigator.userAgent.toLowerCase()

    // *** BROWSER VERSION ***
    this.major = parseInt(navigator.appVersion)
    this.minor = parseFloat(navigator.appVersion)

    //test for Netscape browser
	this.nav  = ((agt.indexOf('mozilla')!=-1) && ((agt.indexOf('spoofer')==-1)
                && (agt.indexOf('compatible') == -1)))
    this.nav2 = (this.nav && (this.major == 2))
    this.nav3 = (this.nav && (this.major == 3))
    this.nav4 = (this.nav && (this.major == 4))
    this.nav4up = this.nav && (this.major >= 4)
    this.nav5up = this.nav && (this.major >= 5)
    
    //test for IE browser
    this.ie   = (agt.indexOf("msie") != -1)
    this.ie3  = (this.ie && (this.major == 2))
    this.ie4  = (this.ie && (this.major == 4))
    this.ie4up  = (this.ie  && (this.major >= 4))    
	
	//test for OS
    this.win   = ( (agt.indexOf("win")!=-1) || (agt.indexOf("16bit")!=-1) )
    // NOTE: On Opera 3.0, the userAgent string includes "Windows 95/NT4" on all
    //        Win32, so you can't distinguish between Win95 and WinNT.
    this.win95 = ((agt.indexOf("win95")!=-1) || (agt.indexOf("windows 95")!=-1))

    // NOTE: Reliable detection of Win98 may not be possible. It appears that:
    //       - On Nav 4.x and before you'll get plain "Windows" in userAgent.
    //       - On Mercury client, the 32-bit version will return "Win98", but
    //         the 16-bit version running on Win98 will still return "Win95".
    this.win98 = ((agt.indexOf("win98")!=-1)||(agt.indexOf("windows 98")!=-1))
    this.winnt = ((agt.indexOf("winnt")!=-1)||(agt.indexOf("windows nt")!=-1))	
    this.win32 = this.win95 || this.winnt || this.win98 || 
                 ((this.major >= 4) && (navigator.platform == "Win32")) ||
                 (agt.indexOf("win32")!=-1) || (agt.indexOf("32bit")!=-1)
                 
    this.mac    = (agt.indexOf("mac")!=-1)
}    
