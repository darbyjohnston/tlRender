set(HEADERS
    IO.h
    Util.h)
set(SOURCE
    IO.cpp
    Util.cpp)

add_library(tlrTimeline ${HEADERS} ${SOURCE})
set(tlrTimeline_LIBRARIES tlrCore OTIO)
if(TLR_ENABLE_PYTHON)
    list(APPEND tlrTimeline_LIBRARIES Python3::Python)
endif()
target_link_libraries(tlrTimeline ${tlrTimeline_LIBRARIES})
set_target_properties(tlrTimeline PROPERTIES FOLDER lib)
