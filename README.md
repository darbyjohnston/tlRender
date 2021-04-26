[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)


tlRender
========
tlRender, or timeline render, is an early stage project for rendering
editorial timelines.

The project includes an application, "tlrplay", that can open an editorial
timeline and play it back in a window. A HUD (heads up display), keyboard
shortcuts, and command line options provide a simple UI to control the
application. 

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
* FFmpeg - https://ffmpeg.org/
* GLFW - https://www.glfw.org/
* glad - https://github.com/Dav1dde/glad
* FreeType - https://www.freetype.org/
* ZLIB - https://zlib.net/
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
$ mkdir tlRender-build
$ cd tlRender-build
```
Run CMake with the super build script:
```
$ cmake ../tlRender/etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install
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
$ mkdir tlRender-build
$ cd tlRender-build
```
Run CMake with the super build script:
```
$ cmake ../tlRender/etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_PREFIX_PATH=$PWD/install
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


Building on Windows
-------------------
Clone the repository:
```
> git clone https://github.com/darbyjohnston/tlRender.git
```
Create a build directory:
```
> mkdir tlRender-build
> cd tlRender-build
```
Run CMake with the super build script:
```
> cmake ../tlRender/etc/SuperBuild/ -DCMAKE_INSTALL_PREFIX=%CD%/install -DCMAKE_PREFIX_PATH=%CD%/install
```
Start the build:
```
> cmake --build . -j
```
Try running the "tlrplay" application:
```
> set PATH=%CD%\install\bin;%PATH%
> .\install\bin\tlrplay ..\tlRender\etc\SampleData\multiple_clips.otio -ws 4
```

