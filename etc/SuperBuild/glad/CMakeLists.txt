cmake_minimum_required(VERSION 3.0)

project(glad LANGUAGES C)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(glad src/gl.c)

install(
    TARGETS glad
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(FILES include/glad/gl.h DESTINATION include/glad)
install(FILES include/KHR/khrplatform.h DESTINATION include/KHR)
