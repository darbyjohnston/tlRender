include(ExternalProject)

set(OpenEXR_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF
    -DOPENEXR_BUILD_UTILS=OFF
    -DINSTALL_OPENEXR_DOCS=OFF)

ExternalProject_Add(
    OpenEXR
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenEXR
    DEPENDS IlmBase ZLIB
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openexr
    GIT_TAG v2.5.3
    SOURCE_SUBDIR OpenEXR
    LIST_SEPARATOR |
    CMAKE_ARGS ${OpenEXR_ARGS})
