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

add_library(tlQt ${HEADERS} ${SOURCE})
if(TLRENDER_BUILD_QT6)
	target_link_libraries(tlQt Qt6::Gui Qt6::OpenGL tlRenderGL)
elseif(TLRENDER_BUILD_QT5)
	target_link_libraries(tlQt Qt5::Gui tlRenderGL)
endif()
set_target_properties(tlQt PROPERTIES FOLDER lib)
set_target_properties(tlQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQt
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlQt)
