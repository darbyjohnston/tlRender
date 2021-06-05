include(ExternalProject)

set(FFmpeg_DEPS ZLIB)
if(WIN32)
else()
    set(FFmpeg_DEPS ${FFmpeg_DEPS} NASM)
endif()

set(FFmpeg_SHARED_LIBS ON)
set(FFmpeg_DEBUG OFF)

if(WIN32)
    # See the directions for building FFmpeg on Windows in "docs/build_windows.html".
else()
    set(FFmpeg_CONFIGURE_ARGS
        --prefix=${CMAKE_INSTALL_PREFIX}
        --disable-programs
        --disable-bzlib
        --disable-iconv
        --disable-lzma
        --disable-appkit
        --disable-avfoundation
        --disable-coreimage
        --disable-audiotoolbox
        --disable-vaapi
        --enable-pic)
    if(FFmpeg_SHARED_LIBS)
        set(FFmpeg_CONFIGURE_ARGS
            ${FFmpeg_CONFIGURE_ARGS}
            --disable-static
            --enable-shared)
    endif()
    if(FFmpeg_DEBUG)
        set(FFmpeg_CONFIGURE_ARGS
            ${FFmpeg_CONFIGURE_ARGS}
            --extra-cflags=-g
            --extra-cxxflags=-g
            --extra-ldflags=-g
            --disable-optimizations
            --disable-stripping
            --enable-debug=3
            --assert-level=2)
    endif()
    ExternalProject_Add(
        FFmpeg
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FFmpeg
        DEPENDS ${FFmpeg_DEPS}
        URL https://ffmpeg.org/releases/ffmpeg-4.4.tar.bz2
        CONFIGURE_COMMAND ./configure ${FFmpeg_CONFIGURE_ARGS}
        BUILD_IN_SOURCE 1)
endif()

