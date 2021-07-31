set(HEADERS
    FilmstripWidget.h
    SpeedLabel.h
    TimeLabel.h
    TimeObject.h
    TimeSpinBox.h
    TimelineControls.h
    TimelinePlayer.h
    TimelineSlider.h
    TimelineThumbnailProvider.h
    TimelineViewport.h
    TimelineWidget.h
    ToolTipsFilter.h
    Util.h)
set(SOURCE
    FilmstripWidget.cpp
    SpeedLabel.cpp
    TimeLabel.cpp
    TimeObject.cpp
    TimeSpinBox.cpp
    TimelineControls.cpp
    TimelinePlayer.cpp
    TimelineSlider.cpp
    TimelineThumbnailProvider.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    ToolTipsFilter.cpp)
qt5_add_resources(SOURCE tlrQt.qrc)

add_library(tlrQt ${HEADERS} ${SOURCE})
target_link_libraries(tlrQt Qt5::Widgets Qt5::Svg tlrGL)
set_target_properties(tlrQt PROPERTIES FOLDER lib)
set_target_properties(tlrQt PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrQt
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrQt)
