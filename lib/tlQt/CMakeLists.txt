set(HEADERS
    ContextObject.h
    Init.h
    MetaTypes.h
    TimeObject.h
    TimelinePlayer.h
    ToolTipsFilter.h)
set(PRIVATE_HEADERS)

set(SOURCE
    ContextObject.cpp
    Init.cpp
    TimeObject.cpp
    TimelinePlayer.cpp
    ToolTipsFilter.cpp)

if(TLRENDER_BMD)
    list(APPEND HEADERS
        BMDOutputDevice.h)
    list(APPEND PRIVATE_HEADERS
        BMDOutputDevicePrivate.h)
    list(APPEND SOURCE
        BMDOutputDevice.cpp
        BMDOutputDevicePrivate.cpp)
endif()

set(LIBRARIES)
if(TLRENDER_QT6)
    list(APPEND LIBRARIES Qt6::Gui Qt6::OpenGL)
elseif(TLRENDER_QT5)
    list(APPEND LIBRARIES Qt5::Gui)
endif()
list(APPEND LIBRARIES tlDevice)

add_library(tlQt ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlQt ${LIBRARIES})
set_target_properties(tlQt PROPERTIES FOLDER lib)
set_target_properties(tlQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQt
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQt)
