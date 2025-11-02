set(HEADERS
    IOTest.h)

set(SOURCE
    IOTest.cpp)

if(TLRENDER_FFMPEG)
    list(APPEND HEADERS FFmpegTest.h)
    list(APPEND SOURCE FFmpegTest.cpp)
endif()
if(TLRENDER_EXR)
    list(APPEND HEADERS OpenEXRTest.h)
    list(APPEND SOURCE OpenEXRTest.cpp)
endif()
if(TLRENDER_OIIO)
    list(APPEND HEADERS OIIOTest.h)
    list(APPEND SOURCE OIIOTest.cpp)
endif()

add_library(tlIOTest ${SOURCE} ${HEADERS})
target_link_libraries(tlIOTest tlTestLib tlIO)
set_target_properties(tlIOTest PROPERTIES FOLDER tests)
