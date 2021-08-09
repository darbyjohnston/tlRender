set(HEADERS
    FilmstripWidget.h
    SpeedLabel.h
    TimeLabel.h
    TimeSpinBox.h
    TimelineControls.h
    TimelineSlider.h
    TimelineViewport.h
    TimelineWidget.h
    Util.h)
set(SOURCE
    FilmstripWidget.cpp
    SpeedLabel.cpp
    TimeLabel.cpp
    TimeSpinBox.cpp
    TimelineControls.cpp
    TimelineSlider.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    Util.cpp)
qt5_add_resources(SOURCE tlrQWidget.qrc)

add_library(tlrQWidget ${HEADERS} ${SOURCE})
target_link_libraries(tlrQWidget Qt5::Widgets Qt5::Svg tlrQt)
set_target_properties(tlrQWidget PROPERTIES FOLDER lib)
set_target_properties(tlrQWidget PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrQWidget
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrQWidget)
