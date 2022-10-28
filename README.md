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

tlCore, tlIO, tlTimeline
------------------------
The core libraries providing timeline rendering, playback, and I/O.

Dependencies:
* [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)
* [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO)
* [FreeType](https://www.freetype.org)
* [nlohmann_json](https://github.com/nlohmann/json)
* [FSeq](https://github.com/darbyjohnston/FSeq)
* [ZLIB](https://zlib.net)

Optional dependencies:
* [FFmpeg](https://ffmpeg.org)
* [JPEG](https://libjpeg-turbo.org)
* [OpenEXR](https://www.openexr.com/)
* [PNG](https://libpng.sourceforge.io/index.html)
* [TIFF](http://www.libtiff.org)

tlRenderGL
----------
Library for rendering timelines with OpenGL.

tlQt, tlQtWidget, tlQtQuick
---------------------------
Libraries for integrating with Qt based applications.

Dependencies:
* [Qt](https://www.qt.io)


Applications
============

tlplay
------
![tlplay](etc/Images/tlplay-screenshot1.png)

Play timelines, movies, and image sequences.

tlbake
------
Render a timeline to a movie or image sequence.


Building
========

Dependencies
------------
A CMake super build script is provided to build the dependencies from source.

Note however that Qt is not included in the super build, you must install it
separately.

CMake Build Options
-------------------
* TLRENDER_MMAP - Enable memory-mapped file I/O
* TLRENDER_COVERAGE - Enable code coverage
* TLRENDER_PYTHON - Enable Python support (for OTIO Python adapters)
* TLRENDER_OCIO - Enable support for OpenColorIO
* TLRENDER_AUDIO - Enable support for audio
* TLRENDER_JPEG - Enable support for JPEG
* TLRENDER_TIFF - Enable support for TIFF
* TLRENDER_PNG - Enable support for PNG
* TLRENDER_EXR - Enable support for OpenEXR
* TLRENDER_FFMPEG - Enable support for FFmpeg
* TLRENDER_GL - Enable support for OpenGL
* TLRENDER_QT6 - Enable support for Qt6
* TLRENDER_QT5 - Enable support for Qt5
* TLRENDER_PROGRAMS - Build programs
* TLRENDER_EXAMPLES - Build examples
* TLRENDER_TESTS - Build tests

Building on Linux
-----------------
Clone the repository:
```
git clone https://github.com/darbyjohnston/tlRender.git
cd tlRender
git submodule init
git submodule update
```
Create a build directory:
```
mkdir build
cd build
```
Run CMake with the super build script:
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
cmake --build . -j 4 --config Debug
```
Try running the "play-glfw" example:
```
export LD_LIBRARY_PATH=$PWD/install/lib:$LD_LIBRARY_PATH
./tlRender/src/tlRender-build/examples/play-glfw/play-glfw ../etc/SampleData/MultipleClips.otio
```

Building on Linux with Qt
-------------------------
When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLRENDER_QT5":
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH="$PWD/install;$HOME/Qt/5.15.2/gcc_64" -DTLRENDER_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

Minimal build on Linux
----------------------
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug -DTLRENDER_OCIO=OFF -DTLRENDER_AUDIO=OFF -DTLRENDER_JPEG=OFF -DTLRENDER_TIFF=OFF -DTLRENDER_PNG=OFF -DTLRENDER_EXR=OFF -DTLRENDER_FFMPEG=OFF -DTLRENDER_PROGRAMS=OFF -DTLRENDER_EXAMPLES=OFF -DTLRENDER_TESTS=OFF
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
gcovr -r ../../../../lib --html --object-directory lib --html-details --output gcov.html lib/tlCore lib/tlIO lib/tlTimeline
```

Building on macOS
-----------------
Clone the repository:
```
git clone https://github.com/darbyjohnston/tlRender.git
cd tlRender
git submodule init
git submodule update
```
Create a build directory:
```
mkdir build
cd build
```
Run CMake with the super build script:
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
cmake --build . -j 4 --config Debug
```
Try running the "play-glfw" example:
```
./tlRender/src/tlRender-build/examples/play-glfw/play-glfw ../etc/SampleData/MultipleClips.otio
```

Building on macOS with Qt
-------------------------
When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLRENDER_QT5":
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH="$PWD/install;$HOME/Qt/5.15.2/clang_64" -DTLRENDER_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

Notes for building on macOS
---------------------------
The CMake variable "CMAKE_OSX_ARCHITECTURES" can be used to specify the build
architecture:
```
-DCMAKE_OSX_ARCHITECTURES=x86_64
```
```
-DCMAKE_OSX_ARCHITECTURES=arm64
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
git submodule init
git submodule update
```
Create a build directory:
```
mkdir build
cd build
```
Run CMake with the super build script:
```
cmake ..\etc\SuperBuild -DCMAKE_INSTALL_PREFIX=%CD%\install -DCMAKE_PREFIX_PATH=%CD%\install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
cmake --build . -j 4 --config Debug
```
Try running the "play-glfw" example:
```
set PATH=%CD%\install\bin;%PATH%
.\tlRender\src\tlRender-build\examples\play-glfw\Debug\play-glfw ..\etc\SampleData\MultipleClips.otio
```

Building on Windows with Qt
---------------------------
When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLRENDER_QT5":
```
cmake ..\etc\SuperBuild -DCMAKE_INSTALL_PREFIX=%CD%\install -DCMAKE_PREFIX_PATH="%CD%\install;C:\Qt\5.15.2\msvc2019_64" -DTLRENDER_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

Notes for building on Windows
-----------------------------
When working on the tlRender codebase you can skip the dependencies
in subsequent builds:
```
cmake --build tlRender\src\tlRender-build -j 4 --config Debug
```
