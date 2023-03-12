The AppearanceSampleUpdate sample code consists of 5 milestones that demonstrate Xcode 2.1 features (mainly focusing on the build system and project structure). 

AppearanceSample-1 is the initial AppearanceSample code which has been cleaned up and upgraded to Xcode 2.1.

In AppearanceSample-2, the files and groups that are a part of the project have been re-organized to show off the abilities of organizing files and groups in an Xcode project.

AppearanceSample-3 has split off the the sources in the "MegaDialog" group into a separate static library target that the "AppearanceSample" application target depends on.

For AppearanceSample-4 the set of configuration names in the project have been improved and the majority of customized build settings have been moved to the project build settings, with target-specific customized build settings remaining in the targets.

AppearanceSample-5 demonstrates how to set up an application target to have a different application name for each configuration. Changes were made to the "AppearanceSample" target settings ("Product Name") and the "Info-AppearanceSample.plist" file.

AppearanceSample-final shows how to pass down build settings through the "Preprocessor Macros" build setting to control code generation. In this case a preprocessor macro (USE_DEBUG_NIB) is being defined in the "AppearanceSample" target (using the "UseSpecialNib" custom build setting) to conditionally load a different application nib at runtime.