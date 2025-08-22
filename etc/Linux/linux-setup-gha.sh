#!/bin/sh

set -x

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
sudo apt-get install libasound2-dev libpulse-dev

# Install Qt support
if [[ $TLRENDER_QT6 = "ON" ]]
then
    sudo apt-get install qt6-base-dev qt6-5compat-dev qt6-declarative-dev qt6-svg-dev
fi
if [[ $TLRENDER_QT5 = "ON" ]]
then
    sudo apt-get install qtdeclarative5-dev libqt5quick5 qtbase5-dev libqt5svg5-dev qtchooser qt5-qmake qtbase5-dev-tools
fi
