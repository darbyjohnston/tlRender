[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)


tlRender
========
tlRender, or timeline render, is an early stage project for rendering
editorial timelines.

The project includes an application, "tlrplay", that can open an editorial
timeline and play it back in a window. A HUD (heads up display), keyboard
shortcuts, and command line options provide a simple UI to control the
application. 

The project is written in C++ and uses OpenGL for rendering. OpenTimelineIO
(https://github.com/PixarAnimationStudios/OpenTimelineIO), FFmpeg (https://ffmpeg.org/),
and other open source libraries are used for file I/O. The tlRender code is
provided under a BSD style open source license.

![tlrplay](etc/Images/tlrplay_screenshot1.jpg)

Currently Supported
-------------------
* Movie clips
* Gaps


To Do
-----
* Transitions
* Effects
* Audio support
* Hardware movie decoding
* Threads
* Memory cache
* Image sequences (OpenEXR, JPEG, TIFF, PNG)
* Render to file


Dependencies
------------
* OpenTimelineIO - https://github.com/PixarAnimationStudios/OpenTimelineIO
* FFmpeg - https://ffmpeg.org
* GLFW - https://www.glfw.org
* glad - https://github.com/Dav1dde/glad
* FreeType - https://www.freetype.org
* ZLIB - https://zlib.net
* FSeq - https://github.com/darbyjohnston/FSeq

A CMake super build script is provided to build the dependencies from source.


CMake Build Options
-------------------
* TLR_ENABLE_PYTHON - Enable Python support (allows use of Python adapters for timeline I/O)


Building on Linux
-----------------
Clone the repository:
```
$ git clone https://github.com/darbyjohnston/tlRender.git
```
Create a build directory:
```
$ mkdir tlRender-Debug
$ cd tlRender-Debug
```
Run CMake with the super build script:
```
$ cmake ../tlRender/etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
$ cmake --build . -j
```
Try running the "tlrplay" application:
```
$ export LD_LIBRARY_PATH=$PWD/install/lib:$LD_LIBRARY_PATH
$ ./install/bin/tlrplay ../tlRender/etc/SampleData/multiple_clips.otio -ws 4
```


Building on macOS
-----------------
Clone the repository:
```
$ git clone https://github.com/darbyjohnston/tlRender.git
```
Create a build directory:
```
$ mkdir tlRender-Debug
$ cd tlRender-Debug
```
Run CMake with the super build script:
```
$ cmake ../tlRender/etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
$ cmake --build . -j
```
Try running the "tlrplay" application:
```
$ export DYLD_LIBRARY_PATH=$PWD/install/lib:$LD_LIBRARY_PATH
$ ./install/bin/tlrplay ../tlRender/etc/SampleData/multiple_clips.otio -ws 4
```


Building FFmepg on Windows
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
$ sudo apt update
$ sudo apt install mingw-w64 yasm make unzip
```

Build FFmpeg, replacing $SOURCE_DIR and $BUILD_DIR with the same directories used in the
"Building on Windows" section:
```
> $SOURCE_DIR/etc/Windows/build_ffmpeg_wsl.sh $BUILD_DIR/install
```


Building on Windows
-------------------
Clone the repository:
```
> git clone https://github.com/darbyjohnston/tlRender.git
```
Create a build directory:
```
> mkdir tlRender-Debug
> cd tlRender-Debug
```
Run CMake with the super build script:
```
> cmake ../tlRender/etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=%CD%/install -DCMAKE_PREFIX_PATH=%CD%/install -DCMAKE_BUILD_TYPE=Debug
```
Start the build:
```
> cmake --build . -j
```
Try running the "tlrplay" application:
```
> set PATH=%CD%\install\bin;%CD%\install\lib;%PATH%
> .\install\bin\tlrplay ..\tlRender\etc\SampleData\multiple_clips.otio -ws 4
```

