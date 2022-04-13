set(HEADERS
    GLFramebufferObject.h
    Util.h)

set(SOURCE
    GLFramebufferObject.cpp
    Util.cpp)

set(LIBRARIES)
if(TLRENDER_BUILD_QT6)
	list(APPEND LIBRARIES Qt6::Quick)
elseif(TLRENDER_BUILD_QT5)
	list(APPEND LIBRARIES Qt5::Quick)
endif()
list(APPEND LIBRARIES tlQt)

add_library(tlQtQuick ${HEADERS} ${SOURCE})
target_link_libraries(tlQtQuick ${LIBRARIES})
set_target_properties(tlQtQuick PROPERTIES FOLDER lib)
set_target_properties(tlQtQuick PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQtQuick
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQtQuick)
