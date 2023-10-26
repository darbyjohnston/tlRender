#!/bin/sh

set -e

wget https://github.com/openssl/openssl/archive/refs/tags/openssl-3.1.4.tar.gz
tar xvf openssl-3.1.4.tar.gz
cd openssl-openssl-3.1.4
./Configure \
    --prefix=$1 \
    --openssldir=$1 \
    no-zlib \
    mingw64 --cross-compile-prefix=x86_64-w64-mingw32-
make -j
make -j install
# \bug Rename the files
cp $1/lib64/libssl.dll.a $1/lib64/libssl.lib
cp $1/lib64/libcrypto.dll.a $1/lib64/libcrypto.lib
cd ..

wget https://ffmpeg.org/releases/ffmpeg-6.0.tar.bz2
tar xvf ffmpeg-6.0.tar.bz2
cd ffmpeg-6.0
./configure \
    --extra-cflags=-I$1/include \
    --extra-cxxflags=-I$1/include \
    --extra-ldflags=-L$1/lib \
    --extra-ldflags=-L$1/lib64 \
    --disable-programs \
    --disable-doc \
    --disable-hwaccels \
    --disable-devices \
    --disable-filters \
    --disable-alsa \
    --disable-appkit \
    --disable-avfoundation \
    --disable-bzlib \
    --disable-coreimage \
    --disable-iconv \
    --disable-libxcb \
    --disable-libxcb-shm \
    --disable-libxcb-xfixes \
    --disable-libxcb-shape \
    --disable-lzma \
    --disable-metal \
    --disable-sndio \
    --disable-schannel \
    --disable-sdl2 \
    --disable-securetransport \
    --disable-vulkan \
    --disable-xlib \
    --disable-zlib \
    --disable-amf \
    --disable-audiotoolbox \
    --disable-cuda-llvm \
    --disable-cuvid \
    --disable-d3d11va \
    --disable-dxva2 \
    --disable-ffnvcodec \
    --disable-nvdec \
    --disable-nvenc \
    --disable-v4l2-m2m \
    --disable-vaapi \
    --disable-vdpau \
    --disable-videotoolbox \
    --enable-openssl \
    --enable-pic \
    --arch=x86_64 \
    --cross-prefix=x86_64-w64-mingw32- \
    --target-os=mingw32 \
    --enable-cross-compile \
    --prefix=$1 \
    --enable-shared
make -j
make -j install

# \bug Move the files into the correct directory
for i in avcodec.lib avdevice.lib avfilter.lib avformat.lib avutil.lib swresample.lib swscale.lib
do
    mv $1/bin/$i $1/lib
done
