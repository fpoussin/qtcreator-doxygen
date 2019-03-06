# qtcreator-doxygen
Doxygen Plugin for Qt Creator

[![GitHub version](https://badge.fury.io/gh/fpoussin%2Fqtcreator-doxygen.svg)](https://github.com/fpoussin/qtcreator-doxygen/releases)
[![Build Status](https://jenkins.netyxia.net/buildStatus/icon?job=MotoLink%2Fmaster)](https://jenkins.netyxia.net/job/MotoLink/job/master/)  

This project is a fork of the original plugins at: http://dev.kofee.org/projects/qtcreator-doxygen  
Built for the latest Qt-Creator versions.

It adds some features such as a file selection dialog for projects, and duplicate blocks detection.

Binaries are available in the releases section.  
https://github.com/fpoussin/qtcreator-doxygen/releases

## Compiling

You will have to install the same Qt version (ie: 5.10 MSVC 2015 32 bit for 4.5.0) that was used to build the Qt creator version you are targeting for the plugin.  
You can check this in the "about" menu of Qt creator.  

* Download and extract the Qt creator sources from the official website  
* Compile them using the correct Qt kit (Optional on linux, you can point to the official binary release which should be in your home folder by default)  
* You don't need to install it when compiled  
  
  
##### To compile the plugin you have 2 options:  
#### Qmake  
* Specify the path of source and binaries for Qt creator using **QTC_SOURCE** and **QTC_BUILD** vars  
* **QTC_SOURCE** must point to the sources you extracted  
* **QTC_BUILD** must point to your build folder (or binary release on Linux)  
* Example command: *qmake QTC_SOURCE=\~/src/qt-creator-opensource-src-4.5.0 QTC_BUILD=\~/qtcreator-4.5.0* .  
  
#### Qt Creator  
* Specify the path of source and binaries for Qt creator by editing the dexygen.pro file  
* You have to change the **QTCREATOR_SOURCES** and **IDE_BUILD_TREE** vars  


## Installing  
If you compiled the plugin, it will be installed automatically.  
  
##### If you downloaded a binary release, the paths are as follow:
* Unix: ~/.local/share/data/QtProject/qtcreator/plugins  
* OSX: ~/Library/Application Support/QtProject/Qt Creator/plugins  
* Windows: %LOCALAPPDATA%\QtProject\qtcreator\plugins  
