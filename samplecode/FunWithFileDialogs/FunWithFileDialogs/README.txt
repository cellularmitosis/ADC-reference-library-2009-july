FunWithFileDialogs
v 1.0

This sample demonstrates the behavior of the AWT FileChooser and Swing Aqua JFileChooser dialogs in Mac OS X, and the available runtime options to handle Mac-specific conditions in each class.  They are:

JFileChooser.packageIsTraversable (JFileChooser)
JFileChooser.appBundleIsTraversable (JFileChooser)
apple.awt.use-file-dialog-packages (FileDialog)

This sample is for developers looking to handle Mac OS X-specific conditions in their AWT/Swing applications.  Keep in mind that the FileDialog can still be used in Swing applications safely (and is recommended because it employs the native "Column" view), and that the runtime properties in question can be set on the fly as long as it's done before the relevant dialog instance is shown.  They are neither permanent nor global.

Two files named "test.app" and "test.pkg" have been included to test the effects of the properties.

To run the sample from a command-line, type

java -cp FunWithFileDialogs.app/Contents/Resources/Java/FunWithFileDialogs.jar com.apple.dts.samplecode.funwithfiledialogs.FunWithFileDialogs

... substituting backslashes on Windows of course.

[04/28/2003]