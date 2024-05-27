include(ExternalProject)

if(WIN32)
    # Build FFmpeg with MSYS2 on Windows.
    find_package(Msys REQUIRED)
endif()

set(FFmpeg_DEPS)
if(TLRENDER_NET)
    list(APPEND FFmpeg_DEPS OpenSSL)
endif()
if(NOT WIN32)
    list(APPEND FFmpeg_DEPS NASM)
endif()

set(FFmpeg_SHARED_LIBS ON)
set(FFmpeg_DEBUG OFF)
set(FFmpeg_CFLAGS "--extra-cflags=-I${CMAKE_INSTALL_PREFIX}/include")
set(FFmpeg_CXXFLAGS "--extra-cxxflags=-I${CMAKE_INSTALL_PREFIX}/include")
set(FFmpeg_OBJCFLAGS "--extra-objcflags=-I${CMAKE_INSTALL_PREFIX}/include")
set(FFmpeg_LDFLAGS)
if(WIN32)
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=/LIBPATH:${CMAKE_INSTALL_PREFIX}/lib")
elseif(APPLE)
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-L${CMAKE_INSTALL_PREFIX}/lib")
else()
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-L${CMAKE_INSTALL_PREFIX}/lib")
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-L${CMAKE_INSTALL_PREFIX}/lib64")
endif()
if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND FFmpeg_CFLAGS "--extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FFmpeg_CXXFLAGS "--extra-cxxflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FFmpeg_OBJCFLAGS "--extra-objcflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
if(FFmpeg_DEBUG)
    list(APPEND FFmpeg_CFLAGS "--extra-cflags=-g")
    list(APPEND FFmpeg_CXXFLAGS "--extra-cxxflags=-g")
    list(APPEND FFmpeg_OBJCFLAGS "--extra-objcflags=-g")
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-g")
endif()
set(FFmpeg_CONFIGURE_ARGS
    --prefix=${CMAKE_INSTALL_PREFIX}
    --disable-programs
    --disable-doc
    --disable-postproc
    --disable-avfilter
    --disable-hwaccels
    --disable-devices
    --disable-filters
    --disable-alsa
    --disable-appkit
    --disable-avfoundation
    --disable-bzlib
    --disable-coreimage
    --disable-iconv
    --disable-libxcb
    --disable-libxcb-shm
    --disable-libxcb-xfixes
    --disable-libxcb-shape
    --disable-lzma
    --disable-metal
    --disable-sndio
    --disable-schannel
    --disable-sdl2
    --disable-securetransport
    --disable-vulkan
    --disable-xlib
    --disable-zlib
    --disable-amf
    --disable-audiotoolbox
    --disable-cuda-llvm
    --disable-cuvid
    --disable-d3d11va
    --disable-dxva2
    --disable-ffnvcodec
    --disable-nvdec
    --disable-nvenc
    --disable-v4l2-m2m
    --disable-vaapi
    --disable-vdpau
    --disable-videotoolbox
    --enable-pic
    ${FFmpeg_CFLAGS}
    ${FFmpeg_CXXFLAGS}
    ${FFmpeg_OBJCFLAGS}
    ${FFmpeg_LDFLAGS})
