/*
 * QuickTime for Java SDK Sample Code

   Usage subject to restrictions in SDK License Agreement
 * Copyright: © 1996-1999 Apple Computer, Inc.

 */

import java.awt.*;
import java.awt.event.*;
import java.util.Vector;

import quicktime.*;
import quicktime.qd.*;
import quicktime.app.display.*;
import quicktime.app.image.*;


public class TextDemo extends Frame
{
	public static final int		width = 1024;
	public static final int		height = 640;
	public static QDRect		myBounds = new QDRect(width, height);
	public QTCanvas				myQTCanvas;
	public QDGraphics			newQDG;
	public ImagePresenter		imagePres;



	public static void main (String args[])
	{
		try
		{ 
			QTSession.open();
			TextDemo td = new TextDemo ("Text Demo in QTJ");
			td.pack();
			td.show();
			td.toFront();
		} catch (Exception e)
		{
			e.printStackTrace();
			QTSession.close();
		}
	}



	TextDemo(String title) throws Exception
	{
		super(title);

		myQTCanvas = new QTCanvas ();
		add("Center", myQTCanvas);

		// add a WindowListener to close the program down
		addWindowListener(new WindowAdapter ()
		{
			public void windowClosing(WindowEvent e)
			{
				QTSession.close();
				dispose();
			}

			public void windowClosed (WindowEvent e)
			{ 
				System.exit(0);
			}
		});

		// create a QDGraphics and QDDrawer to draw the text
		newQDG = new QDGraphics (myBounds);
		newQDG.beginDraw(new QDDrawer()
		{
			public void draw(QDGraphics g) throws QTException
			{
				int			strIndex = 0;
				int			tmpInt, fntNum, x, y;
				int			savedSize, savedFNum, savedStyle, savedMode;
				String		strToDraw;

				savedFNum = g.getTextFont();
				savedSize = g.getTextSize();
				savedStyle = g.getTextFace();
				savedMode = g.getTextMode();

				// prepare canvas
				g.setBackColor(QDColor.gray);
				g.eraseRect(myBounds);

				// calculate where to start centered line of text
				g.textSize(24);
				g.textFace(QDConstants.bold + QDConstants.italic);
				g.setForeColor(QDColor.green);
				strToDraw = new String ("P r e s e n t i n g...");
				tmpInt = g.textWidth(strToDraw, 0, strToDraw.length());
				x = ((TextDemo.width / 2) - (tmpInt / 2));
				y = 30;
				g.moveTo(x, y);
				x = 40;

				// use drawChar
				g.drawChar('P');
				tmpInt = g.charWidth(' ');
				g.move(tmpInt, 0);
				g.drawChar('r');
				g.move(tmpInt, 0);
				g.drawChar('e');
				g.move(tmpInt, 0);
				g.drawChar('s');
				g.move(tmpInt, 0);
				g.drawChar('e');
				g.move(tmpInt, 0);
				g.drawChar('n');
				g.move(tmpInt, 0);
				g.drawChar('t');
				g.move(tmpInt, 0);
				g.drawChar('i');
				g.move(tmpInt, 0);
				g.drawChar('n');
				g.move(tmpInt, 0);
				g.drawChar('g');
				g.drawText("...", 0, "...".length());
				y += 60;
				g.moveTo(x, y);


				strToDraw = new String ("Text in QTJ");
				g.setForeColor(QDColor.black);
				g.drawText(strToDraw, 0, strToDraw.length());
				y += 25;

				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Courier");
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.italic);
				g.setForeColor(QDColor.green);
				g.drawText(strToDraw, 0, strToDraw.length());
				x += tmpInt;
				y += 25;

				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Times");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Times New Roman");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.normal);
				g.setForeColor(QDColor.red);
				g.drawText(strToDraw, 0, strToDraw.length());
				x += tmpInt;
				y += 25;

				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Arial");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Helvetica");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.normal);
				g.setForeColor(QDColor.brown);
				g.drawText(strToDraw, 0, strToDraw.length());
				x += tmpInt;
				y += 25;

				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Lucida Console");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("New York");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.normal);
				g.setForeColor(QDColor.orange);
				g.drawText(strToDraw, 0, strToDraw.length());
				x += tmpInt;
				y += 25;

				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Courier New");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Monaco");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.normal);
				g.setForeColor(QDColor.magenta);
				g.drawText(strToDraw, 0, strToDraw.length());
				y += 25;

				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Verdana");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Charcoal");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.bold);
				g.setForeColor(QDColor.green);
				g.drawText(strToDraw, 0, strToDraw.length());
				y += 25;


				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Symbol");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Trebuchet MS");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.underlined);
				g.setForeColor(QDColor.red);
				g.drawText(strToDraw, 0, strToDraw.length());
				y += 25;



				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Times");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Times New Roman");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.bold + QDConstants.italic);
				g.setForeColor(QDColor.green);
				g.drawText(strToDraw, 0, strToDraw.length());
				y += 25;


