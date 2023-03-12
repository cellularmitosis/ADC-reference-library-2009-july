/*
File:		PictureShow.java
	
Description:	PictureShow is a Java command line tool that takes a list of images and creates a
                Keynote document with a slide for each image, with the image's name below it.
                By default it will create a simple, white-on-black theme.  Optionally a Keynote theme
                document (or a regular Keynote document) may be specified whose theme will be applied to
                the document created.


Author:		<JAG>

Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
		("Apple") in consideration of your agreement to the following terms, and your
		use, installation, modification or redistribution of this Apple software
		constitutes acceptance of these terms.  If you do not agree with these terms,
		please do not use, install, modify or redistribute this Apple software.

		In consideration of your agreement to abide by the following terms, and subject
		to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
		copyrights in this original Apple software (the "Apple Software"), to use,
		reproduce, modify and redistribute the Apple Software, with or without
		modifications, in source and/or binary forms; provided that if you redistribute
		the Apple Software in its entirety and without modifications, you must retain
		this notice and the following text and disclaimers in all such redistributions of
		the Apple Software.  Neither the name, trademarks, service marks or logos of
		Apple Computer, Inc. may be used to endorse or promote products derived from the
		Apple Software without specific prior written permission from Apple.  Except as
		expressly stated in this notice, no other rights or licenses, express or implied,
		are granted by Apple herein, including but not limited to any patent rights that
		may be infringed by your derivative works or by other works in which the Apple
		Software may be incorporated.

		The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
		WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
		WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
		PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
		COMBINATION WITH YOUR PRODUCTS.

		IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
		CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
		GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
		ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
		OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
		(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
		ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
Change History (most recent first):

version 1.0: Feb 21 2003: Initial version released to public.

*/

import java.io.*;
import java.net.URL;
import java.util.ArrayList;
import java.util.StringTokenizer;

import java.awt.Dimension;
import java.awt.Image;
import java.awt.Toolkit;

import com.apple.cocoa.application.NSImageRep;

import org.w3c.dom.*;
import org.w3c.dom.ls.*;
import org.xml.sax.SAXException;

import javax.xml.parsers.*;

/**
 * A class which can create a Keynote slideshow from a series of source
 * images.  The resultant show contains a single slide for each image
 * with the image's file name below it.
 */
public class PictureShow {
    /** A reference to a file whose theme we will use (null if none) */
    private File templateBundle = null;
    
    /** The name of the output bundle */
    private String outputBundleName = null;
    
    /** An array of VALID images for which we will create slides */
    private ImageInfo[] imageFiles = null;

    /** The W3C DOM document builder class. */
    private DocumentBuilder docBuilder = null;
    
    /** The W3C DOM document object representing the target document */
    private Document document = null;
    
    /** The dimesions of slides in our target document */
    private Dimension slideSize = new Dimension(0,0);
    
    /** The id of the master slide to reference when creating new blank slides */
    private String blankMasterId = null;
    
    /**
     * Main entry point for the PictureShow command line tool.
     *
     * @see #printHelpAndExit
     */
    public static void main (String[] args) throws Exception {
        PictureShow show = new PictureShow();
        
        show.parseArgs(args);
        show.generateShow();
    }
    
    /**
     * Displayes the usage information for the tool and exits.
     */
    private void printHelpAndExit() {
        System.err.println("Usage: PictureShow [-options] <imagefile>... <output-bundle>");
        System.err.println("       <imagefile>... = A list of image files to use as the source for the picture show");
        System.err.println("       <output-bundle> = A name to be used for the output bundle.");
        System.err.println("       -t[heme] <themefile> = Use the theme from the indicated file");
        System.err.println("           (May be a .key file or a .kth file.)");
        System.err.println("       -help = this message");
        System.exit(1);
    }
    
    /**
     * The main body of the program that creates the document containing the
     * image slides.
     */
    private void generateShow() throws Exception {
        // Create and configure a document builder factory
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

        // If namespace aware, the parser will complain when reading APXL since
        // the plugin: namespace isn't explicitly defined.
        dbf.setNamespaceAware(false);
        
        // We turn off validation so that we don't need to reference the schema
        dbf.setValidating(false);
        
        // The white space inside elements is significant for Keynote.
        dbf.setIgnoringElementContentWhitespace(false);
        
        // We want to leave any CDATA/text nodes separate so that if we
        // need to serialise the theme data, we don't introduce invalid
        // characters in a text section
        dbf.setCoalescing(false);
        
        docBuilder = dbf.newDocumentBuilder();

        if (templateBundle == null) {
            createBlankTheme();
        } else {
            readTheme(templateBundle);
        }
        
        // Loop through the valid images and create a slide for each one
        for (int i=0; i < imageFiles.length; i++) {
            createImageSlide(imageFiles[i]);
        }

        writeDocument(outputBundleName);
    }
    
