set QTCMAJOR=4.10
set QTCMINOR=0
set QTCVER=%QTCMAJOR%.%QTCMINOR%

ARCH=x64
QT=5.13.1
QTDIR=C:\Qt\%QT%\msvc2017_64

set PATH=%QTDIR%\bin;C:\Qt\Tools\QtCreator\bin;%PATH%

set URL="https://download.qt.io/official_releases/qtcreator/%QTCMAJOR%/%QTCVER%/qt-creator-opensource-src-%QTCVER%.zip"

