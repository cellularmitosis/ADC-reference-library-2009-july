/*
*  File:    Form1.cs
* 
*  Contains:  Main program. Contains movie handling code to open & close movie files,
*             perform editing operations, display a movie fullscreen, handle movie
*             events, perform movie export and more.
*  
*  Version:  1.0
* 
*  Created:  10/25/05
*
*  Disclaimer:  IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*        ("Apple") in consideration of your agreement to the following terms, and your
*        use, installation, modification or redistribution of this Apple software
*        constitutes acceptance of these terms.  If you do not agree with these terms,
*        please do not use, install, modify or redistribute this Apple software.
*
*        In consideration of your agreement to abide by the following terms, and subject
*        to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
*        copyrights in this original Apple software (the "Apple Software"), to use,
*        reproduce, modify and redistribute the Apple Software, with or without
*        modifications, in source and/or binary forms; provided that if you redistribute
*        the Apple Software in its entirety and without modifications, you must retain
*        this notice and the following text and disclaimers in all such redistributions of
*        the Apple Software.  Neither the name, trademarks, service marks or logos of
*        Apple Computer, Inc. may be used to endorse or promote products derived from the
*        Apple Software without specific prior written permission from Apple.  Except as
*        expressly stated in this notice, no other rights or licenses, express or implied,
*        are granted by Apple herein, including but not limited to any patent rights that
*        may be infringed by your derivative works or by other works in which the Apple
*        Software may be incorporated.
*
*        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*        COMBINATION WITH YOUR PRODUCTS.
*
*        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*  Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Runtime.InteropServices;

using MyInputForm;
using MyMovieInfo;

using QTOControlLib;
using QTOLibrary;


namespace MoviePlayer
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private AxQTOControlLib.AxQTControl axQTControl1;
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem5;
		private System.Windows.Forms.MenuItem menuItem10;
		private System.Windows.Forms.MenuItem menuItem13;
		private System.Windows.Forms.MenuItem menuItem14;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		private bool m_bManualFormResize = false;
		private System.Windows.Forms.OpenFileDialog openFileDialog1;
		private System.Windows.Forms.SaveFileDialog saveFileDialog1;
		private System.Windows.Forms.MenuItem mnuOpen;
		private System.Windows.Forms.MenuItem mnuOpenURL;
		private System.Windows.Forms.MenuItem mnuExportWithDialog;
		private System.Windows.Forms.MenuItem mnuExportWithSettings;
		private System.Windows.Forms.MenuItem mnuFullscreen;
		private System.Windows.Forms.MenuItem mnuGetInfo;
		private System.Windows.Forms.MenuItem mnuClose;
		private System.Windows.Forms.MenuItem mnuExit;
		private System.Windows.Forms.MenuItem mnuUndo;
		private System.Windows.Forms.MenuItem mnuCut;
		private System.Windows.Forms.MenuItem mnuCopy;
		private System.Windows.Forms.MenuItem mnuPaste;
		private MovieInfo movInfo = null;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
			this.axQTControl1 = new AxQTOControlLib.AxQTControl();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.mnuOpen = new System.Windows.Forms.MenuItem();
			this.mnuOpenURL = new System.Windows.Forms.MenuItem();
			this.mnuClose = new System.Windows.Forms.MenuItem();
			this.menuItem10 = new System.Windows.Forms.MenuItem();
			this.mnuExportWithDialog = new System.Windows.Forms.MenuItem();
			this.mnuExportWithSettings = new System.Windows.Forms.MenuItem();
			this.menuItem13 = new System.Windows.Forms.MenuItem();
			this.mnuFullscreen = new System.Windows.Forms.MenuItem();
			this.mnuGetInfo = new System.Windows.Forms.MenuItem();
			this.menuItem14 = new System.Windows.Forms.MenuItem();
			this.mnuExit = new System.Windows.Forms.MenuItem();
			this.menuItem5 = new System.Windows.Forms.MenuItem();
			this.mnuUndo = new System.Windows.Forms.MenuItem();
			this.mnuCut = new System.Windows.Forms.MenuItem();
			this.mnuCopy = new System.Windows.Forms.MenuItem();
			this.mnuPaste = new System.Windows.Forms.MenuItem();
			this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
			this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
			((System.ComponentModel.ISupportInitialize)(this.axQTControl1)).BeginInit();
			this.SuspendLayout();
			// 
			// axQTControl1
			// 
			this.axQTControl1.Enabled = true;
			this.axQTControl1.Location = new System.Drawing.Point(0, 0);
			this.axQTControl1.Name = "axQTControl1";
			this.axQTControl1.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("axQTControl1.OcxState")));
			this.axQTControl1.Size = new System.Drawing.Size(192, 192);
			this.axQTControl1.TabIndex = 0;
			this.axQTControl1.Visible = false;
			this.axQTControl1.StatusUpdate += new AxQTOControlLib._IQTControlEvents_StatusUpdateEventHandler(this.axQTControl1_StatusUpdateEventHandler);
			this.axQTControl1.QTEvent += new AxQTOControlLib._IQTControlEvents_QTEventEventHandler(this.axQTControl1_QTEvent);
			this.axQTControl1.SizeChangedEvent += new AxQTOControlLib._IQTControlEvents_SizeChangedEventHandler(this.axQTControl1_SizeChangedEventHandler);
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1,
																					  this.menuItem5});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mnuOpen,
																					  this.mnuOpenURL,
																					  this.mnuClose,
																					  this.menuItem10,
																					  this.mnuExportWithDialog,
																					  this.mnuExportWithSettings,
																					  this.menuItem13,
																					  this.mnuFullscreen,
																					  this.mnuGetInfo,
																					  this.menuItem14,
																					  this.mnuExit});
			this.menuItem1.Text = "File";
			// 
			// mnuOpen
			// 
			this.mnuOpen.Index = 0;
			this.mnuOpen.Text = "&Open...";
			this.mnuOpen.Click += new System.EventHandler(this.mnuOpen_Click);
			// 
			// mnuOpenURL
			// 
			this.mnuOpenURL.Index = 1;
			this.mnuOpenURL.Text = "Open &URL...";
			this.mnuOpenURL.Click += new System.EventHandler(this.mnuOpenURL_Click);
			// 
			// mnuClose
			// 
			this.mnuClose.Index = 2;
			this.mnuClose.Text = "&Close";
			this.mnuClose.Click += new System.EventHandler(this.mnuClose_Click);
			// 
			// menuItem10
			// 
			this.menuItem10.Index = 3;
			this.menuItem10.Text = "-";
			// 
			// mnuExportWithDialog
			// 
			this.mnuExportWithDialog.Index = 4;
			this.mnuExportWithDialog.Text = "Export With Export Dialog...";
			this.mnuExportWithDialog.Click += new System.EventHandler(this.mnuExportWithDialog_Click);
			// 
			// mnuExportWithSettings
			// 
			this.mnuExportWithSettings.Index = 5;
			this.mnuExportWithSettings.Text = "Export With Settings Dialog...";
			this.mnuExportWithSettings.Click += new System.EventHandler(this.mnuExportWithSettings_Click);
			// 
			// menuItem13
			// 
			this.menuItem13.Index = 6;
			this.menuItem13.Text = "-";
			// 
			// mnuFullscreen
			// 
			this.mnuFullscreen.Index = 7;
			this.mnuFullscreen.Text = "Full Screen";
			this.mnuFullscreen.Click += new System.EventHandler(this.mnuFullscreen_Click);
			// 
			// mnuGetInfo
			// 
			this.mnuGetInfo.Index = 8;
			this.mnuGetInfo.Shortcut = System.Windows.Forms.Shortcut.CtrlI;
			this.mnuGetInfo.Text = "Get &Info...";
			this.mnuGetInfo.Click += new System.EventHandler(this.mnuGetInfo_Click);
			// 
			// menuItem14
			// 
			this.menuItem14.Index = 9;
			this.menuItem14.Text = "-";
			// 
			// mnuExit
			// 
			this.mnuExit.Index = 10;
			this.mnuExit.Text = "E&xit";
			this.mnuExit.Click += new System.EventHandler(this.mnuExit_Click);
			// 
			// menuItem5
			// 
			this.menuItem5.Index = 1;
			this.menuItem5.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.mnuUndo,
																					  this.mnuCut,
																					  this.mnuCopy,
																					  this.mnuPaste});
			this.menuItem5.Text = "Edit";
			// 
			// mnuUndo
			// 
			this.mnuUndo.Index = 0;
			this.mnuUndo.Shortcut = System.Windows.Forms.Shortcut.CtrlZ;
			this.mnuUndo.Text = "&Undo";
			this.mnuUndo.Click += new System.EventHandler(this.mnuUndo_Click);
			// 
			// mnuCut
			// 
			this.mnuCut.Index = 1;
			this.mnuCut.Shortcut = System.Windows.Forms.Shortcut.CtrlX;
			this.mnuCut.Text = "Cu&t";
			this.mnuCut.Click += new System.EventHandler(this.mnuCut_Click);
			// 
			// mnuCopy
			// 
			this.mnuCopy.Index = 2;
			this.mnuCopy.Shortcut = System.Windows.Forms.Shortcut.CtrlC;
			this.mnuCopy.Text = "&Copy";
			this.mnuCopy.Click += new System.EventHandler(this.mnuCopy_Click);
			// 
			// mnuPaste
			// 
			this.mnuPaste.Index = 3;
			this.mnuPaste.Shortcut = System.Windows.Forms.Shortcut.CtrlV;
			this.mnuPaste.Text = "&Paste";
			this.mnuPaste.Click += new System.EventHandler(this.mnuPaste_Click);
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(304, 257);
			this.Controls.Add(this.axQTControl1);
			this.Menu = this.mainMenu1;
			this.Name = "Form1";
			this.Text = "QuickTime Control C# Sample";
			this.SizeChanged += new System.EventHandler(this.Form1_Resize);
			this.Load += new System.EventHandler(this.Form1_Load);
			((System.ComponentModel.ISupportInitialize)(this.axQTControl1)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}
		
		// get a movie file from the user
		private string GetFileName()
		{
			if(openFileDialog1.ShowDialog() == DialogResult.OK)
				return openFileDialog1.FileName;
			else
				return "";
		}

		private void axQTControl1_QTEvent(object sender, AxQTOControlLib._IQTControlEvents_QTEventEvent e)
		{
			// Code to handle various QuickTime Events
    
			// When running your code in debug mode you will
			// see the following messages displayed in the
			// Immediate Window
    
			switch (e.eventID)
			{    
				// status strings
				case (int)QTEventIDsEnum.qtEventShowStatusStringRequest:
				{
					string msg = e.eventObject.GetParam(QTEventObjectParametersEnum.qtEventParamStatusString).ToString();
					Console.WriteLine("qtEventShowStatusStringRequest : " + msg + "\n");
				}
				break;
				
				// rate changes
				case (int)QTEventIDsEnum.qtEventRateWillChange:
				{
					string rate = e.eventObject.GetParam(QTEventObjectParametersEnum.qtEventParamMovieRate).ToString();
					Console.WriteLine("RateWillChange to: " + rate + "\n");
				}
				break;

				// time changes
				case (int)QTEventIDsEnum.qtEventTimeWillChange:
				{
					string time = e.eventObject.GetParam(QTEventObjectParametersEnum.qtEventParamMovieTime).ToString();
					Console.WriteLine("TimeWillChange to:" + time + "\n");
				}
				break;

				// audio volume changes
				case (int)QTEventIDsEnum.qtEventAudioVolumeDidChange:
				{
					string vol = e.eventObject.GetParam(QTEventObjectParametersEnum.qtEventParamAudioVolume).ToString();
					Console.WriteLine("AudioVolumeDidChange to: " + vol + "\n");
				}
				break;
			}
		}

		private void addMovieEventListeners(QTMovie myMovie)
		{
			// Contains code to demonstrate how to add listeners
			// for various QuickTime Events
	    
			// Make sure a movie is loaded first
			if (myMovie == null) return;

			// status string listener
			myMovie.EventListeners.Add(QTOLibrary.QTEventClassesEnum.qtEventClassApplicationRequest, 
				QTOLibrary.QTEventIDsEnum.qtEventShowStatusStringRequest, 0, null);

			// rate change listener
			myMovie.EventListeners.Add(QTOLibrary.QTEventClassesEnum.qtEventClassStateChange, 
											QTOLibrary.QTEventIDsEnum.qtEventRateWillChange, 0, null);

			// time change listener
			myMovie.EventListeners.Add(QTOLibrary.QTEventClassesEnum.qtEventClassTemporal, 
									QTOLibrary.QTEventIDsEnum.qtEventTimeWillChange, 0, null);
	    
			// audio volume change listener
			myMovie.EventListeners.Add(QTOLibrary.QTEventClassesEnum.qtEventClassAudio,
					QTOLibrary.QTEventIDsEnum.qtEventAudioVolumeDidChange, 0, null);
						  
		}

		// remove all event listeners for the movie
		private void removeMovieEventListeners(QTMovie myMovie)
		{
			// Make sure a movie is loaded first
			if (myMovie != null)
			{
				// Remove all event listeners
				myMovie.EventListeners.RemoveAll();
			}
		}

		// Open
		private void mnuOpen_Click(object sender, System.EventArgs e)
		{
			// prompt for file
			string filePath = GetFileName();

			if (filePath != "")
				OpenMovie(filePath);
		}

		// Open URL
		private void mnuOpenURL_Click(object sender, System.EventArgs e)
		{
			InputForm input = new InputForm();
			
			// prompt user for url
			DialogResult dlgResult = input.DisplayInputBox();
			if (dlgResult == DialogResult.OK)
				OpenMovie(input.GetTextResult());
		}

		// Open a movie
		private void OpenMovie(string url)
		{
			string errMsg = "";

			try
			{
				axQTControl1.Sizing = QTSizingModeEnum.qtControlFitsMovie;
				axQTControl1.URL = url;
				axQTControl1.Sizing = QTSizingModeEnum.qtMovieFitsControl;
				addMovieEventListeners(axQTControl1.Movie);
				axQTControl1.Show();
				if (movInfo != null)
				{
					if (movInfo.Created && movInfo.Visible)
					{
						// show movie info. window
						movInfo.SetInfoMovie(axQTControl1.Movie);
					}
				}
			}
			catch(COMException ex)
			{
				// catch COM exceptions from QT control
				errMsg = "Error Code: " + ex.ErrorCode.ToString("X") + "\n";
				QTUtils qtu = new QTUtils();
				errMsg += "QT Error code : " + qtu.QTErrorFromErrorCode(ex.ErrorCode).ToString();
			}
			catch(Exception ex)
			{
				// catch any exception
				errMsg = ex.ToString();
			}

			if (errMsg != "")
			{
				string msg = "Unable to open movie:\n\n";
				msg += url + "\n\n";
				msg += errMsg;
				MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		// Export With Export Dialog
		private void mnuExportWithDialog_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;

			// Perform export with user configured
			// settings.
		    
			// If you want to allow the user to configure
			// everything (exporter type, file, options:
			// same as QuickTime Player Export) then use
			// the following:
		    
			try
			{
				QTQuickTime qt = axQTControl1.QuickTime;
		        
				if (qt.Exporters.Count == 0) qt.Exporters.Add();

				QTExporter ex = qt.Exporters[1];
				
				// Set exporter data source - could also be a track
				ex.SetDataSource(axQTControl1.Movie);
		    
				// Display the export dialog
				ex.ShowExportDialog();
			}
			catch(COMException ex)
			{
				if (ex.ErrorCode != -2147156096 )	// Ignore Cancel
					MessageBox.Show("Error code: " + ex.ErrorCode.ToString("X"), "Export Error");
			}
			catch(Exception ex)
			{
				MessageBox.Show(ex.ToString(), "Export Error");
			}
		}

		// Export With Settings Dialog for a specific exporter
		private void mnuExportWithSettings_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;

			// Perform export for a specific exporter.

			// To perform an export operation with custom
			// configured Settings you first need to create
			// an instance of the QTExporter object and then
			// you need to configure it by setting its data
			// source (movie or track) and exporter type
			// (QuickTime Movie, AVI, 3G, MPEG-4, PNG, etc.)
			// as well as the file you want to export to.

			// Then call ShowSettingsDialog to display the
			// export options dialog for the selected exporter
			// type, and finally call BeginExport.

			// Once you have configured an exporter, you can
			// change its data source and reuse it with other
			// movies and/or tracks as many times as you like.
			// You can also persist exporter settings by getting
			// and setting the XML property (a string).
		    
			try
			{
				QTQuickTime qt = axQTControl1.QuickTime;
		        
				if (qt.Exporters.Count == 0) qt.Exporters.Add();

				QTExporter ex = qt.Exporters[1];

				// Set exporter type: AVI, 3G, MPEG-4, PNG, etc.
				ex.TypeName = "QuickTime Movie";
		    
				// Set selection to export (optional)
				ex.StartTime = 0;      // Optional
				ex.EndTime = 600;      // Optional
		    
				// Set exporter data source - could also be a track
				ex.SetDataSource(axQTControl1.Movie);
		    		    
				// Display the settings dialog for the selected exporter type
				ex.ShowSettingsDialog();
		    
				saveFileDialog1.FileName = "exported.mov";

				if(saveFileDialog1.ShowDialog() == DialogResult.OK)
				{				
					ex.DestinationFileName = saveFileDialog1.FileName;

					ex.ShowProgressDialog = true;
					Cursor.Current = Cursors.WaitCursor;
					ex.BeginExport();
					Cursor.Current = Cursors.Arrow;
				}
			}
			catch(COMException ex)
			{
				if (ex.ErrorCode != -2147156096 )	// Ignore Cancel
					MessageBox.Show("Error code: " + ex.ErrorCode.ToString("X"), "Export Error");
			}
			catch(Exception ex)
			{
				MessageBox.Show(ex.ToString(), "Export Error");
			}
		}

		// Full Screen
		private void mnuFullscreen_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.URL != "")
			{
				axQTControl1.FullScreen = true;
			}
		}

		// Close
		private void mnuClose_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;

			removeMovieEventListeners(axQTControl1.Movie);
			axQTControl1.URL = "";

			if (movInfo != null)
			{
				if (movInfo.Created && movInfo.Visible)
					movInfo.Dispose();
			}
			axQTControl1.Hide();
		}

		// Undo
		private void mnuUndo_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;
			axQTControl1.Movie.Undo();
		}

		// Cut
		private void mnuCut_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;
			axQTControl1.Movie.Cut();
		}

		// Copy
		private void mnuCopy_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;
			axQTControl1.Movie.Copy();
		}

		// Paste
		private void mnuPaste_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;
			axQTControl1.Movie.Paste();
		}

		// Exit
		private void mnuExit_Click(object sender, System.EventArgs e)
		{
			Application.Exit();
		}

		private void axQTControl1_SizeChangedEventHandler(object sender, AxQTOControlLib._IQTControlEvents_SizeChangedEvent e)
		{
			if (m_bManualFormResize) return;

			// resize window to accomodate control
			this.ClientSize = axQTControl1.Size;

		}

		// resize the form
		private void Form1_Resize(object sender, System.EventArgs e)
		{
			m_bManualFormResize = true;
			axQTControl1.Size = this.ClientSize;
			m_bManualFormResize = false;
		}

		private void Form1_Load(object sender, System.EventArgs e)
		{
			axQTControl1.ErrorHandling = (int)QTErrorHandlingOptionsEnum.qtErrorHandlingRaiseException;
			axQTControl1.Sizing = QTSizingModeEnum.qtControlFitsMovie;
		}

		// Get Movie Info
		private void mnuGetInfo_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.URL != "")
			{
				if (movInfo == null)
				{
					movInfo = new MovieInfo();	// movie info. object
				}
				else if (movInfo.Created == false)
				{
					movInfo = new MovieInfo();	// movie info. object
				}

				movInfo.SetInfoMovie(axQTControl1.Movie);
				movInfo.Show();
				movInfo.BringToFront();
			}
		}

		// Status update event handler
		// Handle movie fullscreen events
		void axQTControl1_StatusUpdateEventHandler(object sender, AxQTOControlLib._IQTControlEvents_StatusUpdateEvent e)
		{
			switch(e.statusCodeType)
			{
				case (int)QTStatusCodeTypesEnum.qtStatusCodeTypeControl:
				{
					switch (e.statusCode)
					{
							// fullscreen begin
						case (int)QTStatusCodesEnum.qtStatusFullScreenBegin:
							this.Hide();	// hide movie window
							break;
							
							// fullscreen end
						case (int)QTStatusCodesEnum.qtStatusFullScreenEnd:
							axQTControl1.SetScale(1);	// set back to a reasonable size
							this.Show();	// show movie window again
							break;
					}
				}
				break;
			}
		}

	}
}