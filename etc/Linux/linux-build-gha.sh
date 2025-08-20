#!/bin/bash

set -x

BUILD_TYPE=$1

# Update packages
sudo apt-get update

# Install lcov
if [[ $TLRENDER_GCOV = "ON" ]]
then
    sudo apt-get install lcov
fi

# Install OpenGL support
sudo apt-get install xorg-dev libglu1-mesa-dev mesa-common-dev mesa-utils xvfb
xvfb-run glxinfo

# Install ALSA and PulseAudio support
if [[ $TLRENDER_AUDIO = "ON" ]]
then
    sudo apt-get install libasound2-dev libpulse-dev
fi

# Install Qt support
if [[ $TLRENDER_QT6 = "ON" ]]
then
    sudo apt-get install qt6-base-dev qt6-5compat-dev qt6-declarative-dev qt6-svg-dev
fi
if [[ $TLRENDER_QT5 = "ON" ]]
then
    sudo apt-get install qtdeclarative5-dev libqt5quick5 qtbase5-dev libqt5svg5-dev qtchooser qt5-qmake qtbase5-dev-tools
fi

# Build tlRender
JOBS=4

cmake \
    -S tlRender/etc/SuperBuild \
    -B superbuild-$BUILD_TYPE \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PWD/install-$BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$PWD/install-$BUILD_TYPE \
    -DTLRENDER_NET=$TLRENDER_NET \
    -DTLRENDER_OCIO=$TLRENDER_OCIO \
    -DTLRENDER_AUDIO=$TLRENDER_AUDIO \
    -DTLRENDER_JPEG=$TLRENDER_JPEG \
    -DTLRENDER_TIFF=$TLRENDER_TIFF \
    -DTLRENDER_STB=$TLRENDER_STB \
    -DTLRENDER_PNG=$TLRENDER_PNG \
    -DTLRENDER_EXR=$TLRENDER_EXR \
    -DTLRENDER_FFMPEG=$TLRENDER_FFMPEG \
    -DTLRENDER_FFMPEG_MINIMAL=$TLRENDER_FFMPEG_MINIMAL \
    -DTLRENDER_USD=$TLRENDER_USD \
    -DTLRENDER_QT6=$TLRENDER_QT6 \
    -DTLRENDER_QT5=$TLRENDER_QT5 \
    -DTLRENDER_GCOV=$TLRENDER_GCOV \
    -Dfeather_tk_API=$FEATHER_TK_API
cmake --build superbuild-$BUILD_TYPE -j $JOBS --config $BUILD_TYPE

cmake \
    -S tlRender \
    -B build-$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PWD/install-$BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$PWD/install-$BUILD_TYPE \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DTLRENDER_NET=$TLRENDER_NET \
    -DTLRENDER_OCIO=$TLRENDER_OCIO \
    -DTLRENDER_AUDIO=$TLRENDER_AUDIO \
    -DTLRENDER_JPEG=$TLRENDER_JPEG \
    -DTLRENDER_TIFF=$TLRENDER_TIFF \
    -DTLRENDER_STB=$TLRENDER_STB \
    -DTLRENDER_PNG=$TLRENDER_PNG \
    -DTLRENDER_EXR=$TLRENDER_EXR \
    -DTLRENDER_FFMPEG=$TLRENDER_FFMPEG \
    -DTLRENDER_USD=$TLRENDER_USD \
    -DTLRENDER_QT6=$TLRENDER_QT6 \
    -DTLRENDER_QT5=$TLRENDER_QT5 \
    -DTLRENDER_PROGRAMS=$TLRENDER_PROGRAMS \
    -DTLRENDER_EXAMPLES=$TLRENDER_EXAMPLES \
    -DTLRENDER_TESTS=$TLRENDER_TESTS \
    -DTLRENDER_GCOV=$TLRENDER_GCOV \
    -Dfeather_tk_API=$FEATHER_TK_API
cmake --build build-$BUILD_TYPE -j $JOBS --config $BUILD_TYPE
cmake --build build-$BUILD_TYPE --config $BUILD_TYPE --target install
