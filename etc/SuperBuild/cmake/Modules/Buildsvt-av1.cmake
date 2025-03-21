include(ExternalProject)

set(svt-av1_GIT_REPOSITORY "https://gitlab.com/AOMediaCodec/SVT-AV1.git")
set(svt-av1_GIT_TAG "v3.0.1")

set(svt-av1_DEPS)
if(NOT WIN32)
    list(APPEND svt-av1_DEPS NASM)
endif()

#set(svt-av1_PATCH ${CMAKE_COMMAND} -E copy_if_different
#    ${CMAKE_CURRENT_SOURCE_DIR}/svt-av1-patch/Source/Lib/pkg-config.pc.in
#    ${CMAKE_CURRENT_BINARY_DIR}/svt-av1/src/svt-av1/Source/Lib/pkg-config.pc.in)

set(svt-av1_ARGS
    ${toucan_EXTERNAL_PROJECT_ARGS}
    -DBUILD_APPS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
    -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_PREFIX}/bin)

ExternalProject_Add(
    svt-av1
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/svt-av1
    DEPENDS ${svt-av1_DEPS}
    GIT_REPOSITORY ${svt-av1_GIT_REPOSITORY}
    GIT_TAG ${svt-av1_GIT_TAG}
    #PATCH_COMMAND ${svt-av1_PATCH}
    CMAKE_ARGS ${svt-av1_ARGS})
