set(HEADERS
	Audio.h
	AudioInline.h
	CompareOptions.h
	CompareOptionsInline.h
	DisplayOptions.h
	DisplayOptionsInline.h
	ImageOptions.h
	ImageOptionsInline.h
	IRender.h
    Timeline.h
    TimelinePlayer.h
    TimelinePlayerInline.h
	Transition.h
	Util.h
	UtilInline.h
	Video.h
	VideoInline.h)

set(SOURCE
	CompareOptions.cpp
	DisplayOptions.cpp
	ImageOptions.cpp
	IRender.cpp
    Timeline.cpp
    TimelinePlayer.cpp
	Transition.cpp
	Util.cpp)

add_library(tlTimeline ${HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
