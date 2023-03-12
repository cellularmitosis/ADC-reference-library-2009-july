VERSION 5.00
Object = "{7B92F833-027D-402B-BFF9-A67697366F4E}#1.0#0"; "QTOControl.dll"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.Form MovieForm 
   Caption         =   "Movie"
   ClientHeight    =   4260
   ClientLeft      =   165
   ClientTop       =   915
   ClientWidth     =   4080
   LinkTopic       =   "MovieForm"
   ScaleHeight     =   4260
   ScaleWidth      =   4080
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   3240
      Top             =   600
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin QTOControlLibCtl.QTControl QTControl1 
      Height          =   4332
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   4056
      _cx             =   7154
      _cy             =   7641
      BackColor       =   -2147483644
      BorderColor     =   0
      BorderStyle     =   0
      MovieControllerVisible=   -1  'True
      Sizing          =   0
      URL             =   ""
      BaseURL         =   ""
      AutoPlay        =   ""
   End
   Begin VB.Menu mnuBar 
      Caption         =   "&File"
      Index           =   0
      Begin VB.Menu mnuFile 
         Caption         =   "&Open..."
         Index           =   0
         Shortcut        =   ^O
      End
      Begin VB.Menu mnuFile 
         Caption         =   "Open &URL..."
         Index           =   1
      End
      Begin VB.Menu mnuFile 
         Caption         =   "&Close"
         Index           =   2
      End
      Begin VB.Menu mnuFile 
         Caption         =   "-"
         Index           =   3
      End
      Begin VB.Menu mnuFile 
         Caption         =   "Export with User Settings..."
         Index           =   4
      End
      Begin VB.Menu mnuFile 
         Caption         =   "Export with Custom Settings..."
         Index           =   5
      End
      Begin VB.Menu mnuFile 
         Caption         =   "-"
         Index           =   6
      End
      Begin VB.Menu mnuFile 
         Caption         =   "&Full Screen"
         Index           =   7
      End
      Begin VB.Menu mnuFile 
         Caption         =   "Get Info"
         Index           =   8
         Shortcut        =   ^I
      End
      Begin VB.Menu mnuFile 
         Caption         =   "-"
         Index           =   9
      End
      Begin VB.Menu mnuFile 
         Caption         =   "Exit"
         Index           =   10
      End
   End
   Begin VB.Menu mnuBar 
      Caption         =   "Edit"
      Index           =   1
      Begin VB.Menu mnuEdit 
         Caption         =   "Undo"
         Index           =   0
         Shortcut        =   ^Z
      End
      Begin VB.Menu mnuEdit 
         Caption         =   "Cut"
         Index           =   2
         Shortcut        =   ^X
      End
      Begin VB.Menu mnuEdit 
         Caption         =   "Copy"
         Index           =   3
         Shortcut        =   ^C
      End
      Begin VB.Menu mnuEdit 
         Caption         =   "Paste"
         Index           =   4
         Shortcut        =   ^V
      End
   End
   Begin VB.Menu mnuBar 
      Caption         =   "&Help"
      Index           =   2
      Begin VB.Menu mnuHelp 
         Caption         =   "About..."
         Index           =   0
      End
   End
End
Attribute VB_Name = "MovieForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'File: MovieForm.frm

'Abstract: Contains code demonstrating a simple movie player
'          application using the QuickTime COM Control. This
'          code supports the following:
'
'           - Movie playback with a movie controller
'           - Open, Open from URL, Close
'           - Cut/Copy/Paste/Undo
'           - Export, Export with dialog
'           - Quicktime Event handling
'           - Form resizing
'           - Error handling
'           - Fullscreen playback

'Version: 1.0

'© Copyright 2005 Apple Computer, Inc. All rights reserved.