if(TLRENDER_FFMPEG_MINIMAL)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --disable-decoders
        --enable-decoder=aac
        --enable-decoder=ac3
        --enable-decoder=av1
        --enable-decoder=ayuv
        --enable-decoder=dnxhd
        --enable-decoder=eac3
        --enable-decoder=flac
        --enable-decoder=h264
        --enable-decoder=hevc
        --enable-decoder=mjpeg
        --enable-decoder=mp3
        --enable-decoder=mpeg2video
        --enable-decoder=mpeg4
        --enable-decoder=pcm_alaw
        --enable-decoder=pcm_alaw_at
        --enable-decoder=pcm_bluray
        --enable-decoder=pcm_dvd
        --enable-decoder=pcm_f16le
        --enable-decoder=pcm_f24le
        --enable-decoder=pcm_f32be
        --enable-decoder=pcm_f32le
        --enable-decoder=pcm_f64be
        --enable-decoder=pcm_f64le
        --enable-decoder=pcm_lxf
        --enable-decoder=pcm_mulaw
        --enable-decoder=pcm_mulaw_at
        --enable-decoder=pcm_s16be
        --enable-decoder=pcm_s16be_planar
        --enable-decoder=pcm_s16le
        --enable-decoder=pcm_s16le_planar
        --enable-decoder=pcm_s24be
        --enable-decoder=pcm_s24daud
        --enable-decoder=pcm_s24le
        --enable-decoder=pcm_s24le_planar
        --enable-decoder=pcm_s32be
        --enable-decoder=pcm_s32le
        --enable-decoder=pcm_s32le_planar
        --enable-decoder=pcm_s64be
        --enable-decoder=pcm_s64le
        --enable-decoder=pcm_s8
        --enable-decoder=pcm_s8_planar
        --enable-decoder=pcm_sga
        --enable-decoder=pcm_u16be
        --enable-decoder=pcm_u16le
        --enable-decoder=pcm_u24be
        --enable-decoder=pcm_u24le
        --enable-decoder=pcm_u32be
        --enable-decoder=pcm_u32le
        --enable-decoder=pcm_u8
        --enable-decoder=pcm_vidc
        --enable-decoder=prores
        --enable-decoder=rawvideo
        --enable-decoder=v210
        --enable-decoder=v210x
        --enable-decoder=v308
        --enable-decoder=v408
        --enable-decoder=v410
        --enable-decoder=vp9
        --enable-decoder=yuv4
        --disable-encoders
        --enable-encoder=aac
        --enable-encoder=ac3
        --enable-encoder=ayuv
        --enable-encoder=dnxhd
        --enable-encoder=eac3
        --enable-encoder=mjpeg
        --enable-encoder=mpeg2video
        --enable-encoder=mpeg4
        --enable-encoder=pcm_alaw
        --enable-encoder=pcm_alaw_at
        --enable-encoder=pcm_bluray
        --enable-encoder=pcm_dvd
        --enable-encoder=pcm_f32be
        --enable-encoder=pcm_f32le
        --enable-encoder=pcm_f64be
        --enable-encoder=pcm_f64le
        --enable-encoder=pcm_mulaw
        --enable-encoder=pcm_mulaw_at
        --enable-encoder=pcm_s16be
        --enable-encoder=pcm_s16be_planar
        --enable-encoder=pcm_s16le
        --enable-encoder=pcm_s16le_planar
        --enable-encoder=pcm_s24be
        --enable-encoder=pcm_s24daud
        --enable-encoder=pcm_s24le
        --enable-encoder=pcm_s24le_planar
        --enable-encoder=pcm_s32be
        --enable-encoder=pcm_s32le
        --enable-encoder=pcm_s32le_planar
        --enable-encoder=pcm_s64be
        --enable-encoder=pcm_s64le
        --enable-encoder=pcm_s8
        --enable-encoder=pcm_s8_planar
        --enable-encoder=pcm_u16be
        --enable-encoder=pcm_u16le
        --enable-encoder=pcm_u24be
        --enable-encoder=pcm_u24le
        --enable-encoder=pcm_u32be
        --enable-encoder=pcm_u32le
        --enable-encoder=pcm_u8
        --enable-encoder=pcm_vidc
        --enable-encoder=prores
        --enable-encoder=rawvideo
        --enable-encoder=v210
        --enable-encoder=v308
        --enable-encoder=v408
        --enable-encoder=v410
        --enable-encoder=yuv4
        --disable-demuxers
        --enable-demuxer=aac
        --enable-demuxer=ac3
        --enable-demuxer=aiff
        --enable-demuxer=av1
        --enable-demuxer=dnxhd
        --enable-demuxer=dts
        --enable-demuxer=dtshd
        --enable-demuxer=eac3
        --enable-demuxer=flac
        --enable-demuxer=h264
        --enable-demuxer=hevc
        --enable-demuxer=imf
        --enable-demuxer=m4v
        --enable-demuxer=mjpeg
        --enable-demuxer=mov
        --enable-demuxer=mp3
        --enable-demuxer=mxf
        --enable-demuxer=pcm_alaw
        --enable-demuxer=pcm_f32be
        --enable-demuxer=pcm_f32le
        --enable-demuxer=pcm_f64be
        --enable-demuxer=pcm_f64le
        --enable-demuxer=pcm_mulaw
        --enable-demuxer=pcm_s16be
        --enable-demuxer=pcm_s16le
        --enable-demuxer=pcm_s24be
        --enable-demuxer=pcm_s24le
        --enable-demuxer=pcm_s32be
        --enable-demuxer=pcm_s32le
        --enable-demuxer=pcm_s8
        --enable-demuxer=pcm_u16be
        --enable-demuxer=pcm_u16le
        --enable-demuxer=pcm_u24be
        --enable-demuxer=pcm_u24le
        --enable-demuxer=pcm_u32be
        --enable-demuxer=pcm_u32le
        --enable-demuxer=pcm_u8
        --enable-demuxer=pcm_vidc
        --enable-demuxer=rawvideo
        --enable-demuxer=v210
        --enable-demuxer=v210x
        --enable-demuxer=wav
        --enable-demuxer=yuv4mpegpipe
        --disable-muxers
        --enable-muxer=ac3
        --enable-muxer=aiff
        --enable-muxer=dnxhd
        --enable-muxer=dts
        --enable-muxer=eac3
        --enable-muxer=flac
        --enable-muxer=h264
        --enable-muxer=hevc
        --enable-muxer=m4v
        --enable-muxer=mjpeg
        --enable-muxer=mov
        --enable-muxer=mp4
        --enable-muxer=mpeg2video
        --enable-muxer=mxf
        --enable-muxer=pcm_alaw
        --enable-muxer=pcm_f32be
        --enable-muxer=pcm_f32le
        --enable-muxer=pcm_f64be
        --enable-muxer=pcm_f64le
        --enable-muxer=pcm_mulaw
        --enable-muxer=pcm_s16be
        --enable-muxer=pcm_s16le
        --enable-muxer=pcm_s24be
        --enable-muxer=pcm_s24le
        --enable-muxer=pcm_s32be
        --enable-muxer=pcm_s32le
        --enable-muxer=pcm_s8
        --enable-muxer=pcm_u16be
        --enable-muxer=pcm_u16le
        --enable-muxer=pcm_u24be
        --enable-muxer=pcm_u24le
        --enable-muxer=pcm_u32be
        --enable-muxer=pcm_u32le
        --enable-muxer=pcm_u8
        --enable-muxer=pcm_vidc
        --enable-muxer=rawvideo
        --enable-muxer=wav
        --enable-muxer=yuv4mpegpipe
        --disable-parsers
        --enable-parser=aac
        --enable-parser=ac3
        --enable-parser=av1
        --enable-parser=dnxhd
        --enable-parser=dolby_e
        --enable-parser=flac
        --enable-parser=h264
        --enable-parser=hevc
        --enable-parser=mjpeg
        --enable-parser=mpeg4video
        --enable-parser=mpegaudio
        --enable-parser=mpegvideo
        --enable-parser=vp9
        --disable-protocols
        --enable-protocol=crypto
        --enable-protocol=file
        --enable-protocol=ftp
        --enable-protocol=http
        --enable-protocol=httpproxy
        --enable-protocol=https
        --enable-protocol=md5)
