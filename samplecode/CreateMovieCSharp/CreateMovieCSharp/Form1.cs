/*
*  File:    Form1.cs
* 
*  Contains:  Code demonstrating how to create a movie from a series of images
*             Also shows how to perform a save operation for the newly created movie
*  
*  Version:  1.0
* 
*  Created:  10/26/05
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

using QTOControlLib;
using QTOLibrary;


namespace CreateMovie
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private AxQTOControlLib.AxQTControl axQTControl1;
		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem2;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.SaveFileDialog saveFileDialog1;
		private System.Windows.Forms.MenuItem menuItem3;
		private System.Windows.Forms.OpenFileDialog openFileDialog1;

		private bool m_bManualFormResize = false;

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
			this.menuItem3 = new System.Windows.Forms.MenuItem();
			this.menuItem2 = new System.Windows.Forms.MenuItem();
			this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
			this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
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
			this.axQTControl1.SizeChangedEvent += new AxQTOControlLib._IQTControlEvents_SizeChangedEventHandler(this.axQTControl1_SizeChangedEventHandler);
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem3,
																					  this.menuItem2});
			this.menuItem1.Text = "File";
			// 
			// menuItem3
			// 
			this.menuItem3.Index = 0;
			this.menuItem3.Text = "New with Image Sequence...";
			this.menuItem3.Click += new System.EventHandler(this.menuItem3_Click);
			// 
			// menuItem2
			// 
			this.menuItem2.Index = 1;
			this.menuItem2.Text = "Save";
			this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(272, 245);
			this.Controls.Add(this.axQTControl1);
			this.Menu = this.mainMenu1;
			this.Name = "Form1";
			this.Text = "Movie";
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

		private void Form1_Load(object sender, System.EventArgs e)
		{
			axQTControl1.ErrorHandling = (int)QTErrorHandlingOptionsEnum.qtErrorHandlingRaiseException;
			// have the control resize to fit the movie
			axQTControl1.Sizing = QTSizingModeEnum.qtControlFitsMovie;
		}

		// Create a movie from a sequence of images
		private void CreateMovieFromImages(string fileSeqPathName)
		{
			string errMsg = "";

			try
			{
				axQTControl1.Sizing = QTSizingModeEnum.qtControlFitsMovie;

				// create movie from images in our project folder
				axQTControl1.CreateNewMovieFromImages(fileSeqPathName,30,true);
				if (axQTControl1.Movie != null)
				{
					axQTControl1.Sizing = QTSizingModeEnum.qtMovieFitsControl;
					// prepare and play the movie
					axQTControl1.Movie.Preroll();
					axQTControl1.Movie.Play(1.0);
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
				string msg = "Unable to create movie:\n\n";
				msg += errMsg;
				MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}

		}

		// handle form size changes
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

		// Save menu
		private void menuItem2_Click(object sender, System.EventArgs e)
		{
			if (axQTControl1.Movie == null) return;
			if (axQTControl1.URL == "") return;

			saveFileDialog1.FileName = "SavedMovie.mov";

			// ask the user to save the movie file to disk
			if(saveFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string errMsg = "";

				try
				{
					// save the movie as self-contained
					axQTControl1.Movie.SaveSelfContained(saveFileDialog1.FileName,true);
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
					string msg = "Unable to save movie:\n\n";
					msg += errMsg;
					MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
		}

		// New from Image Sequence menu item
		private void menuItem3_Click(object sender, System.EventArgs e)
		{
			// prompt the user to select an image sequence
			if(openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				// create a movie from the sequence of images
				CreateMovieFromImages(openFileDialog1.FileName);
			}
		}
	}
}
