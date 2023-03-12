#import <Cocoa/Cocoa.h>
#import "EncodingManager.h"

@class ScalingScrollView;

/* These get added to the string encodings so we have a common language to refer to file types */
enum {
    UnknownStringEncoding = NoStringEncoding,
    RichTextStringEncoding = 0xFFFFFFFE,
    RichTextWithGraphicsStringEncoding = 0xFFFFFFFD,
    HTMLStringEncoding = 0xFFFFFFFC,
    SimpleTextStringEncoding = 0xFFFFFFFB,
    DocStringEncoding = 0xFFFFFFFA,
    WordMLStringEncoding = 0xFFFFFFF9,
    WebArchiveStringEncoding = 0xFFFFFFF8,
    SmallestCustomStringEncoding = 0xFFFFFFF0
};

typedef enum {
    SaveStatusOK = 1,
    SaveStatusFileNotWritable,    	// File is not writable
    SaveStatusEncodingNotApplicable,	// File can't be converted to specified encoding
    SaveStatusDestinationNotWritable,  	// Destination is not writable
    SaveStatusFileEditedExternally,	// File was edited externally (by another application)
    SaveStatusNotOK = 1000        	// Some other error
} SaveStatus;

typedef enum {
    FileExtensionHidden = 1,
    FileExtensionShown,
    FileExtensionPreviousState
} FileExtensionStatus;


/* Returns the default padding on the left/right edges of text views */
float defaultTextPadding(void);

/* Return a non-blank display name. If the display name is blank, currently returns last path component; should probably do better. */
NSString *displayName(NSString *path);

/* Helper used in toggling menu items in validate methods, based on a condition (useFirst) */
void validateToggleItem(NSMenuItem *menuItem, BOOL useFirst, NSString *first, NSString *second);


/* Struct for carrying info for saving between the various routines...
*/
typedef struct _DocumentSaveInfo {
    NSString *nameForSaving;	// This is retained by this structure
    NSString *fileToBeRemoved;	// This is retained by this structure
    unsigned encodingForSaving;
    BOOL haveToChangeType;
    BOOL showEncodingAccessory;
    BOOL showRichTextDocumentFormatAccessory;
    BOOL showRichTextWithGraphicsDocumentFormatAccessory;
    BOOL rememberName;
    BOOL shouldClose;
    BOOL showSavePanel;
    BOOL doingRTFDConversion;
    SEL whenDoneCallback;
    FileExtensionStatus hideExtension;
    NSPopUpButton *encodingPopUp;
    NSButton *appendPlainTextExtensionButton;
} DocumentSaveInfo;


@interface Document : NSObject {
    NSTextStorage *textStorage;
    NSString *documentName;		/* If nil, never saved */
    NSString *revertDocumentName;	/* For reverting purposes, if the document is made untitled at some point */
    ScalingScrollView *scrollView;	/* ScrollView containing document */
    NSPrintInfo *printInfo;		/* PrintInfo, used when hasMultiplePages is true */
    BOOL isDocumentEdited;
    BOOL hasMultiplePages;
    BOOL isRichText;
    BOOL isReadOnly;
    BOOL uniqueZone;			/* YES if the zone was created specially for this document */
    BOOL openedIgnoringRTF;		/* Setting at the the time the doc was open (so revert does the same thing) */
    BOOL openedIgnoringHTML;		/* Setting at the the time the doc was open (so revert does the same thing) */
    unsigned documentEncoding;		/* NSStringEncoding or one of the above values */
    unsigned untitledDocNumber;		/* If not 0, the untitled sequence number this document has been assigned */
    int changeCount;
    BOOL convertedDocument;		/* Converted (or filtered) from some other format (and hence not writable) */
    BOOL lossyDocument;			/* Loaded lossily, so might not be a good idea to overwrite */
    BOOL rulerIsBeingDisplayed;		/* Indicates that the ruler is being lazily displayed */
    NSDate *fileModDate;		/* File modification date from the last open or save */
    
    IBOutlet NSView *richTextDocumentFormatAccessory;		/* Set when the rich text popup is loaded */
    IBOutlet NSPopUpButton *richTextDocumentFormatPopUp;	/* Set when the rich text popup is loaded */
    
