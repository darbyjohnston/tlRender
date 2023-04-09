#!/bin/sh

wget https://ffmpeg.org/releases/ffmpeg-6.0.tar.bz2
tar xvf ffmpeg-6.0.tar.bz2
cd ffmpeg-6.0
./configure \
    --disable-programs \
    --disable-doc \
    --disable-bzlib \
    --disable-iconv \
    --disable-lzma \
    --disable-vaapi \
    --enable-pic \
    --arch=x86_64 \
    --cross-prefix=x86_64-w64-mingw32- \
    --target-os=mingw32 \
    --enable-cross-compile \
    --prefix=$1 \
    --enable-shared
make -j 8
make -j 8 install

# Currently there is a bug where the .lib files are installed in the bin
# directory instead of the lib directory
for i in avcodec.lib avdevice.lib avfilter.lib avformat.lib avutil.lib swresample.lib swscale.lib
do
    mv $1/bin/$i $1/lib
done
