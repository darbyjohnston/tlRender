include(ExternalProject)

set(tlRender_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DTLR_ENABLE_MMAP=${TLR_ENABLE_MMAP}
    -DTLR_ENABLE_PYTHON=${TLR_ENABLE_PYTHON}
    -DTLR_BUILD_GL=${TLR_BUILD_GL}
    -DTLR_BUILD_QT=${TLR_BUILD_QT}
    -DTLR_BUILD_APPS=${TLR_BUILD_APPS}
    -DTLR_BUILD_EXAMPLES=${TLR_BUILD_EXAMPLES}
    -DTLR_BUILD_TESTS=${TLR_BUILD_TESTS})

set(tlRender_DEPENDS OTIO OCIO FSeq)
if(TLR_BUILD_JPEG)
    list(APPEND tlRender_DEPENDS JPEG)
endif()
if(TLR_BUILD_TIFF)
    list(APPEND tlRender_DEPENDS TIFF)
endif()
if(TLR_BUILD_PNG)
    list(APPEND tlRender_DEPENDS PNG)
endif()
if(TLR_BUILD_FFmpeg)
    list(APPEND tlRender_DEPENDS FFmpeg)
endif()
if(TLR_BUILD_GL)
    list(APPEND tlRender_DEPENDS glad FreeType)
    if(TLR_BUILD_APPS OR TLR_BUILD_EXAMPLES)
        list(PREPEND tlRender_DEPENDS GLFW)
    endif()
endif()

ExternalProject_Add(
    tlRender
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/tlRender
    DEPENDS ${tlRender_DEPENDS}
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../..
    LIST_SEPARATOR |
    CMAKE_ARGS ${tlRender_ARGS})
