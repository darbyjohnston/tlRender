set(HEADERS
    Color.h
    ColorInline.h
    IO.h
    IOInline.h
    Image.h
    ImageInline.h)
set(SOURCE
    IO.cpp
    Image.cpp)

set(tlrAV_LIBRARIES tlrTimeline)
if(JPEG_FOUND)
    set(HEADERS ${HEADERS} JPEG.h)
    set(SOURCE ${SOURCE} JPEG.cpp)
    set(tlrAV_LIBRARIES ${tlrAV_LIBRARIES} JPEG)
endif()
if(OpenEXR_FOUND)
    set(HEADERS ${HEADERS} OpenEXR.h)
    set(SOURCE ${SOURCE} OpenEXR.cpp)
    set(tlrAV_LIBRARIES ${tlrAV_LIBRARIES} OpenEXR)
endif()
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpeg.h)
    set(SOURCE ${SOURCE} FFmpeg.cpp)
    set(tlrAV_LIBRARIES ${tlrAV_LIBRARIES} FFmpeg)
endif()
set(tlrAV_LIBRARIES ${tlrAV_LIBRARIES} IlmBase)

add_library(tlrAV ${HEADERS} ${SOURCE})
target_link_libraries(tlrAV ${tlrAV_LIBRARIES})
set_target_properties(tlrAV PROPERTIES FOLDER lib)
