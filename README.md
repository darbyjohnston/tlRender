[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://github.com/darbyjohnston/tlRender/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/darbyjohnston/tlRender/actions/workflows/ci-workflow.yml)
[![codecov](https://codecov.io/gh/codecov/example-cpp11-cmake/branch/master/graph/badge.svg)](https://codecov.io/gh/darbyjohnston/tlRender)

tlRender
========
tlRender is an open source library for building playback and review
applications for visual effects, film, and animation.

The library can render and playback timelines with multiple video clips,
image sequences, audio clips, and transitions. Examples are provided for
integrating the library with Qt and GLFW applications.

The source code is written in C++14 and uses CMake for the build system.

Currently supported:
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

Contents:
* [Libraries](#libraries)
* [Applications](#applications)
* [Building](#building)
    * [Dependencies](#dependencies)
    * [CMake Build Options](#cmake-build-options)
    * [Building on Linux](#building-on-linux)
    * [Building on Linux with Qt](#building-on-linux-with-qt)
    * [Minimal build on Linux](#minimal-build-on-linux)
    * [Notes for building on Linux](#notes-for-building-on-linux)

# Libraries

## tlCore, tlIO, tlTimeline

The core libraries providing timeline rendering, playback, and I/O.

Required dependencies:
* [ZLIB](https://zlib.net)
* [FSeq](https://github.com/darbyjohnston/FSeq)
* [nlohmann_json](https://github.com/nlohmann/json)
* [Imath](https://github.com/AcademySoftwareFoundation/Imath)
* [FreeType](https://www.freetype.org)
* [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)

Optional dependencies:
* [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO)
* [RtAudio](https://github.com/thestk/rtaudio)
* [libsamplerate](https://github.com/libsndfile/libsamplerate)
* [JPEG](https://libjpeg-turbo.org)
* [TIFF](http://www.libtiff.org)
* [PNG](https://libpng.sourceforge.io/index.html)
* [OpenEXR](https://www.openexr.com/)
* [FFmpeg](https://ffmpeg.org)

## tlGL

Library for rendering timelines with OpenGL.

## tlQt, tlQtWidget, tlQtQuick

Libraries for integrating with Qt based applications.

Required dependencies:
* [Qt](https://www.qt.io)


# Applications

## tlplay

![tlplay](etc/Images/tlplay-screenshot1.png)

Play timelines, movies, and image sequences.

## tlbake

Render a timeline to a movie or image sequence.


# Building

## Dependencies

A CMake super build script is provided to build the dependencies from source,
except for Qt. Qt should be installed separately.

## CMake Build Options

| Name              | Description                                       | Default   |
| ----------------- | ------------------------------------------------- | --------- |
| TLRENDER_MMAP     | Enable memory-mapped file I/O                     | TRUE      |
| TLRENDER_COVERAGE | Enable code coverage                              | FALSE     |
| TLRENDER_PYTHON   | Enable Python support (for OTIO Python adapters)  | FALSE     |
| TLRENDER_OCIO     | Enable support for OpenColorIO                    | TRUE      |
| TLRENDER_AUDIO    | Enable support for audio                          | TRUE      |
| TLRENDER_JPEG     | Enable support for JPEG                           | TRUE      |
| TLRENDER_TIFF     | Enable support for TIFF                           | TRUE      |
| TLRENDER_PNG      | Enable support for PNG                            | TRUE      |
| TLRENDER_EXR      | Enable support for OpenEXR                        | TRUE      |
| TLRENDER_FFMPEG   | Enable support for FFmpeg                         | TRUE      |
| TLRENDER_GL       | Enable support for OpenGL                         | TRUE      |
| TLRENDER_BMD      | Enable support for Blackmagic Design devices      | FALSE     |
| TLRENDER_BMD_SDK  | Full path to the Blackmagic Design SDK            | ""        |
| TLRENDER_QT6      | Enable support for Qt6                            | FALSE     |
| TLRENDER_QT5      | Enable support for Qt5                            | FALSE     |
| TLRENDER_PROGRAMS | Build programs                                    | TRUE      |
| TLRENDER_EXAMPLES | Build examples                                    | TRUE      |
| TLRENDER_TESTS    | Build tests                                       | TRUE      |

## Building on Linux

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

## Building on Linux with Qt

When running CMake with the super build script, add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLRENDER_QT5":
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH="$PWD/install;$HOME/Qt/5.15.2/gcc_64" -DTLRENDER_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```

## Minimal build on Linux

Build with only the required dependencies, disabling all optional dependencies.
```
cmake ../etc/SuperBuild -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug -DTLRENDER_OCIO=OFF -DTLRENDER_AUDIO=OFF -DTLRENDER_JPEG=OFF -DTLRENDER_TIFF=OFF -DTLRENDER_PNG=OFF -DTLRENDER_EXR=OFF -DTLRENDER_FFMPEG=OFF -DTLRENDER_PROGRAMS=OFF -DTLRENDER_EXAMPLES=OFF -DTLRENDER_TESTS=OFF
```

## Notes for building on Linux

Example for running gcovr for code coverage:
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
When running CMake with the super build script add the Qt location to
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
When running CMake with the super build script add the Qt location to
"CMAKE_PREFIX_PATH" (make sure to use quotes), and enable "TLRENDER_QT5":
```
cmake ..\etc\SuperBuild -DCMAKE_INSTALL_PREFIX=%CD%\install -DCMAKE_PREFIX_PATH="%CD%\install;C:\Qt\5.15.2\msvc2019_64" -DTLRENDER_QT5=ON -DCMAKE_BUILD_TYPE=Debug
```
