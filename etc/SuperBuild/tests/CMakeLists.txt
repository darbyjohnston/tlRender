cmake_minimum_required(VERSION 3.19)

project(tlRender-external-tests)

enable_testing()

if(WIN32)
elseif(APPLE)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
else()
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

foreach(EXTERNAL_DEP ${TLRENDER_EXTERNAL_DEPS})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${EXTERNAL_DEP}Test)
        add_subdirectory(${EXTERNAL_DEP}Test)
    endif()
endforeach()
