CONFIG += release
TEMPLATE = lib
TARGET = Doxygen
PROVIDER = Kofee

macx: {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
    CONFIG += x86 x86_64

    # After compilation:
    #install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore /Applications/Qt\ Creator.app/Contents/PlugIns/Kofee/libDoxygen.dylib
    #install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui /Applications/Qt\ Creator.app/Contents/PlugIns/Kofee/libDoxygen.dylib
}

# Define QTC_SOURCE_DIR to the location of Qt Creator sources (i.e: ~/dev/qtcreator/qt-creator-src/)
isEmpty(QTC_SOURCE_DIR) {
    unix: {
        !macx: {
            QTC_SOURCE_DIR = /home/$$(USER)/Dev/Qt/qt-creator-2.1.0-src/
        }
        macx: {
            QTC_SOURCE_DIR = /Users/$$(USER)/Dev/qt-creator-2.1.0-src/
        }
    }
    win32:QTC_SOURCE_DIR = C:/Qt/qt-creator-2.1.0-src/
}
isEmpty(IDE_SOURCE_TREE):IDE_SOURCE_TREE = $$QTC_SOURCE_DIR

# Define QTC_BUILD_DIR to the location of Qt Creator build dir for the plugin (i.e ~/dev/qtcreator-doxygen/)
isEmpty(QTC_BUILD_DIR) {
    unix: {
        !macx: {
            QTC_BUILD_DIR = /home/$$(USER)/Dev/Qt/qtcreator-doxygen/
        }
        macx: {
            QTC_BUILD_DIR = /Users/$$(USER)/Dev/qtcreator-doxygen/
        }
    }
    win32:QTC_BUILD_DIR = C:/Qt/qtcreator-doxygen/
}
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE = $$QTC_BUILD_DIR

# You can define LIBSROOT as the root of qtcreator lib directory (i.e. LIBSROOT=/home/kofee/qtcreator-2.1.0/lib
isEmpty(LIBSROOT) {
    unix: {
        !macx: {
            LIBS += -L/home/$$(USER)/QtSDK/QtCreator/lib/qtcreator \
            -L/home/$$(USER)/QtSDK/QtCreator/lib/qtcreator/plugins/Nokia \
            -L/home/$$(USER)/QtSDK/QtCreator/lib
        }
        macx: {
            LIBS = -L"/Applications/Qt\ Creator.app/Contents/PlugIns" \
            -L"/Applications/Qt\ Creator.app/Contents/PlugIns/Nokia" \
            -F"/Applications/Qt\ Creator.app/Contents/Frameworks \
            -L"/Applications/Qt\ Creator.app/Contents/Frameworks
        }
    }
    win32:LIBS += -LC:/Qt/qtcreator-2.1.0/bin \
        -LC:/Qt/qtcreator-2.1.0/lib/qtcreator/plugins/Nokia/ \
        -LC:/Qt/qtcreator-2.1.0/lib/qtcreator/ \
        -LC:/Qt/qt-creator-2.1.0-src/lib/qtcreator \
        -LC:/Qt/qt-creator-2.1.0-src/lib/qtcreator/plugins/Nokia
} else {
    LIBS += -L$$LIBSROOT \
    -L$$LIBSROOT/qtcreator \
    -L$$LIBSROOT/qtcreator/plugins/Nokia/
}

include( $$IDE_SOURCE_TREE/src/qtcreatorplugin.pri )
include( $$IDE_SOURCE_TREE/src/plugins/coreplugin/coreplugin.pri )
include( $$IDE_SOURCE_TREE/src/plugins/texteditor/texteditor.pri )
include( $$IDE_SOURCE_TREE/src/plugins/cppeditor/cppeditor.pri )

# Define DEST to the location of the installation of Qt creator (if local user)
isEmpty(DEST) {
    unix: {
        !macx: {
            DESTDIR = /home/$$(USER)/QtSDK/QtCreator/lib/qtcreator/plugins/$$PROVIDER
        }
        macx: {
            DESTDIR = "/Applications/Qt\ Creator.app/Contents/PlugIns/Kofee"
        }
    }
} else {
    DESTDIR = $$DEST
}

# copy the pluginspec to its "final" destination 
!isEmpty(DESTDIR) {
    unix: {
        QMAKE_POST_LINK += cp Doxygen.pluginspec $$DESTDIR
    }
}

HEADERS += doxygenplugin.h \
    doxygen_global.h \
    doxygenconstants.h \
    doxygen.h \
    doxygensettings.h \
    doxygensettingswidget.h \
    doxygensettingsstruct.h
SOURCES += doxygenplugin.cpp \
    doxygen.cpp \
    doxygensettings.cpp \
    doxygensettingswidget.cpp \
    doxygensettingsstruct.cpp
FORMS += doxygensettingswidget.ui
RESOURCES += doxygen.qrc
OTHER_FILES += Doxygen.pluginspec \
    doxygen.png
INCLUDEPATH += $$QTC_SOURCE_DIR/src \
    $$QTC_SOURCE_DIR/src/plugins \
    $$QTC_SOURCE_DIR/src/libs \
    $$QTC_SOURCE_DIR/src/libs/cplusplus \
    $$QTC_SOURCE_DIR/src/libs/extensionsystem \
    $$QTC_SOURCE_DIR/src/libs/utils \
    $$QTC_SOURCE_DIR/src/shared \
    $$QTC_SOURCE_DIR/src/shared/cplusplus

message(QTC_SOURCE_DIR = $$QTC_SOURCE_DIR)
message(IDE_SOURCE_TREE = $$IDE_SOURCE_TREE)
message(QTC_BUILD_DIR = $$QTC_BUILD_DIR)
message(IDE_BUILD_TREE = $$IDE_BUILD_TREE)
message(DESTDIR = $$DESTDIR)
message(Good luck with make... :-D)
