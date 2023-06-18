set(HEADERS
    ContextObject.h
    Init.h
    MetaTypes.h
    OutputDevice.h
    TimeObject.h
    TimelinePlayer.h
    TimelineThumbnailObject.h
    ToolTipsFilter.h)
set(PRIVATE_HEADERS
    OutputDevicePrivate.h)

set(SOURCE
    ContextObject.cpp
    Init.cpp
    OutputDevice.cpp
    OutputDevicePrivate.cpp
    TimeObject.cpp
    TimelinePlayer.cpp
    TimelineThumbnailObject.cpp
    ToolTipsFilter.cpp)

set(LIBRARIES)
if(TLRENDER_QT6)
    list(APPEND LIBRARIES Qt6::Gui Qt6::OpenGL)
elseif(TLRENDER_QT5)
    list(APPEND LIBRARIES Qt5::Gui)
endif()
list(APPEND LIBRARIES tlGL tlDevice)

add_library(tlQt ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlQt ${LIBRARIES})
set_target_properties(tlQt PROPERTIES FOLDER lib)
set_target_properties(tlQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQt
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQt)
