TEMPLATE = lib
TARGET = Doxygen
DEFINES += DOXYGEN_LIBRARY
PROVIDER = Kofee
# Define QTC_SOURCE_DIR to the location of Qt Creator sources (i.e: ~/dev/qtcreator/qt-creator-src/)
QTC_SOURCE_DIR = /home/kofee/dev/qtcreator/qt-creator-src/
IDE_SOURCE_TREE = $$QTC_SOURCE_DIR
# Define QTC_BUILD_DIR to the location of Qt Creator build dir for the plugin (i.e ~/dev/qtcreator/doxygen/)
QTC_BUILD_DIR = /home/kofee/dev/qtcreator/doxygen/
IDE_BUILD_TREE = $$QTC_BUILD_DIR
# Define DESTDIR to the local location of the installation of Qt creator (if local user)
# or the system location if building as root
DESTDIR = lib/qtcreator/plugins/$$(PROVIDER)

LIBS += -L/usr/lib/qtcreator -L/usr/lib/qtcreator/plugins/Nokia -L/usr/lib/qt4

include( $$IDE_SOURCE_TREE/src/qtcreatorplugin.pri )
include( $$IDE_SOURCE_TREE/src/plugins/coreplugin/coreplugin.pri )
include( $$IDE_SOURCE_TREE/src/plugins/texteditor/texteditor.pri )
include( $$IDE_SOURCE_TREE/src/plugins/cppeditor/cppeditor.pri )

HEADERS +=  doxygenplugin.h \
            doxygen_global.h \
            doxygenconstants.h \
            doxygen.h
SOURCES +=  doxygenplugin.cpp \
            doxygen.cpp
OTHER_FILES += Doxygen.pluginspec

INCLUDEPATH +=  $$QTC_SOURCE_DIR/src \
                $$QTC_SOURCE_DIR/src/plugins \
                $$QTC_SOURCE_DIR/src/libs \
                $$QTC_SOURCE_DIR/src/libs/cplusplus \
                $$QTC_SOURCE_DIR/src/shared \
                $$QTC_SOURCE_DIR/src/shared/cplusplus

message(QTC_SOURCE_DIR = $$QTC_SOURCE_DIR)
message(IDE_SOURCE_TREE = $$IDE_SOURCE_TREE)
message(QTC_BUILD_DIR = $$QTC_BUILD_DIR)
message(IDE_BUILD_TREE = $$IDE_BUILD_TREE)
message(DESTDIR = $$DESTDIR)
message(Good luck with make... :-D)