'IMPORTANT:  This Apple software is supplied to
'you by Apple Computer, Inc. ("Apple") in
'consideration of your agreement to the following
'terms, and your use, installation, modification
'or redistribution of this Apple software
'constitutes acceptance of these terms.  If you do
'not agree with these terms, please do not use,
'install, modify or redistribute this Apple
'software.
'
'In consideration of your agreement to abide by
'the following terms, and subject to these terms,
'Apple grants you a personal, non-exclusive
'license, under Apple's copyrights in this
'original Apple software (the "Apple Software"),
'to use, reproduce, modify and redistribute the
'Apple Software, with or without modifications, in
'source and/or binary forms; provided that if you
'redistribute the Apple Software in its entirety
'and without modifications, you must retain this
'notice and the following text and disclaimers in
'all such redistributions of the Apple Software.
'Neither the name, trademarks, service marks or
'logos of Apple Computer, Inc. may be used to
'endorse or promote products derived from the
'Apple Software without specific prior written
'permission from Apple.  Except as expressly
'stated in this notice, no other rights or
'licenses, express or implied, are granted by
'Apple herein, including but not limited to any
'patent rights that may be infringed by your
'derivative works or by other works in which the
'Apple Software may be incorporated.

'The Apple Software is provided by Apple on an "AS
'IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR
'IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
'WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY
'AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
'THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE
'OR IN COMBINATION WITH YOUR PRODUCTS.

'IN NO EVENT SHALL APPLE BE LIABLE FOR ANY
'SPECIAL , INDIRECT, INCIDENTAL Or CONSEQUENTIAL
'DAMAGES (INCLUDING, BUT NOT LIMITED TO,
'PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
'OF USE, DATA, OR PROFITS; OR BUSINESS
'INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
'REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF
'THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
'UNDER THEORY OF CONTRACT, TORT (INCLUDING
'NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN
'IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF
'SUCH DAMAGE.
'

Option Explicit
Dim gManualFormResize As Boolean

Private Declare Function GetSystemMetrics Lib "user32" (ByVal nIndex As Long) As Long
Private Const SM_CYCAPTION = 4
Private Const SM_CYFRAME = 33
Private Const SM_CYMENU = 15
' display the common dialog to let the user choose a movie file
Function GetFileName()
    CommonDialog1.InitDir = "d:\QuickTime\Movies"
    CommonDialog1.ShowOpen
    GetFileName = CommonDialog1.FileName
End Function
' calculate form width - used during resizing
Function GetFormBorderWidth()
    GetFormBorderWidth = GetSystemMetrics(SM_CYFRAME) * 2
End Function
' calculate form height - used during resizing
Function GetFormBorderHeight()
    GetFormBorderHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) + (GetSystemMetrics(SM_CYFRAME) * 2)
End Function
' standard form load - perform initialization here
Private Sub Form_Load()
    QTControl1.ErrorHandling = qtErrorHandlingRaiseException
    QTControl1.Sizing = qtControlFitsMovie
    Load MovieInfo
End Sub
' form resize - perform sizing adjustments
Private Sub Form_Resize()
    Dim oldSizingMode As QTSizingModeEnum
    
    gManualFormResize = True  ' global flag so _SizeChanged won't respond
    
    ' change sizing mode so the movie tracks the control's size
    oldSizingMode = QTControl1.Sizing
    QTControl1.Sizing = qtMovieFitsControl
    QTControl1.Move 0, 0, Me.Width - (GetFormBorderWidth() * Screen.TwipsPerPixelX), Me.Height - (GetFormBorderHeight() * Screen.TwipsPerPixelY)
    QTControl1.Sizing = oldSizingMode
    
    gManualFormResize = False
End Sub
' form unload - do cleanup
Private Sub Form_Unload(Cancel As Integer)
    MovieInfo.SetInfoMovie Nothing
    QTControl1.URL = ""
    DoEvents
    End
End Sub
Private Sub QTControl1_QTEvent(ByVal EventClass As Long, ByVal EventID As Long, ByVal Phase As Long, ByVal EventObject As QTOLibrary.IQTEventObject, Cancel As Boolean)

    'Code to handle various QuickTime Events
    
    'When running your code in debug mode you will
    'see the following messages displayed in the
    'Immediate Window
    
    Select Case EventID
    
        'status strings
        Case QTEventIDsEnum.qtEventShowStatusStringRequest
            Dim msg As String
            msg = EventObject.GetParam(QTEventObjectParametersEnum.qtEventParamStatusString).ToString()
            Debug.Print "qtEventShowStatusStringRequest : " & msg
            
        'rate changes
        Case QTEventIDsEnum.qtEventRateWillChange
            Dim rate As String
            rate = EventObject.GetParam( _
                QTEventObjectParametersEnum.qtEventParamMovieRate)
            Debug.Print "RateWillChange to: " + CStr(rate)
            
        'time changes
        Case QTEventIDsEnum.qtEventTimeWillChange
            Dim time As String
            time = EventObject.GetParam( _
                QTEventObjectParametersEnum.qtEventParamMovieTime)
            Debug.Print "TimeWillChange to:" + CStr(time)
            
        'audio volume changes
        Case QTEventIDsEnum.qtEventAudioVolumeDidChange
            Dim vol As String
            vol = EventObject.GetParam( _
                QTEventObjectParametersEnum.qtEventParamAudioVolume)
            Debug.Print "AudioVolumeDidChange to: " + CStr(vol)
    End Select

