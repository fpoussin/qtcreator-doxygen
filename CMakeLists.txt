cmake_minimum_required(VERSION 3.10)

project(Doxygen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QtCreator COMPONENTS Core REQUIRED)

add_qtc_plugin(Doxygen
  PLUGIN_DEPENDS QtCreator::Core QtCreator::CppTools QtCreator::CppEditor QtCreator::ProjectExplorer QtCreator::TextEditor
  SOURCES
    doxygenconstants.h
    doxygenfilesdialog.h
    doxygen_global.h
    doxygen.h  doxygenplugin.h
    doxygensettings.h
    doxygensettingsstruct.h
    doxygensettingswidget.h
    doxygen.cpp
    doxygenfilesdialog.cpp
    doxygenplugin.cpp
    doxygensettings.cpp
    doxygensettingsstruct.cpp
    doxygensettingswidget.cpp
)
