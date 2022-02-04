[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://github.com/darbyjohnston/tlRender/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/darbyjohnston/tlRender/actions/workflows/ci-workflow.yml)
[![codecov](https://codecov.io/gh/codecov/example-cpp11-cmake/branch/master/graph/badge.svg)](https://codecov.io/gh/darbyjohnston/tlRender)

tlRender
========
tlRender is an open source library for building playback and review
applications for visual effects, film, and animation.

The library can render and playback timelines with multiple video clips,
image sequences, audio clips, and transitions. Timeline support is provided
by [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO),
color management by [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO),
and file I/O by [FFmpeg](https://ffmpeg.org), [OpenEXR](https://www.openexr.com/),
and other open source libraries.

The tlRender source code is provided under a BSD style open source license.

Supported:
* Movie files (H264, MP4, etc.)
* Image file sequences (Cineon, DPX, JPEG, OpenEXR, PNG, PPM, TIFF)
* Multi-channel audio
* Color management
* A/B comparison

To do:
* Software rendering
* Apple Metal rendering
* Microsoft DirectX rendering
* Effects
* GPU movie decoding
* Nested timelines
* Python bindings


Libraries
=========

tlrCore
-------
The core library providing timeline rendering, playback, and I/O.

Dependencies:
* [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)
* [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO)
* [FreeType](https://www.freetype.org)
* [FSeq](https://github.com/darbyjohnston/FSeq)
* [ZLIB](https://zlib.net)

Optional dependencies:
* [FFmpeg](https://ffmpeg.org)
* [JPEG](https://libjpeg-turbo.org)
* [OpenEXR](https://www.openexr.com/)
* [PNG](https://libpng.sourceforge.io/index.html)
* [TIFF](http://www.libtiff.org)

tlrGL
-----
Library for rendering timelines with OpenGL.

tlrQt, tlrQWidget, tlrQuick
---------------------------
Libraries for integrating with Qt based applications.

Dependencies:
* [Qt](https://www.qt.io)


Examples
========

play-glfw
------------
![play-glfw](etc/Images/play-glfw-screenshot1.png)

GLFW example playback application.

Dependencies:
* [GLFW](https://www.glfw.org)

play-qwidget
---------------
![play-qwidget](etc/Images/play-qwidget-screenshot1.png)

Qt widget example playback application.

play-quick
----------
Qt Quick example playback application.


Utilities
=========

tlrbake
-------
Command-line application for rendering a timeline to a movie or image sequence.


Building
========

Dependencies
------------
A CMake super build script is provided to build the dependencies from source.

Note however that Qt is not included in the super build, you must install it
separately.

CMake Build Options
-------------------
* TLR_ENABLE_MMAP - Enable memory-mapped file I/O
* TLR_ENABLE_COVERAGE - Enable code coverage
* TLR_ENABLE_PYTHON - Enable Python support (for OTIO Python adapters)
* TLR_BUILD_GL - Build OpenGL libraries
* TLR_BUILD_QT6 - Build Qt6 libraries
* TLR_BUILD_QT5 - Build Qt5 libraries
* TLR_BUILD_PROGRAMS - Build applications
* TLR_BUILD_EXAMPLES - Build examples
* TLR_BUILD_TESTS - Build tests
* TLR_BUILD_FFmpeg - Build FFmpeg support (Linux and macOS only)
* TLR_BUILD_JPEG - Build JPEG support
* TLR_BUILD_PNG - Build PNG support
* TLR_BUILD_OpenEXR - Build OpenEXR support
* TLR_BUILD_TIFF - Build TIFF support

Building on Linux
-----------------
Clone the repository:
```
git clone https://github.com/darbyjohnston/tlRender.git
cd tlRender
```
Create a build directory:
```
mkdir build
cd build
```
Run CMake with the super build script:
```
cmake ../etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
cmake --build . -j 4
```
Try running the "tlrplay-glfw" application:
```
export LD_LIBRARY_PATH=$PWD/install/lib:$LD_LIBRARY_PATH
./install/bin/tlrplay-glfw ../etc/SampleData/multiple_clips.otio
```

Building on Linux with Qt
-------------------------
When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLR_BUILD_QT5":
```
cmake ../etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH="$PWD/install;$HOME/Qt/5.15.2/gcc_64" -DTLR_BUILD_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

Notes for building on Linux
---------------------------
When working on the tlRender codebase you can skip the dependencies
in subsequent builds:
```
cmake --build tlRender/src/tlRender-build -j 4 --config Debug
```
Example for running gcovr for viewing code coverage:
```
gcovr -r ../lib/tlrCore --html --object-directory $PWD --html-details --output gcov.html
```

Building on macOS
-----------------
Clone the repository:
```
git clone https://github.com/darbyjohnston/tlRender.git
cd tlRender
```
Create a build directory:
```
mkdir build
cd build
```
Run CMake with the super build script:
```
cmake ../etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
cmake --build . -j 4
```
Try running the "tlrplay-glfw" application:
```
./install/bin/tlrplay-glfw ../etc/SampleData/multiple_clips.otio
```

Building on macOS with Qt
-------------------------
When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLR_BUILD_QT5":
```
cmake ../etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH="$PWD/install;$HOME/Qt/5.15.2/clang_64" -DTLR_BUILD_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

Notes for building on macOS
---------------------------
The CMake variable "CMAKE_OSX_ARCHITECTURES" can be used to specify the build
architecture:
```
-DCMAKE_OSX_ARCHITECTURES=x86_64
```
When working on the tlRender codebase you can skip the dependencies
in subsequent builds:
```
cmake --build tlRender/src/tlRender-build -j 4 --config Debug
```

Building FFmpeg on Windows
--------------------------
Most of the third party software that tlRender depends upon is built as part
of the CMake super build, except for FFmpeg on Windows. Instead the Windows
Subsystem for Linux (WSL) is used to compile FFmpeg as a separate step before
the CMake super build.

Enable the Windows Subsystem for Linux:

* Open the Windows control panel and click on "Programs and Features"
* Click on "Turn Windows features on or off" on the left side of the "Programs and Features" window
* Check the "Windows Subsystem for Linux" item in the "Windows Features" window
* Restart your computer

Install Ubuntu from the Windows app store, then open a shell and install necessary software:
```
sudo apt update
sudo apt install mingw-w64 yasm make unzip
```

Build FFmpeg, replacing $SOURCE_DIR and $BUILD_DIR with the same directories used in the
"Building on Windows" section:
```
$SOURCE_DIR/etc/Windows/build_ffmpeg_wsl.sh $BUILD_DIR/install
```

Building on Windows
-------------------
Clone the repository:
```
git clone https://github.com/darbyjohnston/tlRender.git
cd tlRender
```
Create a build directory:
```
mkdir build
cd build
```
Run CMake with the super build script:
```
cmake ../etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=%CD%/install -DCMAKE_PREFIX_PATH=%CD%/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
cmake --build . -j 4 --config Debug
```
Try running the "tlrplay-glfw" application:
```
set PATH=%CD%\install\bin;%PATH%
.\install\bin\tlrplay-glfw ..\etc\SampleData\multiple_clips.otio
```

Building on Windows with Qt
---------------------------
When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLR_BUILD_QT5":
```
cmake ../etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=%CD%/install -DCMAKE_PREFIX_PATH="%CD%/install;C:\Qt\5.15.2\msvc2019_64" -DTLR_BUILD_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

Notes for building on Windows
-----------------------------
When working on the tlRender codebase you can skip the dependencies
in subsequent builds:
```
cmake --build tlRender\src\tlRender-build -j 4 --config Debug
```