End Sub
Private Sub removeMovieEventListeners(myMovie As QTMovie)
    'Make sure a movie is loaded first
    If myMovie Is Nothing Then Exit Sub
    'Remove all event listeners
    myMovie.EventListeners.RemoveAll
End Sub
Private Sub addMovieEventListeners(myMovie As QTMovie)
    
    'Contains code to demonstrate how to add listeners
    'for various QuickTime Events
    
    'Make sure a movie is loaded first
    If myMovie Is Nothing Then Exit Sub

    'status string listener
    myMovie.EventListeners.Add _
        QTOLibrary.QTEventClassesEnum.qtEventClassApplicationRequest, _
        QTOLibrary.QTEventIDsEnum.qtEventShowStatusStringRequest

    'rate change listener
    myMovie.EventListeners.Add _
        QTOLibrary.QTEventClassesEnum.qtEventClassStateChange, _
        QTOLibrary.QTEventIDsEnum.qtEventRateWillChange

    'time change listener
    myMovie.EventListeners.Add _
        QTOLibrary.QTEventClassesEnum.qtEventClassTemporal, _
        QTOLibrary.QTEventIDsEnum.qtEventTimeWillChange
    
    'audio volume change listener
    myMovie.EventListeners.Add _
        QTOLibrary.QTEventClassesEnum.qtEventClassAudio, _
        QTOLibrary.QTEventIDsEnum.qtEventAudioVolumeDidChange

End Sub
' handle Edit menu
Private Sub mnuEdit_Click(Index As Integer)
    If QTControl1.Movie Is Nothing Then Exit Sub
    Select Case Index
        Case 0 ' Undo
            QTControl1.Movie.Undo
        Case 2 ' Cut
            QTControl1.Movie.Cut
        Case 3 ' Copy
            QTControl1.Movie.Copy
        Case 4 ' Paste
            QTControl1.Movie.Paste
    End Select
End Sub
Private Sub exportWithUserSettings()
    
    'Perform export with user configured
    'settings.
    
    'If you want to allow the user to configure
    'everything (exporter type, file, options:
    'same as QuickTime Player Export) then use
    'the following:
    
    On Error GoTo ErrorHandler
    
    If QTControl1.Movie Is Nothing Then Exit Sub
    
    Dim qt As QTOLibrary.QTQuickTime
    Dim ex As QTOLibrary.QTExporter
                    
    Set qt = QTControl1.QuickTime
    If qt.Exporters.Count = 0 Then
        qt.Exporters.Add
    End If
    Set ex = qt.Exporters(1)
    
    'Set exporter data source - could also be a track
    ex.SetDataSource QTControl1.Movie
    
    'Display the export dialog
    ex.ShowExportDialog
    
    Exit Sub

ErrorHandler:
    If Err.Number = &H8004FF80 Then
        'Export Operation Canceled - we'll ignore this error...
    Else
        Beep
        Dim errStr As String
        errStr = "Failed with error #" & Hex(Err.Number) & ", " & Err.Description
        MsgBox errStr, vbCritical
    End If

