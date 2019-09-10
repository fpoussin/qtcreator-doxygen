curl -fsSL -o qtc.zip %URL%
7z x qtc.zip
REN qt-creator-opensource-src-%QTCVER% qtc-src

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %ARCH%

SET BASE=%CD%

cd qtc-src
mkdir build
cd build
qmake ..
jom sub-src-qmake_all
cd src
cd libs
jom sub-aggregation sub-extensionsystem sub-utils sub-3rdparty-syntax-highlighting sub-ssh sub-cplusplus
cd ..
cd plugins
jom sub-coreplugin sub-cpptools sub-cppeditor sub-projectexplorer sub-texteditor

cd %BASE%
mkdir build
cd build
qmake QTC_SOURCE=..\qtc-src QTC_BUILD=..\qtc-src\build ..
jom

