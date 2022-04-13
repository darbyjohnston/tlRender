set(HEADERS
	MetaTypes.h
    TimeObject.h
    TimelinePlayer.h
    TimelineThumbnailProvider.h
    ToolTipsFilter.h
    Util.h)

set(SOURCE
    TimeObject.cpp
    TimelinePlayer.cpp
    TimelineThumbnailProvider.cpp
    ToolTipsFilter.cpp
    Util.cpp)

set(LIBRARIES)

if(TLRENDER_BUILD_QT6)
	list(APPEND LIBRARIES Qt6::Gui Qt6::OpenGL)
elseif(TLRENDER_BUILD_QT5)
	list(APPEND LIBRARIES Qt5::Gui)
endif()
if(TLRENDER_BUILD_DL)
	list(APPEND HEADERS DLPlayback.h)
	list(APPEND SOURCE DLPlayback.cpp)
    list(APPEND LIBRARIES tlDL)
endif()
list(APPEND LIBRARIES tlGL)

add_library(tlQt ${HEADERS} ${SOURCE})
target_link_libraries(tlQt ${LIBRARIES})
set_target_properties(tlQt PROPERTIES FOLDER lib)
set_target_properties(tlQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQt
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlQt)
