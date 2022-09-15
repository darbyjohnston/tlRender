set(HEADERS
	Audio.h
	AudioInline.h
	ColorConfigOptions.h
	ColorConfigOptionsInline.h
	CompareOptions.h
	CompareOptionsInline.h
	DisplayOptions.h
	DisplayOptionsInline.h
	ImageOptions.h
	ImageOptionsInline.h
	IRender.h
	LUTOptions.h
	LUTOptionsInline.h
	MemoryReference.h
	SequenceMemoryReference.h
    Timeline.h
    TimelinePlayer.h
    TimelinePlayerInline.h
	Transition.h
	Util.h
	UtilInline.h
	Video.h
	VideoInline.h)
set(PRIVATE_HEADERS
	TimelinePrivate.h
	TimelinePlayerPrivate.h)

set(SOURCE
	ColorConfigOptions.cpp
	CompareOptions.cpp
	DisplayOptions.cpp
	ImageOptions.cpp
	IRender.cpp
	LUTOptions.cpp
	MemoryReference.cpp
	SequenceMemoryReference.cpp
    Timeline.cpp
	TimelineCreate.cpp
	TimelinePrivate.cpp
    TimelinePlayer.cpp
    TimelinePlayerPrivate.cpp
	Transition.cpp
	Util.cpp)

add_library(tlTimeline ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
