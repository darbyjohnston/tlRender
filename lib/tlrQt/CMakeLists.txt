set(HEADERS
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

add_library(tlrQt ${HEADERS} ${SOURCE})
target_link_libraries(tlrQt Qt5::Gui tlrGL)
set_target_properties(tlrQt PROPERTIES FOLDER lib)
set_target_properties(tlrQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrQt
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrQt)
