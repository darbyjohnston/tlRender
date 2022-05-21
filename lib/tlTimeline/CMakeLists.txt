set(HEADERS
	Audio.h
	AudioInline.h
	IRender.h
	RenderOptions.h
	RenderOptionsInline.h
    Timeline.h
    TimelinePlayer.h
    TimelinePlayerInline.h
	Util.h
	UtilInline.h
	Video.h
	VideoInline.h)

set(SOURCE
	Audio.cpp
	IRender.cpp
	RenderOptions.cpp
    Timeline.cpp
    TimelinePlayer.cpp
	Util.cpp
	Video.cpp)

add_library(tlTimeline ${HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