// Draw some scaled text
				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.textWidth(strToDraw, strIndex, (strToDraw.length() - strIndex));
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Comic Sans MS");
				if (fntNum == 0)
				{
					fntNum = QDFont.getFNum("Courier");
				}
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.bold + QDConstants.italic);
				g.setForeColor(QDColor.green);
				g.drawTextScaled(strToDraw.length(), strToDraw, 0.5f, 1.0f);
				y += 40;


				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.measureScaledText(strToDraw.length(), strToDraw, 0.5f, 1.0f);
				tmpInt -= g.measureScaledText(1, strToDraw, 0.5f, 1.0f);
				g.move(-tmpInt, 25);
				fntNum = QDFont.getFNum("Courier");
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.bold + QDConstants.italic);
				g.setForeColor(QDColor.green);
				g.drawTextScaled(strToDraw.length(), strToDraw, 0.5f, 1.0f);
				x += 30;
				y += 40;


				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.measureScaledText(strToDraw.length(), strToDraw, 0.5f, 1.0f);
				g.moveTo(x, y);
				fntNum = QDFont.getFNum("Courier");
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.bold + QDConstants.underlined);
				g.setForeColor(QDColor.blue);
				g.drawTextScaled(strToDraw.length(), strToDraw, 1.0f, 0.5f);
				x += (tmpInt / 2);
				y += 40;


				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.measureScaledText(strToDraw.length(), strToDraw, 1.0f, 0.5f);
				g.moveTo(x, y);
				fntNum = QDFont.getFNum("Courier");
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.normal);
				g.setForeColor(QDColor.pink);
				g.drawTextScaled(strToDraw.length(), strToDraw, 2.0f, 1.0f);
				x += (tmpInt / 3);
				y += 40;


				if (strIndex < (strToDraw.length() - 1))
				{
					strIndex++;
				}
				tmpInt = g.measureScaledText(strToDraw.length(), strToDraw, 2.0f, 1.0f);
				g.moveTo(x, y);
				fntNum = QDFont.getFNum("Courier");
				g.textFont(fntNum);
				g.textSize(16);
				g.textFace(QDConstants.normal);
				g.setForeColor(QDColor.white);
				g.drawTextScaled(strToDraw.length(), strToDraw, 1.0f, 2.0f);
				x += (tmpInt / 4);
				y += 40;

// Restore all and draw again
//				tmpInt = g.measureScaledText(strToDraw.length(), strToDraw, 1.0f, 2.0f);
				g.moveTo(x, y);
				g.textFont(savedFNum);
				g.textSize(savedSize);
				g.textFace(savedStyle);
				g.textMode(savedMode);
				g.setForeColor(QDColor.black);
				g.drawText(strToDraw, 0, strToDraw.length());
			}
		});

		imagePres = ImagePresenter.fromGWorld(newQDG);
		myQTCanvas.setClient(imagePres, true);
	}
}
