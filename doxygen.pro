DEFINES += DOXYGEN_LIBRARY

# Doxygen files

SOURCES += doxygenplugin.cpp \
    doxygensettings.cpp \
    doxygensettingsstruct.cpp \
    doxygensettingswidget.cpp

HEADERS += doxygenplugin.h \
        doxygen_global.h \
        doxygenconstants.h \
    doxygensettings.h \
    doxygensettingsstruct.h \
    doxygensettingswidget.h

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=/home/fpoussin/src/qt-creator-opensource-src-3.5.1

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=/home/fpoussin/src/qt-creator-opensource-src-3.5.1/build

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = Doxygen
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin \
    cpptools \
    cppeditor

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

INCLUDEPATH += $$QTCREATOR_SOURCES/src

DEFINES -= QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

FORMS += \
    doxygensettingswidget.ui
