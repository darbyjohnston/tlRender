set(HEADERS
	IRender.h
	IRenderInline.h
	SoftwareRender.h
    Timeline.h
    TimelineInline.h
    TimelinePlayer.h
    TimelinePlayerInline.h
	TimelineUtil.h)
set(SOURCE
	IRender.cpp
	SoftwareRender.cpp
    Timeline.cpp
    TimelinePlayer.cpp
	TimelineUtil.cpp)

add_library(tlTimeline ${HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