    /**
     * Creates a new, blank slide (calling createBlankSlide()) and adds the
     * specified image to it.  The file name of the image is added beneath
     * as a title.
     *
     * @see createBlankSlide
     */
    private void createImageSlide(ImageInfo sourceImage) {
        Element slide = createBlankSlide();
        
        // createBlankSlide creates the drawables group for us.
        Element drawables = getFirstChildElementNamed(slide, "drawables");
        
        // Below is an example of what the image element looks like that we're
        // creating:
        //
        // <image image-data="image%20URL" transformation="1 0 0 1 286 144"/>

        Element image = document.createElement("image");
        
        // We replace all space characters with "%20", since Keynote needs proper
        // URL escaping for the image-data attribute.  (We should probably escape
        // other characters as well, but they aren't so likely to appear in file
        // names.)
        String name = sourceImage.getFile().getName();
        StringBuffer escapedName = new StringBuffer();
        
        for (int i=0; i < name.length(); i++) {
            char currentCharacter = name.charAt(i);

            if (currentCharacter == ' ') {
                escapedName.append("%20");
            } else {
                escapedName.append(currentCharacter);
            }
        }
        
        image.setAttribute("image-data", escapedName.toString());
        
        // determine the size/scale and location to place the image
        float scale = 1;
        
        Dimension imageSize = sourceImage.getSize();
        
        // we'll center the image in top part of the slide, and make sure there
        // is at least a 25-pixel border on all sides, 125 along the bottom for
        // the label.
        Dimension targetSize = new Dimension(slideSize);
        targetSize.width -= 50;
        targetSize.height -= 150;
        
        // Scale the image down if it's too big
        if (imageSize.width > targetSize.width) {
            float scaleForWidth = (float)targetSize.width / imageSize.width;

            scale *= scaleForWidth;
            imageSize.width *= scaleForWidth;
            imageSize.height *= scaleForWidth;
        }
        
        if (imageSize.height > targetSize.height) {
            float scaleForHeight = (float)targetSize.height / imageSize.height;

            scale *= scaleForHeight;
            imageSize.width *= scaleForHeight;
            imageSize.height *= scaleForHeight;
        }

        int xTranslate = ((targetSize.width - imageSize.width) / 2) + 25;
        int yTranslate = ((targetSize.height - imageSize.height) / 2) + 25;
        
        // For non-rotated objects, the transformation matrix is:
        //     "<xscale> 0 0 <yscale> <xtranslate> <ytranslate>"
        image.setAttribute("transformation", Float.toString(scale) + " 0 0 " +
                Float.toString(scale) + " " +
                Integer.toString(xTranslate) + " " +
                Integer.toString(yTranslate));
        
        drawables.appendChild(image);
        
        // Move the title placeholder so that it is underneath the image, we put
        // it 100 pixels up from the bottom of the slide, 90 pixels tall.  We
        // make it as wide as the slide (less 10 pixels on either side).  The
        // height is based on the font/font-size choosen.
        //
        // Here is a sample of XML of what the title will look like after we
        // change it:
        //
        // <title location="10 500" size="780 90"
        //       vertical-alignment="tracks-master" visibility="visible"/>
        Element titlePlaceholder = getFirstChildElementNamed(drawables, "title");

        titlePlaceholder.setAttribute("location", "10 " +
                Integer.toString(slideSize.height - 100));
        titlePlaceholder.setAttribute("size",
                Integer.toString(slideSize.width - 20) + " 90");
        titlePlaceholder.setAttribute("visibility", "visible");

        // Add a bullet point of level zero (the title) with the name of the
        // image file in it.
        //
        // Here is a sample of XML of what the bullets section will look
        // like when we're done:
        //
        //  <bullets>
        //    <bullet level="0" marker-type="inherited">
        //      <content font-color="g1" font-name="Helvetica" font-size="48"
        //            paragraph-alignment="center">
        //        <![CDATA[image name]]>
        //      </content>
        //    </bullet>
        //  </bullets>
        Element bulletGroup = document.createElement("bullets");
        slide.appendChild(bulletGroup);
        
        Element titleBullet = document.createElement("bullet");
        titleBullet.setAttribute("level", "0");
        titleBullet.setAttribute("marker-type", "inherited");
        bulletGroup.appendChild(titleBullet);

        // The default style for any styled text content is Helvetica 12, black.
        // Text styles are *not* inherited (in terms of the file format).  So we
        // must set them explicitly here.
        Element titleContent = document.createElement("content");
        titleContent.setAttribute("font-name", "Helvetica");
        titleContent.setAttribute("font-size", "48");
        titleContent.setAttribute("font-color", "g1");
        titleContent.setAttribute("paragraph-alignment", "center");
        // NOTE: We use a CDATA section instead of a plain text node since the
        // file name might contain special characters (like "<", ">" and "&").
        titleContent.appendChild(
                document.createCDATASection(sourceImage.getFile().getName()));
        titleBullet.appendChild(titleContent);
    }
    
