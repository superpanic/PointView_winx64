# Plug-in for Adobe Illustrator (Windows version)
An Adobe Illustrator plug-in that visually highlights path endpoints. (A tool I need when editing imported CAD artwork.) The plug-in draws a criss and a cross on all start and end points of selected open paths using the current layer color. The circles are not part of the artwork, but is drawn on the UI layer; using the AIAnnotationDrawerSuite. 

This is the Windows version. [Macos version is found here.](https://github.com/superpanic/PointView)

![PointView path selection](http://superpanic.com/pointview/pvart2.png)

Written in C.  

Based on the HelloWorld project from the `getting-started-guide.pdf` in [Adobe Illustrator SDK Docs](https://console.adobe.io/downloads/ai).  

# Set up:

## Visual Studio:
Use Visual Studio 2017 (not the latest version).  

## Visual C++  
Install the Desktop Development C++ toolset (Tools/Get Tools and Features) with the Visual Studio Installer.  
Make sure the C++ toolset includes Windows 8.1 SDK.  

## MFC  
Also install MFC (In Visual Studio 2017 and later, MFC and ATL are optional sub-components under the Desktop development with C++ workload in the Visual Studio Installer program.)  

## Python
If you want to build the samplecode from Adobe, it uses a Python script to generate the PiPL resource file:  
For this you need to install Python 2.7 for Visual Studio.  
Set the Python environment to 2.7.  
Find your Python 2.7 system path. Mine is: `C:\Python27\`  
Set the python path in the project environment variables at:  
VC++ Directories->Executable Directories.

# Build and run:

1. [Download the Illustrator SDK](https://console.adobe.io/downloads/ai)  
`https://console.adobe.io/downloads/ai`
(See details above.)  

2. Place project folder in the `samplecode` folder

3. In Visual Studio make sure the Build Events > Post-Build Event > Command Line path is pointing to your Illustrator plug-ins folder.

4. Build project

5. Restart Illustrator

# Development notes:

## PICA means the host (Adobe Illustrator) application

## *SetNote* can be used for debug output
I used the Attributes panel for debug output. Each Illustrator object can have a user note (string) that is displayed in the Attributes panel. This is usually empty, and can be set with `SetNote` from the `AIArtSuite`.

## Plug-in notifiers and timers
Plug-in notifiers and timers are used by a plug-in to have Illustrator inform it of certain events.
A notifier plug-in is notified when the state of an Illustrator document changes. For example, **a plug-in may request to be notified when the selection state changes.** A notifier plug-in registers for one or more notifications during start-up.
A timer plug-in is notified at regular time intervals. For example, a timer plug-in may request to be notified
once a second.
For more information, see AINotifierSuite and AITimerSuite in Adobe Illustrator API Reference.

## Plug-In Property List
PiPL is updated to latest framework and works with Adobe Illustrator 25.1.

## Plug-ing code entry point
By convention, the entry point is called `PluginMain` and is compiled with C linkage:  
`extern "C" ASAPI ASErr PluginMain(char* caller, char* selector, void* message);`

All plugin's recieve at least four message actions (during it's lifetime):  
`reload`  
`unload`  
`startup`  
`shutdown`  

`reload` seems to be called before `startup`
and probably `unload` before `shutdown`for 
