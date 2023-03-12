VERSION 5.00
Object = "{7B92F833-027D-402B-BFF9-A67697366F4E}#1.0#0"; "QTOControl.dll"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.Form Form1 
   BackColor       =   &H80000005&
   Caption         =   "Form1"
   ClientHeight    =   3510
   ClientLeft      =   165
   ClientTop       =   915
   ClientWidth     =   4020
   LinkTopic       =   "Form1"
   ScaleHeight     =   3510
   ScaleWidth      =   4020
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   0
      Top             =   3120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin QTOControlLibCtl.QTControl QTControl1 
      Height          =   2415
      Left            =   720
      TabIndex        =   0
      Top             =   360
      Width           =   2535
      _cx             =   4471
      _cy             =   4260
      BackColor       =   -2147483633
      BorderColor     =   0
      BorderStyle     =   1
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
         Caption         =   "&New From Images..."
         Index           =   0
      End
      Begin VB.Menu mnuFile 
         Caption         =   "&Save"
         Index           =   1
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'File: Form1.frm

'Abstract: Contains code demonstrating how to create a QuickTime
'          movie from a sequence of images using the QuickTime 7
'          COM Control. Shows how to save the newly created movie too.


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
Private Const SM_CYCAPTION = 4
Private Const SM_CYFRAME = 33
Private Const SM_CYMENU = 15


Private Declare Function GetSystemMetrics Lib "user32" (ByVal nIndex As Long) As Long
Private Sub Form_Load()
    QTControl1.ErrorHandling = qtErrorHandlingRaiseException
    QTControl1.Sizing = qtControlFitsMovie
End Sub
' File menu
Private Sub mnuBar_Click(Index As Integer)

End Sub
' display the common dialog to let the user choose a movie file
Function GetFileName()
    CommonDialog1.InitDir = "..\..\ImageSeq"
    CommonDialog1.ShowOpen
    GetFileName = CommonDialog1.FileName
End Function
' display the common dialog to let the user choose where to save
' the movie file
Function GetSaveFileName()
    CommonDialog1.CancelError = True
    On Error GoTo ErrHandler
    
    CommonDialog1.FileName = "MyMovie.mov"
    CommonDialog1.ShowSave

    GetSaveFileName = CommonDialog1.FileName
    Exit Function
    
ErrHandler:
    ' User pressed Cancel button.
    GetSaveFileName = ""
End Function

' New From Images menu item
Private Sub mnuFile_Click(Index As Integer)
    Select Case Index
        Case 0  ' New From Images...
            Dim filePath
            filePath = GetFileName()
            If filePath <> "" Then
                
                ' get rid of any existing movie first
                If QTControl1.URL <> "" Then
                    QTControl1.URL = ""
                End If
                
                On Error GoTo ErrorHandler
                
                QTControl1.CreateNewMovieFromImages filePath, 30, True
                QTControl1.Movie.AllowDynamicResize = True
                
            End If
        Case 1  ' Save movie as self-contained
            If QTControl1.URL <> "" Then
                Dim saveFilePath
                saveFilePath = GetSaveFileName()
                If saveFilePath <> "" Then
                    On Error GoTo ErrorHandler

                    QTControl1.Movie.SaveSelfContained saveFilePath, True
                End If
            End If
    End Select

    Exit Sub

ErrorHandler:
    Beep
    Dim errStr As String
    errStr = "Failed with error #" & Hex(Err.Number) & ", " & Err.Description
    MsgBox errStr, vbCritical
    
End Sub

' calculate form width - used during resizing
Function GetFormBorderWidth()
    GetFormBorderWidth = GetSystemMetrics(SM_CYFRAME) * 2
End Function
' calculate form height - used during resizing
Function GetFormBorderHeight()
    GetFormBorderHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) + (GetSystemMetrics(SM_CYFRAME) * 2)
End Function

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
Private Sub QTControl1_SizeChanged(ByVal Width As Long, ByVal Height As Long)
    ' ignore event if control was resized as a result of form being resized.
    If gManualFormResize Then Exit Sub

   ' resize window to accomodate control
    Me.Move Me.Left, Me.Top, (Width + GetFormBorderWidth()) * Screen.TwipsPerPixelX, (Height + GetFormBorderHeight()) * Screen.TwipsPerPixelY
End Sub

' form unload - do cleanup
Private Sub Form_Unload(Cancel As Integer)
    QTControl1.URL = ""
    DoEvents
    End
End Sub

