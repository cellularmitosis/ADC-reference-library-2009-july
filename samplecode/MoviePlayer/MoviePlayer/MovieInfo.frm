VERSION 5.00
Begin VB.Form MovieInfo 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Movie Info"
   ClientHeight    =   5145
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   5880
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5145
   ScaleWidth      =   5880
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame1 
      Height          =   3495
      Left            =   120
      TabIndex        =   2
      Top             =   1560
      Width           =   5655
      Begin VB.Label lblData 
         Height          =   3135
         Left            =   1560
         TabIndex        =   4
         Top             =   240
         Width           =   3975
      End
      Begin VB.Label lblCaptions 
         Alignment       =   1  'Right Justify
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   3135
         Left            =   120
         TabIndex        =   3
         Top             =   240
         Width           =   1335
      End
   End
   Begin VB.Label lblInfo 
      Alignment       =   2  'Center
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   120
      TabIndex        =   1
      Top             =   480
      Width           =   5535
   End
   Begin VB.Label lblName 
      Alignment       =   2  'Center
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   5535
   End
End
Attribute VB_Name = "MovieInfo"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'File: MovieInfo.frm

'Abstract: Contains code which displays movie information
'          for the active movie such as the track types,
'          formats, size, movie duration and name.

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

Dim gMovie As QTMovie
Private Sub Form_Load()
' display movie data when form loads
    DisplayMovieData
End Sub
Sub SetInfoMovie(myMovie As QTMovie)
    Set gMovie = myMovie
    DisplayMovieData
End Sub
Function MovieAnnotation(annoID As Long, mov As QTMovie) As String
    ' Annotation returns an error if asked for an annotation that
    '  does not exist (so you can tell the difference between an
    '  annotation that is missing and one that is set to an empty
    '  string), so we need to be prepared for an exception
    On Error Resume Next
    Dim anno As String
    
    anno$ = ""   ' value will remain if getting value throws exception
    If Not (mov Is Nothing) Then anno$ = mov.Annotation(annoID)
    MovieAnnotation = anno
End Function
' code to display movie info. in a window such
' as movie duration, size, etc.
Sub DisplayMovieData()
    Dim dataStr As String
    Dim captionsStr As String
    Dim annoStr As String
    
    Dim track As QTTrack
    
    lblName = ""
    lblInfo = ""
    lblCaptions = ""
    lblData = ""
    
    If gMovie Is Nothing Then Exit Sub

    On Error GoTo ErrorHandler
    lblName = MovieAnnotation(qtAnnotationFullName, gMovie)

    ' get some annotations
    lblInfo = ""
    ' copyright
    annoStr = MovieAnnotation(qtAnnotationCopyright, gMovie)
    If Trim(annoStr) <> "" Then lblInfo = lblInfo & annoStr & vbCrLf
    ' author
    annoStr = MovieAnnotation(qtAnnotationAuthor, gMovie)
    If Trim(annoStr) <> "" Then lblInfo = lblInfo & annoStr & vbCrLf
    ' comments
    annoStr = MovieAnnotation(qtAnnotationComments, gMovie)
    If Trim(annoStr) <> "" Then lblInfo = lblInfo & annoStr & vbCrLf
    ' description
    annoStr = MovieAnnotation(qtAnnotationDescription, gMovie)
    If Trim(annoStr) <> "" Then lblInfo = lblInfo & annoStr & vbCrLf

    captionsStr = "Source:" & vbCrLf & vbCrLf
    dataStr = Trim(gMovie.URL) & vbCrLf & vbCrLf

    captionsStr = captionsStr & "Size:" & vbCrLf & vbCrLf
    dataStr = dataStr & gMovie.Width & " x " & gMovie.Height & vbCrLf & vbCrLf

    captionsStr = captionsStr & "Duration:" & vbCrLf & vbCrLf
    dataStr = dataStr & gMovie.Duration & vbCrLf & vbCrLf

    ' iterate over tracks and get display name, format, width and height
    For Each track In gMovie.Tracks
        captionsStr = captionsStr & track.DisplayName & ":" & vbCrLf
        dataStr = dataStr & track.Format
        If track.Height > 0 Then
            dataStr = dataStr & ", " & track.Width & " x " & track.Height & vbCrLf
        Else
            dataStr = dataStr & vbCrLf
        End If
    Next track

    lblCaptions = captionsStr
    lblData = dataStr
    Exit Sub

ErrorHandler:
    dataStr = dataStr & "Error #" & Hex(Err.Number) & ", " & Err.Description & vbCrLf
    Resume Next
End Sub
