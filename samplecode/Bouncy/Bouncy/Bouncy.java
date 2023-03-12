/*

File: Bouncy.java

Abstract: Simple demo takes makes text or gif images bounce and measures
their animation frame rate. This program is intentionally written poorly 
with regard to performance and it is usefull to demonstrate the 
performance profiling features of Shark for Java.

Version: 1.0

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

Copyright © 2001-2005 Apple Computer, Inc., All Rights Reserved

*/

import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;

public class Bouncy extends Frame implements KeyListener
{
	final static int WIDTH = 900;
	final static int HEIGHT = 700;
	final static String WWDC = "Macintosh WWDC", STR2005 = "2005";
	
	final static double HOR_WEIGHT = 4.444;
	final static double VER_WEIGHT = 5.555;
	final static double GRAVITY = 0.999;
	
	final static Font LUCIDA = new Font("Lucida Grand", Font.PLAIN, 20);
	final static Font TIMES = new Font("Times", Font.PLAIN, 10);
	
	private long start, end, now, frames, fps;
	private BufferedImage textCache, ballsImages[];
	private Color textColor;
	private Color stringColors[] = {
		Color.RED, Color.BLUE, Color.YELLOW, Color.GREEN, Color.CYAN, Color.GRAY, Color.WHITE	
	};
	private Font textFont;

	private boolean ballsAdded = true;
	private boolean keepAdding = true;
	private int numberOfBalls = 100;
	private int numberOfNewBalls = 1;
	private Ball balls[];
	
	
	public Bouncy()
	{
		super();
	}
	
	public static void main(String[] args)
	{		
		GraphicsEnvironment env = GraphicsEnvironment.getLocalGraphicsEnvironment(); 
		GraphicsDevice gd = env.getDefaultScreenDevice();
		
		Bouncy frame = new Bouncy();
		
		frame.setBackground(Color.black);
		frame.setSize(WIDTH, HEIGHT);			
		frame.setVisible(true);
		
		frame.addKeyListener(frame);
		
	}
	
	public BufferedImage createTextCache() {
		// create text cache
		textColor = Color.YELLOW;
		int textSize = 90;
		textFont = new Font("Monaco", Font.BOLD, textSize*4);
		int textNudge = 5;
		BufferedImage textCache = (BufferedImage)createImage(10*textSize, textSize+textNudge);
		Graphics2D textCacheG = textCache.createGraphics();
		textCacheG.setColor(textColor);
		textCacheG.setFont(textFont);
		textCacheG.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		textCacheG.drawString("0", 0*textSize, textSize-textNudge);
		textCacheG.drawString("1", 1*textSize, textSize-textNudge);
		textCacheG.drawString("2", 2*textSize, textSize-textNudge);
		textCacheG.drawString("3", 3*textSize, textSize-textNudge);
		textCacheG.drawString("4", 4*textSize, textSize-textNudge);
		textCacheG.drawString("5", 5*textSize, textSize-textNudge);
		textCacheG.drawString("6", 6*textSize, textSize-textNudge);
		textCacheG.drawString("7", 7*textSize, textSize-textNudge);
		textCacheG.drawString("8", 8*textSize, textSize-textNudge);
		textCacheG.drawString("9", 9*textSize, textSize-textNudge);
		textCacheG.dispose();
		
		return textCache;
	}
	
	public Ball[] loadBallImages() {
		BufferedImage ball = null;
		try
		{
			ball = javax.imageio.ImageIO.read(getClass().getResource("ball.gif"));
		}
		catch (java.io.IOException e)
		{
			System.out.println(e);
		}
		
		int alpha = 64;
		int saturation = 255;
		
		Color[] colors = { 
			new Color(0, 0, 0, alpha),
			new Color(saturation, saturation, saturation, alpha),
			new Color(saturation, 0, 0, alpha),
			new Color(0, saturation, 0, alpha),
			new Color(0, 0, saturation, alpha),
			new Color(0, saturation, saturation, alpha),
			new Color(saturation, 0, saturation, alpha),
			new Color(saturation, saturation, 0, alpha)
		};
		
		ballsImages = new BufferedImage[colors.length];
		GraphicsEnvironment env = GraphicsEnvironment.getLocalGraphicsEnvironment(); 
		GraphicsDevice gd = env.getDefaultScreenDevice();
		GraphicsConfiguration gc = gd.getDefaultConfiguration();					
		for (int i=0; i<ballsImages.length; i++)
		{
			// colorfull balls
			ballsImages[i] = gc.createCompatibleImage(ball.getWidth(), ball.getHeight(), Transparency.TRANSLUCENT);
			Graphics2D imgG = ballsImages[i].createGraphics();
			imgG.drawImage(ball, 0, 0, null);
			imgG.setColor(colors[i]);
			imgG.setComposite(AlphaComposite.SrcAtop);
			imgG.fillRect(0, 0, ball.getWidth(), ball.getHeight());
			imgG.dispose();
		}

		Ball balls[] = new Ball[numberOfBalls];
		for (int i=0; i<numberOfBalls; i++)
		{
			balls[i] = new Ball(i%2==0 ? WWDC : STR2005, ballsImages);
		}
		return balls;
	}
	
