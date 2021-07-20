include(ExternalProject)

set(glad_BUILD_SHARED_LIBS OFF)

set(glad_ARGS ${TLR_EXTERNAL_ARGS})

ExternalProject_Add(
    glad
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/glad
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/glad
    CMAKE_ARGS ${glad_ARGS})