endif()
if(NOT WIN32)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --x86asmexe=${CMAKE_INSTALL_PREFIX}/bin/nasm)
endif()
if(TLRENDER_NET)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --enable-openssl)
endif()
if(FFmpeg_SHARED_LIBS)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --disable-static
        --enable-shared)
endif()
if(FFmpeg_DEBUG)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --disable-optimizations
        --disable-stripping
        --enable-debug=3
        --assert-level=2)
endif()
if(WIN32)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --arch=x86_64
        --toolchain=msvc)
    set(FFmpeg_MSYS2
        ${MSYS_CMD}
        -use-full-path
        -defterm
        -no-start
        -here)

    # \bug Copy libssl.lib to ssl.lib and libcrypto.lib to crypto.lib so the
    # FFmpeg configure script can find them.
    set(FFmpeg_OPENSSL_COPY)
    if(TLRENDER_NET)
        set(FFmpeg_OPENSSL_COPY
            "cp ${CMAKE_INSTALL_PREFIX}/lib/libssl.lib ${CMAKE_INSTALL_PREFIX}/lib/ssl.lib && \
            cp ${CMAKE_INSTALL_PREFIX}/lib/libcrypto.lib ${CMAKE_INSTALL_PREFIX}/lib/crypto.lib &&")
    endif()

    list(JOIN FFmpeg_CONFIGURE_ARGS " " FFmpeg_CONFIGURE_ARGS_TMP)
    set(FFmpeg_CONFIGURE ${FFmpeg_MSYS2}
        -c "pacman -S diffutils make nasm --noconfirm && \
        ${FFmpeg_OPENSSL_COPY} \
        ./configure ${FFmpeg_CONFIGURE_ARGS_TMP}")
    set(FFmpeg_BUILD ${FFmpeg_MSYS2} -c "make")
    set(FFmpeg_INSTALL ${FFmpeg_MSYS2} -c "make install"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${CMAKE_INSTALL_PREFIX}/bin/avcodec.lib ${CMAKE_INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${CMAKE_INSTALL_PREFIX}/bin/avdevice.lib ${CMAKE_INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${CMAKE_INSTALL_PREFIX}/bin/avformat.lib ${CMAKE_INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${CMAKE_INSTALL_PREFIX}/bin/avutil.lib ${CMAKE_INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${CMAKE_INSTALL_PREFIX}/bin/swresample.lib ${CMAKE_INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${CMAKE_INSTALL_PREFIX}/bin/swscale.lib ${CMAKE_INSTALL_PREFIX}/lib")
else()
    set(FFmpeg_CONFIGURE ./configure ${FFmpeg_CONFIGURE_ARGS})
    set(FFmpeg_BUILD make)
    set(FFmpeg_INSTALL make install)
    if(APPLE)
        list(APPEND FFmpeg_INSTALL
            COMMAND install_name_tool -id @rpath/libavcodec.61.3.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.61.dylib
            COMMAND install_name_tool -id @rpath/libavdevice.61.1.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.61.dylib
            COMMAND install_name_tool -id @rpath/libavformat.61.1.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavformat.61.dylib
            COMMAND install_name_tool -id @rpath/libavutil.59.8.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavutil.59.dylib
            COMMAND install_name_tool -id @rpath/libswresample.5.1.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libswresample.5.dylib
            COMMAND install_name_tool -id @rpath/libswscale.8.1.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libswscale.8.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.5.dylib @rpath/libswresample.5.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.59.dylib @rpath/libavutil.59.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.61.3.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswscale.8.dylib @rpath/libswscale.8.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavformat.61.dylib @rpath/libavformat.61.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.61.dylib @rpath/libavcodec.61.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.5.dylib @rpath/libswresample.5.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.59.dylib @rpath/libavutil.59.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.61.1.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.61.dylib @rpath/libavcodec.61.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.5.dylib @rpath/libswresample.5.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.59.dylib @rpath/libavutil.59.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavformat.61.1.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.59.dylib @rpath/libavutil.59.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libswresample.5.1.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.59.dylib @rpath/libavutil.59.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libswscale.8.1.100.dylib)
    endif()
endif()

ExternalProject_Add(
    FFmpeg
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FFmpeg
    DEPENDS ${FFmpeg_DEPS}
    URL https://ffmpeg.org/releases/ffmpeg-7.0.1.tar.bz2
    CONFIGURE_COMMAND ${FFmpeg_CONFIGURE}
    BUILD_COMMAND ${FFmpeg_BUILD}
    INSTALL_COMMAND ${FFmpeg_INSTALL}
    BUILD_IN_SOURCE 1)
