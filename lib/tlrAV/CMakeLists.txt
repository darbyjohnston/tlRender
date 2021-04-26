set(HEADERS
    Color.h
    ColorInline.h
    IO.h
    IOInline.h
    Image.h
    ImageInline.h
    FFmpeg.h)
set(SOURCE
    IO.cpp
    Image.cpp
    FFmpeg.cpp)

add_library(tlrAV ${HEADERS} ${SOURCE})
target_link_libraries(tlrAV tlrTimeline FFmpeg)
set_target_properties(tlrAV PROPERTIES FOLDER lib)
