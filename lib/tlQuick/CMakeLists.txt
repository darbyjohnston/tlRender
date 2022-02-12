set(HEADERS
    GLFramebufferObject.h
    Util.h)
set(SOURCE
    GLFramebufferObject.cpp
    Util.cpp)

add_library(tlQuick ${HEADERS} ${SOURCE})
if(TLRENDER_BUILD_QT6)
	target_link_libraries(tlQuick Qt6::Quick tlQt)
elseif(TLRENDER_BUILD_QT5)
	target_link_libraries(tlQuick Qt5::Quick tlQt)
endif()
set_target_properties(tlQuick PROPERTIES FOLDER lib)
set_target_properties(tlQuick PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQuick
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlQuick)
