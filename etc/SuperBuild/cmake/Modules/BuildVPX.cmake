include(ExternalProject)


# set(LIBVPX_TAG main) # live on the cutting-edge!

set(VPX_TAG v1.12.0) # proven to work


if(WIN32 OR NOT TLRENDER_FFMPEG)
    # Use media_autobuild-suite to build FFmpeg with VPX support on Windows
else()

    set(VPX_CFLAGS)
    set(VPX_CXXFLAGS)
    set(VPX_OBJCFLAGS)
    set(VPX_LDFLAGS)

    set(VPX_CONFIGURE_ARGS
        --prefix=${CMAKE_INSTALL_PREFIX}
        --enable-pic
        --disable-examples
        --disable-tools
        --disable-docs
        --disable-unit-tests
    )

    ExternalProject_Add(
	VPX
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/VPX
	DEPENDS NASM ${TLRENDER_YASM_DEP}
	GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
	GIT_TAG ${VPX_TAG}
	CONFIGURE_COMMAND PATH=${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH} ./configure ${VPX_CONFIGURE_ARGS}
	BUILD_IN_SOURCE 1
    )

endif()