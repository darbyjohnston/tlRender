set(HEADERS
    FrameBufferObject.h
    Util.h)
set(SOURCE
    FrameBufferObject.cpp
    Util.cpp)

add_library(tlrQuick ${HEADERS} ${SOURCE})
target_link_libraries(tlrQuick Qt5::Quick tlrQt)
set_target_properties(tlrQuick PROPERTIES FOLDER lib)
set_target_properties(tlrQuick PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrQuick
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrQuick)
