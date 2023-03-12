/*

	File:		SwapLAF.java
	
	Abstract:	Main class/logic for SwapLAF sample. See updateLookAndFeel() for main logic.
	
	Version:	1.3
	
	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	IMPORTANT:  This Apple software is supplied to 
	you by Apple Computer, Inc. ("Apple") in 
	consideration of your agreement to the following 
	terms, and your use, installation, modification 
	or redistribution of this Apple software 
	constitutes acceptance of these terms.  If you do 
	not agree with these terms, please do not use, 
	install, modify or redistribute this Apple 
	software.

	In consideration of your agreement to abide by 
	the following terms, and subject to these terms, 
	Apple grants you a personal, non-exclusive 
	license, under Apple's copyrights in this 
	original Apple software (the "Apple Software"), 
	to use, reproduce, modify and redistribute the 
	Apple Software, with or without modifications, in 
	source and/or binary forms; provided that if you 
	redistribute the Apple Software in its entirety 
	and without modifications, you must retain this 
	notice and the following text and disclaimers in 
	all such redistributions of the Apple Software. 
	Neither the name, trademarks, service marks or 
	logos of Apple Computer, Inc. may be used to 
	endorse or promote products derived from the 
	Apple Software without specific prior written 
	permission from Apple.  Except as expressly 
	stated in this notice, no other rights or 
	licenses, express or implied, are granted by 
	Apple herein, including but not limited to any 
	patent rights that may be infringed by your 
	derivative works or by other works in which the 
	Apple Software may be incorporated.

	The Apple Software is provided by Apple on an "AS 
	IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
	IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
	WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
	AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
	THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
	OR IN COMBINATION WITH YOUR PRODUCTS.

	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
	SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
	OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
	REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
	THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
	UNDER THEORY OF CONTRACT, TORT (INCLUDING 
	NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
	IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
	SUCH DAMAGE.

*/
package apple.dts.samplecode.swaplaf;

import java.awt.*;
import java.util.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import com.apple.mrj.*;
import javax.swing.*;

public class SwapLAF extends JFrame
                      implements  ActionListener,
                                  MRJAboutHandler,
                                  MRJQuitHandler
{
    static final String APP_TITLE = "SwapLAF";
    static final String HELLO = "Hello World!";
    static final String BUTTON_TEXT = "Sample Button";
    static final String LAF = "Look-And-Feel";
    static final String FONT_LABEL = "Label.font";
        
    protected JMenu LAFMenu;
    protected JMenuItem currentSelection, previousSelection;
    protected ButtonGroup LAFGroup;
    protected JLabel label;
    protected JMenuBar mainMenuBar = new JMenuBar();
    protected JButton sampleButton = new JButton(BUTTON_TEXT);

    protected AboutBox aboutBox;

    protected HashMap installedLAFs;        
        
        
    public SwapLAF() {
        super(APP_TITLE);
       
        UIManager.LookAndFeelInfo[] plafs = UIManager.getInstalledLookAndFeels();
        installedLAFs = new HashMap(plafs.length);
        for (int i = 0; i < plafs.length; i++){
            installedLAFs.put(plafs[i].getName(), plafs[i].getClassName());
        }
        
        Container c = getContentPane();
        c.setLayout(new BorderLayout());

        LAFGroup = new ButtonGroup();
        setUpLAFMenu();
        setJMenuBar(mainMenuBar);
        
        JPanel bottom = new JPanel(new FlowLayout(FlowLayout.CENTER, 20, 20));
        bottom.add(sampleButton);
        c.add(bottom, BorderLayout.SOUTH);
        label = new JLabel(getMessage(), JLabel.CENTER);
        label.setFont(label.getFont().deriveFont(36f));
        c.add(label);
        
        Toolkit.getDefaultToolkit();
        MRJApplicationUtils.registerAboutHandler(this);
        MRJApplicationUtils.registerQuitHandler(this);

        setSize(400,180);
        setLocation(200,200);
        setVisible(true);
    }
    
    public void setUpLAFMenu() {
        LAFMenu = new JMenu(LAF);
        Iterator LAFNames = installedLAFs.keySet().iterator();
        while (LAFNames.hasNext()) {
            String name = (String)LAFNames.next();
            JRadioButtonMenuItem jmi = new JRadioButtonMenuItem(name);
            LAFMenu.add(jmi);
            LAFGroup.add(jmi);
            jmi.addActionListener(this);
            if (installedLAFs.get(name).equals(UIManager.getLookAndFeel().getClass().getName())) {
                jmi.setSelected(true);
                currentSelection = jmi;
            }
        }
        
        mainMenuBar.add(LAFMenu);
    }

    public String getMessage() {
        String lafName = UIManager.getLookAndFeel().getName();
        if (lafName != null) {
            return "This is " + lafName;
        } else {
            return HELLO;
        }
    }

    public void handleAbout() {
        // create a new AboutBox every time to work around
        // refresh latencies when changing LAF
        aboutBox = new AboutBox(this);
        aboutBox.show();
    }

    public void handleQuit() {	
        System.exit(0);
    }

    // ActionListener interface (for menus)
    public void actionPerformed(ActionEvent newEvent) {
        previousSelection = currentSelection;
        currentSelection = (JMenuItem)newEvent.getSource();
        String name = currentSelection.getText();
        String className = (String) installedLAFs.get(name);
        if (className != null) {
            try {
                updateLookAndFeel(name, className);
            } catch (Exception e) {
                System.out.println("Uh-oh..." + e);
            }
        }
    }

    /** 
     *  Change the look-and-feel to the specified class.
     *  Alert the user if there were problems loading the PLAF.
     *  @param name (String) the presentable name for the class
     *  @param className (String) the className to be fed to the UIManager
     *  @see javax.swing.UIManager#setLookAndFeel
     *  @see javax.swing.SwingUtilities#updateComponentTreeUI
     */ 
    public void updateLookAndFeel(String name, String className) {
	try {
            // Set the new look and feel, and update the sample message to reflect it.
	    UIManager.setLookAndFeel(className);
            label.setText(getMessage());
            // Grab the font from the LAF's defaults hash to use for the sample message
            label.setFont(((Font)UIManager.getLookAndFeel().getDefaults().get(FONT_LABEL)).deriveFont(36f));
            // Call for a UI refresh to the new LAF starting at the highest level
            SwingUtilities.updateComponentTreeUI(this);
        } catch (Exception e) {
            String errMsg = "The " + name + " look-and-feel ";
            if (e instanceof UnsupportedLookAndFeelException){
                errMsg += "is not supported on this platform.";
            } else if (e instanceof ClassNotFoundException){
                errMsg += "could not be found.";
            } else {
                errMsg += "could not be loaded.";
                System.out.println(e);
            }
            
            currentSelection = previousSelection;
            currentSelection.setSelected(true);
            JOptionPane.showMessageDialog(this, errMsg, APP_TITLE, JOptionPane.ERROR_MESSAGE);
            
        }
    }
                
    public static void main(String args[]) {
        new SwapLAF();
    }

}