    // The next six are document properties (applicable only to rich text documents)
    NSString *author;
    NSString *copyright;
    NSString *company;
    NSString *title;
    NSString *subject;
    NSString *comment;
    NSString *keywords;
    
    int lastDocumentPropertyChangeCheckpointCount;
    NSString *lastChangedDocumentProperty;
}

/* Don't call -(id)init; call one of these methods... */
- (id)initWithPath:(NSString *)filename encoding:(unsigned)encoding uniqueZone:(BOOL)flag error:(NSError **)errorPtr;	
- (id)initWithPath:(NSString *)filename encoding:(unsigned)encoding ignoreRTF:(BOOL)ignoreRTF ignoreHTML:(BOOL)ignoreHTML uniqueZone:(BOOL)flag error:(NSError **)errorPtr;	/* Should be an absolute path here; nil for untitled. uniqueZone = YES indicates the zone should be recycled when the doc is dealloced. */
+ (id)openDocumentWithPath:(NSString *)filename encoding:(unsigned)encoding behind:(Document *)otherDoc error:(NSError **)errorPtr;	/* Brings window front as key, or behind otherDoc if otherDoc is not nil. Checks to see if document already open. */
+ (id)openUntitled:(BOOL)isOpenedAutomatically;	/* Brings window front */

/* Put up panels indicating failure to open one or more files. Pass someSucceeded == YES if not known.
*/
+ (void)displayOpenFailures:(NSArray *)errors someSucceeded:(BOOL)someFilesOpened;

/* These set/get the documentName instance var and also set the window title accordingly. "nil" is used if no title. */
- (void)setDocumentName:(NSString *)fileName;
- (NSString *)documentName;

/* These determine if document has been edited since last save */
- (void)setDocumentEdited:(BOOL)flag;
- (BOOL)isDocumentEdited;

/* Is the document rich? */
- (BOOL)isRichText;
- (void)setRichText:(BOOL)flag;
- (void)setRichText:(BOOL)flag dealWithAttachments:(BOOL)attachmentFlag showRuler:(BOOL)rulerFlag;

/* Is the document read-only? */
- (BOOL)isReadOnly;
- (void)setReadOnly:(BOOL)flag;

/* Document background color */
- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;

/* Determining whether file has been edited externally */
- (void)setFileModDate:(NSDate *)date;
- (NSDate *)fileModDate;
- (BOOL)isEditedExternally:(NSDate *)newModDateIfKnown;

/* The encoding of the document... */
- (unsigned)encoding;
- (void)setEncoding:(unsigned)encoding;

/* Whether document was converted from some other format (filter services) */
- (BOOL)converted;
- (void)setConverted:(BOOL)flag;

/* Whether document was loaded lossily */
- (BOOL)lossy;
- (void)setLossy:(BOOL)flag;

/* Hyphenation factor (0.0-1.0, 0.0 == disabled) */
- (float)hyphenationFactor;
- (void)setHyphenationFactor:(float)factor;

/* View size (as it should be saved in a RTF file) */
- (NSSize)viewSize;
- (void)setViewSize:(NSSize)size;

/* Ruler management; first call puts up ruler; second call calls the first one in a delayed fashion */
- (void)showRuler:(id)obj;
- (void)showRulerDelayed:(BOOL)flag;

/* Attributes */
- (NSTextStorage *)textStorage;
- (NSTextView *)firstTextView;
- (NSWindow *)window;
- (NSUndoManager *)undoManager;
- (NSLayoutManager *)layoutManager;

/* Misc methods */
- (NSString *)untitledDocumentName:(unsigned)type;
+ (Document *)documentForWindow:(NSWindow *)window;
+ (Document *)documentForPath:(NSString *)filename;
+ (NSString *)cleanedUpPath:(NSString *)filename;
+ (unsigned)numberOfOpenDocuments;
- (void)doForegroundLayoutToCharacterIndex:(unsigned)loc;
- (void)showWindowBehindDocument:(Document *)otherDoc;
- (void)doRevert;
- (int)undoCheckpointCount;

/* Page-oriented methods */
- (void)addPage;
- (void)removePage;
- (unsigned)numberOfPages;
- (void)setHasMultiplePages:(BOOL)flag;
- (void)setHasMultiplePages:(BOOL)flag force:(BOOL)force;
- (BOOL)hasMultiplePages;
- (void)setPrintInfo:(NSPrintInfo *)anObject;
- (NSPrintInfo *)printInfo;
- (void)printInfoUpdated;	// To let the document know that printInfo has been changed
- (void)setPaperSize:(NSSize)size;
- (NSSize)paperSize;

