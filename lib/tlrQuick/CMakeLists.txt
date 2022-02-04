set(HEADERS
    GLFramebufferObject.h
    Util.h)
set(SOURCE
    GLFramebufferObject.cpp
    Util.cpp)

add_library(tlrQuick ${HEADERS} ${SOURCE})
if(TLR_BUILD_QT6)
	target_link_libraries(tlrQuick Qt6::Quick tlrQt)
elseif(TLR_BUILD_QT5)
	target_link_libraries(tlrQuick Qt5::Quick tlrQt)
endif()
set_target_properties(tlrQuick PROPERTIES FOLDER lib)
set_target_properties(tlrQuick PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrQuick
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrQuick)
