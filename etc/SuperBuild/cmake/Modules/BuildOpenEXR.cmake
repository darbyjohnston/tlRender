include(ExternalProject)

set(OpenEXR_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openexr.git")
set(OpenEXR_GIT_TAG "44d2d0eaf289f54d4f86f45f4b6b68fac2d15f0e") # tag: v3.0.5

set(OpenEXR_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DOPENEXR_INSTALL_TOOLS=OFF
    -DOPENEXR_INSTALL_EXAMPLES=OFF
    -DBUILD_TESTING=OFF)

ExternalProject_Add(
    OpenEXR
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenEXR
    DEPENDS Imath ZLIB
    GIT_REPOSITORY ${OpenEXR_GIT_REPOSITORY}
    GIT_TAG ${OpenEXR_GIT_TAG}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/OpenEXR-patch/src/lib/OpenEXR/ImfDwaCompressor.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/OpenEXR/src/OpenEXR/src/lib/OpenEXR/ImfDwaCompressor.cpp
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenEXR_ARGS})
