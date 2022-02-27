set(HEADERS
    GLFramebufferObject.h
    Util.h)
set(SOURCE
    GLFramebufferObject.cpp
    Util.cpp)

add_library(tlQtQuick ${HEADERS} ${SOURCE})
if(TLRENDER_BUILD_QT6)
	target_link_libraries(tlQtQuick Qt6::Quick tlQt)
elseif(TLRENDER_BUILD_QT5)
	target_link_libraries(tlQtQuick Qt5::Quick tlQt)
endif()
set_target_properties(tlQtQuick PROPERTIES FOLDER lib)
set_target_properties(tlQtQuick PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQtQuick
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlQtQuick)