    /**
     * Creates a new document with a minimal, default white-on-black theme.  This
     * theme contains only one master slide, for the "Blank" layout.  The id of
     * this master slide will be put in blankMasterId.
     */
    private void createBlankTheme() {
        // get the dom implementation
        DOMImplementation impl = docBuilder.getDOMImplementation();

        // What follows below is an example of the XML that we will be generating.  Note
        // that we're actually creating a whole document, not just the theme, but what
        // Keynote calls a theme is simply a document with an empty slide list.
        //
        //  <?xml version="1.0" encoding="UTF-8"?>
        //  <presentation version="36">
        //    <theme slide-size="800 600">
        //      <master-slides>
        //        <master-slide id="master-slide-1" name="Blank">
        //          <drawables>
        //            <title location="78 16" opacity="1" size="644 150"
        //                      stroke-color="g1" stroke-width="1"
        //                      vertical-alignment="middle" visibility="hidden">
        //              <styles>
        //                <fill-style fill-type="none"/>
        //                <dash-style pattern="none"/>
        //                <shadow-style opacity="0" radius="0"/>
        //              </styles>
        //            </title>
        //  
        //            <body bullet-indentation="0 20 48 75 102 130" location="78 170"
        //                      size="644 352"  ... other attributes same as title ... >
        //              <styles>
        //                    ... same styles as title
        //              </styles>
        //            </body>
        //  
        //            <page-number location="380 570" size="39 23"
        //                        ... other attributes same as title ... >
        //              <styles>
        //                   ... same styles as title
        //              </styles>
        //              <text-attributes/>
        //            </page-number>
        //          </drawables>
        //  
        //          <prototype-bullets>
        //            <bullet level="0" marker-type="none" spacing="0">
        //              <content/>
        //            </bullet>
        //               ... more bullets w/ levels 1-5, otherwise identical
        //          </prototype-bullets>
        //  
        //          <background-fill-style fill-color="g0" fill-type="color"/>
        //  
        //          <transition-style duration="0" type="none" />
        //        </master-slide>
        //      </master-slides>
        //    </theme>
        //  
        //    <slide-list/>
        //    <ui-state/>
        //  </presentation>

        // create a new, empty document
        document = impl.createDocument("", "presentation",
                impl.createDocumentType("APXL", null, null));
        
        Element presentation = document.getDocumentElement();
        presentation.setAttribute("version", "36");
        
        // create the theme
        Element theme = document.createElement("theme");
        slideSize.setSize(800, 600);
        theme.setAttribute("slide-size", Integer.toString(slideSize.width) +
                " " + Integer.toString(slideSize.height));
        presentation.appendChild(theme);
        
        // create an empty slide-list
        Element slideList = document.createElement("slide-list");
        presentation.appendChild(slideList);
        
        // create an empty UI state (The ui-state element is required, but
        // is allowed to be empty.)
        Element uiState = document.createElement("ui-state");
        presentation.appendChild(uiState);
        
        // create the master slides group
        Element masterSlidesGroup = document.createElement("master-slides");
        theme.appendChild(masterSlidesGroup);
        
        // create a master slide (we need at least one)
        //
        // NOTE: All master slides must have an "id".  The id must be unique
        // within ALL elements in the document.
        Element masterSlide = document.createElement("master-slide");
        masterSlide.setAttribute("name", "Blank");
        blankMasterId = "master-slide-1";
        masterSlide.setAttribute("id", blankMasterId);
        masterSlidesGroup.appendChild(masterSlide);
        
        // create the drawables group
        Element drawables = document.createElement("drawables");
        masterSlide.appendChild(drawables);
        
        // Every master MUST have a title, body and page-number placeholder,
        // which must be explicitly styled.  We'll use a convenience function
        // to set the styles common to each type.
        //
        // NOTE: the placeholder locations and sizes used are copied from one
        // of the default themes at 800x600
        Element title = document.createElement("title");
        setDefaultPlaceholderStyles(title);
        title.setAttribute("location", "78 16");
        title.setAttribute("size", "644 150");
        drawables.appendChild(title);
        
        Element body = document.createElement("body");
        setDefaultPlaceholderStyles(body);
        body.setAttribute("location", "78 170");
        body.setAttribute("size", "644 352");
        // Bullet indentation indicates how many pixels each level of
        // bullets is indented.  The values here are taken from a
        // sample Keynote document.
        body.setAttribute("bullet-indentation", "0 20 48 75 102 130");
        drawables.appendChild(body);
        
        // Page number needs a text attributes child, but it can be empty.
        Element pageNumber = document.createElement("page-number");
        setDefaultPlaceholderStyles(pageNumber);
        pageNumber.setAttribute("location", "380 570");
        pageNumber.setAttribute("size", "39 23");
        pageNumber.appendChild(document.createElement("text-attributes"));
        drawables.appendChild(pageNumber);
        
        // Create the required 6 prototype bullets.  (A bullet of level 0 is
        // the slide's title.
        Element prototypeBullets = document.createElement("prototype-bullets");
        masterSlide.appendChild(prototypeBullets);
        
        addPrototypeBullet(prototypeBullets, "0");
        addPrototypeBullet(prototypeBullets, "1");
        addPrototypeBullet(prototypeBullets, "2");
        addPrototypeBullet(prototypeBullets, "3");
        addPrototypeBullet(prototypeBullets, "4");
        addPrototypeBullet(prototypeBullets, "5");
        
        // Set a black background
        Element background = document.createElement("background-fill-style");
        background.setAttribute("fill-type", "color");
        background.setAttribute("fill-color", "g0");
        masterSlide.appendChild(background);
        
        // Set a null transition
        Element transition = document.createElement("transition-style");
        transition.setAttribute("type", "none");
        transition.setAttribute("duration", "0");
        masterSlide.appendChild(transition);
    }