End Sub
Private Sub exportWithCustomSettings()

    'Perform export with custom configured
    'settings.

    'To perform an export operation with custom
    'configured Settings you first need to create
    'an instance of the QTExporter object and then
    'you need to configure it by setting its data
    'source (movie or track) and exporter type
    '(QuickTime Movie, AVI, 3G, MPEG-4, PNG, etc.)
    'as well as the file you want to export to.

    'Then call ShowSettingsDialog to display the
    'export options dialog for the selected exporter
    'type, and finally call BeginExport.

    'Once you have configured an exporter, you can
    'change its data source and reuse it with other
    'movies and/or tracks as many times as you like.
    'You can also persist exporter settings by getting
    'and setting the XML property (a string).
    
    On Error GoTo ErrorHandler

    If QTControl1.Movie Is Nothing Then Exit Sub

    Dim qt As QTQuickTime
    Dim ex As QTExporter
    
    Set qt = QTControl1.QuickTime
    If qt.Exporters.Count = 0 Then
        qt.Exporters.Add
    End If
    Set ex = qt.Exporters(1)
    
    'Set exporter type: AVI, 3G, MPEG-4, PNG, etc.
    ex.TypeName = "QuickTime Movie"
    
    'Set selection to export (optional)
    ex.StartTime = 0      'Optional
    ex.EndTime = 600      'Optional
    
    'Set exporter data source - could also be a track
    ex.SetDataSource QTControl1.Movie
    
    'Set name and path for exported file
    ex.DestinationFileName = "C:\Movies\Exported.mov"
    
    'Display the settings dialog for the selected exporter type
    ex.ShowSettingsDialog
    
    'Now do the actual export
    ex.BeginExport
    
    Exit Sub

ErrorHandler:
    If Err.Number = &H8004FF80 Then
        'Export Operation Canceled - we'll ignore this error...
    Else
        Beep
        Dim errStr As String
        errStr = "Failed with error #" & Hex(Err.Number) & ", " & Err.Description
        MsgBox errStr, vbCritical
    End If

End Sub
' handle File menu
Private Sub mnuFile_Click(Index As Integer)
    On Error GoTo ErrorHandler
    
    Dim filePath
    
    Select Case Index
        Case 0  ' Open
            filePath = GetFileName()
            If filePath <> "" Then
                QTControl1.URL = filePath
                MovieInfo.SetInfoMovie QTControl1.Movie
                addMovieEventListeners QTControl1.Movie
            End If
        Case 1  ' Open URL
            Dim movieURL As String
            movieURL = InputBox("Enter a URL:", "URL", "http://www.server.com/movies/sample.mov")
            If movieURL <> "" Then
                QTControl1.URL = movieURL
                MovieInfo.SetInfoMovie QTControl1.Movie
                addMovieEventListeners QTControl1.Movie
            End If
        Case 2  ' Close
            removeMovieEventListeners QTControl1.Movie
            QTControl1.URL = ""
            MovieInfo.SetInfoMovie Nothing
        Case 4  'Export with full user-configurable settings
            exportWithUserSettings
        Case 5  'Export with custom settings
            exportWithCustomSettings
        Case 7  'Full Screen
            If QTControl1.URL <> "" Then QTControl1.FullScreen = True
        Case 8  ' Get Info
            MovieInfo.SetInfoMovie QTControl1.Movie
            MovieInfo.Move Me.Left + Me.Width + 200, Me.Top
            MovieInfo.Show
        Case 10  ' Exit
            Unload Me
            End
    End Select
    
    Exit Sub

ErrorHandler:

    Beep
    Dim errStr As String
    errStr = "Failed with error #" & Hex(Err.Number) & ", " & Err.Description
    MsgBox errStr, vbCritical

End Sub

Private Sub mnuHelp_Click(Index As Integer)
    QTControl1.ShowAboutBox
End Sub

Private Sub QTControl1_SizeChanged(ByVal Width As Long, ByVal Height As Long)
    ' ignore event if control was resized as a result of form being resized.
    If gManualFormResize Then Exit Sub

   ' resize window to accomodate control
    Me.Move Me.Left, Me.Top, (Width + GetFormBorderWidth()) * Screen.TwipsPerPixelX, (Height + GetFormBorderHeight()) * Screen.TwipsPerPixelY
End Sub

Private Sub QTControl1_StatusUpdate(ByVal statusCodeType As Long, ByVal statusCode As Long, ByVal statusMessage As String)
     Select Case statusCodeType
        Case qtStatusCodeTypeControl
            Select Case statusCode
                Case qtStatusFullScreenBegin
                    Me.Hide  ' hide movie window
                
                Case qtStatusFullScreenEnd
                    QTControl1.SetScale 1  ' set back to a reasonable size
                    Me.Show
        End Select
    End Select
End Sub
