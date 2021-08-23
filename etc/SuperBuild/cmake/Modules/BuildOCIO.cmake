include(ExternalProject)

set(OCIO_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DOCIO_BUILD_APPS=OFF
    -DOCIO_BUILD_TESTS=OFF
    -DOCIO_BUILD_GPU_TESTS=OFF
    -DOCIO_BUILD_PYTHON=OFF
    -DOCIO_INSTALL_EXT_PACKAGES=ALL)
if(NOT BUILD_SHARED_LIBS)
    list(APPEND OCIO_ARGS -DHalf_STATIC_LIBRARY=ON)
endif()
cmake_host_system_information(RESULT HAS_SSE2 QUERY HAS_SSE2)
set(OCIO_ARGS ${OCIO_ARGS} -DOCIO_USE_SSE=${HAS_SSE2})
if(APPLE)
    execute_process(
        COMMAND uname -m
        RESULT_VARIABLE result
        OUTPUT_VARIABLE OCIO_OSX_NATIVE_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(OCIO_ARGS ${OCIO_ARGS} -DCMAKE_OSX_ARCHITECTURES=${OCIO_OSX_NATIVE_ARCH})
endif()

ExternalProject_Add(
    OCIO
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OCIO
    DEPENDS IlmBase
    URL "https://github.com/AcademySoftwareFoundation/OpenColorIO/archive/refs/tags/v2.0.1.tar.gz"
    PATCH_COMMAND
        ${CMAKE_COMMAND} -E tar xf
        ${CMAKE_SOURCE_DIR}/OCIO-patch.tar.gz
    LIST_SEPARATOR |
    CMAKE_ARGS ${OCIO_ARGS})
