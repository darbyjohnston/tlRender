set(HEADERS
	ContextObject.h
	DeviceModel.h
	MetaTypes.h
	OutputDevice.h
    TimeObject.h
    TimelinePlayer.h
    TimelineThumbnailProvider.h
    ToolTipsFilter.h
    Util.h)

set(SOURCE
	ContextObject.cpp
	DeviceModel.cpp
	OutputDevice.cpp
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
if(TLRENDER_BUILD_BMD)
    list(APPEND LIBRARIES tlBMD)
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