    /**
     * A convenience function used by createBlankTheme() which sets the
     * required attributes/styles of a master-slide placeholder to
     * reasonable defaults.
     *
     * @see #createBlankTheme
     */
    private void setDefaultPlaceholderStyles(Element placeholder) {
        // NOTE: We need to set the stroke color and width 
        placeholder.setAttribute("opacity", "1");
        placeholder.setAttribute("visibility", "hidden");
        placeholder.setAttribute("stroke-color", "g1");
        placeholder.setAttribute("stroke-width", "1");
        placeholder.setAttribute("vertical-alignment", "middle");
        
        Element styles = document.createElement("styles");
        placeholder.appendChild(styles);
        
        // No fill
        Element fill = document.createElement("fill-style");
        fill.setAttribute("fill-type", "none");
        styles.appendChild(fill);
        
        // No stroke
        Element dash = document.createElement("dash-style");
        dash.setAttribute("pattern", "none");
        styles.appendChild(dash);
        
        // No shadow
        Element shadow = document.createElement("shadow-style");
        shadow.setAttribute("opacity", "0");
        shadow.setAttribute("radius", "0");
        styles.appendChild(shadow);
    }

    /**
     * A convenience function used by createBlankTheme() which creates
     * a minimal, prototype bullet-point with the level specified.
     *
     * @return The newly created bullet element
     */
    private Element addPrototypeBullet(Element prototypeBullets,
            String level) {
        Element bullet = document.createElement("bullet");
        
        bullet.setAttribute("marker-type", "none");
        bullet.setAttribute("spacing", "0");
        bullet.setAttribute("level", level);
        
        // The content element is required, even if empty.  Text styles can be
        // applied to the content to set the default styling for bullets of
        // that level.
        Element content = document.createElement("content");
        bullet.appendChild(content);
        
        prototypeBullets.appendChild(bullet);
        
        return bullet;
    }

