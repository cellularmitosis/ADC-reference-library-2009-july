/*

File: Open_URL.java

Abstract: This demo program shows how to display any QuickTime content
			within a java.awt.Frame using the QTFactory to create a
			QTComponent.

Version: 1.1

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
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

Copyright © 1996-2005 Apple Computer, Inc., All Rights Reserved

*/

import java.awt.*;
import java.awt.event.*;

// Open_URL class implements the Open_URL window which appears
//  when the user selects the "Open URL..." menu item.
public class Open_URL extends Dialog implements ActionListener
{
	public Open_URL(PlayMovie parent)
	{
		super ( parent, true );
		myPlayMovie = parent;
		
		// Setup dialog attributes
		setLayout(new FlowLayout (FlowLayout.CENTER));
		setResizable(true);
		setSize(452,126);
		setFont(new Font ("Dialog", Font.PLAIN, 12));
		setBackground (new Color (12632256));
		
		// The text label to the left of the URL text entry field
		label2 = new Label ("URL:");
		add(label2);
		
		// The URL text entry field
		urlTextField = new TextField ("file:///... Enter an URL to a movie");
		urlTextField.setFont(new Font ("Dialog", Font.PLAIN, 10));
		add(urlTextField);
		setTitle("Open URL");
		
		// OK and Cancel buttons
		okButton = new Button ();
		okButton.setLabel("OK");
		add(okButton);
                
		cancelButton = new Button ();
		cancelButton.setLabel("Cancel");
		add(cancelButton);
		
		urlTextField.addActionListener(this);
		cancelButton.addActionListener(new ActionListener () {
			public void actionPerformed(ActionEvent event)
			{
				dispose();
			}
		});
		
		// When the OK button is clicked, attempt to open the file
		//  entered into the URL text field.
		okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event)
			{
				myPlayMovie.createNewMovieFromURL(urlTextField.getText());
				dispose();
			}
		});
		
		pack();
	}
	
	Label		label2;
	Label		label1;
	Button		okButton;
	Button		cancelButton;
	TextField   urlTextField;
	
	private PlayMovie myPlayMovie;

	public void actionPerformed(ActionEvent evt)
	{		
		myPlayMovie.createNewMovieFromURL(urlTextField.getText());
		dispose();
	}
}
