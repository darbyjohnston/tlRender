cmake_minimum_required(VERSION 3.18)

project(tlRender-external-tests)

enable_testing()

foreach(EXTERNAL_DEP ${TLR_EXTERNAL_DEPS})
    find_package(${EXTERNAL_DEP})
    if(${EXTERNAL_DEP}_FOUND AND EXISTS ${CMAKE_SOURCE_DIR}/${EXTERNAL_DEP}Test)
        add_subdirectory(${EXTERNAL_DEP}Test)
    endif()
endforeach()
