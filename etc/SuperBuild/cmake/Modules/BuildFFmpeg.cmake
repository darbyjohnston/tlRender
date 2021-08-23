include(ExternalProject)

set(FFmpeg_DEPS ZLIB NASM)

set(FFmpeg_SHARED_LIBS ON)
set(FFmpeg_DEBUG OFF)

if(WIN32)
    # See the directions for building FFmpeg on Windows in "docs/build_windows.html".
else()
    set(FFmpeg_CFLAGS)
    set(FFmpeg_CXXFLAGS)
    set(FFmpeg_LDFLAGS)
    if(APPLE)
        set(FFmpeg_CFLAGS "${FFmpeg_CFLAGS} -mmacosx-version-min ${CMAKE_OSX_DEPLOYMENT_TARGET}")
        set(FFmpeg_CXXFLAGS "${FFmpeg_CXXFLAGS} -mmacosx-version-min ${CMAKE_OSX_DEPLOYMENT_TARGET}")
        set(FFmpeg_LDFLAGS "${FFmpeg_LDFLAGS} -mmacosx-version-min ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
    if(FFmpeg_DEBUG)
        set(FFmpeg_CFLAGS "${FFmpeg_CFLAGS} -g")
        set(FFmpeg_CXXFLAGS "${FFmpeg_CXXFLAGS} -g")
        set(FFmpeg_LDFLAGS "${FFmpeg_LDFLAGS} -g")
    endif()
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
        --enable-pic
        --extra-cflags="${FFmpeg_CFLAGS}"
        --extra-cxxflags="${FFmpeg_CXXFLAGS}"
        --extra-ldflags="${FFmpeg_LDFLAGS}"
        --x86asmexe=${CMAKE_INSTALL_PREFIX}/bin/nasm)
    if(FFmpeg_SHARED_LIBS)
        set(FFmpeg_CONFIGURE_ARGS
            ${FFmpeg_CONFIGURE_ARGS}
            --disable-static
            --enable-shared)
    endif()
    if(FFmpeg_DEBUG)
        set(FFmpeg_CONFIGURE_ARGS
            ${FFmpeg_CONFIGURE_ARGS}
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

