include(ExternalProject)

set(IlmBase_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openexr.git")
set(IlmBase_GIT_TAG "c32f82c5f1833d959321fc5f615ca52836c7ba65") # v2.5.3

set(IlmBase_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF)

ExternalProject_Add(
    IlmBase
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/IlmBase
    GIT_REPOSITORY ${IlmBase_GIT_REPOSITORY}
    GIT_TAG ${IlmBase_GIT_TAG}
    SOURCE_SUBDIR IlmBase
    LIST_SEPARATOR |
    CMAKE_ARGS ${IlmBase_ARGS})
