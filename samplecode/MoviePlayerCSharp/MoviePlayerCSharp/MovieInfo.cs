/*
*  File:    MovieInfo.cs
* 
*  Contains:  Code which displays movie attributes.
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

using QTOControlLib;
using QTOLibrary;

namespace MyMovieInfo
{
	/// <summary>
	/// Summary description for MovieInfo.
	/// </summary>
	public class MovieInfo : System.Windows.Forms.Form
	{
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label lblName;
		private System.Windows.Forms.Label lblInfo;
		private System.Windows.Forms.Label lblCaptions;
		private System.Windows.Forms.Label lblData;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public MovieInfo()
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
				if(components != null)
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.lblData = new System.Windows.Forms.Label();
			this.lblCaptions = new System.Windows.Forms.Label();
			this.lblName = new System.Windows.Forms.Label();
			this.lblInfo = new System.Windows.Forms.Label();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.lblData);
			this.groupBox1.Controls.Add(this.lblCaptions);
			this.groupBox1.Location = new System.Drawing.Point(8, 128);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(400, 240);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			// 
			// lblData
			// 
			this.lblData.Location = new System.Drawing.Point(128, 24);
			this.lblData.Name = "lblData";
			this.lblData.Size = new System.Drawing.Size(264, 208);
			this.lblData.TabIndex = 1;
			// 
			// lblCaptions
			// 
			this.lblCaptions.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblCaptions.Location = new System.Drawing.Point(16, 24);
			this.lblCaptions.Name = "lblCaptions";
			this.lblCaptions.Size = new System.Drawing.Size(96, 208);
			this.lblCaptions.TabIndex = 0;
			this.lblCaptions.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// lblName
			// 
			this.lblName.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblName.Location = new System.Drawing.Point(16, 16);
			this.lblName.Name = "lblName";
			this.lblName.Size = new System.Drawing.Size(384, 24);
			this.lblName.TabIndex = 1;
			this.lblName.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblInfo
			// 
			this.lblInfo.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblInfo.Location = new System.Drawing.Point(16, 56);
			this.lblInfo.Name = "lblInfo";
			this.lblInfo.Size = new System.Drawing.Size(384, 64);
			this.lblInfo.TabIndex = 2;
			this.lblInfo.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// MovieInfo
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(416, 378);
			this.Controls.Add(this.lblInfo);
			this.Controls.Add(this.lblName);
			this.Controls.Add(this.groupBox1);
			this.Name = "MovieInfo";
			this.Text = "MovieInfo";
			this.Load += new System.EventHandler(this.MovieInfo_Load);
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private QTMovie m_movie;	// active movie

		private void MovieInfo_Load(object sender, System.EventArgs e)
		{
			DisplayMovieData();
		}

		// Set the active movie for our info. display
		public void SetInfoMovie(QTMovie inMovie)
		{
			m_movie = inMovie;
			DisplayMovieData();
		}
		
		// Get the specified annotation from the movie
		private string MovieAnnotation(int inAnnoID, QTMovie inMovie)
		{
			string anno = string.Empty;
			if (inMovie != null)
			{
				try
				{
					// get movie annotation
					anno = inMovie.get_Annotation(inAnnoID);
				}
				catch
				{
					// an error here means movie does not contain
					// the desired annotation
				}
			}
			return anno;
		}

		// show movie info. in a window 
		private void DisplayMovieData()
		{
			lblName.Text = "";
			lblInfo.Text = "";
			lblCaptions.Text = "";
			lblData.Text = "";

			if (m_movie == null) return;
			
			try
			{
				string annoStr = string.Empty;

				// Display some popular movie annotations
				annoStr = this.MovieAnnotation((int)QTAnnotationsEnum.qtAnnotationFullName, m_movie);
				if (annoStr.Trim() != "")
					lblName.Text += annoStr + "\n";

				// copyright
				annoStr = this.MovieAnnotation((int)QTAnnotationsEnum.qtAnnotationCopyright, m_movie);
				if (annoStr.Trim() != "")
					lblInfo.Text += annoStr + "\n";

				// author
				annoStr = this.MovieAnnotation((int)QTAnnotationsEnum.qtAnnotationAuthor, m_movie);
				if (annoStr.Trim() != "")
					lblInfo.Text += annoStr + "\n";

				// comments
				annoStr = this.MovieAnnotation((int)QTAnnotationsEnum.qtAnnotationComments, m_movie);
				if (annoStr.Trim() != "")
					lblInfo.Text += annoStr + "\n";

				// description
				annoStr = this.MovieAnnotation((int)QTAnnotationsEnum.qtAnnotationDescription, m_movie);
				if (annoStr.Trim() != "")
					lblInfo.Text += annoStr + "\n";
			
				// Display movie characteristics

				// width, height
				lblCaptions.Text += "Size:" + "\n";
				lblData.Text += m_movie.Width.ToString() + " x " + m_movie.Height.ToString() + "\n";

				lblCaptions.Text = lblCaptions.Text + "\n";
				lblData.Text = lblData.Text + "\n";

				// duration
				lblCaptions.Text += "Duration:" + "\n";
				lblData.Text += m_movie.Duration.ToString() + "\n";

				lblCaptions.Text = lblCaptions.Text + "\n";
				lblData.Text = lblData.Text + "\n";

				QTTracks tracks = m_movie.Tracks;
				IEnumerator trackEnum = tracks.GetEnumerator();

				// display movie track information
				while (trackEnum.MoveNext() == true)
				{
					QTTrack currTrackObj = (QTTrack)trackEnum.Current;

					// track display name
					lblCaptions.Text += currTrackObj.DisplayName + ": " + "\n";
					// track format
					lblData.Text += currTrackObj.Format;

					if (currTrackObj.Height > 0)
					{
						// track width/height
						lblData.Text += ", " + 
							currTrackObj.Width.ToString() + " x " + 
								currTrackObj.Height.ToString();
					}

					lblData.Text +=  "\n";
				}

				lblCaptions.Text = lblCaptions.Text + "\n";
				lblData.Text = lblData.Text + "\n";
				
				// movie name
				lblCaptions.Text += "Source:" + "\n";
				lblData.Text += m_movie.URL + "\n";

			}
			catch(Exception ex)
			{
				MessageBox.Show(ex.ToString());
			}


		}
	}
}