    /** 
     * Creates a new, blank slide with the minimum required styling/content. The
     * master slide referenced by the new slide will be the one whose id is in
     * blankMasterId.
     */
    private Element createBlankSlide() {
        Element slideList = getFirstChildElementNamed(
                document.getDocumentElement(), "slide-list");

        // Below is an example of the XML generated by this function
        //      <slide master-slide-id="(blankMasterId)">
        //        <drawables>
        //          <title vertical-alignment="tracks-master" visibility="hidden"/>
        //          <body vertical-alignment="tracks-master" visibility="hidden"/>
        //        </drawables>
        //  
        //        <transition-style type="inherited" />
        //      </slide>

        Element slide = document.createElement("slide");
        slide.setAttribute("master-slide-id", blankMasterId);
        slideList.appendChild(slide);
        
        Element drawables = document.createElement("drawables");
        slide.appendChild(drawables);
        
        // Title and body placeholders are required in every slide, but can
        // be invisible
        Element title = document.createElement("title");
        title.setAttribute("visibility", "hidden");
        title.setAttribute("vertical-alignment", "tracks-master");
        drawables.appendChild(title);

        Element body = document.createElement("body");
        body.setAttribute("visibility", "hidden");
        body.setAttribute("vertical-alignment", "tracks-master");
        drawables.appendChild(body);
        
        Element transition = document.createElement("transition-style");
        transition.setAttribute("type", "inherited");
        slide.appendChild(transition);
        
        return slide;
    }

    /**
     * Creates a new, empty document containing the theme from the referenced
     * file.  This is actually achieved by reading in the document and modifying
     * it place to remove all slides, plus the contents UI state (which may
     * refer to objects in the now-missing slides).
     */
    private void readTheme(File themeFile) throws IOException, SAXException {
        // Parse the document
        document = docBuilder.parse(new File(themeFile, "presentation.apxl"));

        // Get the slide size from the theme
        Element theme = getFirstChildElementNamed(
                document.getDocumentElement(), "theme");

        StringTokenizer sizeTokens = new StringTokenizer(
                theme.getAttribute("slide-size"), " ");

        String width = sizeTokens.nextToken();
        String height = sizeTokens.nextToken();

        slideSize.setSize(Integer.parseInt(width), Integer.parseInt(height));

        // Look for a master slide named "Blank"
        Element masters = getFirstChildElementNamed(theme, "master-slides");
        Element blankMaster = null;
        Node currentChild = masters.getFirstChild();
        
        while (currentChild != null) {
            if ((currentChild.getNodeType() == Node.ELEMENT_NODE) &&
                    (currentChild.getNodeName().equals("master-slide")) &&
                    (((Element)currentChild).getAttribute("name").equals("Blank"))) {
                blankMaster = (Element)currentChild;
                break;
            }
            currentChild = currentChild.getNextSibling();
        }
        
        // no master named "Blank"?  Just use the first master in the theme.
        if (blankMaster == null) {
            blankMaster = getFirstChildElementNamed(masters, "master-slide");
        }
        
        blankMasterId = blankMaster.getAttribute("id");
        
        // Remove any slides in the slide list
        Element slideList = getFirstChildElementNamed(
                document.getDocumentElement(), "slide-list");
        
        currentChild = slideList.getFirstChild();
        
        while (currentChild != null) {
            Node nextChild = currentChild.getNextSibling();
            slideList.removeChild(currentChild);
            currentChild = nextChild;
        }

        // Remove the content of the UI state
        Element uiState = getFirstChildElementNamed(
                document.getDocumentElement(), "ui-state");
        
        currentChild = uiState.getFirstChild();
        
        while (currentChild != null) {
            Node nextChild = currentChild.getNextSibling();
            uiState.removeChild(currentChild);
            currentChild = nextChild;
        }
    }
    
