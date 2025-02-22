include(ExternalProject)

set(tlRender_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DTLRENDER_NET=${TLRENDER_NET}
    -DTLRENDER_OCIO=${TLRENDER_OCIO}
    -DTLRENDER_SDL2=${TLRENDER_SDL2}
    -DTLRENDER_SDL3=${TLRENDER_SDL3}
    -DTLRENDER_JPEG=${TLRENDER_JPEG}
    -DTLRENDER_TIFF=${TLRENDER_TIFF}
    -DTLRENDER_STB=${TLRENDER_STB}
    -DTLRENDER_PNG=${TLRENDER_PNG}
    -DTLRENDER_EXR=${TLRENDER_EXR}
    -DTLRENDER_FFMPEG=${TLRENDER_FFMPEG}
    -DTLRENDER_USD=${TLRENDER_USD}
    -DTLRENDER_BMD=${TLRENDER_BMD}
    -DTLRENDER_BMD_SDK=${TLRENDER_BMD_SDK}
    -DTLRENDER_QT6=${TLRENDER_QT6}
    -DTLRENDER_QT5=${TLRENDER_QT5}
    -DTLRENDER_PROGRAMS=${TLRENDER_PROGRAMS}
    -DTLRENDER_EXAMPLES=${TLRENDER_EXAMPLES}
    -DTLRENDER_TESTS=${TLRENDER_TESTS}
    -DTLRENDER_IGNORE_PREFIX_PATH=${TLRENDER_IGNORE_PREFIX_PATH}
    -DTLRENDER_GCOV=${TLRENDER_GCOV}
    -DTLRENDER_GPROF=${TLRENDER_GPROF})

ExternalProject_Add(
    tlRender
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/tlRender
    DEPENDS ${TLRENDER_EXTERNAL_DEPS}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..
    LIST_SEPARATOR |
    CMAKE_ARGS ${tlRender_ARGS})
