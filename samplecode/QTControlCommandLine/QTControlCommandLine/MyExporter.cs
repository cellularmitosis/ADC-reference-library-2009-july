/*

File: MyExporter.cs

Abstract: Contains code which uses the QTOControl control hosted
 in a Form to export an existing QuickTime movie to a different
 format file.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

using System;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using QTOLibrary;

namespace QTControlExporter
{
    /// <summary>
    /// MyExporter Class
	/// 
    /// Contains code to export an existing QuickTime
    /// movie to a different file format
    /// </summary>
	class MyExporter
	{
		private static AxQTOControlLib.AxQTControl qtc;
		private static Form1 frm;

		private static int exportProgress;

        private static Boolean promptForSettings = false;
        private static Boolean saveSettings = false;
        private static Boolean loadSettings = false;
        private static String inFile;
        private static String outFile;
        private static String codec;
        private static string loadSettingsFile;
        private static string saveSettingsFile;

        // display command-line usage requirements
        static void PrintUsage()
        {
            Console.WriteLine("\tUsage error:");

            Console.WriteLine("\tQTControlExporter.exe -i \\path\\to\\inputfile -o \\path\\to\\outputfile -codec codecName | { [-loadSettings \\path\\to\\settingsFile] | [-promptForSettings] | {[-promptForSettings]  &&  [-saveSettings \\path\\to\\settingsFile]} }\n");

            Console.WriteLine("\t-codec               = New format for the exported file.  Valid values are: 3G, AVI, DV Stream, FLC, Hinted Movie, Image Sequence, iPod (320X240), MPEG-4, QuickTime Media Link, QuickTime Movie, AIFF, AU, Wave, BMP, JPEG, MacPaint, Photoshop, PICT, Picture, PNG, QuickTime Image, SGI, TGA, TIFF.\n");
            Console.WriteLine("\t-promptForSettings   = Will display the settings dialog during export.\n");
            Console.WriteLine("\t-saveSettings        = Save exporter settings to a settings file.\n");
            Console.WriteLine("\t-loadSettings        = Load exporter settings from a settings file.\n");
        }

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
            int arguments = args.Length;
            int i;
            // parse the command-line arguments
            for (i = 0; i < arguments; i++)
            {
                if ((0 == args[i].CompareTo("-i")) && i + 1 < arguments)
                    inFile = args[++i];
                if ((0 == args[i].CompareTo("-o")) && i + 1 < arguments)
                    outFile = args[++i];
                if ((0 == args[i].CompareTo("-codec")) && i + 1 < arguments)
                    codec = args[++i];
                if ((0 == args[i].CompareTo("-promptForSettings")) && i + 1 <= arguments)
                    promptForSettings = true;
                if ((0 == args[i].CompareTo("-saveSettings")) && i + 1 <= arguments)
                {
                    saveSettings = true;
                    saveSettingsFile = args[++i];
                }
                if ((0 == args[i].CompareTo("-loadSettings")) && i + 1 <= arguments)
                {
                    loadSettings = true;
                    loadSettingsFile = args[++i];
                }
            }

            // check for command usage errors
            if (null == inFile || null == outFile || ((promptForSettings == false) && (null == codec))
                || ((saveSettings == true) && (promptForSettings == false)))
            {
                PrintUsage();
                return;
            }

            // create a new Form - the QTControl must be hosted on a Form or Window
			frm = new Form1();

			try
			{	
                // retrieve the instance of the QTControl object from the Form
				qtc = frm.QTControl;

                // specify that any errors generated will raise an exception
				qtc.ErrorHandling = (int) QTErrorHandlingOptionsEnum.qtErrorHandlingRaiseException;

                // use a QTEvent handler to display export progress information
				qtc.QTEvent += new AxQTOControlLib._IQTControlEvents_QTEventEventHandler(OnQTEvent);

                // get the source movie for the export operation
				qtc.URL = inFile;
                Console.WriteLine("Input file  : " + inFile);
                Console.WriteLine("Output file : " + outFile);

				if (qtc.Movie != null)
				{
                    // get the QuickTime object from the QTControl
					QTOLibrary.QTQuickTime qt = qtc.QuickTime;

					if (qt != null)
					{
                        // must first add an exporter to the collection
						if (qt.Exporters.Count == 0)
							qt.Exporters.Add();

                        // retrieve the QTExporter object from the collection
						QTExporter qtexp = qt.Exporters[1];

						if (qtexp != null)
						{
							qtexp.SetDataSource(qtc.Movie);
                            // export our movie to specified format 
                            // (can be overriden by load settings)
							qtexp.TypeName = codec;
                            // dont show the progress dialog
                            qtexp.ShowProgressDialog = false;
                            // specify destination file for the export operation
							qtexp.DestinationFileName = outFile;
                            // optionally load exporter settings from a file if one is specified
                            if (loadSettings)
                            {
                                // first check if settings file actually exists
                                if (File.Exists(loadSettingsFile))
                                {
                                    StreamReader reader = new StreamReader(loadSettingsFile);
                                    CFObject settingsObject = new CFObject();
                                    string xml = reader.ReadToEnd();    // read settings file as an XML string
                                    settingsObject.XML = xml;   // load XML settings string into a CFObject
                                    qtexp.Settings = settingsObject;    // set the XML onto the exporter
                                    reader.Close();
                                    Console.WriteLine("Settings loaded from: " + loadSettingsFile);
                                }
                            }
                            // we'll show the settings dialog if the user asks for it
                            if (promptForSettings)
                            {
                                // show the exporter settings dialog
                                qtexp.ShowSettingsDialog();
                                
                                // Optionally save the exporter settings to a file
                                if (saveSettings)
                                {
                                    StreamWriter writer = new StreamWriter(saveSettingsFile, false);
                                    writer.Write(qtexp.Settings.XML);   // write the settings as XML
                                    writer.Close();
                                    Console.WriteLine("Settings saved to: " + saveSettingsFile);
                                }
                            }
                            // we want export progress events
							qtexp.EventListeners.Add(QTEventClassesEnum.qtEventClassProgress, QTEventIDsEnum.qtEventExportProgress, 0, 0);
							
                            Console.WriteLine("Export started...");
							
                            exportProgress = 0;
							
                            // do the export!
                            qtexp.BeginExport();
							
                            Console.WriteLine("\nExport done.");
						}
					}
				}
			}
			catch(COMException ex)
			{
				QTUtils qtu = new QTUtils();					
				Console.WriteLine("Error occurred: " + ex.ErrorCode.ToString("x") + " " + qtu.QTErrorFromErrorCode(ex.ErrorCode).ToString());
			}
			qtc.URL = "";
			frm.Dispose();
		}

        /// <summary>
        /// OnQTEvent
        /// 
        /// QuickTime event listener to display export progress information
        /// </summary>

		static void OnQTEvent(object sender, AxQTOControlLib._IQTControlEvents_QTEventEvent e)
		{
			switch (e.eventID)
			{
				case (int) QTEventIDsEnum.qtEventExportProgress:

                    // display export progress
					int pcnt = (int)(float) e.eventObject.GetParam(QTEventObjectParametersEnum.qtEventParamAmount);
					if (pcnt >= (exportProgress + 10))
					{
						Console.Write(pcnt.ToString() + "% ");
						exportProgress = pcnt;
					}
					break;
			}
		}
	}
}
