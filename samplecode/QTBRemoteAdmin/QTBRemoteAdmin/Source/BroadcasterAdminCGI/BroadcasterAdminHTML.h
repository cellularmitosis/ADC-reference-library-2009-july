/*
    File:			BroadcasterAdminHTML.h
    Description:	This file contains html tags used by the BroadcastAdmin class.

*/

    // cgi parameters
#define kCGIParamRequest				@"request"
#define kCGIParamAudioPreset			@"audioPreset"
#define kCGIParamVideoPreset			@"videoPreset"
#define kCGIParamNetworkPreset			@"networkPreset"
#define kCGIParamRecording				@"recording"
#define kCGIParamSettingsFile			@"settingsFile"
#define kCGIParamLaunchWithUI			@"launchWithUI"
#define kCGIParamLaunchPath				@"launchPath"

    // cgi request parameter values
#define kCGIParamRequestSetup			@"Setup"
#define kCGIParamRequestRefreshSetup    @"Refresh Setup"
#define kCGIParamRequestBroadcast		@"Broadcast"
#define kCGIParamRequestStopBroadcast	@"Stop Broadcast"
#define kCGIParamRequestStatistics		@"Update Stats"
#define kCGIParamRequestQuit			@"Quit"
#define kCGIParamRequestLaunch			@"Launch"
#define kCGIParamRequestLaunching		@"Launching"

    // html tags
#define kHTTPHeader						@"Content-type: text/html\n\n"
#define kHTMLOpenTag					@"<html>\n"
#define kHTMLCloseTag					@"</html>\n"
#define kHTMLHeadOpenTag				@"\t<head>\n"
#define kHTMLHeadCloseTag				@"\t</head>\n"
#define kHTMLTitleOpenTag				@"\t\t<title>"
#define kHTMLTitleCloseTag				@"</title>\n"
#define kHTMLBodyOpenTag				@"\t<body bgcolor=\"white\">\n"
#define kHTMLBodyCloseTag				@"\t</body>\n"

    // meta refresh
#define kHTMLMETAOpen					@"<meta http-equiv=Refresh content=\"3; URL=http://"
#define kHTMLMETAMiddle					@"/cgi-bin/BroadcasterAdmin?request="
#define kHTMLMETAClose					@"\">\n"

    // forms
#define kHTMLFormOpen					@"\t<form method=\"get\" action=\"http://"
#define kHTMLFormMiddle					@"/cgi-bin/BroadcasterAdmin\">\n"
#define kHTMLFormClose					@"\t</form>\n"

    // submit button
#define kHTMLSubmitOpen					@"\t<center>\n<input type=\"submit\" name=\""
#define kHTMLSubmitMiddle				@"\" value=\""
#define kHTMLSubmitClose				@"\">\n\t</center>\n<br>\n"

    // tables
#define kHTMLTableOpen					@"<center>\n<table bgcolor=\"lightblue\" bordercolor=\"#008080\" cellspacing=\"2\" cellpadding=\"2\">\n"
#define kHTMLTableClose					@"</table>\n</center>\n<br>\n"
#define kHTMLTableRowOpen				@"<tr>\n"
#define kHTMLTableRowClose				@"</tr>\n"
#define kHTMLTableItemOpen				@"<td>\n"
#define kHTMLTableItemClose				@"</td>\n"
#define kHTMLInputTextOpen				@"<input type=\"text\" name=\""
#define kHTMLInputTextMiddle			@"\" value=\""
#define kHTMLInputTextClose				@"\">"
#define kHTMLSelectOpen					@"</td>\n<td>\n<select name=\""
#define kHTMLSelectClose				@"</select>\n</td>\n</tr>\n"
#define kHTMLOptionOpen					@"<option value=\""

    // header
#define kHTMLPageHeaderOpen				@"<center><font size=\"+1\" face=\"Verdana\">"
#define kHTMLPageHeaderClose			@"</font></center>\n"

    // image
#define kHTMLImageOpen					@"<center>\n<img src=\"http://"
#define kHTMLImageClose					@"\">\n</center>\n<br>"

    // other
#define kTitle							@"QuickTime Broadcaster - Web Admin"
#define kDefaultImagePath				@"images/QuickTimeLogo.gif"
#define kTagClose						@"\">"