    /**
     * Writes out the document bundle.  This includes copying in the image data
     * from the source images, plus copying any extra media from a source theme,
     * and, of course, writing the document out as XML.
     */
    private void writeDocument(String outputBundleName) throws IOException {
        // First: ensure removal of any file that may already be in the way, and
        // create the target bundle directory
        File outputBundle = new File(outputBundleName);
        boolean okay = true;
        
        if (outputBundle.exists()) {
            if (outputBundle.isDirectory()) {
                okay = recursiveDelete(outputBundle);
            } else {
                okay = outputBundle.delete();
            }
        }
        
        if (okay) {
            okay = outputBundle.mkdir();
        }
        
        if (!okay) {
            throw new IOException("Unable to create output bundle.");
        }

        // Copy the image data into the bundle, both the images that make up the
        // slide-show, plus any media from the theme.
        for (int i=0; i < imageFiles.length; i++) {
            copyFileIntoDirectory(imageFiles[i].getFile(), outputBundle);
        }
        
        if (templateBundle != null) {
            File[] templateFiles = templateBundle.listFiles();
            
            for (int i=0; i < templateFiles.length; i++) {
                copyFileIntoDirectory(templateFiles[i], outputBundle);
            }
        }

        // NOTE: we write the presentation.apxl AFTER copying the data from the
        // theme, because the theme will contain a presentation.apxl of its own.
        // If we do the copy afterwards, we'd have to special case
        // "presentation.apxl", or we'll blow away the XML we've just created!
        File outputXML = new File(outputBundle, "presentation.apxl");
        
        // Write out the DOM tree
        DOMImplementationLS domls =
                (DOMImplementationLS)docBuilder.getDOMImplementation();

        DOMWriter writer = domls.createDOMWriter();
        writer.setEncoding("UTF-8");

        // pretty printing should give more text-editor friendly content, but
        // unfortunately the current implementation of the DOMWriter doesn't
        // support the setFeature functionality.
//        writer.setFeature("pretty-print", true);
        
        writer.writeNode(new FileOutputStream(outputXML), document);
    }

    /**
     * Parses the argument list and fills in the appropriate member variables.
     */
    private void parseArgs(String args[]) {
        ArrayList imageList = new ArrayList();
        
        for (int i = 0; i < args.length; i++) {
            if (args[i].startsWith("-t")) {
                if (i == args.length - 1) {
                    printHelpAndExit();
                }
                i++;
                templateBundle = new File(args[i]);
            } else if (args[i].equals("-help")) {
                printHelpAndExit();
            } else {
                if (i == args.length - 1) {
                    // If we're on the last argument, then this is the name of
                    // the output bundle
                    outputBundleName = args[i];
                } else {
                    // If not the last argument, then this is a source image. If
                    // it is a valid image file, add it to the image list.
                    
                    // NOTE: It might be useful to recursively search any
                    // specified directories for images.
                    ImageInfo theInfo = new ImageInfo(args[i]);
                    
                    if (theInfo.isValid()) {
                        imageList.add(theInfo);
                    } else {
                        System.err.println("Skipping \"" + args[i] +
                                "\": not a recognised image file.");
                    }
                }
            }
        }
        
        if ((outputBundleName == null) || (imageList.isEmpty())) {
            printHelpAndExit();
        }
        
        if (!outputBundleName.endsWith(".key")) {
            outputBundleName += ".key";
        }
        
        imageFiles = (ImageInfo[])imageList.toArray(new ImageInfo[0]);
    }

    /**
     * Walks the immediate decendents of a DOM element and returns the first one
     * with the given tagname.
     */
    private static Element getFirstChildElementNamed(Element inParent,
            String inString) {
        Element result = null;
        Node currentChild = inParent.getFirstChild();
        
        while (currentChild != null) {
            if ((currentChild.getNodeType() == Node.ELEMENT_NODE) &&
                    (currentChild.getNodeName().equals(inString))) {
                result = (Element)currentChild;
                break;
            }
            currentChild = currentChild.getNextSibling();
        }
        
        return result;
    }

    /**
     * Copies the specified file or directory into the specified directory,
     * keeping the original file name.  Works recursively on directories.
     */
    private static void copyFileIntoDirectory(File file, File directory)
            throws IOException {
        File target = new File(directory, file.getName());
            
        if (file.isDirectory()) {
            target.mkdir();
            
            File[] contents = file.listFiles();
            
            for (int i=0; i<contents.length; i++) {
                copyFileIntoDirectory(contents[i], target);
            }
        } else {
            FileInputStream streamIn  = new FileInputStream(file);
            FileOutputStream streamOut = new FileOutputStream(target);
            
            // 16k is likely to be a multiple of the file block size.
            byte[] buffer = new byte[16 * 1024];

            int readBytes = 0;

            while((readBytes=streamIn.read(buffer)) != -1) {
                streamOut.write(buffer, 0, readBytes);
            }

            streamIn.close();
            streamOut.close();
        }
    }
    
