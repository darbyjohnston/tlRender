include(ExternalProject)

set(OpenEXR_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openexr.git")
set(OpenEXR_GIT_TAG "v3.1.7")

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
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenEXR_ARGS})
