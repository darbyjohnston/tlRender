include(ExternalProject)

set(IlmBase_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF)
if(CMAKE_CXX_STANDARD)
    set(IlmBase_ARGS ${IlmBase_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
endif()

ExternalProject_Add(
    IlmBase
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/IlmBase
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openexr
    GIT_TAG v2.5.3
    SOURCE_SUBDIR IlmBase
    CMAKE_ARGS ${IlmBase_ARGS})