	public void paint(Graphics gw) //(BufferStrategy bufferStrategy)
	{		
		if (textCache == null) textCache = createTextCache();

		if (balls == null) balls = loadBallImages();
		
		long now = System.currentTimeMillis();
		frames++;
		
		Graphics2D g = (Graphics2D)gw;  //(Graphics2D)bufferStrategy.getDrawGraphics();
		g.setColor(Color.black);
		g.fillRect(0, 0, WIDTH, HEIGHT);
		paintText(g, textCache, textColor, textFont, fps, numberOfBalls);
		paintBalls(g, balls, numberOfBalls);

		// make adjustments every 1 second
		if (now-start >= 1000)
		{
			long end = System.currentTimeMillis();
			fps = (long)(1000.0f*frames/(float)(end-start)) * 2;
			
			frames = 0;
			start = end;
		}
		repaint(1000);
	}
	
	public void paintText(Graphics2D g, BufferedImage cachedText, Color textColor, Font textFont, long fps, long ballsNr)
	{		
		int textMargin = 20;
		int textSize = cachedText.getWidth()/10;
		
		if (true)
		{
			// print "fps" status text
			String fpsString = "fps:"+fps;
			int fpsStringLength = fpsString.length();
			g.setColor(Color.white);
			g.setFont(LUCIDA);
			g.drawString(fpsString, textMargin, 22+textMargin);
		}
	}
	
	public void paintBalls(Graphics2D g, Ball balls[], int numberOfBalls)
	{
		for (int i=0; i<numberOfBalls; i++) {
		
			if ((i % 2) == 0) {
				// BOTTLENECK!!! replace with LUCIDA global
				g.setFont(new Font("Lucida", Font.PLAIN, 20));
//				g.setFont(LUCIDA);
			}
			else {
				// BOTTLENECK!!! replace with TIMES global
				g.setFont(new Font("Times", Font.PLAIN, 10));
//				g.setFont(TIMES);
			}
			((Ball)balls[i]).show(g);
		}
	}
	
	public class Ball
	{
		BufferedImage img;
		int w, h, translateX, translateY, plateauCounter;
		double x, y, lastY, dx, dy, g;
		private Color myColor;
		private String myString;
				
		public Ball(String s, BufferedImage images[])
		{
			myColor = stringColors[(int)(Math.random()*stringColors.length)];
			myString = s;

			this.img = images[(int)(Math.random()*images.length)];
			this.w = this.img.getWidth();
			this.h = this.img.getHeight();
			
			this.translateX = (getBounds().width-WIDTH)/2;
			this.translateY = (getBounds().height-HEIGHT)/2;
					
			java.util.Random random = new java.util.Random();
			
			this.x = Math.random() * WIDTH;
			this.y = this.y = HEIGHT-this.h;
			this.dx = random.nextGaussian()*HOR_WEIGHT; // creates a few slow pokes and fast balls, but mostly average speed
			this.dy = Math.random()*VER_WEIGHT; // uniform distribution
			this.g = GRAVITY;

		}
		
		public void tick()
		{
			this.dy += this.g;
			this.x += this.dx;
			this.y += this.dy;
			
			if ((int)this.lastY == (int)this.y)
			{
				this.plateauCounter++;
				if (this.plateauCounter == (int)Math.random()*30+5)
				{
					this.plateauCounter = 0;
					this.dy = Math.random()*this.h;
				}
			}
			else 
			{
				this.plateauCounter = 0;
			}
			
			if (this.x <= 0)
			{
				this.x = -this.x;
				this.dx = -this.dx;
			}
			else if (this.x+this.w >= WIDTH)
			{
				this.x = WIDTH-this.w;
				this.dx = -this.dx;				
			}
			
			if (this.y <= 0)
			{
				this.y = -this.y;
				this.dy = -this.dy;
			}
			else if (this.y+this.h > HEIGHT)
			{
				this.y = HEIGHT-this.h;
				this.dy = -this.dy;				
			}
			
			this.lastY = y;
		}
		
		public void show(Graphics2D g)
		{
			tick();
			g.setColor(myColor);
			g.drawString(myString, (int)this.x, (int)this.y);
		}
	}
	
	boolean doubleBalls = false;
	public void keyPressed(KeyEvent e)
	{
		switch(e.getKeyCode())
		{
			case KeyEvent.VK_ESCAPE:
				System.exit(0);
				break;
			case KeyEvent.VK_ENTER:
			case KeyEvent.VK_SPACE:
		  		break;
			default:
				this.doubleBalls = true;
			 	break;
		}		
	}
	public void keyReleased(KeyEvent e)
	{
		
	}
	public void keyTyped(KeyEvent e)
	{
		
	}
}
