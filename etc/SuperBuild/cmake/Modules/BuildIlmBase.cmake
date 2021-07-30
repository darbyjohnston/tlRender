include(ExternalProject)

set(IlmBase_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF)

ExternalProject_Add(
    IlmBase
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/IlmBase
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openexr
    GIT_TAG v2.5.3
    SOURCE_SUBDIR IlmBase
    LIST_SEPARATOR |
    CMAKE_ARGS ${IlmBase_ARGS})
