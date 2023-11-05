include(ExternalProject)

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
    # Build FFmpeg with MSYS2 on Windows.
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --arch=x86_64
        --toolchain=msvc)
    set(FFmpeg_MSYS2
        ${TLRENDER_FFMPEG_MSYS2}/msys2_shell.cmd
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
    set(FFmpeg_BUILD ${FFmpeg_MSYS2} -c "make -j")
    set(FFmpeg_INSTALL ${FFmpeg_MSYS2} -c "make install")
else()
    set(FFmpeg_CONFIGURE ./configure ${FFmpeg_CONFIGURE_ARGS})
    set(FFmpeg_BUILD make)
    set(FFmpeg_INSTALL make install)
    if(APPLE)
        list(APPEND FFmpeg_INSTALL
            COMMAND install_name_tool -id @rpath/libavcodec.60.3.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.dylib
            COMMAND install_name_tool -id @rpath/libavdevice.60.1.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.60.dylib
            COMMAND install_name_tool -id @rpath/libavfilter.9.3.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavfilter.9.dylib
            COMMAND install_name_tool -id @rpath/libavformat.60.3.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavformat.60.dylib
            COMMAND install_name_tool -id @rpath/libavutil.58.2.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib
            COMMAND install_name_tool -id @rpath/libswresample.4.10.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.dylib
            COMMAND install_name_tool -id @rpath/libswscale.7.1.100.dylib ${CMAKE_INSTALL_PREFIX}/lib/libswscale.7.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.dylib @rpath/libswresample.4.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib @rpath/libavutil.58.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.3.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavfilter.9.dylib @rpath/libavfilter.9.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswscale.7.dylib @rpath/libswscale.7.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavformat.60.dylib @rpath/libavformat.60.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.dylib @rpath/libavcodec.60.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.dylib @rpath/libswresample.4.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib @rpath/libavutil.58.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavdevice.60.1.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswscale.7.dylib @rpath/libswscale.7.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavformat.60.dylib @rpath/libavformat.60.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.dylib @rpath/libavcodec.60.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.dylib @rpath/libswresample.4.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib @rpath/libavutil.58.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavfilter.9.3.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavcodec.60.dylib @rpath/libavcodec.60.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.dylib @rpath/libswresample.4.dylib
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib @rpath/libavutil.58.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libavformat.60.3.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib @rpath/libavutil.58.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libswresample.4.10.100.dylib
            COMMAND install_name_tool
                -change ${CMAKE_INSTALL_PREFIX}/lib/libavutil.58.dylib @rpath/libavutil.58.dylib
                ${CMAKE_INSTALL_PREFIX}/lib/libswscale.7.1.100.dylib)
    endif()
endif()

ExternalProject_Add(
    FFmpeg
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FFmpeg
    DEPENDS ${FFmpeg_DEPS}
    URL https://ffmpeg.org/releases/ffmpeg-6.0.tar.bz2
    CONFIGURE_COMMAND ${FFmpeg_CONFIGURE}
    BUILD_COMMAND ${FFmpeg_BUILD}
    INSTALL_COMMAND ${FFmpeg_INSTALL}
    BUILD_IN_SOURCE 1)