/* Printing a document; return value indicates cancel */
- (BOOL)printDocumentModally:(BOOL)modalFlag;

/* Saving helpers. */
- (void)saveDocument:(BOOL)showSavePanel rememberName:(BOOL)rememberNewNameAndSuch shouldClose:(BOOL)shouldClose;  /* Entry point for saving w/UI; will show panels and such as necessary */
- (void)saveDocument:(BOOL)showSavePanel name:(NSString *)pathForSaving rememberName:(BOOL)rememberNewNameAndSuch shouldClose:(BOOL)shouldClose whenDone:(SEL)callback;	/* Entry point for saving w/UI; will show panels and such as necessary */
- (void)getDocumentNameAndSave:(DocumentSaveInfo *)docInfo;
- (void)doSaveWithName:(DocumentSaveInfo *)docInfo overwriteOK:(BOOL)overwrite;

- (void)askToSave:(SEL)callback;
- (BOOL)canCloseDocument;	/* Assures document is saved or user doesn't care about the changes; returns NO if user cancels */
+ (void)openWithEncodingAccessory:(BOOL)flag;

/* Enumerations for saving all edited documents. */
+ (void)saveAllEnumeration:(BOOL)cont;
+ (void)reviewChangesAndQuitEnumeration:(BOOL)cont;

/* Action methods */
+ (void)open:(id)sender;
- (void)saveAs:(id)sender;
- (void)saveTo:(id)sender;
- (void)save:(id)sender;
- (void)revert:(id)sender;
- (void)close:(id)sender;
- (void)doPageLayout:(id)sender;
- (void)toggleRich:(id)sender;
- (void)toggleReadOnly:(id)sender;
- (void)togglePageBreaks:(id)sender;
- (void)printDocument:(id)sender;  /* action cover for [self printDocumentUsingPrintPanel:YES] */
- (void)richTextDocumentFormatChanged:(id)sender;
- (void)richTextWithGraphicsDocumentFormatChanged:(id)sender;

/* When the preference "OpenPanelFollowsMainWindow" is set to YES, this is used to get the directory of document in the main window. */
+ (NSString *)directoryOfMainWindow;

/* Delegation messages */
- (void)textView:(NSTextView *)view doubleClickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)rect;
- (NSArray *)textView:(NSTextView *)view writablePasteboardTypesForCell:(id <NSTextAttachmentCell>)cell atIndex:(unsigned)charIndex;
- (BOOL)textView:(NSTextView *)view writeCell:(id <NSTextAttachmentCell>)cell atIndex:(unsigned)charIndex toPasteboard:(NSPasteboard *)pboard type:(NSString *)type;
- (void)layoutManager:(NSLayoutManager *)layoutManager didCompleteLayoutForTextContainer:(NSTextContainer *)textContainer atEnd:(BOOL)layoutFinishedFlag;
- (BOOL)windowShouldClose:(id)sender;
- (void)windowWillClose:(NSNotification *)notification;
- (void)undoManagerChangeDone:(NSNotification *)notification;
- (void)undoManagerCheckpoint:(NSNotification *)notification;
- (void)undoManagerChangeUndone:(NSNotification *)notification;

/* Document properties */
- (NSDictionary *)documentPropertyToAttributeNameMappings;
- (NSArray *)knownDocumentProperties;
- (void)clearDocumentProperties;
- (void)setDocumentPropertiesToDefaults;
- (BOOL)hasDocumentProperties;

@end

@interface Document (ReadWrite)

/* File loading. Returns NO if not successful. Doesn't set documentName. */
- (BOOL)loadFromPath:(NSString *)fileName encoding:(unsigned)encoding ignoreRTF:(BOOL)ignoreRTF ignoreHTML:(BOOL)ignoreHTML error:(NSError **)errorPtr;	/* If encoding is Unknown, tries to guess */
- (SaveStatus)saveToPath:(NSString *)fileName encoding:(unsigned)encoding updateFilenames:(BOOL)updateFileNamesFlag overwriteOK:(BOOL)overwrite hideExtension:(FileExtensionStatus)extensionStatus;

@end