    /**
     * Deletes a directory and all its contents.  The argument must refer to a
     * directory.
     */
    private static boolean recursiveDelete(File directory) throws IOException {
        File[] contents = directory.listFiles();
        boolean deleteOkay = true;
        
        for (int i=0; i<contents.length; i++) {
            if (contents[i].isDirectory()) {
                deleteOkay = recursiveDelete(contents[i]);
            } else {
                deleteOkay = contents[i].delete();
            }
            
            if (!deleteOkay) {
                break;
            }
        }
        
        if (deleteOkay) {
            deleteOkay = directory.delete();
        }
        
        return deleteOkay;
    }
    
    /**
     * A private class that abstracts the functionality of checking to see if
     * a particular file is a valid image file, and if so, what its dimensions
     * are. Currently it is implemented in terms of NSImage, although it would
     * relatively straight-forward to include a fallback case that uses
     * java.awt.Image on platforms where NSImage is not available.  (Or some
     * other image library, like JIMI.)  NSImage was used here since it supports
     * all of the image types that Keynote does, whereas AWT's Image class only
     * supports a subset of that (GIF, JPEG and PNG).
     *
     * If you wished to use some other image library, you would add code in the
     * catch block of the constructor that used the alternate classes to check
     * the image file.  In order to get the NSImage code to compile on other
     * platforms, however, you would need to use introspection to call the
     * methods instead of simply calling them directly.
     */
    private static class ImageInfo {
        Dimension size = null;
        File file = null;
        
        static {
            try {
                // NOTE: NSImageReps don't work correctly unless you access the
                // NSImage class first.  So, in the static initialiser, we create
                // a throw-away image instance (supressing all exceptions and
                // errors; we'll deal with any problems when they come up later).
                new com.apple.cocoa.application.NSImage();
            } catch(Throwable t) {}
        }

        /**
         * Creates a new ImageInfo instance, which immediately analyzes the image
         * file to see if it is a valid image and, if so, what its dimensions
         * are.
         */
        public ImageInfo(String imageFilename) {
            try {
                // NOTE: When calling a static member functions, the class in
                // question is loaded as soon as the function is called.  Thus,
                // we must put the call to imageRepWithContentsOfFile() in a
                // sub-function, or else the constructor itself will trigger
                // the NoClassDefFoundError.
                examineImageWithCocoa(imageFilename);
            } catch (NoClassDefFoundError e) {
                System.err.println("NSImageRep class not found.  PictureShow requires Cocoa for Java (Mac OS X).");
                System.err.println("If running on Mac OS X, ensure that your classpath includes /System/Library/Java.");
                System.exit(-1);
                // For code that needs to run on other platforms, you should
                // fall back to an alternate image library here.
            }
            
            if (size != null) {
                file = new File(imageFilename);
            }
        }
        
        /**
         * Tries to read the image file using NSImageRep.  Determines the
         * dimensions of the image if succesful.
         */
        private void examineImageWithCocoa(String imageFilename) {
            NSImageRep imageRep = NSImageRep.imageRepWithContentsOfFile(imageFilename);
            
            if (imageRep != null) {
                // When possible, we use the pixels wide/high metric, since that
                // is correct for bitmaps that use non-standard DPI.  However, in
                // the case of PDF files, there is no pixels wide/high metric, so
                // we use the normal size in that case.
                if ((imageRep.pixelsWide() == 0) || (imageRep.pixelsHigh() == 0)) {
                    size = imageRep.size().toAWTDimension();
                } else {
                    size = new Dimension(imageRep.pixelsWide(),
                            imageRep.pixelsHigh());
                }
            }
        }
        
        /**
         * Returns true if the path passed in the constructer references a valid
         * image file, of a known type.
         */
        public boolean isValid() {
            return (size != null);
        }
        
        /**
         * Returns the size of the image file.  It is an error to call this
         * method unles isValid() returns true.
         */
        public Dimension getSize() {
            return size;
        }
        
        /**
         * Returns a File object that refers to the image file.  It is an error
         * to call this method unles isValid() returns true.
         */
        public File getFile() {
            return file;
        }
    }
}
